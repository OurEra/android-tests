#define LOGTAG "srwDebug"
#include <os_log.h>
#include <string.h>

#include <utility_auto_attach.h>
#include <utility_jni.h>
#include <shared_resource.h>
#include <os_mutex.h>
#include <os_assert.h>
#include "vid_cap_java.h"

static jclass   gs_jclass = NULL;  // VideoCaptureAndroid

static const char* const VideoCapPathName   = "com/example/tests/VideoCaptureAndroid";
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

//Called by Java to get the global application context.
jobject JNICALL GetContext(JNIEnv* env, jobject) {
  CHECK(kvidshare::VidShared::GetContext());
  return kvidshare::VidShared::GetContext();
}

//Called by Java when the camera has a new frame to deliver.
void JNICALL ProvideCameraFrame(
    JNIEnv* env,
    jobject,
    jbyteArray javaCameraFrame,
    jint length,
    jlong timeStamp,
    jlong context) {

  //TODO: FIXME deliver frame
  qiniutest::VidCaptureJava* capture =
      reinterpret_cast<qiniutest::VidCaptureJava*>(
          context);

  jbyte* frame = env->GetByteArrayElements(javaCameraFrame, NULL);
  capture->OnIncomingFrame(reinterpret_cast<uint8_t*>(frame), length, timeStamp);
  env->ReleaseByteArrayElements(javaCameraFrame, frame, JNI_ABORT);
}

//Called by Java when the device orientation has changed.
void JNICALL OnOrientationChanged(
    JNIEnv* env, jobject, jlong context, jint degrees) {

  //TODO: FIXME deliver frame
  /*
  webrtc::videocapturemodule::VidCaptureJava* captureModule =
      reinterpret_cast<webrtc::videocapturemodule::VidCaptureJava*>(
          context);
  degrees = (360 + degrees) % 360;
  CHECK(degrees >= 0 && degrees < 360);
  VideoCaptureRotation rotation =
      (degrees <= 45 || degrees > 315) ? kCameraRotate0 :
      (degrees > 45 && degrees <= 135) ? kCameraRotate90 :
      (degrees > 135 && degrees <= 225) ? kCameraRotate180 :
      (degrees > 225 && degrees <= 315) ? kCameraRotate270 :
      kCameraRotate0;  // Impossible.
  int32_t status =
      captureModule->VideoCaptureImpl::SetCaptureRotation(rotation);
  CHECK(status == 0);
  */
}

void JNICALL onCapSendResult(JNIEnv* env, jobject, jlong context, jint id, jint value) {

  qiniutest::VidCaptureJava* capture =
      reinterpret_cast<qiniutest::VidCaptureJava*>(
          context);
  capture->onIncomingEvent(id, value);
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

namespace qiniutest {

int32_t InitCaptureJavaRes(bool init) {
  logi("init");
  if (init) {
    AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
    JNIEnv *env = ats.env();

    SAVE_GLOBAL_CLASS(env, VideoCapPathName, gs_jclass);
    SAVE_GLOBAL_CLASS(env, CameraAreaPathName, gs_jclsCameraArea);
    SAVE_GLOBAL_CLASS(env, RectPathName, gs_jclsRect);
    SAVE_GLOBAL_CLASS(env, ListPathName, gs_jclsList);

    VidCaptureJava::cacheJavaRes(env);

    JNINativeMethod native_methods[] = {
        {"GetContext",
         "()Landroid/content/Context;",
         reinterpret_cast<void*>(&GetContext)},
        {"OnOrientationChanged",
         "(JI)V",
         reinterpret_cast<void*>(&OnOrientationChanged)},
        {"ProvideCameraFrame",
         "([BIJJ)V",
         reinterpret_cast<void*>(&ProvideCameraFrame)},
        {"sendResult",
         "(JII)V",
         reinterpret_cast<void*>(&onCapSendResult)}};
    if (ats.env()->RegisterNatives(gs_jclass,
                                   native_methods, 4) != 0)
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
      _inited(false),
      _started(false) {
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

  encoder.InitEncode();
  return 0;
}

int32_t VidCaptureJava::DeInit() {
  os::AutoLock lock(_apiCs);
  if (!_inited)
    return 0;
  // Ensure Java camera is released even if our caller didn't explicitly Stop.
  if (_started) {
    logw("Waring camera not stopped, force stop it\n");
    Stop();
  }
  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  ats.env()->DeleteGlobalRef(_jcapturer);
  _inited = false;
  encoder.Release();
  return 0;
}

int32_t VidCaptureJava::Start() {
  os::AutoLock lock(_apiCs);
  if (_started)
    return 0;
  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv* env = ats.env();

  //TODO:FIXME pls check capability validated

  jmethodID j_start =
      env->GetMethodID(gs_jclass, "startCapture", "(IIII)Z");
  CHECK(j_start);

  //TODO:FIXME get fixable framerate
  int32_t min_mfps = 25 * 1000;
  int32_t max_mfps = 25 * 1000;

  bool started = env->CallBooleanMethod(_jcapturer, j_start,
                                        1280,
                                        720,
                                        min_mfps, max_mfps);
  if (started) {
    _started = true;
  }
  return started ? 0 : -1;
}

int32_t VidCaptureJava::Stop() {
  _apiCs->lock();
  if (!_started)
    return 0;
  AttachThreadScoped ats(kvidshare::VidShared::GetJvm());
  JNIEnv* env = ats.env();

  _started = false;
  // Exit critical section to avoid blocking camera thread inside
  // onIncomingFrame() call.
  _apiCs->unlock();

  jmethodID j_stop =
      env->GetMethodID(gs_jclass, "stopCapture", "()Z");
  return env->CallBooleanMethod(_jcapturer, j_stop) ? 0 : -1;
}

int32_t VidCaptureJava::OnIncomingFrame(
        uint8_t *frame,
        int32_t size, int64_t ts) {

  os::AutoLock lock(_apiCs);
  if (_started) {
    //VideoFrameInfo info;
    //info.width = 1280;
    //info.height = 720;
    //info.color = 1;//eVidFrameColorNV21;
    //info.timestamp = ts;
    //info.planes = 2;

    //_video_cb->onIncomingFrame(vplane, info, _video_cb_ctx);
    encoder.Encode(frame, false, ts);
  }
  return 0;
}

int32_t VidCaptureJava::onIncomingEvent(int32_t id, int32_t state) {

  return 0;
}

}  // namespace qiniutest
