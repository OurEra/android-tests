package com.srw.opengl_test.activity;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.opengl.GLES20;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Choreographer;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.example.util.BitmapOperations;
import com.srw.opengl_test.EGLBase;
import com.srw.opengl_test.EGLDrawer;
import com.srw.opengl_test.EGLUtil;
import com.srw.opengl_test.R;

import java.text.SimpleDateFormat;
import java.util.Date;

public class GLDrawSurfaceViewActivity extends AppCompatActivity implements Choreographer.FrameCallback {

  private static final String TAG = "GLTEST-" + GLDrawSurfaceViewActivity.class.getSimpleName();

  private Handler mWorkHandler;
  private EGLDrawer mDrawer;
  private EGLBase mEGLBase;
  private int mTexture;

  private int mViewWidth;
  private int mViewHeight;

  private DrawOperation mDrawOperations;

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
            Choreographer.getInstance().postFrameCallback(GLDrawSurfaceViewActivity.this);
        });
      }

      @Override
      public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mViewWidth = width;
        mViewHeight = height;
      }

      @Override
      public void surfaceDestroyed(SurfaceHolder holder) {
        Choreographer.getInstance().removeFrameCallback(GLDrawSurfaceViewActivity.this);
      }
    });

  }

  @Override
  public void doFrame(long frameTimeNanos) {
    if (mDrawOperations == null) {
      mDrawOperations = new DrawOperation();
    }
    mWorkHandler.post(mDrawOperations);
  }

  private class DrawOperation implements Runnable {
    float xTranslate = 0.0f;
    float yTranslate = 0.0f;
    float translateStep = 0.004f;

    @Override
    public void run() {
      if (mEGLBase != null) {
        // just draw date
        String DATE_PATTERN = "yyyy-MM-dd HH:mm:ss";
        Date date = new Date(System.currentTimeMillis());
        SimpleDateFormat sf = new SimpleDateFormat(DATE_PATTERN);
        Bitmap bitmap = BitmapOperations.createBitmapWithString(800, 800, sf.format(date));
        EGLUtil.uploadBitmapToTexture(mTexture, bitmap);

        // darw buffer starting at top-left corner, not bottom-left.
        final android.graphics.Matrix renderMatrix = new android.graphics.Matrix();
        renderMatrix.preTranslate(0.5f, 0.5f);
        renderMatrix.preScale(1f, -1f);
        renderMatrix.preTranslate(-0.5f, -0.5f);

        final android.graphics.Matrix mvpMatrix = new android.graphics.Matrix();
        xTranslate += translateStep;
        yTranslate += translateStep;
        if (xTranslate >= 0.5f || yTranslate >= 0.5f) {
          translateStep = -translateStep;
        } else if (xTranslate <= -0.5f || yTranslate <= -0.5f) {
          translateStep = -translateStep;
        }
        mvpMatrix.preTranslate(xTranslate, yTranslate);
        mvpMatrix.preScale(0.5f, 0.5f);

        GLES20.glClearColor(0 /* red */, 0 /* green */, 0 /* blue */, 0 /* alpha */);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        mDrawer.drawTexture(mTexture, mViewWidth, mViewHeight,
                EGLUtil.convertMatrixFromAndroidGraphicsMatrix(mvpMatrix),
                EGLUtil.convertMatrixFromAndroidGraphicsMatrix(renderMatrix));
        mEGLBase.swapBuffers();
      }
      Choreographer.getInstance().postFrameCallback(GLDrawSurfaceViewActivity.this);
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
      mDrawer.release();
      GLES20.glDeleteTextures(1, new int[] {mTexture}, 0);
      mEGLBase.release();
    });
    mWorkHandler.post(() -> {
      Log.i(TAG, "quit opengl worker");
      mWorkHandler.getLooper().quit();
    });
  }
}
