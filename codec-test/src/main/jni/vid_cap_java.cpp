#define LOGTAG "CODEC-vid-cap"
#include <os_log.h>
#include <string.h>

#include <utility_auto_attach.h>
#include <utility_jni.h>
#include <shared_resource.h>
#include <os_mutex.h>
#include <os_assert.h>
#include "vid_cap_java.h"

static bool USE_SDK_CODEC = false;

static jclass   gs_jclass = NULL;

static const char* const VideoCapPathName   = "com/example/tests/VideoCaptureAndroidNative";
static const char* const CameraAreaPathName = "android/hardware/Camera$Area";
static const char* const RectPathName       = "android/graphics/Rect";
static const char* const ListPathName       = "java/util/ArrayList";

static jclass   gs_jclsCameraArea = NULL;
static jclass   gs_jclsRect       = NULL;
static jclass   gs_jclsList       = NULL;

static struct fields {
  jmethodID areaInit;
  jfieldID areaRectID;
  jfieldID areaWeightID;
  jmethodID rectInit;
  jfieldID rectLeftID;
  jfieldID rectTopID;
  jfieldID rectRightID;
  jfieldID rectBottomID;
  jmethodID listInit;
  jmethodID listAdd;

} fields_;

//Called by Java when the camera has a new frame to deliver.
void JNICALL ProvideCameraFrame(
    JNIEnv* env,
    jobject,
    jbyteArray javaCameraFrame,
    jint length,
    jlong timeStamp,
    jlong context) {

  //TODO: FIXME deliver frame
  VidCaptureJava* capture =
      reinterpret_cast<VidCaptureJava*>(
          context);

  jbyte* frame = env->GetByteArrayElements(javaCameraFrame, NULL);
  capture->OnIncomingFrame(reinterpret_cast<uint8_t*>(frame), length, timeStamp);
  env->ReleaseByteArrayElements(javaCameraFrame, frame, JNI_ABORT);
}

static int32_t jstring2chars(JNIEnv *env, jstring  jstr, char *szBuf, int32_t size)
{
  bool bsuccess = false;
  jclass clsstring = env->FindClass("java/lang/String");
  jstring strencode = env->NewStringUTF("GB2312");
  jmethodID mid = env->GetMethodID(clsstring,"getBytes","(Ljava/lang/String;)[B");
  jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
  jsize alen = env->GetArrayLength(barr);
  jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
  if(alen < size) {
    memcpy(szBuf, ba, alen);
    szBuf[alen] = 0;
    bsuccess = true;
  }
  env->ReleaseByteArrayElements(barr, ba, 0);
  env->DeleteLocalRef(strencode);
  env->DeleteLocalRef(barr);
  return bsuccess;
}

int32_t InitCaptureJavaRes(bool init) {
  if (init) {
    AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
    JNIEnv *env = ats.env();

    SAVE_GLOBAL_CLASS(env, VideoCapPathName, gs_jclass);
    SAVE_GLOBAL_CLASS(env, CameraAreaPathName, gs_jclsCameraArea);
    SAVE_GLOBAL_CLASS(env, RectPathName, gs_jclsRect);
    SAVE_GLOBAL_CLASS(env, ListPathName, gs_jclsList);

    VidCaptureJava::cacheJavaRes(env);

    JNINativeMethod native_methods[] = {
        {"ProvideCameraFrame",
         "([BIJJ)V",
         reinterpret_cast<void*>(&ProvideCameraFrame)}};
    if (ats.env()->RegisterNatives(gs_jclass,
                                   native_methods, 1) != 0)
      CHECK(false);
  } else {
      AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
      if (gs_jclass) {
        ats.env()->UnregisterNatives(gs_jclass);
        ats.env()->DeleteGlobalRef(gs_jclass);
        gs_jclass = NULL;
      }
      if (gs_jclsCameraArea) ats.env()->DeleteGlobalRef(gs_jclsCameraArea);
      gs_jclsCameraArea = NULL;
      if (gs_jclsRect) ats.env()->DeleteGlobalRef(gs_jclsRect);
      gs_jclsRect = NULL;
      if (gs_jclsList) ats.env()->DeleteGlobalRef(gs_jclsList);
      gs_jclsList = NULL;
  }
  return 0;
}

