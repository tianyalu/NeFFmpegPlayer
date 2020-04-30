package com.sty.ne.ffmpegplayer;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.sty.ne.ffmpegplayer.util.PermissionUtils;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener, View.OnClickListener {

        private static final String DIR_PATH = Environment.getExternalStorageDirectory()
            + File.separator + "sty" + File.separator + "input.mp4";

//            + File.separator + "sty" + File.separator + "input_crop.mp4";
//            + File.separator + "sty" + File.separator + "chengdu.mp4";
//            + File.separator + "视频/dance/sandymandy" + File.separator + "[牛人]Whatcha Doin' Today_超清.mp4";
//            + File.separator + "视频/dance/sandymandy" + File.separator + "4Minute - Hate by Sandy Mandy_超清.mp4";

    private static final String srcUrl = "rtmp://58.200.131.2:1935/livetv/hunantv";
//    private static final String srcUrl = new File(DIR_PATH).getAbsolutePath();

    private String[] needPermissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE};

    private static final int STATUS_STOP = 0;
    private static final int STATUS_PLAY = 1;
    private static final int STATUS_PAUSE = 2;
    private int status = STATUS_STOP;

    private NeFFmpegPlayer player;
    private SurfaceView surfaceView;
    private Button btnPlayOrPause;
    private Button btnStop;
    private Button btnFullScreen;
    private TextView tvTime; //显示播放时间
    private SeekBar seekBar; //进度条-与播放总时长挂钩
    private boolean isTouch; //用户是否拖拽了进度条
    private boolean isSeek; //是否seek
    private int mDuration = 0; //视频总时长

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        initView();
        setListener();
    }

    private void initView() {
        surfaceView = findViewById(R.id.surface_view);
        tvTime = findViewById(R.id.tv_time);
        seekBar = findViewById(R.id.seek_bar);
        btnPlayOrPause = findViewById(R.id.btn_play_or_pause);
        btnStop = findViewById(R.id.btn_stop);
        btnFullScreen = findViewById(R.id.btn_full_screen);

        player = new NeFFmpegPlayer();
        player.setSurfaceView(surfaceView);
        player.setDataSource(srcUrl);
    }

    private void setListener() {
        seekBar.setOnSeekBarChangeListener(this);
        btnPlayOrPause.setOnClickListener(this);
        btnStop.setOnClickListener(this);
        btnFullScreen.setOnClickListener(this);
        player.setOnPreparedListener(new NeFFmpegPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                //得到视频总时长
                mDuration = player.getDuration();
                //准备好了
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        //直播 通过FFmpeg 得到的 duration 是0
                        if (mDuration != 0) {
                            //本地视频文件
                            tvTime.setText("00:00/" + getMinutes(mDuration) + ":" + getSeconds(mDuration));
                            tvTime.setVisibility(View.VISIBLE);
                            seekBar.setVisibility(View.VISIBLE);
                        }
                        Toast.makeText(MainActivity.this, "可以开始播放了", Toast.LENGTH_SHORT).show();
                    }
                });
                player.start();
            }
        });
        player.setOnErrorListener(new NeFFmpegPlayer.OnErrorListener() {
            @Override
            public void onError(final String msg, final int errCode) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "errCode: " + errCode
                                + "\nerrMsg: " + msg, Toast.LENGTH_SHORT).show();
                    }
                });
            }
        });
        player.setOnProgressListener(new NeFFmpegPlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                // progress 底层FFmpeg 获取得到的当前播放时间（秒）
                if (!isTouch) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (mDuration != 0) {
                                if (isSeek) {
                                    isSeek = false;
                                    return;
                                }
                                tvTime.setText(getMinutes(progress) + ":" + getSeconds(progress) +
                                        "/" + getMinutes(mDuration) + ":" + getSeconds(mDuration));
                                seekBar.setProgress(progress * 100 / mDuration);

                                //进度完成之后停止播放
                                //Log.i("sty", "progress : " + progress  + " mDuration: " + mDuration);
                                if(progress == mDuration) {
                                    stopVideo();
                                }
                            }
                        }
                    });
                }
            }
        });
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_play_or_pause:
                onBtnPlayOrPauseClicked();
                break;
            case R.id.btn_stop:
                stopVideo();
                break;
            case R.id.btn_full_screen:
                Toast.makeText(this, "待完善", Toast.LENGTH_SHORT).show();
                break;
            default:
                break;
        }
    }

    private void onBtnPlayOrPauseClicked() {
        if (PermissionUtils.checkPermissions(this, needPermissions)) {
            playOrPause();
        } else {
            PermissionUtils.requestPermissions(this, needPermissions);
        }
    }

    private void playOrPause() {
        if (status == STATUS_STOP) {
            playVideo();
        } else if (status == STATUS_PAUSE) {
            continueVideo();
        } else if (status == STATUS_PLAY) {
            pauseVideo();
        }
    }

    private void pauseVideo() {
        if (player != null && status == STATUS_PLAY) {
            player.pause();
            status = STATUS_PAUSE;
            btnPlayOrPause.setText("play");
        }
    }

    private void continueVideo() {
        if (player != null && status == STATUS_PAUSE) {
            player.continuePlay();
            status = STATUS_PLAY;
            btnPlayOrPause.setText("pause");
        }
    }

    private void stopVideo() {
        if (player != null && status == STATUS_PLAY) {
            player.stop();
            status = STATUS_STOP;
            btnPlayOrPause.setText("play");
        }
    }

    private void playVideo() {
        if (status == STATUS_STOP && player != null) {
            try {
                player.prepare();
                status = STATUS_PLAY;
                btnPlayOrPause.setText("pause");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] paramArrayOfInt) {
        Log.e("sty", "onRequestPermissionsResult");
        if (requestCode == PermissionUtils.REQUEST_PERMISSIONS_CODE) {
            if (!PermissionUtils.verifyPermissions(paramArrayOfInt)) {
                PermissionUtils.showMissingPermissionDialog(this);
            } else {
                playOrPause();
            }
        }
    }

    private String getMinutes(int duration) {
        int minutes = duration / 60;
        if (minutes <= 9) {
            return "0" + minutes;
        }
        return "" + minutes;
    }

    private String getSeconds(int duration) {
        int seconds = duration % 60;
        if (seconds <= 9) {
            return "0" + seconds;
        }
        return "" + seconds;
    }

    @Override
    protected void onResume() {
        super.onResume();
//        try {
//            player.prepare();
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
    }


    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
//        pauseVideo();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        player.stop();
        player.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            tvTime.setText(getMinutes(progress * mDuration / 100) + ":" +
                    getSeconds(progress * mDuration / 100) + "/" +
                    getMinutes(mDuration) + ":" + getSeconds(mDuration));
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isTouch = false;
        isSeek = false;
        int seekBarProgress = seekBar.getProgress(); //seekBar 的进度
        int playProgress = seekBarProgress * mDuration / 100; //转成播放时间
        player.seek(playProgress);
    }
}
