#include <jni.h>
#include <string>
#include "NeFFmpegPlayer.h"
#include "JniCallbackHelper.h"
#include <android/native_window_jni.h>

extern "C"{
#include <libavutil/avutil.h>
}
JavaVM *javaVm = 0;
NeFFmpegPlayer *player = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //互斥锁--静态初始化

jint JNI_OnLoad(JavaVM *vm, void *args) { //onCreate
    javaVm = vm;
    return JNI_VERSION_1_6;
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

void renderFrame(uint8_t *src_data, int width, int height, int src_line_size) {
    pthread_mutex_lock(&mutex);


    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                          jstring data_source_) {
    const char* data_source = env->GetStringUTFChars(data_source_, 0);
    JniCallbackHelper *jni_callback_helper = new JniCallbackHelper(javaVm, env, thiz);
    player = new NeFFmpegPlayer(data_source, jni_callback_helper);
    player->setRenderCallback(renderFrame);
    player->prepare();
    env->ReleaseStringUTFChars(data_source_, data_source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_startNative(JNIEnv *env, jobject thiz) {
    if(player) {
        player->start();
    }
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

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                             jobject surface) {
    pthread_mutex_lock(&mutex);
    //先释放之前的窗口
    if(window) {
        ANativeWindow_release(window);
        window = 0;
    }
    //创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}