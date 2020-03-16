LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)   
LOCAL_PREBUILT_LIBS := libusb.so   
include $(BUILD_MULTI_PREBUILT)

common_src_files := \
	accessory_authentication.c \
	iap2.c \
	carlifeIPd.c

common_c_includes := \
	$(KERNEL_HEADERS) \
	system/extras/ext4_utils \
        kernel_imx/include\

common_shared_libraries := \
	libcutils \
	liblog \
	liblogwrap \
        libusb

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        $(common_src_files)

LOCAL_MODULE:= carlifeIPd

LOCAL_C_INCLUDES := \
        $(common_c_includes)

LOCAL_CFLAGS := 

LOCAL_SHARED_LIBRARIES := \
        $(common_shared_libraries)

LOCAL_STATIC_LIBRARIES := \

include $(BUILD_EXECUTABLE)
