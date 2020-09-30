#ifndef MEDIACTRL_SDK_VID_SDK_CODEC_H_
#define MEDIACTRL_SDK_VID_SDK_CODEC_H_

#include <os_mutex.h>
#include <os_event.h>
#include <os_cond.h>
#include <utility_scoped_ptr.h>
#include <jni.h>
#include <list>

#include <media/NdkMediaFormat.h>
#include <media/NdkMediaCodec.h>

namespace qiniutest {

int32_t InitCodecJavaRes(bool init);

struct CodecConf {

  int32_t width;
  int32_t height;
  int32_t framerate;
  int32_t colorformat;
  int32_t bitrate;
  int32_t gop;
};

class SdkCodec {
public:
  SdkCodec(const char *mime, bool encoder);
  ~SdkCodec();

  //ISdkCodec cls
  virtual int32_t start();
  virtual int32_t stop();
  virtual int32_t enqueue(uint8_t* input_frame, long long ts);
  virtual int32_t set_bitrate(int32_t bitrate);

  static void cacheJavaObj();
private:
  const char *_mime;
  bool _encoder;
  bool _started;

  CodecConf conf;
  utility::scoped_ptr<os::Mutex> _api_lock;

  JNIEnv *_jniEnvCb;
  JNIEnv *_jniEnvQueue;

  pthread_t _callback_thread;
  static void* callback_thread(void *);
  os::Event *_cb_event;
  bool do_callback();
  bool _cb_shutdown;

  os::Mutex *_work_lock;
  bool _wk_shutdown;

};

}//namespace qiniutest

#endif //_
