LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := os_shared

LOCAL_SRC_FILES := ./src/shared_resource.cpp

LOCAL_LINK_MODE := c++
LOCAL_CFLAGS := -D_OS_ANDROID -D_OS_LINUX -Werror -Wall

LOCAL_CPPFLAGS :=
LOCAL_LDFLAGS :=
LOCAL_LDLIBS :=

LOCAL_STATIC_LIBRARIES := os_wrapper os_utility
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc \
                    $(LOCAL_PATH)/src

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/inc
include $(BUILD_STATIC_LIBRARY)
