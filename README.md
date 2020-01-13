# NeFFmpegPlayer FFmpeg播放器-项目框架搭建
## 一、前言
### 1.1 遇到问题可参考：
[最新版本FFmpeg编译(基于v4.2.1)](https://www.jianshu.com/p/212c61cac89c)  
[ffmpeg 编译&集成问题汇总](https://www.jianshu.com/p/c413873350a2)
[JNI基础](https://www.jianshu.com/p/e3bcff7e3b24)
[Android Native 视频绘制](https://www.jianshu.com/p/75f018f11eb9)

### 1.2 集成第三方库
头文件--编译的库

## 二、实操
### 2.1 app模块下的`build.gradle`文件
```groovy
android {
    compileSdkVersion 27
    buildToolsVersion "29.0.2"
    defaultConfig {
        applicationId "com.sty.ne.ffmpegplayer"
        minSdkVersion 21
        targetSdkVersion 27
        versionCode 1
        versionName "1.0"
        testInstrumentationRunner "android.support.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags ""
                abiFilters "armeabi-v7a"
            }
        }
        ndk {
            abiFilters "armeabi-v7a"
        }
    }
    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.10.2"
        }
    }
}
```
### 2.2 `CMakeLists.txt`文件
```cmake
cmake_minimum_required(VERSION 3.4.1)

set(ffmpeg ${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${ffmpeg}/include)  # path;%JAVA_HOME%
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${ffmpeg}/libs/${CMAKE_ANDROID_ARCH_ABI}") #追加

file(GLOB src_files *.cpp)

add_library( # Sets the name of the library.
        native-lib
        SHARED
        ${src_files}) #相对路径

target_link_libraries( # Specifies the target library.
        native-lib
        -Wl,--start-group  # (WL)以group的形式链接库避免链接顺序的问题
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        z
        log #liblog.so
        )
```

### 2.3  `MainActivity.java`文件（入口）
```java
public class MainActivity extends AppCompatActivity {
    private static final int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 1;
    private static final String DIR_PATH = Environment.getExternalStorageDirectory()
            + File.separator + "sty" + File.separator + "input.mp4";
    private NeFFmpegPlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        player = new NeFFmpegPlayer();
        player.setDataSource(new File(DIR_PATH).getAbsolutePath());
        player.setOnPreparedListener(new NeFFmpegPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                //准备好了
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "可以开始播放了", Toast.LENGTH_SHORT).show();
                    }
                });
            }

            @Override
            public void onError(final String msg) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
                    }
                });
            }
        });
//        requestPermission(); //测试错误回调
    }

    @Override
    protected void onResume() {
        super.onResume();
        try {
            player.prepare();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.stop();
    }

    private void requestPermission() {
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED){
            if(ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)){
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
            }else {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
            }
        }
    }
}
```
### 2.4 `NeFFmpegPlayer.java`文件（Java调JNI层）
```java
public class NeFFmpegPlayer {
    static {
        System.loadLibrary("native-lib");
    }

    private OnPreparedListener onPreparedListener;

    private String dataSource; //媒体源（文件路径/直播地址）

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放前的准备工作
     */
    public void prepare() {
        prepareNative(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    /**
     * 释放资源
     */
    public void release() {
        releaseNative();
    }

    /**
     * 给JNI反射调用的
     */
    public void onPrepared() {
        if(null != onPreparedListener) {
            onPreparedListener.onPrepared();
        }
    }

    public void onError(String msg) {
        if(onPreparedListener != null) {
            onPreparedListener.onError(msg);
        }
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    interface OnPreparedListener{
        void onPrepared();
        void onError(String msg);
    }

    //native 方法
    private native void prepareNative(String dataSource);
    private native void startNative();
    private native void stopNative();
    private native void releaseNative();
}
```

### 2.5 `native-lib.cpp`文件（JNI接口层）
```c++
#include <jni.h>
#include <string>
#include "NeFFmpegPlayer.h"
#include "JniCallbackHelper.h"

extern "C"{
#include <libavutil/avutil.h>
}
JavaVM *javaVm = 0;
jint JNI_OnLoad(JavaVM *vm, void *args) { //onCreate
    javaVm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_ffmpegplayer_NeFFmpegPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                          jstring data_source_) {
    const char* data_source = env->GetStringUTFChars(data_source_, 0);
    JniCallbackHelper *jni_callback_helper = new JniCallbackHelper(javaVm, env, thiz);
    NeFFmpegPlayer *player = new NeFFmpegPlayer(data_source, jni_callback_helper);
    player->prepare();
    env->ReleaseStringUTFChars(data_source_, data_source);
}
```

### 2.6 `NeFFmpegPlayer.cpp`文件（JNI C++实现层）
```c++
#include "NeFFmpegPlayer.h"
NeFFmpegPlayer::NeFFmpegPlayer(const char *data_source, JniCallbackHelper *jni_callback_helper) {
//    this->data_source = data_source;  //悬空指针问题

    this->data_source = new char[strlen(data_source) + 1];  //字符串'\0'结尾
    strcpy(this->data_source, data_source);

    this->jni_callback_helper = jni_callback_helper;
}

NeFFmpegPlayer::~NeFFmpegPlayer() {
    if(data_source) {
        delete data_source;
        data_source = 0;
    }
}

void *task_prepare(void *args) {
    //处理访问域的问题
    NeFFmpegPlayer *player = static_cast<NeFFmpegPlayer *>(args);
    player->_prepare();

    return 0; //一定要return！！！
}
void NeFFmpegPlayer::prepare() {
    /**
     * 开子线程
     * 1.文件：IO流
     * 2.直播: 网络
     * 都不适合在主线程来操作
     */
//    pthread_create(&pid_prepare, 0, task_prepare, 0);
    pthread_create(&pid_prepare, 0, task_prepare, this); //把this作为参数传值给task_prepare
}

void NeFFmpegPlayer::_prepare() {
    //1.打开媒体地址
    AVFormatContext* formatContext = avformat_alloc_context();

    AVDictionary *dictionary = 0;
    av_dict_set( &dictionary, "timeout", "5000000", 0); //单位：微秒
    /**
     * AVFormatContext： 上下文
     * url: 文件路径或直播地址
     * AVInputFormat: 输入的封装格式
     * AVDictionary：参数
     */
    int ret = avformat_open_input(&formatContext, data_source, 0, &dictionary);
    av_dict_free(&dictionary);
    if(ret) {
        //告诉用户错误信息
        char buf[1024];
        av_strerror(ret, buf, 1024);
        LOGE("ERROR INFO1: %s", buf);
        if(jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf);
        }
        return;
    }

    //2.查找流信息
    ret = avformat_find_stream_info(formatContext, 0);
    if(ret < 0) {
        //告诉用户错误信息
        char buf[1024];
        av_strerror(ret, buf, 1024);
        LOGE("ERROR INFO2: %s", buf);
        if(jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf);
        }
        return;
    }

    //3.根据流信息个数循环查找
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //4.获取媒体流（音/视频）
        AVStream* stream = formatContext->streams[i];
        //5.从流中获取解码这段流的参数
        AVCodecParameters* codecParameters = stream->codecpar;
        //6.通过流的编解码参数中的编解码ID，来获取当前流的解码器
        AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
        if(!codec) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO3: %s", buf);
            if(jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf);
            }
            return;
        }
        //7.解码器上下文
        AVCodecContext* codecContext = avcodec_alloc_context3(codec);
        if(!codecContext) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO4: %s", buf);
            if(jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf);
            }
            return;
        }
        //8.设置上下文参数
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if(ret < 0) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO5: %s", buf);
            if(jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf);
            }
            return;
        }
        //9.打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if(ret) {
            //告诉用户错误信息
            char buf[1024];
            av_strerror(ret, buf, 1024);
            LOGE("ERROR INFO6: %s", buf);
            if(jni_callback_helper) {
                jni_callback_helper->onError(THREAD_CHILD, buf);
            }
            return;
        }
        //10.从编码器的参数中获取流类型
        if(codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频流
            audio_channel = new AudioChannel();
        }else if(codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频流
            video_channel = new VideoChannel();
        }
    } //end for

    //11.如果流中没有音频也没有视频
    if(!audio_channel && !video_channel) {
        //告诉用户错误信息
        char *buf = const_cast<char *>("数据源中午音频和视频流信息");
        LOGE("ERROR INFO7: %s", buf);
        if(jni_callback_helper) {
            jni_callback_helper->onError(THREAD_CHILD, buf);
        }
        return;
    }

    //准备工作做好了，告诉Java可以开始播放了
    if(jni_callback_helper){
        jni_callback_helper->onPrepared(THREAD_CHILD);
    }
}
```
### 2.7 `JniCallbackHelper.cpp`文件（C++层调Java层），用于C++向Java通讯
```c++
#include "JniCallbackHelper.h"

JniCallbackHelper::JniCallbackHelper(JavaVM *javaVm, JNIEnv *env, jobject instance_) {
    this->javaVm = javaVm;
    this->env = env;
    //一旦涉及到跨方法、跨线程，需要创建全局引用
    //this->instance = instance; //todo
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(this->instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V"); //javap 可以获取方法签名
    jmd_on_error = env->GetMethodID(clazz, "onError", "(Ljava/lang/String;)V"); //javap 可以获取方法签名
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

void JniCallbackHelper::onError(int thread_mode, char *err_msg) {
    //jstring errMsg = env->NewStringUTF(err_msg); //不同线程的env是不同的，所以必须放到对应的线程中去
    if(thread_mode == THREAD_MAIN) {
        jstring errMsg = env->NewStringUTF(err_msg);
        env->CallVoidMethod(instance, jmd_on_error, errMsg);
    }else {
        //env不支持跨线程
        JNIEnv *env_child;
        javaVm->AttachCurrentThread(&env_child, 0);
        jstring errMsg = env_child->NewStringUTF(err_msg);
        env_child->CallVoidMethod(instance, jmd_on_error, errMsg);
        javaVm->DetachCurrentThread();
    }
}
```