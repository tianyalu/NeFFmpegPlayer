//
// Created by 天涯路 on 2020-01-02.
//
//中转站
#include <cstring>

#include "NeFFmpegPlayer.h"
extern "C" {
#include <libavformat/avformat.h>

}

NeFFmpegPlayer::NeFFmpegPlayer() {

}

NeFFmpegPlayer::NeFFmpegPlayer(const char *data_source) {
//    this->data_source = data_source;  //悬空指针问题

    this->data_source = new char[strlen(data_source) + 1];  //字符串'\0'结尾
    strcpy(this->data_source, data_source);
}


NeFFmpegPlayer::~NeFFmpegPlayer() {
    if(data_source) {
        delete data_source;
        data_source = 0;
    }
}

void NeFFmpegPlayer::prepare() {
    //1.打开媒体地址
    AVFormatContext* formatContext = avformat_alloc_context();

//    AVDictionary *dictionary = 0;
//    av_dict_set( &dictionary, "timeout", "5000000", 0); //单位：微秒
//    /**
//     * AVFormatContext： 上下文
//     * url: 文件路径或直播地址
//     * AVInputFormat: 输入的封装格式
//     * AVDictionary：参数
//     */
//    avformat_open_input(&formatContext, data_source, 0, &dictionary);
}


