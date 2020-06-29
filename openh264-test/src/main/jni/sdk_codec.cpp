#define LOGTAG "srwDebug"
#include <os_log.h>
#include <os_assert.h>
#include <utility_auto_attach.h>
#include <utility_jni.h>
#include <shared_resource.h>
#include <sys/prctl.h>
#include "sdk_codec.h"

static const char* const MediaCodecPathName     = "android/media/MediaCodec";
static const char* const MediaFormatPathName    = "android/media/MediaFormat";
static const char* const BundlePathName         = "android/os/Bundle";
static const char* const BufferInfoPathName     = "android/media/MediaCodec$BufferInfo";

static jclass   gs_jclsMediaCodec  = NULL;
static jclass   gs_jclsMedaiFormat = NULL;
static jclass   gs_jclsBundle = NULL;
static jclass   gs_jclsBufferInfo = NULL;

static jobject _jgcodec;// Global ref to Java MediaCodec object.
static struct fields_saver {

  jmethodID MF_SetIntegerID;
  jmethodID MF_SetStringID;
  jmethodID MF_GetIntegerID;
  jmethodID MC_dequeueInputBuffID;
  jmethodID MC_getInputBuffID;
  jmethodID MC_queueInputBuffID;
  jmethodID MC_setParametersID;
  jmethodID MC_dequeueOutputBuffID;
  jmethodID MC_getOutputBuffID;
  jmethodID MC_releaseOutputBuffID;
  jmethodID MC_getOutputFormatID;

  jmethodID BI_initID;
  jfieldID  BI_offsetID;
  jfieldID  BI_sizeID;
  jfieldID  BI_presentationTimeID;
  jfieldID  BI_flagsID;

} fields_;

#define DEL_HELP(name) \
    if (name) { delete name; name = NULL; }

namespace qiniutest {

int32_t InitCodecJavaRes(bool init) {
  if (init) {
    AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
    JNIEnv *env = ats.env();

    SAVE_GLOBAL_CLASS(env, MediaCodecPathName, gs_jclsMediaCodec);
    SAVE_GLOBAL_CLASS(env, MediaFormatPathName, gs_jclsMedaiFormat);
    SAVE_GLOBAL_CLASS(env, BundlePathName, gs_jclsBundle);
    SAVE_GLOBAL_CLASS(env, BufferInfoPathName, gs_jclsBufferInfo);

    SdkCodec::cacheJavaObj();
  } else {
      AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
      if (gs_jclsMediaCodec) ats.env()->DeleteGlobalRef(gs_jclsMediaCodec);
      gs_jclsMediaCodec = NULL;
      if (gs_jclsMedaiFormat) ats.env()->DeleteGlobalRef(gs_jclsMedaiFormat);
      gs_jclsMedaiFormat = NULL;
      if (gs_jclsBundle) ats.env()->DeleteGlobalRef(gs_jclsBundle);
      gs_jclsBundle = NULL;
      if (gs_jclsBufferInfo) ats.env()->DeleteGlobalRef(gs_jclsBufferInfo);
      gs_jclsBufferInfo = NULL;
  }
  return 0;
}

SdkCodec::SdkCodec(const char *mime, bool encoder):
              _mime(mime),
              _encoder(encoder),
              _started(false),
              _jniEnvCb(NULL),
              _jniEnvQueue(NULL),
              _cb_event(os::Event::Create()),
              _cb_shutdown(false) {
  _api_lock.reset(os::Mutex::Create());

  conf.width = 1280;
  conf.height = 720;
  conf.framerate = 25;
  conf.colorformat = 21;
  conf.bitrate = 1000 * 1000;
  conf.gop = 1;
}

SdkCodec::~SdkCodec() {

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();
  env->DeleteGlobalRef(_jgcodec);
  _jgcodec = NULL;

  DEL_HELP(_cb_event)
}

// static
void SdkCodec::cacheJavaObj() {

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();

  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "setInteger", "(Ljava/lang/String;I)V",
                    fields_.MF_SetIntegerID);
  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "setString", "(Ljava/lang/String;Ljava/lang/String;)V",
                    fields_.MF_SetStringID);
  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "getInteger", "(Ljava/lang/String;I)I",
                     fields_.MF_GetIntegerID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "dequeueInputBuffer", "(J)I",
                     fields_.MC_dequeueInputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "getInputBuffer", "(I)Ljava/nio/ByteBuffer;",
                     fields_.MC_getInputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "queueInputBuffer", "(IIIJI)V",
                     fields_.MC_queueInputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "setParameters", "(Landroid/os/Bundle;)V",
                     fields_.MC_setParametersID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "dequeueOutputBuffer", "(Landroid/media/MediaCodec$BufferInfo;J)I",
                     fields_.MC_dequeueOutputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "getOutputBuffer", "(I)Ljava/nio/ByteBuffer;",
                     fields_.MC_getOutputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "releaseOutputBuffer", "(IZ)V",
                     fields_.MC_releaseOutputBuffID);
  GET_METHOD_ID(env, gs_jclsMediaCodec,
                    "getOutputFormat", "()Landroid/media/MediaFormat;",
                     fields_.MC_getOutputFormatID);
  GET_METHOD_ID(env, gs_jclsBufferInfo,
                    "<init>", "()V",
                     fields_.BI_initID);

  GET_FIELD_ID(env, gs_jclsBufferInfo, "offset", "I", fields_.BI_offsetID);
  GET_FIELD_ID(env, gs_jclsBufferInfo, "size", "I", fields_.BI_sizeID);
  GET_FIELD_ID(env, gs_jclsBufferInfo, "presentationTimeUs", "J", fields_.BI_presentationTimeID);
  GET_FIELD_ID(env, gs_jclsBufferInfo, "flags", "I", fields_.BI_flagsID);

  logd("%s cache fields over", __FUNCTION__);
}

