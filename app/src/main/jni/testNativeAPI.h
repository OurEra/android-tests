#include <jni.h>

#ifndef __V7TEST_NATIVE_API_H__
#define __V7TEST_NATIVE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#define JNI_API(x) Java_com_example_tests_NativeApi_##x

JNIEXPORT jint JNICALL JNI_API(init)
  (JNIEnv *, jobject, jobject, jobject);

JNIEXPORT jint JNICALL JNI_API(deinit)
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif

#endif // __V7TEST_NATIVE_API_H__
