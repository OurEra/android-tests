#ifndef _UTILITY_JNI_H_
#define _UTILITY_JNI_H_

#include <stdint.h>
#include <os_assert.h>
#include <jni.h>

#define GET_METHOD_ID(env, cls, name, signature, save) { \
  save = env->GetMethodID(cls, name, signature); \
  CHECK(save); \
}

#define SAVE_GLOBAL_CLASS(env, name, save) { \
  jclass local = env->FindClass(name); CHECK(local); \
  save = reinterpret_cast<jclass>(env->NewGlobalRef(local)); CHECK(save); \
  env->DeleteLocalRef(local); \
}

#define GET_FIELD_ID(env, cls, name, signature, save) { \
  save = env->GetFieldID(cls, name, signature); CHECK(save); \
}

#define GET_STATIC_FIELD_ID(env, cls, name, signature, save) { \
  save = env->GetStaticFieldID(cls, name, signature); CHECK(save); \
}

#endif //_UTILITY_JNI_H_
