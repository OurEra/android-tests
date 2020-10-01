#ifndef SDK_CODEC_IMPL_H_
#define SDK_CODEC_IMPL_H_

#include <os_mutex.h>
#include <os_event.h>
#include <os_cond.h>
#include <utility_scoped_ptr.h>
#include <jni.h>
#include <list>

#include <media/NdkMediaFormat.h>
#include <media/NdkMediaCodec.h>
#include "encoder_base.h"

int32_t InitCodecJavaRes(bool init);
class SdkCodecImpl : public EncoderBase {
public:
  SdkCodecImpl();
  ~SdkCodecImpl();

  // implements EncoderBase
  virtual int32_t InitEncode(CodecSetting& setting);
  virtual void    RegisterCallback(EncodeCallback * callback) { callback_ = callback; }
  virtual int32_t Encode(const uint8_t* frame, long long ts, bool force_key);
  virtual int32_t Release();

  static void cacheJavaObj();
private:
  std::string mime_;
  bool encoder_ = true;
  bool started_ = false;

  EncodeCallback *callback_;
  CodecSetting codecSetting_;

  JNIEnv *jniEnvCb_;
  JNIEnv *jniEnvQueue_;

  pthread_t callback_thread_;
  static void* callback_thread(void *);
  os::Event *cb_event_;
  bool do_callback();
  bool cb_shutdown_;
};

#endif
