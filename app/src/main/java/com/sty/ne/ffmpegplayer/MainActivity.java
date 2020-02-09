package com.sty.ne.ffmpegplayer;

import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener{
    private static final int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 1;
    private static final String DIR_PATH = Environment.getExternalStorageDirectory()
//            + File.separator + "sty" + File.separator + "input.mp4";
            + File.separator + "视频/dance/sandymandy" + File.separator + "[牛人]Whatcha Doin' Today_超清.mp4";
    private NeFFmpegPlayer player;
    private SurfaceView surfaceView;
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

        surfaceView = findViewById(R.id.surface_view);
        tvTime = findViewById(R.id.tv_time);
        seekBar = findViewById(R.id.seek_bar);
        seekBar.setOnSeekBarChangeListener(this);

        player = new NeFFmpegPlayer();
        player.setSurfaceView(surfaceView);
        player.setDataSource(new File(DIR_PATH).getAbsolutePath());
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
                        if(mDuration != 0) {
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
                if(!isTouch) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if(mDuration != 0) {
                                if(isSeek) {
                                    isSeek = false;
                                    return;
                                }
                                tvTime.setText(getMinutes(progress) + ":" + getSeconds(progress) +
                                        "/" + getMinutes(mDuration) + ":" + getSeconds(mDuration));
                                seekBar.setProgress(progress * 100 / mDuration );
                            }
                        }
                    });
                }
            }
        });
        requestPermission();
    }

    private String getMinutes(int duration) {
        int minutes = duration / 60;
        if(minutes <= 9) {
            return "0" + minutes;
        }
        return "" + minutes;
    }

    private String getSeconds(int duration) {
        int seconds = duration % 60;
        if(seconds <= 9) {
            return "0" + seconds;
        }
        return "" + seconds;
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
        player.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if(fromUser) {
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

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE: {
                if(grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED){
                    Log.i("sty", "onRequestPermissionResult granted");
                }else {
                    Log.i("sty", "onRequestPermissionResult denied");
                    showWarningDialog();
                }
                break;
            }
            default:
                break;
        }
    }

    private void showWarningDialog() {
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle("警告")
                .setMessage("请前往设置->应用—>PermissionDemo->权限中打开相关权限，否则功能无法正常使用！")
                .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        //finish();
                    }
                }).show();
    }

}
