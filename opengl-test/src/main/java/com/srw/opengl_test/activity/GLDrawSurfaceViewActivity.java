package com.srw.opengl_test.activity;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.srw.opengl_test.EGLBase;
import com.srw.opengl_test.EGLDrawer;
import com.srw.opengl_test.EGLUtil;
import com.srw.opengl_test.R;

public class GLDrawSurfaceViewActivity extends AppCompatActivity {

  private static final String TAG = "GLTEST-" + GLDrawSurfaceViewActivity.class.getSimpleName();

  private Handler mWorkHandler;
  private EGLDrawer mDrawer;
  private EGLBase mEGLBase;
  private int mTexture;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.draw_surfaceview);

    HandlerThread openglWorkThread = new HandlerThread("opengl worker");
    openglWorkThread.start();
    mWorkHandler = new Handler(openglWorkThread.getLooper());
    mWorkHandler.post(() -> {
        mEGLBase = new EGLBase(EGLBase.CONFIG_PLAIN);
        mDrawer = new EGLDrawer();
    });

    ((SurfaceView)findViewById(R.id.renderview)).getHolder().addCallback(new SurfaceHolder.Callback() {
      @Override
      public void surfaceCreated(SurfaceHolder holder) {
        mWorkHandler.post(() -> {
            mEGLBase.createEGLSurface(holder.getSurface());
            mEGLBase.makeCurrent();

            mTexture = EGLUtil.generateTexture();
            Log.i(TAG, "handle " + mTexture);
        });
      }

      @Override
      public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

      }

      @Override
      public void surfaceDestroyed(SurfaceHolder holder) {

      }
    });

    mWorkHandler.postDelayed(new DrawOperation(), 1000);
  }

  private class DrawOperation implements Runnable {
    @Override
    public void run() {
      if (mEGLBase != null) {
        EGLUtil.uploadBitmapToTexture(mTexture, 800, 800);

        // darw buffer starting at top-left corner, not bottom-left.
        final android.graphics.Matrix renderMatrix = new android.graphics.Matrix();
        renderMatrix.preTranslate(0.5f, 0.5f);
        renderMatrix.preScale(1f, -1f);
        renderMatrix.preTranslate(-0.5f, -0.5f);
        mDrawer.drawTexture(mTexture, 800, 800, EGLUtil.convertMatrixFromAndroidGraphicsMatrix(renderMatrix));
        mEGLBase.swapBuffers();
      }
      mWorkHandler.postDelayed(this, 100);
    }
  }

  @Override
  protected void onResume() {
    super.onResume();
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    mWorkHandler.postAtFrontOfQueue(() -> {
      mEGLBase.release();
      mDrawer.release();
    });
    mWorkHandler.post(() -> {
      Log.i(TAG, "quit opengl worker");
      mWorkHandler.getLooper().quit();
    });
  }
}
