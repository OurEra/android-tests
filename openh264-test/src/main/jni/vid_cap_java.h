#ifndef _MEDIACTRL_SDK_VID_CAPTURE_JAVA_H_
#define _MEDIACTRL_SDK_VID_CAPTURE_JAVA_H_

#include <jni.h>
#include <os_mutex.h>
#include "h264_encoder_impl.h"
#include "sdk_codec.h"

namespace qiniutest {

int32_t InitCaptureJavaRes(bool init);

class VidCaptureJava {
public:
  VidCaptureJava();
  virtual ~VidCaptureJava();

  int32_t Init(uint32_t dev_idx, void *surface);
  int32_t DeInit();

  int32_t Start();
  int32_t Stop();

  int32_t OnIncomingFrame(uint8_t *frame, int32_t size, int64_t ts);
  int32_t onIncomingEvent(int32_t id, int32_t state);

  static void cacheJavaRes(JNIEnv* env);
private:

  jobject _jcapturer;// Global ref to Java VideoCaptureAndroid object.
  bool _inited;
  bool _started;
  H264EncoderImpl encoder;
  SdkCodec* sdkEncoder;
  os::Mutex *_apiCs;
};

}//namespace qiniutest
#endif // _MEDIACTRL_SDK_VID_CAPTURE_JAVA_H_
