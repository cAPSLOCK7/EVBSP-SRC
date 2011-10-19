
LOCAL_PATH := $(call my-dir)
ifeq ($(USE_CAMERA_STUB),false)
include $(CLEAR_VARS)
LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += CameraHardware.cpp
LOCAL_SRC_FILES += V4l2camera.cpp

LOCAL_SHARED_LIBRARIES += \
    libcutils \
    libutils \
    libmedia \
    libui \
    libandroid_runtime \
    libbinder \
    libjpeg \
    libcamera_client \
    libjpegemxx

LOCAL_C_INCLUDES += external/jpeg \
		    external/emxx_jpeg

LOCAL_MODULE := libcamera

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
endif
