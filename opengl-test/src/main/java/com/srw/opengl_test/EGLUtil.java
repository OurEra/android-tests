package com.srw.opengl_test;

import android.opengl.GLES20;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

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

    public static FloatBuffer convertToFloatBuffer(float[] input) {
        ByteBuffer buffer = ByteBuffer.allocateDirect(input.length * 4);
        FloatBuffer fb = buffer.asFloatBuffer();
        fb.put(input);
        fb.rewind();
        return fb;
    }
}
