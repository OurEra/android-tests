#include "shared_resource.h"
#include <sys/system_properties.h>
#include <utility_auto_attach.h>
#include <os_assert.h>
#include <os_log.h>

namespace kvidshare {

  JavaVM*  VidShared::gs_jvm = NULL;
  jobject  VidShared::gs_context = NULL;
  int32_t  VidShared::gs_sdk_version = -1;

  int32_t VidShared::setJavaRes(void* jvm, void * context) {

    logi("jvm %p gs_jvm %p", jvm, gs_jvm);
    if (jvm && context) {

      CHECK(!gs_jvm);
      gs_jvm = (JavaVM *)jvm;
      AttachThreadScoped ats(gs_jvm);
      gs_context = ats.env()->NewGlobalRef((jobject)context);

    } else {
      AttachThreadScoped ats(gs_jvm);
      ats.env()->DeleteGlobalRef(gs_context);
      gs_context = NULL;
      gs_jvm = NULL;
    }
    return 0;
  }

  // static
  int32_t VidShared::getSdkVersion() {

    if (gs_sdk_version == -1) {
      intmax_t _result = 0;
      char *end = NULL;
      char buf[PROP_VALUE_MAX] = {'\0',};

      __system_property_get("ro.build.version.sdk", buf);
      _result = strtoimax(buf, &end, /*base*/0);
      gs_sdk_version = _result;
    }
    return gs_sdk_version;
  }

} // namespace kvidshare