// static
void VidCaptureJava::cacheJavaRes(JNIEnv* env) {

  GET_FIELD_ID(env, gs_jclsCameraArea, "rect", "Landroid/graphics/Rect;", fields_.areaRectID);
  GET_FIELD_ID(env, gs_jclsCameraArea, "weight", "I", fields_.areaWeightID);
  GET_FIELD_ID(env, gs_jclsRect, "left", "I", fields_.rectLeftID);
  GET_FIELD_ID(env, gs_jclsRect, "top", "I", fields_.rectTopID);
  GET_FIELD_ID(env, gs_jclsRect, "right", "I", fields_.rectRightID);
  GET_FIELD_ID(env, gs_jclsRect, "bottom", "I", fields_.rectBottomID);

  GET_METHOD_ID(env, gs_jclsCameraArea,
                    "<init>", "(Landroid/graphics/Rect;I)V",
                     fields_.areaInit);
  GET_METHOD_ID(env, gs_jclsRect,
                    "<init>", "(IIII)V",
                     fields_.rectInit);
  GET_METHOD_ID(env, gs_jclsList,
                    "<init>", "()V",
                     fields_.listInit);
  GET_METHOD_ID(env, gs_jclsList,
                    "add", "(Ljava/lang/Object;)Z",
                     fields_.listAdd);
}

VidCaptureJava::VidCaptureJava()
    : _jcapturer(NULL),
      _inited(false) {
  _apiCs = os::Mutex::Create();
  CHECK(_apiCs);
}

VidCaptureJava::~VidCaptureJava() {
  delete _apiCs;
}


int32_t VidCaptureJava::Init(uint32_t dev_idx, void *surface) {
  int32_t ret = 0;
  int32_t name_len = 128;
  os::AutoLock lock(_apiCs);
  if (_inited)
    return 0;

  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv* env = ats.env();
  jmethodID ctor = env->GetMethodID(gs_jclass, "<init>", "(IJ)V");
  CHECK(ctor);
  jlong j_this = reinterpret_cast<intptr_t>(this);

  jobject jcapture = env->NewObject(gs_jclass, ctor, dev_idx, j_this);
  _jcapturer = env->NewGlobalRef(jcapture);
  env->DeleteLocalRef(jcapture); jcapture = NULL;
  CHECK(_jcapturer);
  _inited = true;

  char szName[1024] = {0};
  sprintf(szName, "/sdcard/codectest.%s", "h264");
  dump_fd_ = fopen(szName, "w+");
  if (!dump_fd_)
    loge("open %s failed %s", szName, strerror(errno));
  else
    loge("open %s ok", szName);


  CodecSetting settings;
  settings.width = 1280;
  settings.height = 720;
  settings.bitrate = 2000 * 1000;
  settings.keyinterval = 70;
  settings.framerate= 25;
  logi("vid java init");
  open264Encoder.InitEncode(settings);
  open264Encoder.RegisterCallback(this);
  x264Encoder.InitEncode(settings);
  x264Encoder.RegisterCallback(this);
  if (USE_SDK_CODEC) {
    sdkEncoder = new SdkCodecImpl();
    sdkEncoder->InitEncode(settings);
  }
  return 0;
}


void VidCaptureJava::onEncoded(EncodedImage& image) {
  if (dump_fd_) {
    fwrite(image.buffer, image.length, 1, dump_fd_);
    fflush(dump_fd_);
  }
}

int32_t VidCaptureJava::DeInit() {
  os::AutoLock lock(_apiCs);
  if (!_inited)
    return 0;
  // Ensure Java camera is released even if our caller didn't explicitly Stop.
  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  ats.env()->DeleteGlobalRef(_jcapturer);
  _inited = false;
  open264Encoder.Release();
  x264Encoder.Release();
  if (USE_SDK_CODEC) {
    sdkEncoder->Release();
    delete sdkEncoder;
  }
  if (dump_fd_) {
    fclose(dump_fd_);
  }

  return 0;
}

static int32_t counter = 0;
int32_t VidCaptureJava::OnIncomingFrame(
        uint8_t *frame,
        int32_t size, int64_t ts) {

  os::AutoLock lock(_apiCs);
  //VideoFrameInfo info;
  //info.width = 1280;
  //info.height = 720;
  //info.color = 1;//eVidFrameColorNV21;
  //info.timestamp = ts;
  //info.planes = 2;

  //_video_cb->onIncomingFrame(vplane, info, _video_cb_ctx);
  //if (counter++ % 100 == 0)

  if (counter++ < 50) {
    open264Encoder.Encode(frame, ts, false);
  } else {
    x264Encoder.Encode(frame, ts, false);
  }
  if (USE_SDK_CODEC) {
    sdkEncoder->Encode(frame, ts, false);
  }
  return 0;
}
