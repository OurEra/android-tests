package com.srw.opengl_test;

import android.annotation.TargetApi;
import android.graphics.Matrix;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;
import android.view.Surface;

import com.example.util.ThreadUtil;
import com.example.util.ThreadUtil.ThreadChecker;
import java.io.IOException;
import java.nio.ByteBuffer;

@TargetApi(19)
public class HardwareEncoder {

  private static final String TAG = "GLTEST-" + HardwareEncoder.class.getSimpleName();

  static final String AVC_MIME = "video/avc";
  static final String QCOM_PREFIX = "OMX.qcom.";
  static final String MTK_PREFIX = "OMX.MTK.";
  static final String HISI_PREFIX = "OMX.hisi.";


  private int mWidth;
  private int mHeight;
  private int mBitrate;
  private int mKeyIntervalBySec;
  private int mFrameRate;
  private EncodeCallback mCallback;

  private MediaCodec mCodec;
  private EGLBase mEglBase;
  private Surface mCodecSurface;
  private EGLDrawer mDrawer;
  private ThreadChecker mEncodeContextChecker = new ThreadChecker();

  private Thread mCallbackThread;
  private boolean mCodecRunning;

  public HardwareEncoder() {
    mEncodeContextChecker.detachThread();
  }

  public void initEncode(int width, int height, int bitrate, int framerate, int keyIntervalbysec, EncodeCallback callback) {
    mEncodeContextChecker.checkIsOnValidThread();

    mWidth = width;
    mHeight = height;
    mBitrate = bitrate;
    mFrameRate = framerate;
    mKeyIntervalBySec = keyIntervalbysec;
    mCallback = callback;
    MediaCodecInfo codecInfo = findH264HardwareEncoder();
    try {
      mCodec = MediaCodec.createByCodecName(codecInfo.getName());
    } catch (IOException e) {
      Log.e(TAG, "err: " + e.getMessage());
    }

    try {
      MediaFormat format = MediaFormat.createVideoFormat(AVC_MIME, mWidth, mHeight);
      format.setInteger(MediaFormat.KEY_BIT_RATE, mBitrate);
      format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, mKeyIntervalBySec);
      format.setInteger(MediaFormat.KEY_FRAME_RATE, mFrameRate);
      format.setInteger(MediaFormat.KEY_COLOR_FORMAT, selectColorFormat(getTextureColorFormats(), codecInfo.getCapabilitiesForType(AVC_MIME)));

      mCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

      mCodecSurface = mCodec.createInputSurface();
      mEglBase = new EGLBase(EGLBase.CONFIG_RECORDABLE);
      mEglBase.createEGLSurface(mCodecSurface);
      mEglBase.makeCurrent();

      mCodec.start();
      mCodecRunning = true;
      Log.i(TAG, "init encode done");

      // start callback thread
      mCallbackThread = new CallbackThread();
      mCallbackThread.start();
    } catch (IllegalStateException e) {
       Log.e(TAG, "e " + e.toString());
       release();
    }
  }

  public void encodeTexture(int texture) {
    mEncodeContextChecker.checkIsOnValidThread();
    if (mDrawer == null) {
      mDrawer = new EGLDrawer();
    }
    final android.graphics.Matrix renderMatrix = new android.graphics.Matrix();
    mDrawer.drawTexture(texture, mWidth, mHeight,
            EGLUtil.convertMatrixFromAndroidGraphicsMatrix(new Matrix()),
            EGLUtil.convertMatrixFromAndroidGraphicsMatrix(renderMatrix));
    mEglBase.swapBuffers();
  }

  public void release() {
    mEncodeContextChecker.checkIsOnValidThread();
    if (mCallbackThread != null) {
      mCodecRunning = false;
      ThreadUtil.joinThreadUninterrupt(mCallbackThread, 5000);
    }

    if (mCodecSurface != null) {
      mCodecSurface.release();
    }
    if (mDrawer != null) {
      mDrawer.release();
    }
    if (mEglBase != null) {
      mEglBase.release();
    }
  }

  private MediaCodecInfo findH264HardwareEncoder() {
    int numCodecs = MediaCodecList.getCodecCount();
    for (int i = 0; i < numCodecs; i++) {
      MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
      if (!codecInfo.isEncoder()) {
        continue;
      }

      String[] types = codecInfo.getSupportedTypes();
      for (int j = 0; j < types.length; j++) {
        if (types[j].equalsIgnoreCase(AVC_MIME) &&
            (codecInfo.getName().startsWith(QCOM_PREFIX)
            || codecInfo.getName().startsWith(MTK_PREFIX)
            || codecInfo.getName().startsWith(HISI_PREFIX))) {
          return codecInfo;
        }
      }
    }
    throw new RuntimeException("No 264 hardware encoder");
  }

  private Integer selectColorFormat(int[] supports, MediaCodecInfo.CodecCapabilities caps) {
    for (int support : supports) {
      for (int cap : caps.colorFormats) {
        if (support == cap) {
          Log.i(TAG, "color " + cap);
          return cap;
        }
      }
    }
    return null;
  }

  private static int[] getTextureColorFormats() {
    if (Build.VERSION.SDK_INT >= 18) {
      return new int[] {MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface};
    } else {
      return new int[] {};
    }
  }

  private void releaseCodec() {
    mCodec.stop();
    mCodec.release();
    Log.i(TAG, "release codec");
  }

  private class CallbackThread extends Thread {
    ByteBuffer configBuffer;
    @Override
    public void run() {
      while (mCodecRunning) {
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int index = mCodec.dequeueOutputBuffer(info, 100 * 1000);
        if (index < 0) {
          continue;
        }

        ByteBuffer outputBuffer = mCodec.getOutputBuffers()[index];
        outputBuffer.position(info.offset);
        outputBuffer.limit(info.offset + info.size);

        if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
          configBuffer = ByteBuffer.allocateDirect(info.size);
        } else {
          boolean isKeyFrame = (info.flags & MediaCodec.BUFFER_FLAG_SYNC_FRAME) != 0;
          ByteBuffer frameBuffer;
          if (isKeyFrame) {
            frameBuffer = ByteBuffer.allocateDirect(configBuffer.capacity() + info.size);
            frameBuffer.rewind();

            configBuffer.rewind();
            frameBuffer.put(configBuffer);
            frameBuffer.put(outputBuffer);
            frameBuffer.rewind();
          } else {
            frameBuffer = outputBuffer.slice();
          }
          Log.i(TAG, "encoded key " + isKeyFrame + " size " + frameBuffer.limit());
          mCallback.onEncodedImage(isKeyFrame, frameBuffer);
        }

        mCodec.releaseOutputBuffer(index, false);
      }
      releaseCodec();
    }
  }

  public interface EncodeCallback {
    void onEncodedImage(boolean isKey, ByteBuffer buffer);
  }

}

