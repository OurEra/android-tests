#define LOGTAG "CODEC-sdk-codec"

#include <os_log.h>
#include <os_assert.h>
#include <utility_auto_attach.h>
#include <utility_jni.h>
#include <shared_resource.h>
#include <sys/prctl.h>
#include "sdk_codec_impl.h"

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

int32_t InitCodecJavaRes(bool init) {
  if (init) {
    AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
    JNIEnv *env = ats.env();

    SAVE_GLOBAL_CLASS(env, MediaCodecPathName, gs_jclsMediaCodec);
    SAVE_GLOBAL_CLASS(env, MediaFormatPathName, gs_jclsMedaiFormat);
    SAVE_GLOBAL_CLASS(env, BundlePathName, gs_jclsBundle);
    SAVE_GLOBAL_CLASS(env, BufferInfoPathName, gs_jclsBufferInfo);

    SdkCodecImpl::cacheJavaObj();
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

SdkCodecImpl::SdkCodecImpl():
              mime_("video/avc"),
              jniEnvCb_(NULL),
              jniEnvQueue_(NULL),
              cb_event_(os::Event::Create()),
              cb_shutdown_(false) {
}

SdkCodecImpl::~SdkCodecImpl() {

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();
  env->DeleteGlobalRef(_jgcodec);
  _jgcodec = NULL;

  DEL_HELP(cb_event_)
}

// static
void SdkCodecImpl::cacheJavaObj() {

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();

  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "setInteger", "(Ljava/lang/String;I)V",
                    fields_.MF_SetIntegerID);
  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "setString", "(Ljava/lang/String;Ljava/lang/String;)V",
                    fields_.MF_SetStringID);
  GET_METHOD_ID(env, gs_jclsMedaiFormat,
                    "getInteger", "(Ljava/lang/String;)I",
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

int32_t SdkCodecImpl::InitEncode(CodecSetting& setting) {
  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv * env = ats.env();

  jmethodID j_create = NULL;
  if (encoder_) {
    j_create = env->GetStaticMethodID(gs_jclsMediaCodec, "createEncoderByType",
                        "(Ljava/lang/String;)Landroid/media/MediaCodec;");
  } else {
    j_create = env->GetStaticMethodID(gs_jclsMediaCodec, "createDecoderByType",
                        "(Ljava/lang/String;)Landroid/media/MediaCodec;");
  }
  CHECK(j_create);

  jstring keyObj = env->NewStringUTF(mime_.c_str());
  jobject codec = env->CallStaticObjectMethod(gs_jclsMediaCodec, j_create, keyObj);
  _jgcodec = env->NewGlobalRef(codec);
  env->DeleteLocalRef(keyObj); keyObj = NULL;
  env->DeleteLocalRef(codec); codec = NULL;

  if (!_jgcodec) {
    loge("create %s %s failed\n", encoder_ ? "Encoder" : "Decoder", mime_.c_str());
    return -2;
  }

  codecSetting_ = setting;

  // construct MediaFormat and configure
  jmethodID ctor = env->GetMethodID(gs_jclsMedaiFormat, "<init>", "()V");
  CHECK(ctor);
  jobject jmedaiformat = env->NewObject(gs_jclsMedaiFormat, ctor);

  jstring jname = env->NewStringUTF(AMEDIAFORMAT_KEY_MIME);
  jstring jval  = env->NewStringUTF(mime_.c_str());
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetStringID, jname, jval);
  env->DeleteLocalRef(jname); jname = NULL;
  env->DeleteLocalRef(jval); jval = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_WIDTH);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, codecSetting_.width);
  env->DeleteLocalRef(jname); jname = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_HEIGHT);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, codecSetting_.height);
  env->DeleteLocalRef(jname); jname = NULL;

  jname = env->NewStringUTF(AMEDIAFORMAT_KEY_COLOR_FORMAT);
  env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, 21);
  env->DeleteLocalRef(jname); jname = NULL;

  if (!encoder_) {
    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_STRIDE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, codecSetting_.width);
    env->DeleteLocalRef(jname); jname = NULL;
  } else {

    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_BIT_RATE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, codecSetting_.bitrate);
    env->DeleteLocalRef(jname); jname = NULL;

    jname = env->NewStringUTF(AMEDIAFORMAT_KEY_FRAME_RATE);
    env->CallVoidMethod(jmedaiformat, fields_.MF_SetIntegerID, jname, codecSetting_.framerate);
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
  env->CallVoidMethod(_jgcodec, configure, jmedaiformat, NULL, NULL, encoder_);

  jmethodID start = env->GetMethodID(gs_jclsMediaCodec, "start", "()V");
  CHECK(start);
  env->CallVoidMethod(_jgcodec, start);
  env->DeleteLocalRef(jmedaiformat);

  pthread_create(&callback_thread_, NULL, SdkCodecImpl::callback_thread, this);
  started_ = true;
  return 0;

}

