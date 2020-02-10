//
// Created by 天涯路 on 2020-01-02.
//
//中转站




#include "NeFFmpegPlayer.h"

NeFFmpegPlayer::NeFFmpegPlayer() {

}

NeFFmpegPlayer::NeFFmpegPlayer(const char *data_source, JniCallbackHelper *jni_callback_helper) {
//    this->data_source = data_source;  //悬空指针问题

    this->data_source = new char[strlen(data_source) + 1];  //字符串'\0'结尾
    strcpy(this->data_source, data_source);

    this->jni_callback_helper = jni_callback_helper;
    pthread_mutex_init(&seek_mutex, 0);
    pthread_mutex_init(&pause_mutex, 0);
    pthread_mutex_init(&continue_mutex, 0);
}

NeFFmpegPlayer::~NeFFmpegPlayer() {
//    if (data_source) {
//        delete data_source;
//        data_source = 0;
//    }
    DELETE(data_source);
    DELETE(jni_callback_helper);
    pthread_mutex_destroy(&seek_mutex);
    pthread_mutex_destroy(&pause_mutex);
    pthread_mutex_destroy(&continue_mutex);
}

void *task_prepare(void *args) {
    //处理访问域的问题
    NeFFmpegPlayer *player = static_cast<NeFFmpegPlayer *>(args);
    player->_prepare();

    return 0; //一定要return！！！
}

void NeFFmpegPlayer::prepare() {
    /**
     * 开子线程
     * 1.文件：IO流
     * 2.直播: 网络
     * 都不适合在主线程来操作
     */
//    pthread_create(&pid_prepare, 0, task_prepare, 0);
    pthread_create(&pid_prepare, 0, task_prepare, this); //把this作为参数传值给task_prepare
}

