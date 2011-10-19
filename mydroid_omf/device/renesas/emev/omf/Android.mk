LOCAL_PATH := $(call my-dir)

# OMF library (DSP F/W) files
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/pvplayer.cfg:system/etc/pvplayer.cfg \
	$(LOCAL_PATH)/omx_av_codec.cfg:system/lib/omf/omx_av_codec.cfg \
	$(LOCAL_PATH)/dspfw/omf_dsp_manager.em-ev:system/lib/omf/dspfw/omf_dsp_manager.em-ev \
	$(LOCAL_PATH)/dspfw/omf_me_aacd.em-ev:system/lib/omf/dspfw/omf_me_aacd.em-ev \
	$(LOCAL_PATH)/dspfw/omf_me_jpegd.em-ev:system/lib/omf/dspfw/omf_me_jpegd.em-ev \
	$(LOCAL_PATH)/dspfw/omf_me_jpege.em-ev:system/lib/omf/dspfw/omf_me_jpege.em-ev

# OMF library (shared lib) files
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/libomf_manager.so:obj/lib/libomf_manager.so \
	$(LOCAL_PATH)/libomf_manager.so:system/lib/libomf_manager.so \
	$(LOCAL_PATH)/libVendor_renesas_omx.so:system/lib/libVendor_renesas_omx.so \
	$(LOCAL_PATH)/libomf_me_video_cmn.so:system/lib/libomf_me_video_cmn.so \
	$(LOCAL_PATH)/omf_mc_aacd.so:system/lib/omf_mc_aacd.so \
	$(LOCAL_PATH)/omf_mc_h264d.so:system/lib/omf_mc_h264d.so \
	$(LOCAL_PATH)/omf_mc_m4vd.so:system/lib/omf_mc_m4vd.so \
	$(LOCAL_PATH)/omf_mc_m2vd.so:system/lib/omf_mc_m2vd.so \
	$(LOCAL_PATH)/omf_mc_vc1d.so:system/lib/omf_mc_vc1d.so \
	$(LOCAL_PATH)/omf_mc_jpegd.so:system/lib/omf_mc_jpegd.so \
	$(LOCAL_PATH)/omf_mc_jpege.so:system/lib/omf_mc_jpege.so
