#define LOGTAG "CODEC-jni"
#include <jni.h>
#include <os_log.h>
#include <shared_resource.h>
#include <android/native_window_jni.h>
#include "codec_test_jni.h"
#include "vid_cap_java.h"

static JavaVM* gs_vm = NULL;

static jobject surface = NULL;
static ANativeWindow* windows = NULL;

static VidCaptureJava *capture = NULL;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNI_API(init)
(JNIEnv *env, jobject, jobject context, jobject surf) {

  surface = env->NewGlobalRef(surf);
  windows = ANativeWindow_fromSurface(env, surface);

  kvidshare::VidShared::setJavaRes(gs_vm, context);
  InitCaptureJavaRes(true);
  InitCodecJavaRes(true);

  capture = new VidCaptureJava();
  capture->Init(1, windows);

  return 0;
}

JNIEXPORT jint JNI_API(deinit)
(JNIEnv *env, jobject) {

  InitCaptureJavaRes(false);
  InitCodecJavaRes(false);
  ANativeWindow_release(windows);
  if (surface)
    env->DeleteGlobalRef(surface);
  delete capture;
  return 0;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
  // Only called once.
  gs_vm = vm;
  return JNI_VERSION_1_4;
}

#ifdef __cplusplus
} //extern "C"
#endif
