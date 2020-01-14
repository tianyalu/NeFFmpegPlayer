//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_AUDIOCHANNEL_H
#define NEFFMPEGPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int streamIndex, AVCodecContext *codecContext);

    virtual ~AudioChannel();

    void start();

    void audio_decode();

    void audio_play();

    int getPCM();

    int out_sample_rate;

    int out_sample_size;
    int out_channels;
    int out_buffers_size;
    uint8_t *out_buffers = 0;

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    //引擎
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf  engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerPlay = 0;
    //播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = 0;

    SwrContext *swr_context = 0;

};


#endif //NEFFMPEGPLAYER_AUDIOCHANNEL_H
