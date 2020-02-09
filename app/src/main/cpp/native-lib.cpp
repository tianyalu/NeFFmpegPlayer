#include <jni.h>
#include <string>
#include "NeFFmpegPlayer.h"
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
    if(!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置窗口属性
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if(ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        return;
    }

    //填充rgb数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_line_size = window_buffer.stride * 4;
    for (int i = 0; i < window_buffer.height; ++i) {
        //逐行拷贝:从src所指的内存地址的起始位置开始拷贝n个字节到dest所指的内存地址的起始位置中
        //第一个参数：目的地址
        //第二个参数：源地址
        //第三个参数：所需要复制的字节数
        memcpy(dst_data + i * dst_line_size, src_data + i * src_line_size, dst_line_size);
    }
    ANativeWindow_unlockAndPost(window);

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
    if(player) {
        player->stop();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&mutex);
    //先释放之前的显示窗口
    if(window) {
        ANativeWindow_release(window);
        window = 0;
    }
    //创建新的窗口用于视频显示
    pthread_mutex_unlock(&mutex);
    DELETE(player);
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

extern "C"
JNIEXPORT jint JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_getDurationNative(JNIEnv *env, jobject thiz) {
    if(player) {
        return player->getDuration();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_seekNative(JNIEnv *env, jobject thiz,
                                                       jint play_progress) {
    if(player) {
        player->seek(play_progress);
    }
}