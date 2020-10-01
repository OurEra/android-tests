#ifndef VID_CAPTURE_JAVA_H_
#define VID_CAPTURE_JAVA_H_

#include <jni.h>
#include <os_mutex.h>
#include "h264_encoder_impl.h"
#include "sdk_codec_impl.h"

int32_t InitCaptureJavaRes(bool init);

class VidCaptureJava {
public:
  VidCaptureJava();
  virtual ~VidCaptureJava();

  int32_t Init(uint32_t dev_idx, void *surface);
  int32_t DeInit();

  int32_t OnIncomingFrame(uint8_t *frame, int32_t size, int64_t ts);

  static void cacheJavaRes(JNIEnv* env);
private:

  jobject _jcapturer;// Global ref to Java VideoCaptureAndroid object.
  bool _inited;
  H264EncoderImpl encoder;
  SdkCodecImpl* sdkEncoder;
  os::Mutex *_apiCs;
};

#endif
