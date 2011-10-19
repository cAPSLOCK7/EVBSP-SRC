LOCAL_PATH := $(call my-dir)

# kernel binary
#
#ifeq ($(TARGET_PREBUILT_KERNEL),)
#TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/kernel
#endif

#file := $(INSTALLED_KERNEL_TARGET)
#ALL_PREBUILT += $(file)
#$(file): $(TARGET_PREBUILT_KERNEL) | $(ACP)
#	$(transform-prebuilt-to-target)

# kernel modules
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/modules/inter_dsp.ko:root/lib/modules/inter_dsp.ko \
	$(LOCAL_PATH)/modules/em_ave.ko:root/lib/modules/em_ave.ko \
	$(LOCAL_PATH)/modules/pwm.ko:root/lib/modules/pwm.ko

# EMEV Setting file
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/init.rc:root/init.rc \
	$(LOCAL_PATH)/init.emev.rc:root/init.emxx.rc \
	$(LOCAL_PATH)/init.emev.sh:system/etc/init.emev.sh \
	$(LOCAL_PATH)/vold.fstab:system/etc/vold.fstab

# EMEV logo file
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/initlogo.rle:root/initlogo.rle

# EMEV keycode
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/emev-keypad.kl:system/usr/keylayout/qwerty.kl

