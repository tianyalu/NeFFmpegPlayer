package com.sty.ne.ffmpegplayer;

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

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    interface OnPreparedListener{
        void onPrepared();
    }

    //native 方法
    private native void prepareNative(String dataSource);
    private native void startNative();
    private native void stopNative();
    private native void releaseNative();
}
