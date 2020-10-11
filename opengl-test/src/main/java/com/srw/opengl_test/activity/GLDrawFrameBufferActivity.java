package com.srw.opengl_test.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.opengl.GLES20;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.example.util.BitmapOperations;
import com.example.util.FileOperations;
import com.srw.opengl_test.EGLBase;
import com.srw.opengl_test.EGLDrawer;
import com.srw.opengl_test.EGLFrameBuffer;
import com.srw.opengl_test.EGLUtil;
import com.srw.opengl_test.R;

import java.nio.ByteBuffer;

public class GLDrawFrameBufferActivity extends AppCompatActivity {

  private static final String TAG = "GLTEST-" + GLDrawFrameBufferActivity.class.getSimpleName();

  private Handler mWorkHandler;
  private EGLDrawer mDrawer;
  private EGLBase mEGLBase;
  private EGLFrameBuffer mFrameBuffer;
  private int mTexture;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.draw_framebuffer);

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

            mFrameBuffer = new EGLFrameBuffer();
            mFrameBuffer.setFrameBufferSize(1200, 800);
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
      Bitmap pic = BitmapFactory.decodeResource(GLDrawFrameBufferActivity.this.getResources(),
              R.mipmap.pic1);
      EGLUtil.uploadBitmapToTexture(mTexture, pic);

      GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffer.getFrameBuffer());

      // darw buffer starting at top-left corner, not bottom-left.
      final android.graphics.Matrix renderMatrix = new android.graphics.Matrix();
      renderMatrix.preTranslate(0.5f, 0.5f);
      renderMatrix.preScale(1f, 1f);
      renderMatrix.preTranslate(-0.5f, -0.5f);
      mDrawer.drawTexture(mTexture, 1200, 800,
              EGLUtil.convertMatrixFromAndroidGraphicsMatrix(new Matrix()),
              EGLUtil.convertMatrixFromAndroidGraphicsMatrix(renderMatrix));

      // readout what gl draw
      final ByteBuffer rgbaData = ByteBuffer.allocateDirect(mFrameBuffer.getWidth() * mFrameBuffer.getHeight() * 4);
      GLES20.glReadPixels(0, 0, mFrameBuffer.getWidth(), mFrameBuffer.getHeight(),
              GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, rgbaData);
      EGLUtil.checkGlError("readPixel");

      GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

      // draw FBO output to surfaceview
      final android.graphics.Matrix surfaceMatrix = new android.graphics.Matrix();
      surfaceMatrix.preTranslate(0.5f, 0.5f);
      surfaceMatrix.preScale(1f, -1f);
      surfaceMatrix.preTranslate(-0.5f, -0.5f);
      mDrawer.drawTexture(mFrameBuffer.getTexture(), 1200, 800,
              EGLUtil.convertMatrixFromAndroidGraphicsMatrix(new Matrix()),
              EGLUtil.convertMatrixFromAndroidGraphicsMatrix(surfaceMatrix));
      mEGLBase.swapBuffers();
      runOnUiThread(() -> {
        final Bitmap output = Bitmap.createBitmap(mFrameBuffer.getWidth(), mFrameBuffer.getHeight(), Bitmap.Config.ARGB_8888);
        output.copyPixelsFromBuffer(rgbaData);
        FileOperations.saveBitmapToFile(output, "/sdcard/fb_rgba.png");
      });
    }
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    mWorkHandler.postAtFrontOfQueue(() -> {
      mDrawer.release();
      GLES20.glDeleteTextures(1, new int[] {mTexture}, 0);
      mFrameBuffer.release();
      mEGLBase.release();
    });
    mWorkHandler.post(() -> {
      Log.i(TAG, "quit opengl worker");
      mWorkHandler.getLooper().quit();
    });
  }

}

