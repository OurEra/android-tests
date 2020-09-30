#ifndef _UTILITY_AUTO_ATTACH_H_
#define _UTILITY_AUTO_ATTACH_H_

#include <jni.h>

class AttachThreadScoped {
public:
  explicit AttachThreadScoped(JavaVM* jvm);
  ~AttachThreadScoped();
  JNIEnv* env();

private:
  bool _attached;
  JavaVM* _jvm;
  JNIEnv* _env;
};

#endif // _UTILITY_AUTO_ATTACH_H_
