package com.srw.javatest;

public class TestEntry {

    private static final boolean TEST_BYTE_BUFFER = false;
    private static final boolean TEST_JSON        = false;
    private static final boolean TEST_STACK_TRACE = false;
    private static final boolean TEST_SYNC        = true;
    public static void main(String[] args) {

        if (TEST_BYTE_BUFFER) {
            System.out.println("TEST BEGIN " + TestByteBuffer.class.getSimpleName() + "\n");
            TestByteBuffer testByteBuffer = new TestByteBuffer();
            testByteBuffer.test();
            System.out.println("\nTEST END " + TestByteBuffer.class.getSimpleName());
        }
        if (TEST_STACK_TRACE) {
            System.out.println("TEST BEGIN " + TestStackTrace.class.getSimpleName() + "\n");
            TestStackTrace testStack = new TestStackTrace();
            testStack.test();
            System.out.println("\nTEST END " + TestStackTrace.class.getSimpleName());
        }
        if (TEST_JSON) {
            System.out.println("TEST BEGIN " + TestJSON.class.getSimpleName() + "\n");
            TestJSON testJSON = new TestJSON();
            testJSON.test();
            System.out.println("\nTEST END " + TestJSON.class.getSimpleName());
        }
        if (TEST_SYNC) {
            System.out.println("TEST BEGIN " + TestSync.class.getSimpleName() + "\n");
            TestSync testSync = new TestSync();
            testSync.test();
            System.out.println("\nTEST END " + TestSync.class.getSimpleName());
        }
    }
}