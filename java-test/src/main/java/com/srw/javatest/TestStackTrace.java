package com.srw.javatest;

public class TestStackTrace extends BaseTest {

    @Override
    void test() {
        addStack();
    }

    void addStack() {
        StackTraceElement[] trace = Thread.currentThread().getStackTrace();
        System.out.println("size " + trace.length);
        for (int i = 0; i < trace.length; i++) {
            dump(trace[i]);
        }
    }

    void dump(StackTraceElement element) {
        System.out.println("class " + element.getFileName() +
                " method " + element.getMethodName() +
                " line " + element.getLineNumber());
    }
}
