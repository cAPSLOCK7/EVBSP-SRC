LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

#KERNEL_DIR    = /work/kernel/HOYA
#ALSA_LIB_DIR  = external/alsa-lib

OPT           = -O2
WARNING       = -Wall -Wstrict-prototypes -Wno-trigraphs
OTHER         = -fpic -fno-strict-aliasing -fno-common -pipe -msoft-float

# Use EMEV
DEFINE        = -DCONFIG_VIDEO_EMXX
DEFINE       += -DCONFIG_VIDEO_EMXX_FILTER
DEFINE       += -DCONFIG_EMXX_NTS

# Use TP
## Not Use for Android
# DEFINE       += -DUSE_ALSA

# Use alsa-lib
#DEFINE       += -D_POSIX_C_SOURCE

LOCAL_SRC_FILES:= \
	emxx_jpeglib.c \
	utility.c

LOCAL_C_INCLUDES := \
	device/renesas/emev/omf                   \
        ./external/opencore/extern_libs_v2/khronos/openmax/include/
#	$(LOCAL_PATH)/include                     
#	$(ALSA_LIB_DIR)/include                   \
#	$(KERNEL_DIR)/include                   
#	$(KERNEL_DIR)/arch/arm/mach-emxx/include/ \
#	$(KERNEL_DIR)/arch/arm/include

LOCAL_SHARED_LIBRARIES := \
	libomf_manager	

#LOCAL_STATIC_LIBRARIES :=libasound

LOCAL_LDLIBS := -lpthread -ldl -lomx_core -lomf_core_hw_dep -lomf_core_os_dep

LOCAL_CFLAGS := $(OPT) $(WARNING) $(OTHER) $(LOCAL_C_INCLUDES) $(DEFINE)

LOCAL_LDFLAGS :=-L$(LIBRARY_DIR) -Wl,-rpath,/system/lib/omf/

LOCAL_MODULE:= libjpegemxx
LOCAL_PRELINK_MODULE := false
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
