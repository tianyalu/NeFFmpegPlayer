//
// Created by 天涯路 on 2020-01-11.
//

#ifndef NEFFMPEGPLAYER_SAFE_QUEUE_H
#define NEFFMPEGPLAYER_SAFE_QUEUE_H

//#include <queue>
//#include <pthread.h>
//using namespace std;

template<typename T>
class SafeQueue {
public:
    SafeQueue() {
//        pthread_mutex_init(&mutex, 0); //动态初始化
    }

    ~SafeQueue() {

    }

    /**
     * 入队
     * @param value
     */
    void push(T value) {

    }
};
#endif //NEFFMPEGPLAYER_SAFE_QUEUE_H
