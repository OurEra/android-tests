package com.example.tests;

import com.example.lib.camera.VideoCaptureAndroid;

public class VideoCaptureAndroidNative extends VideoCaptureAndroid {

    private final long native_capturer;  // |VideoCaptureAndroid*| in C++.

    private native void ProvideCameraFrame(byte[] data, int length, long timeStamp, long captureObject);

    public VideoCaptureAndroidNative(int id, long native_handle) {
        super(id);
        this.native_capturer = native_handle;

        registerFrameCallback(new PreviewFrameCallback() {
            @Override
            public void onFrame(byte[] bytes, long length, long timeStamp) {
                ProvideCameraFrame(bytes, (int)length, timeStamp, native_capturer);
            }
        });
    }
}