int32_t SdkCodec::start() {
  os::AutoLock lock(_api_lock.get());

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();

  jmethodID j_create = NULL;
  if (_encoder) {
    j_create = env->GetStaticMethodID(gs_jclsMediaCodec, "createEncoderByType",
                        "(Ljava/lang/String;)Landroid/media/MediaCodec;");
  } else {
    j_create = env->GetStaticMethodID(gs_jclsMediaCodec, "createDecoderByType",
                        "(Ljava/lang/String;)Landroid/media/MediaCodec;");
  }
  CHECK(j_create);

  jstring keyObj = env->NewStringUTF(_mime);
  jobject codec = env->CallStaticObjectMethod(gs_jclsMediaCodec, j_create, keyObj);
  _jgcodec = env->NewGlobalRef(codec);
  env->DeleteLocalRef(keyObj); keyObj = NULL;
  env->DeleteLocalRef(codec); codec = NULL;

  if (!_jgcodec) {
    loge("create %s %s failed\n", _encoder ? "Encoder" : "Decoder", _mime);
    return -2;
  }

  // construct MediaFormat and configure
  jmethodID ctor = env->GetMethodID(gs_jclsMedaiFormat, "<init>", "()V");
  CHECK(ctor);
  jobject jmedaiformat = env->NewObject(gs_jclsMedaiFormat, ctor);

  jstring jname = env->NewStringUTF(AMEDIAFORMAT_KEY_MIME);
  jstring jval  = env->NewStringUTF(_mime);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetStringID, jname, jval);
  env->DeleteLocalRef(jname); jname = NULL;
  env->DeleteLocalRef(jval); jval = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_WIDTH);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.width);
  env->DeleteLocalRef(jname); jname = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_HEIGHT);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.height);
  env->DeleteLocalRef(jname); jname = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_COLOR_FORMAT);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.colorformat);
  env->DeleteLocalRef(jname); jname = NULL;

  if (!_encoder) {
    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_STRIDE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.width);
    env->DeleteLocalRef(jname); jname = NULL;
  } else {

    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_BIT_RATE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.bitrate);
    env->DeleteLocalRef(jname); jname = NULL;

    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_FRAME_RATE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, conf.framerate);
    env->DeleteLocalRef(jname); jname = NULL;

    int32_t i_frame_interval = 1;
    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_I_FRAME_INTERVAL);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, i_frame_interval);
    env->DeleteLocalRef(jname); jname = NULL;

    int32_t profile = -1;
    int32_t level = -1;
    if (-1 != profile) {
      jname = env->NewStringUTF("profile");
      env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, profile);
      env->DeleteLocalRef(jname); jname = NULL;

      jname = env->NewStringUTF("level");
      env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, level);
      env->DeleteLocalRef(jname); jname = NULL;
    }
  }

  jmethodID configure = env->GetMethodID(gs_jclsMediaCodec, "configure",
                "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V");
  CHECK(configure);
  env->CallVoidMethod(_jgcodec, configure, jmedaiformat, NULL, NULL, _encoder);

  jmethodID start = env->GetMethodID(gs_jclsMediaCodec, "start", "()V");
  CHECK(start);
  env->CallVoidMethod(_jgcodec, start);
  env->DeleteLocalRef(jmedaiformat);

  pthread_create(&_callback_thread, NULL, SdkCodec::callback_thread, this);
  _started = true;
  return 0;
}

