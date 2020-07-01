package com.srw.javatest;

import org.json.JSONObject;

public class TestJSON extends BaseTest {

    @Override
    void test() {
        JSONObject original = new JSONObject();
        original.put("key1", "value1");
        original.put("key2", "value2");
        original.put("key3", "value3");

        String[] copyNames = {"key1", "key4"};
        JSONObject copy = new JSONObject(original, copyNames);

        System.out.println("original " + original + " copy " + copy);

    }
}
