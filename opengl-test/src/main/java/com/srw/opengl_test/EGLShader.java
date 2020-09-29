package com.srw.opengl_test;

import android.opengl.GLES20;
import android.util.Log;

public class EGLShader {
    private static final String TAG = "GLTEST-" + EGLShader.class.getSimpleName();

    private int mProgram;

    public EGLShader(String vertex, String fragment) {
        int vertexShader = compileShader(GLES20.GL_VERTEX_SHADER, vertex);
        int fragmentShader = compileShader(GLES20.GL_FRAGMENT_SHADER, fragment);

        mProgram = GLES20.glCreateProgram();
        if (mProgram == 0) {
            throw new RuntimeException("glCreateProgram error");
        }
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);

        // check status
        int[] linkStats = new int[] {GLES20.GL_FALSE};
        GLES20.glGetProgramiv(mProgram, GLES20.GL_LINK_STATUS, linkStats, 0);
        if (linkStats[0] != GLES20.GL_TRUE) {
            throw new RuntimeException("glLinkProgram error");
        }
        Log.i(TAG, "Link program " + mProgram + " result " + linkStats[0]);

        GLES20.glDeleteShader(vertexShader);
        GLES20.glDeleteShader(fragmentShader);
    }

    private int compileShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader == 0) {
            throw new RuntimeException("glCreateShader failed " + + GLES20.glGetError());
        }
        GLES20.glShaderSource(shader, source);
        GLES20.glCompileShader(shader);

        int[] compileStatus = new int[] {GLES20.GL_FALSE};
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);
        if (compileStatus[0] != GLES20.GL_TRUE) {
            throw new RuntimeException("glCompileShader error " + GLES20.glGetShaderInfoLog(shader));
        }
        EGLUtil.checkGlError("glCompileShader");
        return shader;
    }

    public int getUniformLocation(String name) {
        if (mProgram == 0) {
            throw new RuntimeException("getUniformLocation without program");
        }
        int location = GLES20.glGetUniformLocation(mProgram, name);
        if (location < 0) {
            throw new RuntimeException("getUniformLocation return bad location");
        }
        return location;
    }

    public int getAttributeLocation(String name) {
        if (mProgram == 0) {
            throw new RuntimeException("getUniformLocation without program");
        }
        int location = GLES20.glGetAttribLocation(mProgram, name);
        if (location < 0) {
            throw new RuntimeException("glGetAttribLocation return bad location");
        }
        return location;
    }

    public void useProgram() {
        if (mProgram > 0) {
            GLES20.glUseProgram(mProgram);
        } else {
            Log.w(TAG, "invalid program");
        }
    }

    public void release() {
        if (mProgram > 0) {
            GLES20.glDeleteProgram(mProgram);
            mProgram = 0;
        }
    }
}