int32_t SdkCodec::stop() {
  os::AutoLock lock(_api_lock.get());

  _cb_shutdown = true;
  // wait callback thread and work thread quit
  // if thread already exit, event will be set before
  // and ensure we won't wait forever here
  _cb_event->wait(OS_EVENT_INFINITE);

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv *env = ats.env();
  if (_started) {
    jmethodID stop = env->GetMethodID(gs_jclsMediaCodec  , "stop", "()V");
    CHECK(stop);
    env->CallVoidMethod(_jgcodec, stop);
    jmethodID release  = env->GetMethodID(gs_jclsMediaCodec  , "release", "()V");
    CHECK(release);
    env->CallVoidMethod(_jgcodec, release);
  }
  _started = false;
  return 0;
}

int32_t SdkCodec::set_bitrate(int32_t bitrate) {
  os::AutoLock lock(_api_lock.get());

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();

  jmethodID ctor = env->GetMethodID(gs_jclsBundle, "<init>", "()V");
  CHECK(ctor);

  jobject jbundle = env->NewObject(gs_jclsBundle , ctor);
  jmethodID putint = env->GetMethodID(gs_jclsBundle , "putInt", "(Ljava/lang/String;I)V");
  CHECK(putint);

  jstring keyname = env->NewStringUTF("video-bitrate");
  env->CallVoidMethod(jbundle, putint, keyname, bitrate);
  env->DeleteLocalRef(keyname); keyname = NULL;

  env->CallVoidMethod(_jgcodec, fields_.MC_setParametersID, jbundle);
  env->DeleteLocalRef(jbundle);
  logd("SdkCodec set bps %d", bitrate);

  return 0;
}

int32_t SdkCodec::enqueue(uint8_t* input_frame, long long ts) {

  ssize_t idx;
  int64_t timeout_us = 500000;
  uint8_t *buff = NULL;
  size_t buf_size = 0;
  uint32_t frame_size = (conf.width * conf.height * 3) >> 1;
  int64_t frame_ts = ts*1000;//us

  if (!_jniEnvQueue) {

     jint res = kvidshare::VidShared::GetJvm()->AttachCurrentThread(&_jniEnvQueue, NULL);
     if ((res < 0) || !_jniEnvQueue) {
       loge("Could not attach playout thread to JVM (%d, %p)", res, _jniEnvQueue);
       return false; // Close down thread
      }
  }

  idx = _jniEnvQueue->CallIntMethod(_jgcodec, fields_.MC_dequeueInputBuffID, timeout_us);
  if (idx < 0) {
    loge("dequeueInput buffer failed idx %d LINE %d\n", idx, __LINE__);
    return -2;
  }

  jobject bufferobj = _jniEnvQueue->CallObjectMethod(_jgcodec, fields_.MC_getInputBuffID, idx);
  buff = (uint8_t *)_jniEnvQueue->GetDirectBufferAddress(bufferobj);
  buf_size = _jniEnvQueue->GetDirectBufferCapacity(bufferobj);
  if (!buff) {
    loge("getInput buffer failed LINE %d", __LINE__);
    return -3;
  }
  if (frame_size > buf_size) {
    loge("invalid size: %u (%u)", frame_size, buf_size);
    return -4;
  }

  //Case: NV12 or NV21
  //Y Copy
  uint8_t *ysrc = input_frame;
  uint8_t *ydst = buff;

  int32_t src_y_stride = conf.width;
  int32_t src_y_size = conf.height * conf.height;
  int32_t dst_y_stride = conf.width;
  int32_t dst_y_size = conf.width * conf.height;
  int32_t w = conf.width;
  int32_t h = conf.height;

  uint8_t *uv_src = ysrc + src_y_size;
  uint8_t *uv_dst = ydst + dst_y_size;

  uint8_t *src_tmp = ysrc;
  uint8_t *dst_tmp = ydst;
  for (int32_t ih = 0; ih < h; ++ih) {
    memcpy(dst_tmp, src_tmp, w);
    src_tmp += src_y_stride;
    dst_tmp += dst_y_stride;
  }
  //UV Copy
  //NV21
  src_tmp = uv_src;
  dst_tmp = uv_dst;
  for (int32_t ih = 0; ih < h/2; ++ih) {
    //ConvertN21ToNV12UVRow
    for (int32_t i = 0; i < w/2; ++i) {
      *(dst_tmp + 2 * i) = *(src_tmp + 2 * i + 1);
      *(dst_tmp + 2 * i + 1) = *(src_tmp + 2 * i);
    }
    src_tmp += src_y_stride;
    dst_tmp += dst_y_stride;
  }
  _jniEnvQueue->CallVoidMethod(_jgcodec, fields_.MC_queueInputBuffID, idx, 0, frame_size, frame_ts, 0);
  _jniEnvQueue->DeleteLocalRef(bufferobj);

  return 0;

}

