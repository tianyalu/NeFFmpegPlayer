//
// Created by 天涯路 on 2020-01-02.
//


#include "VideoChannel.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();

    return 0; //一定要return ！！！
}

void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while(isPlaying) {
        //从队列中取视频压缩数据包 AVPacket
        int ret = packets.pop(packet);
        if(!isPlaying) {
            //如果停止播放了，跳出循环
            break;
        }
        if(!ret) {
            continue;
        }
        //把数据包发给解码器进行解码
        ret = avcodec_send_packet(codecContext, packet);
        if(ret == AVERROR(EAGAIN)) {
            continue;
        }else if(ret != 0) {
            break;
        }
        //发送一个数据包成功
        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, avFrame);
        if(ret == AVERROR(EAGAIN)) {
            continue;
        }else if( ret != 0) {
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

        //dst_data : rgba格式的图像数据
        //宽+高+linesize
        //渲染（绘制）
        //TODO
        renderCallback(dst_data[0], codecContext->width, codecContext->height, dst_line_size[0]);

        releaseAVFrame(&frame);
    }
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





