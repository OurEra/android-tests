package com.example.lib.java;

public class FormatLog {

    private static final int OFFSET = 2;
    private static FormatLogCallback mCallback;

    public interface FormatLogCallback {
        void onLog(String log);
    }

    public static void LogI(String msg) {
        long tid = Thread.currentThread().getId();
        StackTraceElement[] trace = Thread.currentThread().getStackTrace();
        String format = "[" +
                trace[OFFSET].getFileName() + "(" + tid + ")"
                + " "
                + trace[OFFSET].getMethodName() + ":" + trace[OFFSET].getLineNumber()
                + "]";
        if (mCallback != null) {
            mCallback.onLog(format + msg);
        } else {
            System.out.println(format + msg);
        }
    }
}
