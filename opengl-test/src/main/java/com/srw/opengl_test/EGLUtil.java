package com.srw.opengl_test;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.opengl.GLES20;
import android.util.Log;

import com.srw.utils.BitmapOperations;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;

public class EGLUtil {

    private static final String TAG = "GLTEST-" + EGLUtil.class.getSimpleName();

    public static boolean checkGlError(String op) {
        int error = GLES20.glGetError();
        if (error != GLES20.GL_NO_ERROR) {
            String msg = op + ": glError 0x" + Integer.toHexString(error);
            Log.e(TAG, msg);
            return false;
        }
        return true;
    }

    // Buffers to be passed to gl*Pointer() functions
    // must be direct, i.e., they must be placed on the
    // native heap where the garbage collector cannot
    // move them.
    //
    // Buffers with multi-byte datatypes (e.g., short, int, float)
    // must have their byte order set to native order
    public static FloatBuffer convertToFloatBuffer(float[] input) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(input.length * 4);
        buffer.order(ByteOrder.nativeOrder());
        FloatBuffer fb = buffer.asFloatBuffer();
        fb.put(input);
        fb.rewind();
        return fb;
    }

   /** Converts android.graphics.Matrix to a float[16] matrix array. */
   public static float[] convertMatrixFromAndroidGraphicsMatrix(android.graphics.Matrix matrix) {
     float[] values = new float[9];
     matrix.getValues(values);

     // The android.graphics.Matrix looks like this:
     // [x1 y1 w1]
     // [x2 y2 w2]
     // [x3 y3 w3]
     // We want to contruct a matrix that looks like this:
     // [x1 y1  0 w1]
     // [x2 y2  0 w2]
     // [ 0  0  1  0]
     // [x3 y3  0 w3]
     // Since it is stored in column-major order, it looks like this:
     // [x1 x2 0 x3
     //  y1 y2 0 y3
     //   0  0 1  0
     //  w1 w2 0 w3]
     // clang-format off
     float[] matrix4x4 = {
         values[0 * 3 + 0],  values[1 * 3 + 0], 0,  values[2 * 3 + 0],
         values[0 * 3 + 1],  values[1 * 3 + 1], 0,  values[2 * 3 + 1],
         0,                  0,                 1,  0,
         values[0 * 3 + 2],  values[1 * 3 + 2], 0,  values[2 * 3 + 2],
     };
     // clang-format on
     return matrix4x4;
   }

    public static int generateTexture() {
        int[] textureHandles = new int[1];
        int textureHandle;

        GLES20.glGenTextures(1, textureHandles, 0);
        textureHandle = textureHandles[0];
        EGLUtil.checkGlError("glGenTextures");
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
        return textureHandle;
    }

    public static void uploadBitmapToTexture(int texture, int width, int height) {
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
        // just draw date
        String DATE_PATTERN = "yyyy-MM-dd HH:mm:ss";
        Date date = new Date(System.currentTimeMillis());
        SimpleDateFormat sf = new SimpleDateFormat(DATE_PATTERN);
        c.drawText(sf.format(date), 200, 200, paint);

		// read to byte buffer
        int length = b.getWidth() * b.getHeight() * 4;

        ByteBuffer pixels = ByteBuffer.allocateDirect(length);
        pixels.order(ByteOrder.LITTLE_ENDIAN);

        b.copyPixelsToBuffer(pixels);
        pixels.position(0);

        //BitmapOperations.saveBitmapToFile(b, "/sdcard/test_gl.png");

        // upload to texture
        // Bind the texture handle to the 2D texture target.
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);

        // Load the data from the buffer into the texture handle.
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, /*level*/ 0, GLES20.GL_RGBA,
                width, height, /*border*/ 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixels);
        EGLUtil.checkGlError("loadImageTexture");
    }
}
