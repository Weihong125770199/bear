LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  drm_test.c  

LOCAL_CFLAGS += -D_GNU_SOURCE -DCONFIG_LIBNL20

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libdrm \
        libEGL \
        libGLESv2 \
        libhardware \
        liblog \
        libsync \
        libui \
        libutils

LOCAL_C_INCLUDES := \
        external/libdrm \
        external/libdrm/include/drm \
        system/core/include/utils \
        system/core/libsync \
        system/core/libsync/include \

# Silence some warnings for now. Needs to be fixed upstream. b/26105799
LOCAL_CFLAGS += -Wno-unused-parameter \
                -Wno-sign-compare \
                -Wno-format \
                -Wno-absolute-value 
LOCAL_CLANG_CFLAGS += -Wno-enum-conversion

LOCAL_LDFLAGS := -Wl,--no-gc-sections
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := drm_simple

LOCAL_MODULE_CLASS := EXECUTABLES

include $(BUILD_EXECUTABLE)
