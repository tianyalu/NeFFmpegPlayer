package com.sty.ne.ffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class NeFFmpegPlayer implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("native-lib");
    }

    private OnPreparedListener onPreparedListener;
    private OnErrorListener onErrorListener;
    private OnProgressListener onProgressListener;

    private SurfaceHolder surfaceHolder;
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

    public void pause() {
        pauseNative();
    }

    public void continuePlay() {
        continuePlayNative();
    }

    /**
     * 释放资源
     */
    public void release() {
        releaseNative();
    }

    /**
     * 获取视频总时长
     * @return
     */
    public int getDuration() {
        return getDurationNative();
    }

    public void seek(int playProgress) {
        seekNative(playProgress);
    }
    /**
     * 给JNI反射调用的
     */
    public void onPrepared() {
        if(null != onPreparedListener) {
            onPreparedListener.onPrepared();
        }
    }
    /**
     * 给JNI反射调用的
     */
    public void onError(String msg, int errCode) {
        if(onErrorListener != null) {
            onErrorListener.onError(msg, errCode);
        }
    }

    /**
     * 给JNI反射调用的
     */
    public void onProgress(int progress) {
        if(null != this.onProgressListener) {
            this.onProgressListener.onProgress(progress);
        }
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if(null != surfaceHolder) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }

    /**
     * 画布创建好时回调
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布发生改变时回调
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    interface OnPreparedListener{
        void onPrepared();

    }
    interface OnErrorListener{
        void onError(String msg, int errCode);
    }

    interface OnProgressListener{
        void onProgress(int progress);
    }

    interface OnEndListener {
        void onEnd();
    }

    //native 方法
    private native void prepareNative(String dataSource);
    private native void startNative();
    private native void stopNative();
    private native void pauseNative();
    private native void continuePlayNative();
    private native void releaseNative();
    private native void setSurfaceNative(Surface surface);
    private native int getDurationNative();
    private native void seekNative(int playProgress);
}
