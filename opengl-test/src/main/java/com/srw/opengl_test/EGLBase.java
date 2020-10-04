package com.srw.opengl_test;

import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.util.Log;
import android.view.Surface;

import javax.microedition.khronos.egl.EGL10;

import static android.opengl.EGL14.EGL_OPENGL_ES2_BIT;

public class EGLBase {

	private EGLDisplay mDisplay;
	private EGLConfig mConfig;
	private EGLContext mContext;
	private EGLSurface mSurface = EGL14.EGL_NO_SURFACE;

	private static final String TAG = "GLTEST-" + EGLBase.class.getSimpleName();

	public static final int[] CONFIG_PLAIN = {
        EGL10.EGL_RED_SIZE, 8,
        EGL10.EGL_GREEN_SIZE, 8,
        EGL10.EGL_BLUE_SIZE, 8,
        EGL10.EGL_ALPHA_SIZE, 8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_NONE
	};

	public EGLBase(int[] attrs) {
		mDisplay = getEglDisplay();
		mConfig = getEGLConfig(mDisplay, attrs);
		mContext = createEglContext(mDisplay, mConfig);
	}

	public void createEGLSurface(SurfaceTexture surfaceTexture) {
		if (mSurface != EGL14.EGL_NO_SURFACE) {
			throw new RuntimeException("Already have EGLSurface");
		}
		if (surfaceTexture == null) {
			throw new RuntimeException("Invalid texutre instance");
		}
		int[] surfaceAttribs = {EGL14.EGL_NONE};
		mSurface = EGL14.eglCreateWindowSurface(mDisplay, mConfig, surfaceTexture, surfaceAttribs, 0);
		Log.i(TAG, "EGLSurface " + mSurface);
	}

  public void createEGLSurface(Surface surface) {
    if (mSurface != EGL14.EGL_NO_SURFACE) {
      throw new RuntimeException("Already have EGLSurface");
    }
    if (surface == null) {
      throw new RuntimeException("Invalid texutre instance");
    }
    int[] surfaceAttribs = {EGL14.EGL_NONE};
    mSurface = EGL14.eglCreateWindowSurface(mDisplay, mConfig, surface, surfaceAttribs, 0);
    Log.i(TAG, "EGLSurface " + mSurface);
  }

	public void makeCurrent() {
		if (mSurface == EGL14.EGL_NO_SURFACE) {
			throw new RuntimeException("No EGLSurface when makeCurrent");
		}
		if (!EGL14.eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)) {
			throw new RuntimeException("egl MakeCurrent failed");
		}
	}

	public void detachCurrent() {
		if (!EGL14.eglMakeCurrent(mDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT)) {
			throw new RuntimeException("egl detachCurrent failed");
		}
	}

	public void swapBuffers() {
		EGL14.eglSwapBuffers(mDisplay, mSurface);
	}

	private EGLDisplay getEglDisplay() {
		EGLDisplay display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
		if (display == EGL14.EGL_NO_DISPLAY) {
			throw new RuntimeException("Can't get valid EGLDisplay");
		}
		int []version = new int[2];
		if (!EGL14.eglInitialize(display, version, 0, version, 1)) {
			throw new RuntimeException("eglInitialize error");
		}
		Log.i(TAG, "version " + version[0] + version[1]);
		return display;
	}

	private EGLConfig getEGLConfig(EGLDisplay display, int[] attrs) {
		EGLConfig[] configs = new EGLConfig[1];
		int[] nums = new int[1];
		if (!EGL14.eglChooseConfig(display, attrs, 0, configs, 0, configs.length, nums, 0)) {
			throw new RuntimeException("Error when eglChooseConfig");
		}
		if (nums[0] <= 0) {
			throw new RuntimeException("can't for EGLConfig");
		}
		if (configs[0] == null) {
			throw new RuntimeException("null EGLConfig");
		}
		Log.i(TAG, "EGLConfig " + configs[0]);
		return configs[0];
	}

	private EGLContext createEglContext(EGLDisplay display, EGLConfig config) {
		int[] contextAttr = { EGL14.EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
		EGLContext context= EGL14.eglCreateContext(display, config, EGL14.EGL_NO_CONTEXT, contextAttr, 0);
		if (context == EGL14.EGL_NO_CONTEXT) {
			throw new RuntimeException("create EGLContext failed");
		}
		Log.i(TAG, "EGLContext " + context);
		return context;
	}

	public void release() {
		if (mDisplay == EGL14.EGL_NO_DISPLAY) {
			throw new RuntimeException("repeat EGL release");
		}
		detachCurrent();
		EGL14.eglDestroySurface(mDisplay, mSurface);
		EGL14.eglDestroyContext(mDisplay, mContext);
		EGL14.eglReleaseThread();
		EGL14.eglTerminate(mDisplay);
		mSurface = EGL14.EGL_NO_SURFACE;
		mContext = EGL14.EGL_NO_CONTEXT;
		mDisplay = EGL14.EGL_NO_DISPLAY;
		mConfig = null;
		Log.i(TAG, "release");
	}
}