int32_t SdkCodecImpl::Encode(const uint8_t* frame, long long ts, bool force_key) {

  ssize_t idx;
  int64_t timeout_us = 500000;
  uint8_t *buff = NULL;
  size_t buf_size = 0;
  uint32_t frame_size = (codecSetting_.width * codecSetting_.height * 3) >> 1;
  int64_t frame_ts = ts*1000;//us

  if (!jniEnvQueue_) {

     jint res = kvidshare::VidShared::GetJvm()->AttachCurrentThread(&jniEnvQueue_, NULL);
     if ((res < 0) || !jniEnvQueue_) {
       loge("Could not attach playout thread to JVM (%d, %p)", res, jniEnvQueue_);
       return false; // Close down thread
      }
  }

  idx = jniEnvQueue_->CallIntMethod(_jgcodec, fields_.MC_dequeueInputBuffID, timeout_us);
  if (idx < 0) {
    loge("dequeueInput buffer failed idx %d LINE %d\n", idx, __LINE__);
    return -2;
  }

  jobject bufferobj = jniEnvQueue_->CallObjectMethod(_jgcodec, fields_.MC_getInputBuffID, idx);
  buff = (uint8_t *)jniEnvQueue_->GetDirectBufferAddress(bufferobj);
  buf_size = jniEnvQueue_->GetDirectBufferCapacity(bufferobj);
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
  uint8_t *ysrc = const_cast<uint8_t *> (frame);
  uint8_t *ydst = buff;

  int32_t src_y_stride = codecSetting_.width;
  int32_t src_y_size = codecSetting_.height * codecSetting_.height;
  int32_t dst_y_stride = codecSetting_.width;
  int32_t dst_y_size = codecSetting_.width * codecSetting_.height;
  int32_t w = codecSetting_.width;
  int32_t h = codecSetting_.height;

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
  jniEnvQueue_->CallVoidMethod(_jgcodec, fields_.MC_queueInputBuffID, idx, 0, frame_size, ts, 0);
  jniEnvQueue_->DeleteLocalRef(bufferobj);

  return 0;

}

int32_t SdkCodecImpl::Release() {
  cb_shutdown_ = true;
  // wait callback thread and work thread quit
  // if thread already exit, event will be set before
  // and ensure we won't wait forever here
  cb_event_->wait(OS_EVENT_INFINITE);

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv *env = ats.env();
  if (started_) {
    jmethodID stop = env->GetMethodID(gs_jclsMediaCodec  , "stop", "()V");
    CHECK(stop);
    env->CallVoidMethod(_jgcodec, stop);
    jmethodID release  = env->GetMethodID(gs_jclsMediaCodec  , "release", "()V");
    CHECK(release);
    env->CallVoidMethod(_jgcodec, release);
  }
  started_ = false;
  return 0;

}

void* SdkCodecImpl::callback_thread(void *arg) {

  bool _active = true;
  SdkCodecImpl* module = (SdkCodecImpl *)arg;
  prctl(PR_SET_NAME, "qiniu_codec_cb", 0, 0, 0);
  do {
    _active = module->do_callback();
  } while(_active && !module->cb_shutdown_);

  if (module->jniEnvCb_)
    if (kvidshare::VidShared::GetJvm()->DetachCurrentThread() < 0)
        loge("%s detach callback thread failed!!", __func__);
  module->jniEnvCb_ = NULL;
  module->cb_event_->set();

  return NULL;
}

