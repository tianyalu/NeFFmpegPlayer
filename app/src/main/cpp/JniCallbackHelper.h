//
// Created by tian on 2020/1/6.
//

#ifndef NEFFMPEGPLAYER_JNICALLBACKHELPER_H
#define NEFFMPEGPLAYER_JNICALLBACKHELPER_H

#include <jni.h>
#include "macro.h"

class JniCallbackHelper {
public:
    JniCallbackHelper(JavaVM *javaVm, JNIEnv *env, jobject instance);

    ~JniCallbackHelper();

    void onPrepared(int thread_mode);

    void onError(int thread_mode, char* err_msg, int err_code);

    void onProgress(int thread_mode, int progress);

private:
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject instance;
    jmethodID jmd_prepared;
    jmethodID jmd_on_error;
    jmethodID jmd_onProgress;
};


#endif //NEFFMPEGPLAYER_JNICALLBACKHELPER_H
