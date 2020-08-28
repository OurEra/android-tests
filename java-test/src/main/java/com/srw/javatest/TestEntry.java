package com.srw.javatest;

import com.example.lib.java.FormatLog;


public class TestEntry {

    private static final boolean TEST_BYTE_BUFFER = true;
    private static final boolean TEST_JSON        = false;
    private static final boolean TEST_STACK_TRACE = false;
    private static final boolean TEST_SYNC        = false;
    private static final boolean TEST_EQUAL       = false;
    private static final boolean TEST_ENUM        = false;
    public static void main(String[] args) {

        if (TEST_BYTE_BUFFER) {
            FormatLog.LogI("TEST BEGIN " + TestByteBuffer.class.getSimpleName());
            TestByteBuffer testByteBuffer = new TestByteBuffer();
            testByteBuffer.test();
            FormatLog.LogI("TEST END " + TestByteBuffer.class.getSimpleName());
        }
        if (TEST_STACK_TRACE) {
            FormatLog.LogI("TEST BEGIN " + TestStackTrace.class.getSimpleName());
            TestStackTrace testStack = new TestStackTrace();
            testStack.test();
            FormatLog.LogI("TEST END " + TestStackTrace.class.getSimpleName());
        }
        if (TEST_JSON) {
            FormatLog.LogI("TEST BEGIN " + TestJSON.class.getSimpleName());
            TestJSON testJSON = new TestJSON();
            testJSON.test();
            FormatLog.LogI("TEST END " + TestJSON.class.getSimpleName());
        }
        if (TEST_SYNC) {
            FormatLog.LogI("TEST BEGIN " + TestSync.class.getSimpleName());
            TestSync testSync = new TestSync();
            testSync.test();
            FormatLog.LogI("TEST END " + TestSync.class.getSimpleName());
        }
        if (TEST_EQUAL) {
            FormatLog.LogI("TEST BEGIN " + TestEquals.class.getSimpleName());
            TestEquals testEquals = new TestEquals();
            testEquals.test();
            FormatLog.LogI("TEST END " + TestEquals.class.getSimpleName());
        }
        if (TEST_ENUM) {
            FormatLog.LogI("TEST BEGIN " + TestEnum.class.getSimpleName());
            TestEnum testEnum = new TestEnum();
            testEnum.test();
            FormatLog.LogI("TEST END " + TestEnum.class.getSimpleName());
        }
    }
}