void* SdkCodec::callback_thread(void *arg) {

  bool _active = true;
  SdkCodec* module = (SdkCodec *)arg;
  prctl(PR_SET_NAME, "qiniu_codec_cb", 0, 0, 0);
  do {
    _active = module->do_callback();
  } while(_active && !module->_cb_shutdown);

  if (module->_jniEnvCb)
    if (kvidshare::VidShared::GetJvm()->DetachCurrentThread() < 0)
        loge("%s detach callback thread failed!!", __func__);
  module->_jniEnvCb = NULL;
  module->_cb_event->set();

  return NULL;
}

bool SdkCodec::do_callback()
{
  if (!_jniEnvCb) {

     jint res = kvidshare::VidShared::GetJvm()->AttachCurrentThread(&_jniEnvCb, NULL);
     if ((res < 0) || !_jniEnvCb) {
       loge("Could not attach playout thread to JVM (%d, %p)", res, _jniEnvCb);
       return false; // Close down thread
      }
  }

  int64_t timeout_us = 50000;
  jobject jbufferInfo = _jniEnvCb->NewObject(gs_jclsBufferInfo, fields_.BI_initID);

  ssize_t idx = _jniEnvCb->CallIntMethod(_jgcodec, fields_.MC_dequeueOutputBuffID, jbufferInfo, timeout_us);
  if (idx == AMEDIACODEC_INFO_TRY_AGAIN_LATER/* ||
    idx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED*/) {
    _jniEnvCb->DeleteLocalRef(jbufferInfo);
    return true;
  }

  if (idx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
    if (idx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
      logi("FORMATE CHANGED\n");

    jobject jformat = _jniEnvCb->CallObjectMethod(_jgcodec, fields_.MC_getOutputFormatID);
    if (!jformat) {
      loge("get output format failed");
      _jniEnvCb->DeleteLocalRef(jbufferInfo);
      return true;
    }

    int32_t width = 0;
    int32_t height = 0;
    int32_t color = 0;

    jstring  jname = _jniEnvCb->NewStringUTF(AMEDIAFORMAT_KEY_WIDTH);
    width = _jniEnvCb->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname, 0);
    _jniEnvCb->DeleteLocalRef(jname); jname = NULL;

    jname = _jniEnvCb->NewStringUTF(AMEDIAFORMAT_KEY_HEIGHT);
    height = _jniEnvCb->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname, 0);
    _jniEnvCb->DeleteLocalRef(jname); jname = NULL;

    jname = _jniEnvCb->NewStringUTF(AMEDIAFORMAT_KEY_COLOR_FORMAT);
    color = _jniEnvCb->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname, 0);
    _jniEnvCb->DeleteLocalRef(jname); jname = NULL;

    _jniEnvCb->DeleteLocalRef(jformat);
    logd("FORMAT [%dx%d]-[%d]\n", width, height, color);

    if (idx < 0) {
      _jniEnvCb->DeleteLocalRef(jbufferInfo);
      return true;
    }
  }
  jobject bufferobj = _jniEnvCb->CallObjectMethod(_jgcodec, fields_.MC_getOutputBuffID, idx);
  uint8_t *buff = (uint8_t *)_jniEnvCb->GetDirectBufferAddress(bufferobj);
  if (!buff) {
    loge("get output buffer failed");
    _jniEnvCb->DeleteLocalRef(bufferobj);
    _jniEnvCb->CallVoidMethod(_jgcodec, fields_.MC_releaseOutputBuffID, idx, false);
    _jniEnvCb->DeleteLocalRef(jbufferInfo);
    return true;
  }

  int32_t size = _jniEnvCb->GetIntField(jbufferInfo, fields_.BI_sizeID);
  int32_t flags = _jniEnvCb->GetIntField(jbufferInfo, fields_.BI_flagsID);
  long presentationTime = _jniEnvCb->GetLongField(jbufferInfo, fields_.BI_presentationTimeID);

  logv("SdkCodec FBD len[%d] ts[%lld] flag[%08x]\n",
          size, presentationTime, flags);

  int32_t _flags = 0x0;
  switch(flags) {
    case 2:
      _flags = 0x80;
    break;
    case 1:
      _flags = 0x20;
    break;
    default:
      _flags = 0x00;
    break;
  }
  logd("callback encoded frame %p idx %d", buff, idx);
  _jniEnvCb->DeleteLocalRef(bufferobj);

  _jniEnvCb->CallVoidMethod(_jgcodec, fields_.MC_releaseOutputBuffID, idx, false);
  _jniEnvCb->DeleteLocalRef(jbufferInfo);
  return true;
}

} //namespace qiniutest
