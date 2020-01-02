//
// Created by 天涯路 on 2020-01-02.
//

#ifndef NEFFMPEGPLAYER_NEFFMPEGPLAYER_H
#define NEFFMPEGPLAYER_NEFFMPEGPLAYER_H


class NeFFmpegPlayer {
public:
    NeFFmpegPlayer();

    NeFFmpegPlayer(const char *string);

    ~NeFFmpegPlayer();

    void prepare();

private:
    char* data_source;
};


#endif //NEFFMPEGPLAYER_NEFFMPEGPLAYER_H
