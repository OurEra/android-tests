package com.srw.javatest;

import com.example.lib.java.FormatLog;

public class TestEquals extends BaseTest {

    class Base {
        int id;
        Base(int id) {
            this.id = id;
        }

        @Override
        public boolean equals(Object obj) {
            FormatLog.LogI("Base equals");
            if (obj instanceof Base) {
                return id == ((Base) obj).id;
            } else {
                return false;
            }
        }
    }

    class SubClass extends Base {
        SubClass(int id) {
            super(id);
        }
        @Override
        public boolean equals(Object obj) {
            FormatLog.LogI("Sub equals");
            if (obj instanceof Base) {
                return id == ((Base) obj).id;
            } else {
                return false;
            }
        }
    }

    @Override
    void test() {
        Base s1 = new SubClass(1);
        Base s2 = new SubClass(1);
        if (s1.equals(s2)) {
            FormatLog.LogI("s1 equal s2");
        } else {
            FormatLog.LogI("s1 not equal s2");
        }
    }
}
