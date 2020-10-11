package com.srw.opengl_test;

import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;

import java.nio.FloatBuffer;

public class EGLDrawer {

  private static final String TAG = "GLTEST-" + EGLDrawer.class.getSimpleName();

  public static FloatBuffer VERTEX_COOR_BUFFER = EGLUtil.convertToFloatBuffer(new float[] {
      -1.0f,   -1.0f, // Bottom left
      1.0f,    -1.0f, // Bottom right
      -1.0f,   1.0f,  // Top left
      1.0f,    1.0f   // Top right
  });

  /**
   * from bottom left to top right
   * ----->
   *-
   * -
   *  -
   *   -
   *    -
   *     -
   * *****>
   * 0.0f,   0.0f, // Bottom left
   * 1.0f,   0.0f, // Bottom right
   * 0.0f,   1.0f, // Top left
   * 1.0f,   1.0f // Top right
   *
   * from top left to bottom right, this can draw a texture to right direction
   * *****>
   *     -
   *    -
   *   -
   *  -
   * -
   *-
   * ----->
   * 0.0f,   1.0f, // Top left
   * 1.0f,   1.0f, // Top right
   * 0.0f,   0.0f, // Bottom left
   * 1.0f,   0.0f, // Bottom right
   **/
  public static FloatBuffer TEXTURE_COOR_BUFFER = EGLUtil.convertToFloatBuffer(new float[] {
      0.0f,   0.0f, // Bottom left
      1.0f,   0.0f, // Bottom right
      0.0f,   1.0f, // Top left
      1.0f,   1.0f // Top right
  });

  public static String DEFAULT_VERTEX_SHADER =
    " attribute vec4 in_pos;\n" +
    " attribute vec4 in_tc;\n" +
    " uniform mat4 mvp_mat;\n" +
    " uniform mat4 tex_mat;\n" +
    " varying vec2 out_tc;\n" +
    " void main() {\n" +
    "  gl_Position = mvp_mat * in_pos;\n" +
    "  out_tc = (tex_mat * in_tc).xy;\n" +
    " }\n";

  public static String DEFAULT_FRAGMENT_SHADER =
    " precision mediump float;\n" +
    " varying vec2 out_tc;\n" +
    " uniform sampler2D tex;\n" +
    " void main() {\n" +
    "  gl_FragColor = texture2D(tex, out_tc);\n" +
    " }\n";

  private EGLShader mShader;
  // "in_pos"
  private int inPosAttrLocation;
  // "in_tc"
  private int inTcAttrLocation;
  // "tex_mat"
  private int texMatUniformLocation;
  // "mvp_mat"
  private int mvpMatUniformLocation;

  public void drawTexture(int texture, int viewWidth, int viewHeight,  float[] mvpMatrix, float[] texMatrix) {
    prepareShader(mvpMatrix, texMatrix);

    GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);

    GLES20.glViewport(0, 0, viewWidth, viewHeight);
    GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    EGLUtil.checkGlError("glDrawArrays");

    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
  }

  private void prepareShader(float[] mvpMatrix, float[] texMatrix) {
    if (mShader == null) {
      mShader = new EGLShader(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
      mShader.useProgram();
      EGLUtil.checkGlError("new Shader");

      inPosAttrLocation = mShader.getAttributeLocation("in_pos");
      inTcAttrLocation = mShader.getAttributeLocation("in_tc");
      texMatUniformLocation = mShader.getUniformLocation("tex_mat");
      mvpMatUniformLocation = mShader.getUniformLocation("mvp_mat");
      GLES20.glUniform1i(mShader.getUniformLocation("tex"), 0); // TEXTURE0
      Log.i(TAG, "in_pos " + inPosAttrLocation + " in_tc " + inTcAttrLocation + " tex_mat " + texMatUniformLocation);
    }

    mShader.useProgram();

    // upload vertex coordinate
    GLES20.glEnableVertexAttribArray(inPosAttrLocation);
    GLES20.glVertexAttribPointer(inPosAttrLocation, 2, GLES20.GL_FLOAT, false, 0, VERTEX_COOR_BUFFER);
    EGLUtil.checkGlError("glVertexAttribPointer");

    // upload texture coordinate
    GLES20.glEnableVertexAttribArray(inTcAttrLocation);
    GLES20.glVertexAttribPointer(inTcAttrLocation, 2, GLES20.GL_FLOAT, false, 0, TEXTURE_COOR_BUFFER);

    // upload uniform matrix
    GLES20.glUniformMatrix4fv(texMatUniformLocation, 1, false, texMatrix, 0);
    GLES20.glUniformMatrix4fv(mvpMatUniformLocation, 1, false, mvpMatrix, 0);
  }

  public void release() {
    if (mShader != null) {
      mShader.release();
      mShader = null;
    }
  }
}
