package com.srw.opengl_test.activity;

import android.Manifest;
import android.content.Intent;
import android.graphics.Matrix;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.example.util.PermissionChecker;
import com.srw.opengl_test.R;

import java.security.Permission;

public class HomeActivity extends AppCompatActivity {

    private static final String TAG = "GLTEST-" + HomeActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        isPermissionOK();
   }

    public void onClickGLDrawSurfaceView(View view) {
        startActivity(new Intent(HomeActivity.this, GLDrawSurfaceViewActivity.class));
    }

    public void onClickGLDrawGLSurfaceView(View view) {
        startActivity(new Intent(HomeActivity.this, GLDrawGLSurfaceViewActivity.class));
    }

    public void onClickGLDrawFrameBuffer(View view) {
        startActivity(new Intent(HomeActivity.this, GLDrawFrameBufferActivity.class));
    }

    private boolean isPermissionOK() {
        PermissionChecker permissionChecker = new PermissionChecker(this);
        boolean isPermissionOK = Build.VERSION.SDK_INT < Build.VERSION_CODES.M || permissionChecker.checkPermission(
                new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE});
        if (!isPermissionOK) {
            Toast.makeText(this, "Some permissions is not approved !!!", Toast.LENGTH_SHORT).show();
        }
        return isPermissionOK;
    }

}
