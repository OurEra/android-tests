package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestStackTrace extends BaseTest {

    @Override
    void test() {
        addStack();
    }

    void addStack() {
        StackTraceElement[] trace = Thread.currentThread().getStackTrace();
        FormatLog.LogI("size " + trace.length);
        for (int i = 0; i < trace.length; i++) {
            dump(trace[i]);
        }
    }

    void dump(StackTraceElement element) {
        FormatLog.LogI("class " + element.getFileName() +
                " method " + element.getMethodName() +
                " line " + element.getLineNumber());
    }
}
