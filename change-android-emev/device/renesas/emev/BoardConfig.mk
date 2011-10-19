# These definitions override the defaults in config/config.make.

TARGET_BOARD_PLATFORM := emxx
TARGET_CPU_ABI := armeabi

TARGET_ARCH_VARIANT = armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true

TARGET_NO_BOOTLOADER := true
TARGET_NO_KERNEL := true
TARGET_NO_RADIOIMAGE := true
TARGET_NO_RECOVERY := true

TARGET_HARDWARE_3D := false

# Don't copy generic init.rc
TARGET_PROVIDES_INIT_RC := true

# Use EXT2
TARGET_USERIMAGES_USE_EXT2 := true
TARGET_BOOTIMAGE_USE_EXT2 := true

# MultiMedia defines
#BOARD_USES_GENERIC_AUDIO := true
BOARD_USES_ALSA_AUDIO := true
USE_CAMERA_STUB := false

WITH_JIT := true
JS_ENGINE := v8

