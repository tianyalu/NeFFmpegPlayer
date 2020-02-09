//
// Created by 天涯路 on 2020-01-11.
//

#ifndef NEFFMPEGPLAYER_SAFE_QUEUE_H
#define NEFFMPEGPLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>
using namespace std;


template<typename T>
class SafeQueue {
    typedef void(*ReleaseCallback)(T *); //函数指针，用于用户回调
    typedef void (*SyncCallback)(queue<T> &);
public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0); //动态初始化
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /**
     * 入队
     * @param value
     */
    void push(T value) {
        pthread_mutex_lock(&mutex);
        if(work) {
            q.push(value);
            pthread_cond_signal(&cond); //发送通知，队列有数据了
        }else {
            //需要释放？-->交给调用者释放
            if(releaseCallback) {
                releaseCallback(&value);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 出队
     * @param value
     * @return
     */
    int pop(T &value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while(work && q.empty()) {
            //工作状态且队列为空时，一直等待
            pthread_cond_wait(&cond, &mutex);
        }
        if(!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            //释放
            if(releaseCallback) {
                releaseCallback(&value);
            }
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond); //发送通知，队列状态改变了
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 同步
     */
    void sync() {
        pthread_mutex_lock(&mutex);
        syncCallback(q);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }

    void setSyncCallback(SyncCallback syncCallback) {
        this->syncCallback = syncCallback;
    }

private:
    queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond; //条件变量
    int work; //标记当前队列是否是工作状态
    ReleaseCallback  releaseCallback;
    SyncCallback syncCallback;
};
#endif //NEFFMPEGPLAYER_SAFE_QUEUE_H
