#include <jni.h>
#include <string>


extern "C"{
#include <libavutil/avutil.h>
#include "NeFFmpegPlayer.h"
}

//#include "ffmpeg/include/libavutil/avutil.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_sty_ne_ffmpegplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
//    return env->NewStringUTF(hello.c_str());
    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                          jstring data_source_) {
    const char* data_source = env->GetStringUTFChars(data_source_, 0);
    NeFFmpegPlayer *player = new NeFFmpegPlayer(data_source);
    player->prepare();
    env->ReleaseStringUTFChars(data_source_, data_source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_startNative(JNIEnv *env, jobject thiz) {
    // TODO: implement startNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}