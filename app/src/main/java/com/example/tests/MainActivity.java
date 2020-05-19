package com.example.tests;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Build;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static boolean mIsIint = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        isPermissionOK();
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

    private boolean isPermissionOK() {
        PermissionChecker permissionChecker = new PermissionChecker(this);
        boolean isPermissionOK = Build.VERSION.SDK_INT < Build.VERSION_CODES.M || permissionChecker.checkPermission();
        if (!isPermissionOK) {
            Toast.makeText(this, "Some permissions is not approved !!!", Toast.LENGTH_SHORT).show();
        }
        return isPermissionOK;
    }
}
