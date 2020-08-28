package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestEnum extends BaseTest {

    enum SimpleEnum {
        V1("v-1"),
        V2("v-2"),
        V3("vv3");

        private static SimpleEnum[] mValues = values();

        private final String mValue;
        SimpleEnum(String v) {
            mValue = v;
        }
        public String getValue() {
            return mValue;
        }
        public SimpleEnum[] getValues() {
            return mValues;
        }
    }

    @Override
    void test() {
        String str = "V1";
        SimpleEnum simpleEnum = SimpleEnum.valueOf(str);
        FormatLog.LogI("enum " + simpleEnum.mValue);

        SimpleEnum[] vals = simpleEnum.getValues();
        for (SimpleEnum v : vals)
            FormatLog.LogI("value " + v.getValue());
    }
}
