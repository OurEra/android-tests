#ifndef _KDC_MEDIACTRL_SDK_VID_SHARED_RESOURCE_H_
#define _KDC_MEDIACTRL_SDK_VID_SHARED_RESOURCE_H_

#include <jni.h>
#include <inttypes.h>

namespace kvidshare {
class VidShared {

public:
  static int32_t setJavaRes(void * jvm, void * context);
  static JavaVM* GetJvm()     { return gs_jvm; }
  static jobject GetContext() { return gs_context; }

  static int32_t getSdkVersion();

private:
  static JavaVM*  gs_jvm;
  static jobject  gs_context;

  static int32_t gs_sdk_version;
};

} // namespace kvidshare


#endif // _KDC_MEDIACTRL_SDK_VID_SHARED_RESOURCE_H_
