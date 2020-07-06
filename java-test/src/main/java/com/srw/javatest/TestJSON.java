package com.srw.javatest;

import com.example.lib.java.FormatLog;

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

        FormatLog.LogI("original " + original + " copy " + copy);

    }
}
