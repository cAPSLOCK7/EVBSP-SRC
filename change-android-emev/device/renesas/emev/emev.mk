
$(call inherit-product, build/target/product/generic.mk)

PRODUCT_PACKAGES += \
    LiveWallpapersPicker \
    LiveWallpapers \
    MagicSmokeWallpapers \
    VisualizationWallpapers \
    librs_jni

$(call inherit-product, frameworks/base/data/sounds/OriginalAudio.mk)

# include available languages for TTS in the system image
include external/svox/pico/lang/PicoLangDeDeInSystem.mk
include external/svox/pico/lang/PicoLangEnGBInSystem.mk
include external/svox/pico/lang/PicoLangEnUsInSystem.mk
include external/svox/pico/lang/PicoLangEsEsInSystem.mk
include external/svox/pico/lang/PicoLangFrFrInSystem.mk
include external/svox/pico/lang/PicoLangItItInSystem.mk

# Overrides
PRODUCT_NAME := renesas_emev
PRODUCT_DEVICE := emev
PRODUCT_BOARD := emev
PRODUCT_BRAND := emxx
PRODUCT_MANUFACTURER := renesas
PRODUCT_LOCALES += ldpi hdpi mdpi ja_JP en_US
PRODUCT_PROPERTY_OVERRIDES += \
    media.stagefright.enable-player=false \
    media.stagefright.enable-meta=false   \
    media.stagefright.enable-scan=false   \
    media.stagefright.enable-http=false   \
    ro.com.android.dataroaming=true \
    keyguard.no_require_sim=true

