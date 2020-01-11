//
// Created by tian on 2020/1/6.
//

#include "JniCallbackHelper.h"

JniCallbackHelper::JniCallbackHelper(JavaVM *javaVm, JNIEnv *env, jobject instance_) {
    this->javaVm = javaVm;
    this->env = env;
    //一旦涉及到跨方法、跨线程，需要创建全局引用
    //this->instance = instance; //todo
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(this->instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V"); //javap 可以获取方法签名
    jmd_on_error = env->GetMethodID(clazz, "onError", "(Ljava/lang/String;I)V"); //javap 可以获取方法签名
}

JniCallbackHelper::~JniCallbackHelper() {
    javaVm = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;
    env = 0;
}

void JniCallbackHelper::onPrepared(int thread_mode) {
    if(thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(instance, jmd_prepared);
    }else {
        //env不支持跨线程
        JNIEnv *env_child;
        javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);
        javaVm->DetachCurrentThread();
    }
}

void JniCallbackHelper::onError(int thread_mode, char *err_msg, int err_code) {
    //jstring errMsg = env->NewStringUTF(err_msg); //不同线程的env是不同的，所以必须放到对应的线程中去
    if(thread_mode == THREAD_MAIN) {
        jstring errMsg = env->NewStringUTF(err_msg);
        env->CallVoidMethod(instance, jmd_on_error, errMsg, err_code);
    }else {
        //env不支持跨线程
        JNIEnv *env_child;
        javaVm->AttachCurrentThread(&env_child, 0);
        jstring errMsg = env_child->NewStringUTF(err_msg);
        env_child->CallVoidMethod(instance, jmd_on_error, errMsg, err_code);
        javaVm->DetachCurrentThread();
    }
}
