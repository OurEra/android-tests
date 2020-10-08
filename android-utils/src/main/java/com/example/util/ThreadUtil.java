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

  public static boolean joinThreadUninterrupt(Thread target, long timeoutMs) {

    long remainMs = timeoutMs;
    long startMS = System.currentTimeMillis();
    while(remainMs > 0) {
      try {
        target.join(remainMs);
      } catch (InterruptedException e) {
        long spend = System.currentTimeMillis() - startMS;
        remainMs = timeoutMs - spend;
      }
    }
    return !target.isAlive();
  }

}
