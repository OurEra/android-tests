package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestEntry {

    private static final boolean TEST_BYTE_BUFFER = false;
    private static final boolean TEST_JSON        = false;
    private static final boolean TEST_STACK_TRACE = false;
    private static final boolean TEST_SYNC        = false;
    private static final boolean TEST_EQUAL       = true;
    public static void main(String[] args) {

        if (TEST_BYTE_BUFFER) {
            FormatLog.LogI("TEST BEGIN " + TestByteBuffer.class.getSimpleName() + "\n");
            TestByteBuffer testByteBuffer = new TestByteBuffer();
            testByteBuffer.test();
            FormatLog.LogI("\nTEST END " + TestByteBuffer.class.getSimpleName());
        }
        if (TEST_STACK_TRACE) {
            FormatLog.LogI("TEST BEGIN " + TestStackTrace.class.getSimpleName() + "\n");
            TestStackTrace testStack = new TestStackTrace();
            testStack.test();
            FormatLog.LogI("\nTEST END " + TestStackTrace.class.getSimpleName());
        }
        if (TEST_JSON) {
            FormatLog.LogI("TEST BEGIN " + TestJSON.class.getSimpleName() + "\n");
            TestJSON testJSON = new TestJSON();
            testJSON.test();
            FormatLog.LogI("\nTEST END " + TestJSON.class.getSimpleName());
        }
        if (TEST_SYNC) {
            FormatLog.LogI("TEST BEGIN " + TestSync.class.getSimpleName() + "\n");
            TestSync testSync = new TestSync();
            testSync.test();
            FormatLog.LogI("\nTEST END " + TestSync.class.getSimpleName());
        }
        if (TEST_EQUAL) {
            FormatLog.LogI("TEST BEGIN " + TestEquals.class.getSimpleName() + "\n");
            TestEquals testEquals = new TestEquals();
            testEquals.test();
            FormatLog.LogI("TEST END " + TestEquals.class.getSimpleName());
        }
    }
}