void NeFFmpegPlayer::_prepare() {
    //1.打开媒体地址
    formatContext = avformat_alloc_context();

    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "5000000", 0); //单位：微秒
    /**
     * AVFormatContext： 上下文
     * url: 文件路径或直播地址
     * AVInputFormat: 输入的封装格式
     * AVDictionary：参数
     */
    int ret = avformat_open_input(&formatContext, data_source, 0, &dictionary);
    av_dict_free(&dictionary);
    if (ret) {
        //告诉用户错误信息
        char buf[1024];
        av_strerror(ret, buf, 1024);
        LOGE("ERROR INFO1: %s", buf);
        LOGE2("ERROR INFO ------");
        if (jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    mDuration = formatContext->duration / AV_TIME_BASE;

    //2.查找流信息
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        //告诉用户错误信息
        char buf[1024];
        av_strerror(ret, buf, 1024);
        LOGE("ERROR INFO2: %s", buf);
        if (jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    //3.根据流信息个数循环查找
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //4.获取媒体流（音/视频）
        AVStream *stream = formatContext->streams[i];
        //5.从流中获取解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;
        //6.通过流的编解码参数中的编解码ID，来获取当前流的解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO3: %s", buf);
            if (jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        //7.解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO4: %s", buf);
            if (jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //8.设置上下文参数
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO5: %s", buf);
            if (jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf,
                                             FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //9.打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if (ret) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO6: %s", buf);
            if (jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        //10.从编解码器的参数中获取流类型
        AVRational time_base = stream->time_base;
        if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频流
            if(stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                //如果这个标记是附加图
                //过滤当前的封面视频流
                continue;
            }
            AVRational frame_rate = stream->avg_frame_rate;
            //转FPS
            int fps = av_q2d(frame_rate);

            video_channel = new VideoChannel(i, codecContext, time_base, fps);
            video_channel->setRenderCallback(renderCallback);
            
            if(mDuration != 0) { //直播不需要回调进度
                video_channel->setJniCallbackHelper(jni_callback_helper);
            }
        }else if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频流
            audio_channel = new AudioChannel(i, codecContext, time_base);
            
            if(mDuration != 0) { //直播不需要回调进度
                audio_channel->setJniCallbackHelper(jni_callback_helper);
            }
        }
    } //end for

    //11.如果流中没有音频也没有视频
    if (!audio_channel && !video_channel) {
        //告诉用户错误信息
        char *buf = const_cast<char *>("数据源中午音频和视频流信息");
        LOGE("ERROR INFO7: %s", buf);
        if (jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_NOMEDIA);
        }
        return;
    }

    //准备工作做好了，告诉Java可以开始播放了
    if (jni_callback_helper) {
        jni_callback_helper->onPrepared(THREAD_CHILD);
    }
}

void *task_start(void *args) {
    //处理访问域的问题
    NeFFmpegPlayer *player = static_cast<NeFFmpegPlayer *>(args);
    player->_start();

    return 0; //一定要return！！！
}

/**
 * 开始播放（先解码后播放）
 */
void NeFFmpegPlayer::start() {
    isPlaying = 1;
    if(video_channel) {
        video_channel->setAudioChannel(audio_channel);
        video_channel->start();
    }
    if(audio_channel) {
        audio_channel->start();
    }
    pthread_create(&pid_start, 0, task_start, this); //把this作为参数传值给task_start
}

/**
 * AVPacket 生产
 * 真正开始播放
 * （读取 音视频包 加入相应的音频/视频队列）
 */
void NeFFmpegPlayer::_start() {

    while (isPlaying) {
        /**
        * 泄漏点1：控制AVPacket队列大小，等待队列中的数据被消费
        */
        if( (video_channel && video_channel->packets.size() > 100) ||
                (audio_channel && audio_channel->packets.size() > 100) ) {
            //休眠10微秒
            av_usleep(10 * 1000); //microseconds 微秒
            continue;
        }

//        if(audio_channel && audio_channel->packets.size() > 100) {
//            //休眠10微秒
//            av_usleep(10 * 1000); //microseconds 微秒
//            continue;
//        }

        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatContext, packet);
        if(!ret) {
            //读取成功 判断数据包类型是音频还是视频 根据流索引来判断
            if(video_channel && video_channel->stream_index == packet->stream_index) {
                //视频数据包
                video_channel->packets.push(packet);
            }else if(audio_channel && audio_channel->stream_index == packet->stream_index) {
                //音频数据包
                audio_channel->packets.push(packet);
            }

        }else if(ret == AVERROR_EOF) {
            //end of file 读完了，考虑是否播放完了
            //todo
        }else {
            break;
        }
    }
    isPlaying = 0;
    video_channel->stop();
    audio_channel->stop();

}

void NeFFmpegPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

int NeFFmpegPlayer::getDuration() {
    return mDuration;
}

/**
 * 指定时间点重新开始播放
 * @param progress 要seek的时间戳
 */
void NeFFmpegPlayer::seek(int progress) {
    if(progress < 0 || progress > mDuration) {
        return;
    }
    if(!audio_channel && !video_channel) {
        return;
    }
    if(!formatContext) {
        return;
    }
    pthread_mutex_lock(&seek_mutex);

    int ret = av_seek_frame(formatContext, -1, progress * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if(ret < 0) {
        //告诉用户错误信息
        char buf[1024];
        av_strerror(ret, buf, 1024);
        LOGE("ERROR INFO6: %s", buf);
        if(jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf, FFMPEG_SEEK_FAIL);
        }
        return;
    }

    //4个队列可能存在未消费数据，需要 reset
    if(audio_channel) {
        audio_channel->packets.setWork(0);
        audio_channel->frames.setWork(0);
        audio_channel->packets.clear();
        audio_channel->frames.clear();
        audio_channel->packets.setWork(1);
        audio_channel->frames.setWork(1);
    }
    if(video_channel) {
        video_channel->packets.setWork(0);
        video_channel->frames.setWork(0);
        video_channel->packets.clear();
        video_channel->frames.clear();
        video_channel->packets.setWork(1);
        video_channel->frames.setWork(1);
    }

    pthread_mutex_unlock(&seek_mutex);
}

void *task_stop(void *args) {
    NeFFmpegPlayer *player = static_cast<NeFFmpegPlayer *>(args);
    player->isPlaying = 0;
    //pthread_join 引发anr
    pthread_join(player->pid_prepare, 0);
    pthread_join(player->pid_start, 0);
    if(player->formatContext) {
        avformat_close_input(&player->formatContext);
        avformat_free_context(player->formatContext);
        player->formatContext = 0;
    }
    DELETE(player->audio_channel);
    DELETE(player->video_channel);

    return 0; //一定要return！

}
void NeFFmpegPlayer::stop() {
    jni_callback_helper = 0;
    if(video_channel) {
        video_channel->jni_callback_helper = 0;
    }
    if(audio_channel) {
        audio_channel->jni_callback_helper = 0;
    }
    pthread_create(&pid_stop, 0, task_stop, this);
}

void NeFFmpegPlayer::pausePlay() {
    pthread_mutex_lock(&pause_mutex);
    if(!video_channel && !audio_channel) {
        return;
    }
    if(video_channel) {
        video_channel->pausePlay();
    }
    if(audio_channel) {
        audio_channel->pausePlay();
    }
    pthread_mutex_unlock(&pause_mutex);
}

void NeFFmpegPlayer::continuePlay() {
    pthread_mutex_lock(&continue_mutex);
    if(!video_channel && !audio_channel) {
        return;
    }
    if(video_channel) {
        video_channel->start();
    }
    if(audio_channel) {
        audio_channel->start();
    }
    pthread_mutex_unlock(&continue_mutex);
}




