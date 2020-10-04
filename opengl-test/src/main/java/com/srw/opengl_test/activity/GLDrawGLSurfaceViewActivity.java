package com.srw.opengl_test.activity;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.srw.opengl_test.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLDrawGLSurfaceViewActivity extends AppCompatActivity {

    private static final String TAG = "GLTEST-" + GLDrawGLSurfaceViewActivity.class.getSimpleName();

    GLSurfaceView mRenderView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.draw_glsurfaceview);
        mRenderView = findViewById(R.id.renderview);
        mRenderView.setEGLContextClientVersion(2);

        // draw simple color animation
        //mRenderView.setRenderer(new GLRendererSimpleColor());

        // draw simple triangle
        mRenderView.setRenderer(new GLRendererTriangle());
    }

	public class GLRendererTriangle implements GLSurfaceView.Renderer {

	    private int mProgram;
	    private FloatBuffer vertexBuffer;

	    private final String vertexShaderCode =
	            "attribute vec4 vPosition;" +
	             "void main() {" +
	             "  gl_Position = vPosition;" +
	             "}";

	    private final String fragmentShaderCode =
	            "precision mediump float;" +
	             "uniform vec4 vColor;" +
	             "void main() {" +
	             "  gl_FragColor = vColor;" +
	             "}";

	    float triangleCoords[] = {
	            0.5f,  0.5f,  // top
	            -0.5f, -0.5f, // bottom left
	            0.5f, -0.5f,  // bottom right
	    };

	    float color[] = { 0.0f, 0.5f, 0.5f, 1.0f };

	    private int mPositionHandle;
	    private int mColorHandle;

	    @Override
	    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
	        GLES20.glClearColor(0.5f,0.5f,0.5f,1.0f);
	        ByteBuffer bb = ByteBuffer.allocateDirect(triangleCoords.length * 4);
	        bb.order(ByteOrder.nativeOrder());
	        vertexBuffer = bb.asFloatBuffer();
	        vertexBuffer.put(triangleCoords);
	        vertexBuffer.position(0);

	        int vertexShader =  loadShader(GLES20.GL_VERTEX_SHADER,vertexShaderCode);
	        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER,fragmentShaderCode);

	        mProgram = GLES20.glCreateProgram();
	        GLES20.glAttachShader(mProgram,vertexShader);
	        GLES20.glAttachShader(mProgram,fragmentShader);
	        GLES20.glLinkProgram(mProgram);
	    }

	    @Override
	    public void onSurfaceChanged(GL10 gl10, int width, int height) {
	        GLES20.glViewport(0,0,width,height);
	    }

	    @Override
	    public void onDrawFrame(GL10 gl10) {
	        GLES20.glUseProgram(mProgram);

	        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
	        GLES20.glEnableVertexAttribArray(mPositionHandle);
	        GLES20.glVertexAttribPointer(mPositionHandle, 2,
	                GLES20.GL_FLOAT, false,
	                0, vertexBuffer);
	        mColorHandle = GLES20.glGetUniformLocation(mProgram, "vColor");
	        GLES20.glUniform4fv(mColorHandle, 1, color, 0);
	        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 3);
	    }

		public int loadShader(int type, String shaderCode){
		    int shader = GLES20.glCreateShader(type);
		    GLES20.glShaderSource(shader, shaderCode);
		    GLES20.glCompileShader(shader);
		    return shader;
		}
	}

    private class GLRendererSimpleColor implements GLSurfaceView.Renderer {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            GLES20.glClearColor(0f, 0f, 0f, 0f);
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLES20.glViewport(0, 0, width, height);
            Log.i(TAG, "width " + width + " height " + height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            r += 0.01;
            b += 0.01;
            b += 0.01;
            if (r >= 1.0f) {
                r = 0.0f;
            }
            if (g >= 1.0f) {
                g = 0.0f;
            }
            if (b >= 1.0f) {
                b = 0.0f;
            }
            GLES20.glClearColor(r, g, b,0.5f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            Log.i(TAG, "onDraw");
        }
    }

}
