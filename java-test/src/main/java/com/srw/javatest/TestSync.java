package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestSync extends BaseTest {

    private int count = 10;
    @Override
    void test() {
        while (count-- > 0) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    target();
                }
            }).start();
        }
    }

    void target() {
        synchronized (this) {
            FormatLog.LogI("target begin");
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            FormatLog.LogI("target end");
        }
    }
}
