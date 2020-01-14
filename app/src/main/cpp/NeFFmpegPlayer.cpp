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
}

NeFFmpegPlayer::~NeFFmpegPlayer() {
    if (data_source) {
        delete data_source;
        data_source = 0;
    }
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
        if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频流
            video_channel = new VideoChannel(i, codecContext);
            video_channel->setRenderCallback(renderCallback);
        }else if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频流
            audio_channel = new AudioChannel(i, codecContext);
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
        video_channel->start();
    }
    if(audio_channel) {
        audio_channel->start();
    }
    pthread_create(&pid_start, 0, task_start, this); //把this作为参数传值给task_start
}

/**
 * 真正开始播放
 * （读取 音视频包 加入相应的音频/视频队列）
 */
void NeFFmpegPlayer::_start() {
    while (isPlaying) {
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
//  video_channel->stop();

}

void NeFFmpegPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}




