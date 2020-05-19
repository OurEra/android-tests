#include "utility_auto_attach.h"
#include <os_assert.h>

AttachThreadScoped::AttachThreadScoped(JavaVM* jvm)
    : _attached(false), _jvm(jvm), _env(NULL) {
  CHECK(jvm);
  jint ret = jvm->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_4);
  if (ret == JNI_EDETACHED) {
    // Attach the thread to the Java VM.
    ret = _jvm->AttachCurrentThread(&_env, NULL);
    _attached = ret == JNI_OK;
    CHECK(_attached);
  }
}

AttachThreadScoped::~AttachThreadScoped() {
  if (_attached && (_jvm->DetachCurrentThread() < 0)) {
    CHECK(false);
  }
}

JNIEnv* AttachThreadScoped::env() { return _env; }
