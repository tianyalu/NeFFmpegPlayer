//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_VIDEOCHANNEL_H
#define NEFFMPEGPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"
extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{

public:
    VideoChannel(int stream_index, AVCodecContext *pContext);

    void start();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;

};


#endif //NEFFMPEGPLAYER_VIDEOCHANNEL_H
