package com.example.tests;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Build;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.widget.Toast;

public class TestActivity extends AppCompatActivity {

    private static boolean mIsIint = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
    }

    @Override
    protected  void onResume() {
        super.onResume();
        if (!mIsIint) {
            VideoCaptureAndroid.setLocalPreview(((SurfaceView) findViewById(R.id.test_sf)).getHolder());
            NativeApi.init(getApplicationContext(), ((SurfaceView) findViewById(R.id.test_sf)).getHolder().getSurface());
            mIsIint = true;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        NativeApi.deinit();
    }
}
