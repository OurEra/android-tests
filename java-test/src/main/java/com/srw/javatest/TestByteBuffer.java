package com.srw.javatest;

import com.example.lib.java.FormatLog;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public class TestByteBuffer extends BaseTest {

    ByteBuffer testObj;
    @Override
    void test() {

        testObj = ByteBuffer.allocate(5);
        FormatLog.LogI("has arrary " + testObj.hasArray());
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
        FormatLog.LogI("attr read " + readonly.get() + " mark " + readonly.mark());

        // test byte order
        int testInt = 0x12345678;
        ByteBuffer testOrder = ByteBuffer.allocate(10);
        testOrder.order(ByteOrder.BIG_ENDIAN);
        testOrder.putInt(testInt);
        testOrder.order(ByteOrder.LITTLE_ENDIAN);
        testOrder.putInt(testInt);
        FormatLog.LogI("order buffer: " + Arrays.toString(testOrder.array()));

    }

    void dump(String mark) {

        FormatLog.LogI(mark + "===");
        FormatLog.LogI("buffer cap " + testObj.capacity() +
                " buffer limit " + testObj.limit() +
                " buffer pos " + testObj.position() +
                " buffer mark " + testObj.mark());
        FormatLog.LogI("===" + mark + "\n");
    }
}
