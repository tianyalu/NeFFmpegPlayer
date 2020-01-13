//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_NEFFMPEGPLAYER_H
#define NEFFMPEGPLAYER_NEFFMPEGPLAYER_H

#include <cstring>
#include <pthread.h>
#include <android/log.h>
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JniCallbackHelper.h"
#include "macro.h"

extern "C" {
#include <libavformat/avformat.h>
}

class NeFFmpegPlayer {
public:
    NeFFmpegPlayer();

    NeFFmpegPlayer(const char *string, JniCallbackHelper *pHelper);

    ~NeFFmpegPlayer();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

private:
    char* data_source = 0;
    pthread_t pid_prepare;
    pthread_t pid_start;
    AudioChannel *audio_channel = 0;
    VideoChannel *video_channel = 0;
    JniCallbackHelper *jni_callback_helper = 0;
    AVFormatContext *formatContext = 0;
    bool isPlaying;
    RenderCallback renderCallback;
};


#endif //NEFFMPEGPLAYER_NEFFMPEGPLAYER_H
