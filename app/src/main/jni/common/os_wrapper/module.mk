LOCAL_PATH := $(call my-dir) 

##################Module 'os_wrapper' begin####################
include $(CLEAR_VARS)
# Declare module name
LOCAL_MODULE := os_wrapper
# LOCAL_MODULE_FILENAME :=

LOCAL_SRC_FILES := \
                ./src/os_atomic.cpp \
                ./src/os_cond.cpp \
                ./src/os_cond_posix.cpp \
                ./src/os_event.cpp \
                ./src/os_event_posix.cpp \
                ./src/os_log.cpp \
                ./src/os_mutex.cpp \
                ./src/os_mutex_posix.cpp \
                ./src/os_thread.cpp \
                ./src/os_thread_posix.cpp \
                ./src/os_time.cpp

LOCAL_LINK_MODE := c++
LOCAL_CFLAGS := -D_OS_ANDROID -D_OS_LINUX -Werror -Wall
LOCAL_CPPFLAGS :=
LOCAL_LDFLAGS :=
LOCAL_LDLIBS :=

# Append any include's paths.
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc $(LOCAL_PATH)/src

# If need to them, the comments can be removed
# LOCAL_CPP_EXTENSION :=
# LOCAL_DEPS_MODULES :=
# LOCAL_WHOLE_STATIC_LIBRARIES :=
# LOCAL_SHARED_LIBRARIES :=
# LOCAL_STATIC_LIBRARIES :=
# LOCAL_EXPORT_CFLAGS :=
# LOCAL_EXPORT_CPPFLAGS :=
# LOCAL_EXPORT_LDFLAGS :=
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/inc
ifdef TARGET_RELEASE_DIR
# LOCAL_RELEASE_PATH := $(TARGET_RELEASE_DIR)/....
endif
include $(BUILD_STATIC_LIBRARY)
###################Module 'os_wrapper' end####################
