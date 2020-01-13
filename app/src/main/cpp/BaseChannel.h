//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_BASECHANNEL_H
#define NEFFMPEGPLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
};
#include "safe_queue.h"

class BaseChannel {
public:
    BaseChannel(int streamIndex, AVCodecContext *codecContext) : stream_index(streamIndex){
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
        if(*packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    /**
     * AVFrame *
     * @param frame
     */
    static void releaseAVFrame(AVFrame **frame) {
        if(*frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    int stream_index;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    AVCodecContext *codecContext = 0;
};
#endif //NEFFMPEGPLAYER_BASECHANNEL_H
