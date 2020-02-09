//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_BASECHANNEL_H
#define NEFFMPEGPLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"
#include "JniCallbackHelper.h"

class BaseChannel {
public:
    //匿名构造函数
    BaseChannel(int streamIndex, AVCodecContext *codecContext, AVRational time_base)
            : stream_index(streamIndex),
              codecContext(codecContext),
              time_base(time_base) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
    }

    /**
     * 释放AVPacket *
     * @param packet
     */
    static void releaseAVPacket(AVPacket **packet) {
        if (*packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    /**
     * AVFrame *
     * @param frame
     */
    static void releaseAVFrame(AVFrame **frame) {
        if (*frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    void setJniCallbackHelper(JniCallbackHelper *jni_callback_helper) {
        this->jni_callback_helper = jni_callback_helper;
    }

    int isPlaying;
    int stream_index;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    AVCodecContext *codecContext = 0;
    AVRational time_base;
    double audio_time;
    JniCallbackHelper *jni_callback_helper = 0;
};

#endif //NEFFMPEGPLAYER_BASECHANNEL_H
