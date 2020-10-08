package com.srw.opengl_test;

import android.opengl.GLES20;
import android.util.Log;

import java.nio.ByteBuffer;

public class Texture2YUV {

  private static final String TAG = "GLTEST-" + Texture2YUV.class.getSimpleName();

  public static String CONVERT_FRAGMENT_SHADER =
		"	precision mediump float;\n" +
		"	varying vec2 out_tc;\n" +
		"	uniform sampler2D tex;\n" +
		"	uniform vec2 xUnit;\n" +
		"	uniform vec4 coeffs;\n" +
		"\n" +
		"	void main() {\n" +
		"  gl_FragColor.r = coeffs.a + dot(coeffs.rgb,\n" +
		"      texture2D(tex, out_tc - 1.5 * xUnit).rgb);\n" +
		"  gl_FragColor.g = coeffs.a + dot(coeffs.rgb,\n" +
		"      texture2D(tex, out_tc - 0.5 * xUnit).rgb);\n" +
		"  gl_FragColor.b = coeffs.a + dot(coeffs.rgb,\n" +
		"      texture2D(tex, out_tc + 0.5 * xUnit).rgb);\n" +
		"  gl_FragColor.a = coeffs.a + dot(coeffs.rgb," +
		"      texture2D(tex, out_tc + 1.5 * xUnit).rgb);\n" +
    " }\n";

  private EGLFrameBuffer mFrameBuffer;

  private EGLShader mShader;
  // "in_pos"
  private int inPosAttrLocation;
  // "in_tc"
  private int inTcAttrLocation;
  // "tex_mat"
  private int texMatUniformLocation;
  // "xUnit"
	private int xUnitUniformatLocation;
  // "coeffs"
	private int coeffsUniformatLocation;


	// Y'UV444 to RGB888, see https://en.wikipedia.org/wiki/YUV#Y%E2%80%B2UV444_to_RGB888_conversion
	// We use the ITU-R BT.601 coefficients for Y, U and V.
	// The values in Wikipedia are inaccurate, the accurate values derived from the spec are:
	// Y = 0.299 * R + 0.587 * G + 0.114 * B
	// U = -0.168736 * R - 0.331264 * G + 0.5 * B + 0.5
	// V = 0.5 * R - 0.418688 * G - 0.0813124 * B + 0.5
	// To map the Y-values to range [16-235] and U- and V-values to range [16-240], the matrix has
	// been multiplied with matrix:
	// {{219 / 255, 0, 0, 16 / 255},
	// {0, 224 / 255, 0, 16 / 255},
	// {0, 0, 224 / 255, 16 / 255},
	// {0, 0, 0, 1}}
	private static final float[] yCoeffs =
			new float[] {0.256788f, 0.504129f, 0.0979059f, 0.0627451f};
	private static final float[] uCoeffs =
			new float[] {-0.148223f, -0.290993f, 0.439216f, 0.501961f};
	private static final float[] vCoeffs =
			new float[] {0.439216f, -0.367788f, -0.0714274f, 0.501961f};

  // YUV sampling
  // https://docs.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
  public ByteBuffer convert(int texture, float[] texMatrix, int width, int height) {
    if (mFrameBuffer == null) {
      mFrameBuffer = new EGLFrameBuffer();
    }
    // target framebuffer size need 1/4 of describe width, cause we store four byte in one gl_FragColor.rgba
    mFrameBuffer.setFrameBufferSize(width / 4, height + (height / 2));

    prepareShader(texMatrix);

    GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffer.getFrameBuffer());
    // draw Y plane
    int step = 1;
    GLES20.glUniform4fv(coeffsUniformatLocation, 1, yCoeffs, 0);
    GLES20.glUniform2f(xUnitUniformatLocation, step *  texMatrix[0] / width, step * texMatrix[1] / width);
    drawTexture(texture, 0, 0, width / 4, height);

    // draw U plane, 1/4 of Y samples, sampling both horizontal and vertical
    // U plane occupy half width of frame buffer
    step = 2;
    GLES20.glUniform4fv(coeffsUniformatLocation, 1, uCoeffs, 0);
    GLES20.glUniform2f(xUnitUniformatLocation, step *  texMatrix[0] / width, step * texMatrix[1] / width);
    drawTexture(texture, 0, height, width / 4 / 2, height / 2);

    // draw V plane, 1/4 of Y samples, sampling both horizontal and vertical
    // V plane draw start from (frame buffer) / 2 horizonal
    step = 2;
    GLES20.glUniform4fv(coeffsUniformatLocation, 1, vCoeffs, 0);
    GLES20.glUniform2f(xUnitUniformatLocation, step *  texMatrix[0] / width, step * texMatrix[1] / width);
    drawTexture(texture, width / 4 / 2, height, width / 4 / 2, height / 2);

    // read out yuv
    //  YYYYYYYY
    //  YYYYYYYY
    //  YYYYYYYY
    //  YYYYYYYY
    //  UUUUVVVV
    //  UUUUVVVV
    final ByteBuffer yuvData = ByteBuffer.allocateDirect(width * (height + height / 2));
    GLES20.glReadPixels(0, 0, mFrameBuffer.getWidth(), mFrameBuffer.getHeight(),
            GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, yuvData);
    EGLUtil.checkGlError("readPixel");

    GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

    return yuvData;
  }

  private void drawTexture(int texture, int viewX, int viewY, int width, int height) {
    GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);

    GLES20.glViewport(0, 0, width, height);
    GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    EGLUtil.checkGlError("glDrawArrays");

    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
  }

  public void release() {
    mFrameBuffer.release();
    mShader.release();
  }

  private void prepareShader(float[] texMatrix) {
    if (mShader == null) {
      mShader = new EGLShader(EGLDrawer.DEFAULT_VERTEX_SHADER, CONVERT_FRAGMENT_SHADER);
      mShader.useProgram();
      EGLUtil.checkGlError("new Shader");

      inPosAttrLocation = mShader.getAttributeLocation("in_pos");
      inTcAttrLocation = mShader.getAttributeLocation("in_tc");
      texMatUniformLocation = mShader.getUniformLocation("tex_mat");
			xUnitUniformatLocation = mShader.getUniformLocation("xUnit");
			coeffsUniformatLocation = mShader.getUniformLocation("coeffs");
      GLES20.glUniform1i(mShader.getUniformLocation("tex"), 0); // TEXTURE0
      Log.i(TAG, "in_pos " + inPosAttrLocation + " in_tc " + inTcAttrLocation + " tex_mat " + texMatUniformLocation);
    }

    mShader.useProgram();

    // upload vertex coordinate
    GLES20.glEnableVertexAttribArray(inPosAttrLocation);
    GLES20.glVertexAttribPointer(inPosAttrLocation, 2, GLES20.GL_FLOAT, false, 0, EGLDrawer.VERTEX_COOR_BUFFER);
    EGLUtil.checkGlError("glVertexAttribPointer");

    // upload texture coordinate
    GLES20.glEnableVertexAttribArray(inTcAttrLocation);
    GLES20.glVertexAttribPointer(inTcAttrLocation, 2, GLES20.GL_FLOAT, false, 0, EGLDrawer.TEXTURE_COOR_BUFFER);

    // upload uniform matrix
    GLES20.glUniformMatrix4fv(texMatUniformLocation, 1, false, texMatrix, 0);
  }

}

