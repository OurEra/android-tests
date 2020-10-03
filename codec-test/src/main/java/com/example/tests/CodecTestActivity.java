package com.example.tests;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;

public class CodecTestActivity extends AppCompatActivity {

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
            VideoCaptureAndroidNative.setLocalPreview(((SurfaceView) findViewById(R.id.test_sf)).getHolder());
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
