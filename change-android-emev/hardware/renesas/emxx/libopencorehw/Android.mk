LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Set up the OpenCore variables.
LOCAL_C_INCLUDES := $(PV_INCLUDES)

LOCAL_SRC_FILES := \
	android_surface_output_emxx.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libui \
    libhardware\
    libandroid_runtime \
    libmedia \
    libopencore_common \
    libicuuc \
    libopencore_player

LOCAL_MODULE := libopencorehw

LOCAL_LDLIBS += 

include $(BUILD_SHARED_LIBRARY)

