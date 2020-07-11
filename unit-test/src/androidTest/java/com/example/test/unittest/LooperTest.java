package com.example.test.unittest;

import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import android.support.test.runner.AndroidJUnit4

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class LooperTest {

    private Handler threadHandler;

    @Test
    public void testLooperQuit() {
        HandlerThread thread = new HandlerThread("test");
        thread.start();
        threadHandler = new Handler(thread.getLooper());

        threadHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(LooperTest.class.getSimpleName(), "before sleep");
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                Log.i(LooperTest.class.getSimpleName(), "sleep over");
            }
        });
        waitDone(2000);
        Log.i(LooperTest.class.getSimpleName(), "before quit");
        threadHandler.getLooper().quit();
        Log.i(LooperTest.class.getSimpleName(), "quit over");
    }

    // @See packages/apps/SnapdragonCamera/src/com/android/camera/AndroidCameraManagerImpl.java
    public boolean waitDone(long timeout) {
        final Object waitDoneLock = new Object();
        final Runnable unlockRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (waitDoneLock) {
                    waitDoneLock.notifyAll();
                }
            }
        };
        synchronized (waitDoneLock) {
            threadHandler.post(unlockRunnable);
            try {
                waitDoneLock.wait(timeout);
                threadHandler.removeCallbacks(unlockRunnable);
            } catch (InterruptedException ex) {
                Log.i(LooperTest.class.getSimpleName(), "waitDone interrupted");
                return false;
            }
        }
        return true;
    }

}
