package com.example.tests;

import android.content.Context;
import android.view.Surface;

public class NativeApi {

    static {
        System.loadLibrary("qntest");
    }

    public static native int init(Context context, Surface surface);
    public static native int deinit();
}
