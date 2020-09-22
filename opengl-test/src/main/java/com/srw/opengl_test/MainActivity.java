package com.srw.opengl_test;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.TextureView;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private Handler mWorkHandler;
    private EGLBase mEGLBase;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        HandlerThread openglWorkThread = new HandlerThread("opengl worker");
        openglWorkThread.start();
        mWorkHandler = new Handler(openglWorkThread.getLooper());
        mWorkHandler.post(() -> {
            mEGLBase = new EGLBase(EGLBase.CONFIG_PLAIN);
        });
        ((TextureView)findViewById(R.id.textureview)).setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
                mWorkHandler.post(() -> {
                    mEGLBase.createEGLSurface(((TextureView)findViewById(R.id.textureview)).getSurfaceTexture());
                    mEGLBase.makeCurrent();

                    int handle = generateTexture(640, 480);
                    Log.i(TAG, "handle " + handle);
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
        c.drawColor(Color.GREEN);

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

        // upload to texture
        int[] textureHandles = new int[1];
        int textureHandle;

        GLES20.glGenTextures(1, textureHandles, 0);
        textureHandle = textureHandles[0];
        checkGlError("glGenTextures");

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
        checkGlError("loadImageTexture");

        // Load the data from the buffer into the texture handle.
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, /*level*/ 0, GLES20.GL_RGBA,
                width, height, /*border*/ 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixels);
        checkGlError("loadImageTexture");
        return textureHandle;
    }

    public static boolean checkGlError(String op) {
        int error = GLES20.glGetError();
        if (error != GLES20.GL_NO_ERROR) {
            String msg = op + ": glError 0x" + Integer.toHexString(error);
            Log.e(TAG, msg);
            return false;
        }
        return true;
    }


}
