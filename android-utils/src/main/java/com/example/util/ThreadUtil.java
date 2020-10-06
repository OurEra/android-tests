package com.example.util;

import android.support.annotation.Nullable;

public class ThreadUtil {

	public static class ThreadChecker {
		@Nullable
        private Thread thread = Thread.currentThread();

    public void checkIsOnValidThread() {
      if (thread == null) {
        thread = Thread.currentThread();
      }
      if (Thread.currentThread() != thread) {
        throw new IllegalStateException("Wrong thread");
      }
    }

    public void detachThread() {
      thread = null;
    }
	}

}
