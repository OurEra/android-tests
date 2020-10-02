#ifndef VID_CAPTURE_JAVA_H_
#define VID_CAPTURE_JAVA_H_

#include <jni.h>
#include <os_mutex.h>
#include "h264_encoder_impl.h"
#include "sdk_codec_impl.h"
#include "x264_encoder_impl.h"

int32_t InitCaptureJavaRes(bool init);

class VidCaptureJava : public EncodeCallback {
public:
  VidCaptureJava();
  virtual ~VidCaptureJava();

  int32_t Init(uint32_t dev_idx, void *surface);
  int32_t DeInit();

  int32_t OnIncomingFrame(uint8_t *frame, int32_t size, int64_t ts);

  static void cacheJavaRes(JNIEnv* env);

  // implements EncodeCallback
  virtual void onEncoded(EncodedImage& image);
private:

  jobject _jcapturer;// Global ref to Java VideoCaptureAndroid object.
  bool _inited;
  H264EncoderImpl open264Encoder;
  X264EncoderImpl x264Encoder;
  SdkCodecImpl* sdkEncoder;
  os::Mutex *_apiCs;
  FILE* dump_fd_ = NULL;
};

#endif
