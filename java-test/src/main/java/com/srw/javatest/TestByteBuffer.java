package com.srw.javatest;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class TestByteBuffer extends BaseTest {

    ByteBuffer testObj;
    @Override
    void test() {

        testObj = ByteBuffer.allocate(5);
        dump("allocate");
        testObj.limit(4);
        dump("limit");

        testObj.put((byte)0x11);
        dump("put");
        testObj.put((byte)0x11);
        dump("put");
        testObj.flip();
        dump("flip");

        testObj.put((byte)0x11);
        dump("put");
        testObj.put((byte)0x11);
        dump("put");

        testObj.limit(3);
        testObj.rewind();
        dump("limit and rewind");

        ByteBuffer readonly = testObj.asReadOnlyBuffer();
        System.out.println("attr read " + readonly.get() + " mark " + readonly.mark());
    }

    void dump(String mark) {

        System.out.println(mark + "===");
        System.out.println("buffer cap " + testObj.capacity() +
                " buffer limit " + testObj.limit() +
                " buffer pos " + testObj.position() +
                " buffer mark " + testObj.mark());
        System.out.println("===" + mark + "\n");
    }
}
