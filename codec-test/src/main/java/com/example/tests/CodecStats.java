package com.example.tests;

import com.example.annotations.CalledByNative;

public class CodecStats {

    public static class CodecsStatsInfo {
        private String name;
        private int width;
        private int height;
        private int bps;
        private int fps;

        @CalledByNative("CodecsStatsInfo")
        CodecsStatsInfo(String name, int width, int height, int bps, int fps) {
            this.name = name;
            this.width = width;
            this.height = height;
            this.bps = bps;
            this.fps = fps;
        }
    }

    private static native CodecsStatsInfo nativeGetStatsInfo(long nativeCodecStats);
}
