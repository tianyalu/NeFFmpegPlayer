//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_AUDIOCHANNEL_H
#define NEFFMPEGPLAYER_AUDIOCHANNEL_H


#include <libavcodec/avcodec.h>
#include "BaseChannel.h"

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int streamIndex, AVCodecContext *codecContext);
};


#endif //NEFFMPEGPLAYER_AUDIOCHANNEL_H
