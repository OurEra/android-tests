package com.srw.opengl_test.activity;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.TextureView;

import com.srw.opengl_test.EGLBase;
import com.srw.opengl_test.EGLDrawer;
import com.srw.opengl_test.EGLUtil;
import com.srw.opengl_test.R;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "GLTEST-" + MainActivity.class.getCanonicalName();

    private Handler mWorkHandler;
    private EGLDrawer mDrawer;
    private EGLBase mEGLBase;
    private int mTexture;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        HandlerThread openglWorkThread = new HandlerThread("opengl worker");
        openglWorkThread.start();
        mWorkHandler = new Handler(openglWorkThread.getLooper());
        mWorkHandler.post(() -> {
            mEGLBase = new EGLBase(EGLBase.CONFIG_PLAIN);
            mDrawer = new EGLDrawer();
        });

        ((TextureView)findViewById(R.id.textureview)).setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
                mWorkHandler.post(() -> {
                    mEGLBase.createEGLSurface(((TextureView)findViewById(R.id.textureview)).getSurfaceTexture());
                    mEGLBase.makeCurrent();

                    mTexture = generateTexture(640, 480);
                    Log.i(TAG, "handle " + mTexture);
                });
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                return false;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surface) {

            }
        });

        mWorkHandler.postDelayed(new DrawOperation(), 1000);
    }

    private class DrawOperation implements Runnable {
        @Override
        public void run() {
            if (mEGLBase != null) {
				GLES20.glClearColor(0 /* red */, 0 /* green */, 0 /* blue */, 0 /* alpha */);
				GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
                mDrawer.drawTexture(mTexture, 800, 800, new float[] {
                        1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f,
                        });
                mEGLBase.swapBuffers();
            }
            mWorkHandler.postDelayed(this, 1000);
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
        });
        mWorkHandler.post(() -> {
            Log.i(TAG, "quit opengl worker");
            mWorkHandler.getLooper().quit();
        });
    }

    int generateTexture(int width, int height) {
        Bitmap b = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Paint paint = new Paint();
        Canvas c = new Canvas(b);
        c.drawRect(0, 0, width, height, paint);
        c.drawColor(Color.WHITE);

        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
        paint.setTextSize(40);
        paint.setTextScaleX(1.f);
        paint.setAlpha(0);
        paint.setAntiAlias(true);
        paint.setColor(Color.RED);
        c.drawText(System.currentTimeMillis() + "MS", 30, 40, paint);

		// read to byte buffer
        int length = b.getWidth() * b.getHeight() * 4;

        ByteBuffer pixels = ByteBuffer.allocateDirect(length);
        pixels.order(ByteOrder.LITTLE_ENDIAN);

        b.copyPixelsToBuffer(pixels);
        pixels.position(0);

        saveFile(b);

        // upload to texture
        int[] textureHandles = new int[1];
        int textureHandle;

        GLES20.glGenTextures(1, textureHandles, 0);
        textureHandle = textureHandles[0];
        EGLUtil.checkGlError("glGenTextures");

        // Bind the texture handle to the 2D texture target.
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle);

        // Configure min/mag filtering, i.e. what scaling method do we use if what we're rendering
        // is smaller or larger than the source image.
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);
        EGLUtil.checkGlError("loadImageTexture");

        // Load the data from the buffer into the texture handle.
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, /*level*/ 0, GLES20.GL_RGBA,
                width, height, /*border*/ 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixels);
        EGLUtil.checkGlError("loadImageTexture");
        return textureHandle;
    }

    private void saveFile(Bitmap bitmap) {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, baos);

        File file = new File("/sdcard/test_texture.png");
        try {
            FileOutputStream fos = new FileOutputStream(file);
            try {
                fos.write(baos.toByteArray());
                fos.flush();
                fos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
}
