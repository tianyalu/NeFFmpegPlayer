//
// Created by 天涯路 on 2020-01-02.
//


#include "VideoChannel.h"

/**
 * 还未解码的帧，要区分关键帧
 * @param q
 */
void dropAVPacket(queue<AVPacket *> &q) {
    while(!q.empty()) {
        AVPacket *packet = q.front(); //拿到队首元素的引用，此时并未出队列
        if(packet->flags != AV_PKT_FLAG_KEY) {
            //非关键帧才丢
            BaseChannel::releaseAVPacket(&packet);
            q.pop(); //队首元素出队列
        }else {
            break;
        }
    }
}

/**
 * 已经解过码的帧
 * @param q
 */
void dropAVFrame(queue<AVFrame *> &q) {
    if(!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *pContext, AVRational time_base, int fps)
        : BaseChannel(
        stream_index, pContext, time_base) {
    this->fps = fps;
    packets.setSyncCallback(dropAVPacket);
    frames.setSyncCallback(dropAVFrame);
}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();

    return 0; //一定要return ！！！
}

/**
 * 消费
 * 消费速度比生产速度慢
 */
void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while(isPlaying) {
        /**
         * 泄漏点2：控制 AVFrame队列
         */
        //休眠10微秒,等待队列中的数据被消费
        if(isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000); //microseconds 微秒
            continue;
        }
        //从队列中取视频压缩数据包 AVPacket
        int ret = packets.pop(packet);
        //注意：取出packet后packet仍然占着内存
        if(!isPlaying) {
            //如果停止播放了，跳出循环，释放packet
            break;
        }
        if(!ret) {
            //给解码器发送packet失败
            continue;
        }
        //把数据包发给解码器进行解码
        ret = avcodec_send_packet(codecContext, packet);
        if(ret == AVERROR(EAGAIN)) {
            continue;
        }else if(ret != 0) {
            break;
        }
        releaseAVPacket(&packet); //packet 不需要了，可以释放掉

        //发送一个数据包成功
        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, avFrame);
        if(ret == AVERROR(EAGAIN)) {
            //重来
            releaseAVFrame(&avFrame); //重来也可以丢掉自己申请的内存
            continue;
        }else if( ret != 0) {
            releaseAVFrame(&avFrame);
            break;
        }
        //成功解码一个数据包，得到解码后的数据包 AVFrame，加入队列
        frames.push(avFrame);
    } //end while
    releaseAVPacket(&packet);
}

void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();

    return 0; //一定要return ！！！
}


void VideoChannel::video_play() {
    AVFrame * frame = 0;
    uint8_t *dst_data[4];
    int dst_line_size[4];

    SwsContext *sws_ctx = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
    //申请图像内存
    av_image_alloc(dst_data, dst_line_size, codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);
    while (isPlaying) {
        int ret = frames.pop(frame);
        if(!isPlaying) {
            //如果停止播放了，跳出循环
            break;
        }
        if(!ret) {
            continue;
        }
        //格式转换 yuv --> rgba
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, codecContext->height, dst_data, dst_line_size);


        //extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict / (2 * fps); //每帧的额外延时时间
        double avg_delay = 1.0 / fps; //根据FPS得到的平均延时时间
        double real_delay = extra_delay + avg_delay;

        //视频时间
        double video_time = frame->best_effort_timestamp * av_q2d(time_base);

        if(!audio_channel) { //没音频
            av_usleep(real_delay *  1000000);
            if(jni_callback_helper) {
                jni_callback_helper->onProgress(THREAD_CHILD, video_time);
            }
        } else {
            //以音频的时间为基准
            double audio_time = audio_channel->audio_time;
            double time_diff = video_time - audio_time;
            if(time_diff > 0) {
                //视频比音频快，等音频
                if(time_diff > 1) { //拖动进度条导致time_diff过大的情况
                    av_usleep((real_delay * 2) * 1000000); //慢慢追
                }else {
                    av_usleep((real_delay + time_diff) * 1000000);
                }
            }else if(time_diff < 0) {
                //视频比音频慢，追音频（丢帧）
                //画面延时了
                if(fabs(time_diff) >= 0.05 ) {
                    //packets.sync(); //todo 目前实测该种方式存在花屏和卡顿现象
                    frames.sync();
                    continue;
                }
            }else {
                //完美同步
                LOGE2("完美同步");
            }
        }

        //dst_data : rgba格式的图像数据
        //宽+高+linesize
        //渲染（绘制）
        renderCallback(dst_data[0], codecContext->width, codecContext->height, dst_line_size[0]);

        releaseAVFrame(&frame); //内存泄漏的关键原因之一
    }

    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
}

/**
 * 1、解码
 * 2、播放
 */
void VideoChannel::start() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    pthread_create(&pid_video_play, 0, task_video_play, this);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audio_channel) {
    this->audio_channel = audio_channel;
}

void VideoChannel::stop() {
    isPlaying = 0;
    packets.setWork(0);
    frames.setWork(0);
    pthread_join(pid_video_decode, 0);
    pthread_join(pid_video_play, 0);
}

void VideoChannel::pausePlay() {
    isPlaying = 0;
    packets.setWork(0);
    frames.setWork(0);
}





