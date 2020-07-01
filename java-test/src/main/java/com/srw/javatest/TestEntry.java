package com.srw.javatest;

public class TestEntry {

    private static final boolean TEST_BYTE_BUFFER = false;
    private static final boolean TEST_JSON        = true;
    public static void main(String[] args) {
        System.out.println("test begin\n");
        if (TEST_BYTE_BUFFER) {
            TestByteBuffer testByteBuffer = new TestByteBuffer();
            testByteBuffer.test();
        }

        if (TEST_JSON) {
            TestJSON testJSON = new TestJSON();
            testJSON.test();
        }
        System.out.println("test end\n");
    }
}