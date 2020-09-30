package com.srw.opengl_test.activity;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.srw.opengl_test.R;

public class HomeActivity extends AppCompatActivity {

    private static final String TAG = "GLTEST-" + HomeActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
   }

    public void onClickGLDrawSurfaceView(View view) {
        startActivity(new Intent(HomeActivity.this, GLDrawSurfaceViewActivity.class));
    }

    public void onClickGLDrawGLSurfaceView(View view) {
        startActivity(new Intent(HomeActivity.this, GLDrawGLSurfaceViewActivity.class));
    }
}
