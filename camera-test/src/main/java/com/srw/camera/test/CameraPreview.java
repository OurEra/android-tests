package com.srw.camera.test;

import android.os.Bundle;
import android.view.SurfaceView;
import android.support.v7.app.AppCompatActivity;

import com.example.lib.camera.VideoCaptureAndroid;
import com.example.lib.camera.VideoCaptureDeviceInfoAndroid;

public class CameraPreview extends AppCompatActivity {

    private boolean mInited;
    private VideoCaptureAndroid mCapture;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        VideoCaptureDeviceInfoAndroid.getDeviceInfo();
        VideoCaptureAndroid.setLocalPreview(((SurfaceView) findViewById(R.id.test_sf)).getHolder());
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!mInited) {
            mInited = true;
            mCapture = new VideoCaptureAndroid(1);
            mCapture.startCapture(1280, 720, 30 * 1000, 30 * 1000);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mCapture != null) {
            mCapture.stopCapture();
        }
    }
}
