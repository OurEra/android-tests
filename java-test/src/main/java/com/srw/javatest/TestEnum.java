package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestEnum extends BaseTest {

    enum SimpleEnum {
        V1("v-1"),
        V2("v-2");

        private final String mValue;
        SimpleEnum(String v) {
            mValue = v;
        }
        public String getValue() {
            return mValue;
        }
    }

    @Override
    void test() {
        String str = "V1";
        SimpleEnum simpleEnum = SimpleEnum.valueOf(str);
        FormatLog.LogI("enum " + simpleEnum.mValue);
    }
}
