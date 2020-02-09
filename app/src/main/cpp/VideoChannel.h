//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_VIDEOCHANNEL_H
#define NEFFMPEGPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"
#include <android/log.h>
#include "macro.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{

public:
    VideoChannel(int stream_index, AVCodecContext *pContext, AVRational time_base, int fps);

    void start();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

    void setAudioChannel(AudioChannel *audio_channel);

    void stop();

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;
    int fps;
    AudioChannel *audio_channel = 0;
};


#endif //NEFFMPEGPLAYER_VIDEOCHANNEL_H