bool SdkCodecImpl::do_callback() {
  if (!jniEnvCb_) {

     jint res = kvidshare::VidShared::GetJvm()->AttachCurrentThread(&jniEnvCb_, NULL);
     if ((res < 0) || !jniEnvCb_) {
       loge("Could not attach playout thread to JVM (%d, %p)", res, jniEnvCb_);
       return false; // Close down thread
      }
  }

  int64_t timeout_us = 50000;
  jobject jbufferInfo = jniEnvCb_->NewObject(gs_jclsBufferInfo, fields_.BI_initID);

  ssize_t idx = jniEnvCb_->CallIntMethod(_jgcodec, fields_.MC_dequeueOutputBuffID, jbufferInfo, timeout_us);
  if (idx == AMEDIACODEC_INFO_TRY_AGAIN_LATER/* ||
    idx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED*/) {
    jniEnvCb_->DeleteLocalRef(jbufferInfo);
    return true;
  }

  if (idx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
    if (idx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
      logi("FORMATE CHANGED\n");

    jobject jformat = jniEnvCb_->CallObjectMethod(_jgcodec, fields_.MC_getOutputFormatID);
    if (!jformat) {
      loge("get output format failed");
      jniEnvCb_->DeleteLocalRef(jbufferInfo);
      return true;
    }

    int32_t width = 0;
    int32_t height = 0;
    int32_t color = 0;

    jstring  jname = jniEnvCb_->NewStringUTF(AMEDIAFORMAT_KEY_WIDTH);
    width = jniEnvCb_->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname);
    jniEnvCb_->DeleteLocalRef(jname); jname = NULL;

    jname = jniEnvCb_->NewStringUTF(AMEDIAFORMAT_KEY_HEIGHT);
    height = jniEnvCb_->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname);
    jniEnvCb_->DeleteLocalRef(jname); jname = NULL;

    jname = jniEnvCb_->NewStringUTF(AMEDIAFORMAT_KEY_COLOR_FORMAT);
    color = jniEnvCb_->CallIntMethod(jformat, fields_.MF_GetIntegerID, jname);
    jniEnvCb_->DeleteLocalRef(jname); jname = NULL;

    jniEnvCb_->DeleteLocalRef(jformat);
    logd("FORMAT [%dx%d]-[%d]\n", width, height, color);

    if (idx < 0) {
      jniEnvCb_->DeleteLocalRef(jbufferInfo);
      return true;
    }
  }
  jobject bufferobj = jniEnvCb_->CallObjectMethod(_jgcodec, fields_.MC_getOutputBuffID, idx);
  uint8_t *buff = (uint8_t *)jniEnvCb_->GetDirectBufferAddress(bufferobj);
  if (!buff) {
    loge("get output buffer failed");
    jniEnvCb_->DeleteLocalRef(bufferobj);
    jniEnvCb_->CallVoidMethod(_jgcodec, fields_.MC_releaseOutputBuffID, idx, false);
    jniEnvCb_->DeleteLocalRef(jbufferInfo);
    return true;
  }

  int32_t size = jniEnvCb_->GetIntField(jbufferInfo, fields_.BI_sizeID);
  int32_t flags = jniEnvCb_->GetIntField(jbufferInfo, fields_.BI_flagsID);
  long presentationTime = jniEnvCb_->GetLongField(jbufferInfo, fields_.BI_presentationTimeID);

  logi("SdkCodecImpl FBD len[%d] ts[%lld] flag[%08x]\n",
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
  jniEnvCb_->DeleteLocalRef(bufferobj);

  jniEnvCb_->CallVoidMethod(_jgcodec, fields_.MC_releaseOutputBuffID, idx, false);
  jniEnvCb_->DeleteLocalRef(jbufferInfo);
  return true;
}
