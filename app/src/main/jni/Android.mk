LOCAL_PATH := $(call my-dir)
ROOT_PATH := $(LOCAL_PATH)/common

include $(LOCAL_PATH)/build_help.mk

##
include $(call all-module-makefiles-under,$(ROOT_PATH))

##
include $(CLEAR_VARS)
LOCAL_MODULE := qntest
                      
LOCAL_LDLIBS := -L$(ROOT_PATH)/../thirdparty -lz -llog -landroid -lopenh264
LOCAL_CPPFLAGS := -std=c++11
LOCAL_C_INCLUDES := $(ROOT_PATH)/../thirdparty/openh264
LOCAL_SRC_FILES := $(ROOT_PATH)/../vid_cap_java.cpp $(ROOT_PATH)/../test_jni.cpp $(ROOT_PATH)/../h264_encoder_impl.cc
#LOCAL_CPPFLAGS := -ffunction-sections -funwind-tables -fstack-protector-strong \
#                 -no-canonical-prefixes -Wa,--noexecstack -Wformat \
#                 -Werror=format-security -std=c++11
#LOCAL_CPPFLAGS := -fexceptions -frtti
LOCAL_CFLAGS := -D_OS_ANDROID -D_OS_LINUX
LOCAL_WHOLE_STATIC_LIBRARIES := os_wrapper os_shared os_utility

include $(BUILD_SHARED_LIBRARY)
