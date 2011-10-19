/**********************************************************************
 *
 * PURPOSE
 *   Media Component Library Header File
 *
 * AUTHOR
 *   Renesas Electronics Corporation
 *
 * DATE
 *   2010/06/30
 *
 **********************************************************************/
/*
 * Copyright (C) Renesas Electronics Corporation 2008-2010
 * RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
 * This program must be used solely for the purpose for which
 * it was furnished by Renesas Electronics Corporation.
 * No part of this program may be reproduced or disclosed to
 * others, in any form, without the prior written permission
 * of Renesas Electronics Corporation.
 *
 **********************************************************************/

#ifndef __OMF_EXTENSION_H__
#define __OMF_EXTENSION_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_Index.h"
#include "OMX_Audio.h"
#include "OMX_Video.h"
#include "OMX_Image.h"
#include "OMX_IVCommon.h"
#include "OMX_Other.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
/***********************************/
/* Media Component Common Version. */
/***********************************/
#define OMF_MC_CMN_MAJOR_VERSION    0x03
#define OMF_MC_CMN_MINOR_VERSION    0x00
#define OMF_MC_CMN_REVISION_VERSION 0x33
#ifdef ANDROID
#define OMF_MC_CMN_STEP_VERSION     0x31
#else
#define OMF_MC_CMN_STEP_VERSION     0x30
#endif
#define OMF_MC_CMN_VERSION          ((OMF_MC_CMN_STEP_VERSION << 24) | \
                                     (OMF_MC_CMN_REVISION_VERSION << 16) | \
                                     (OMF_MC_CMN_MINOR_VERSION << 8) | \
                                     (OMF_MC_CMN_MAJOR_VERSION))

/*************************************/
/* Extended Index & Parameter Name . */
/*************************************/
/* EM1 SDK extention index name. */
#define OMF_MC_PARAM_VIDEO_POST_FILTER_NAME          "OMX.RENESAS.INDEX.VIDEO.POST.FILETER"
#define OMF_MC_PARAM_VIDEO_DECODE_OPTION_NAME        "OMX.RENESAS.INDEX.VIDEO.DECODE.OPTION"
/* EM1 SDK and EM/EV SDK extention index name. */
#define OMF_MC_PARAM_VIDEO_REORDER_NAME              "OMX.RENESAS.INDEX.VIDEO.REORDER"
#define OMF_MC_PARAM_IMAGE_CONVERT_COLOR_FORMAT_NAME "OMX.RENESAS.INDEX.IMAGE.CONVERT.COLOR.FORMAT"
#define OMF_MC_CONFIG_AUDIO_PCM_NAME                 "OMX.RENESAS.INDEX.AUDIO.CONFIG.PCM.MODE"
#define OMF_MC_CONFIG_IMAGE_DCT_DECODE_MODE_NAME     "OMX.RENESAS.INDEX.IMAGE.DCT.DECODE.MODE"
#define OMF_MC_CONFIG_IMAGE_HUFFMAN_TABLE_MODE_NAME  "OMX.RENESAS.INDEX.IMAGE.HUFFMAN.TABLE.MODE"
#define OMF_MC_PARAM_AUDIO_AC3                       "OMX.RENESAS.INDEX.AUDIO.AC3"
#define OMF_MC_PARAM_VIDEO_VC1_NAME                  "OMX.RENESAS.INDEX.VIDEO.VC1"
enum {
    /* EM1 SDK extention index. */
    OMF_MC_IndexParamVideoPostFilter = OMX_IndexVendorStartUnused,
    OMF_MC_IndexParamVideoDecodeOption,
    /* EM1 SDK and EM/EV SDK extention index. */
    OMF_MC_IndexParamVideoReorder,
    OMF_MC_IndexConfigAudioPcm,
    OMF_MC_IndexParamImageConvertColorFormat,
    OMF_MC_IndexConfigImageDctDecodeMode,
    OMF_MC_IndexConfigImageHuffmanTableMode,
    OMF_MC_IndexParamAudioAC3,
    OMF_MC_IndexParamVideoVc1,
    OMF_MC_IndexCustomizeStartUnused
};

/************************/
/* Extended Coding type.*/
/************************/
enum {
    OMF_MC_AUDIO_CodingAC3 = OMX_AUDIO_CodingVendorStartUnused,
    OMF_MC_VIDEO_CodingVC1 = OMX_VIDEO_CodingVendorStartUnused,
    OMF_MC_AUDIO_CodingCustomizeStartUnused
};

/************************/
/* Extended Error Code. */
/************************/
#define OMF_MC_ErrorEffectuateErrorConcealment (OMX_S32)(0x90000000)
#define OMF_MC_ErrorPcmTimeout                 (OMX_S32)(0x90000001)

/*************************/
/* Extended Buffer Flas. */
/*************************/
#define OMF_MC_BUFFERFLAG_SEEK                       0x00010000
#define OMF_MC_BUFFERFLAG_CLEARPCM                   0x00020000

/**************************/
/* Extended Channel Mode. */
/**************************/
typedef enum OMF_MC_AUDIO_CHANNELMODETYPE {
    OMF_MC_AUDIO_ChannelModeMono   = 0,
    OMF_MC_AUDIO_ChannelModeStereo = 1,
    OMF_MC_AUDIO_ChannelModeMonoR  = 2,
    OMF_MC_AUDIO_ChannelModeMonoL  = 3,
    OMF_MC_AUDIO_ChannelModeEnd    = 0x7FFFFFFF
} OMF_MC_AUDIO_CHANNELMODETYPE;

/****************************/
/* Extended Dual/Mono Mode. */
/****************************/
typedef enum OMF_MC_AUDIO_DUALMONOTYPE {
    OMF_MC_AUDIO_DualMonoModeMain    = 1,
    OMF_MC_AUDIO_DualMonoModeSub     = 2,
    OMF_MC_AUDIO_DualMonoModeMainSub = 3,
    OMF_MC_AUDIO_DualMonoModeEnd     = 0x7FFFFFFF
} OMF_MC_AUDIO_DUALMONOTYPE;

/*********************************/
/* Extended Profile/Level Value. */
/*********************************/
#define OMF_MC_VIDEO_MPEG4ProfileNone    (OMX_VIDEO_MPEG4ProfileVendorStartUnused + 0)
#define OMF_MC_VIDEO_MPEG4ProfileUnknown (OMX_VIDEO_MPEG4ProfileVendorStartUnused + 1)
#define OMF_MC_VIDEO_MPEG4Level3b        (OMX_VIDEO_MPEG4LevelVendorStartUnused   + 0)
#define OMF_MC_VIDEO_MPEG4Level6         (OMX_VIDEO_MPEG4LevelVendorStartUnused   + 1)
#define OMF_MC_VIDEO_MPEG4LevelNone      (OMX_VIDEO_MPEG4LevelVendorStartUnused   + 2)
#define OMF_MC_VIDEO_MPEG4LevelUnknown   (OMX_VIDEO_MPEG4LevelVendorStartUnused   + 3)
#define OMF_MC_VIDEO_AVCProfileUnknown   (OMX_VIDEO_AVCProfileVendorStartUnused   + 0)
#define OMF_MC_VIDEO_AVCLevelUnknown     (OMX_VIDEO_AVCLevelVendorStartUnused     + 0)
#define OMF_MC_VIDEO_MPEG2ProfileMPEG1   (OMX_VIDEO_MPEG2ProfileVendorStartUnused + 0)
#define OMF_MC_VIDEO_MPEG2ProfileUnknown (OMX_VIDEO_MPEG2ProfileVendorStartUnused + 1)
#define OMF_MC_VIDEO_MPEG2LevelMPEG1     (OMX_VIDEO_MPEG2LevelVendorStartUnused   + 0)
#define OMF_MC_VIDEO_MPEG2LevelUnknown   (OMX_VIDEO_MPEG2LevelVendorStartUnused   + 1)

typedef enum OMF_MC_VIDEO_VC1PROFILETYPE {
    OMF_MC_VIDEO_VC1ProfileSimple   = 0x01,   /**< baseline profile */
    OMF_MC_VIDEO_VC1ProfileMain     = 0x02,   /**< main     profile */
    OMF_MC_VIDEO_VC1ProfileAdvanced = 0x04    /**< advanced profile */
} OMF_MC_VIDEO_VC1PROFILETYPE;

typedef enum OMF_MC_VIDEO_VC1LEVELTYPE {
    OMF_MC_VIDEO_VC1LevelLow     = 0x01,      /**< Level low     */
    OMF_MC_VIDEO_VC1LevelMedium  = 0x02,      /**< Level medium  */
    OMF_MC_VIDEO_VC1LevelHigh    = 0x04,      /**< Level high    */
    OMF_MC_VIDEO_VC1Level0       = 0x08,      /**< Level 0       */
    OMF_MC_VIDEO_VC1Level1       = 0x10,      /**< Level 1       */
    OMF_MC_VIDEO_VC1Level2       = 0x20,      /**< Level 2       */
    OMF_MC_VIDEO_VC1Level3       = 0x40,      /**< Level 3       */
	OMF_MC_VIDEO_VC1LevelUnknown = 0x7F000000 /**< Level Unknown */
} OMF_MC_VIDEO_VC1LEVELTYPE;

/****************************/
/* Extended Kcapable Mode.  */
/****************************/
typedef enum OMF_MC_KCAPABLE_MODETYPE {
    OMF_MC_AC3_VOCAL_NO = 0,
    OMF_MC_AC3_VOCAL_LEFT,
    OMF_MC_AC3_VOCAL_RIGHT,
    OMF_MC_AC3_VOCAL_BOTH
} OMF_MC_KCAPABLE_MODETYPE;

/****************************/
/*Extended Compression Mode */
/****************************/
typedef enum OMF_MC_COMPRESSION_MODETYPE {
    OMF_MC_AC3_NO_DIGITAL_DIALOG_NORMALIZATION = 0,
    OMF_MC_AC3_DIGITAL_DIALOG_NORMALIZATION,
    OMF_MC_AC3_LINE_OUT_MODE,
    OMF_MC_AC3_RF_MODE
} OMF_MC_COMPRESSION_MODETYPE;

/****************************/
/* Extended AC3 Output Mode */
/****************************/
typedef enum OMF_MC_OUTPUT_MODETYPE {
    OMF_MC_AC3_OUTPUT_MODE_2_0_DSC = 0,
    OMF_MC_AC3_OUTPUT_MODE_1_0,
    OMF_MC_AC3_OUTPUT_MODE_2_0,
    OMF_MC_AC3_OUTPUT_MODE_3_0,
    OMF_MC_AC3_OUTPUT_MODE_2_1,
    OMF_MC_AC3_OUTPUT_MODE_3_1,
    OMF_MC_AC3_OUTPUT_MODE_2_2,
    OMF_MC_AC3_OUTPUT_MODE_3_2
} OMF_MC_OUTPUT_MODETYPE;

/****************************/
/* Extended AC3 Stereo Mode */
/****************************/
typedef enum OMF_MC_STEREO_MODETYPE {
    OMF_MC_AC3_AUTO_DETECT = 0,
    OMF_MC_AC3_DOLBY_SURROUND_COMPATIBLE,
    OMF_MC_AC3_STEREO
} OMF_MC_STEREO_MODETYPE;

/****************************/
/* Extended AC3 Stereo Mode */
/****************************/
typedef enum OMF_MC_DUALMONO_MODETYPE {
    OMF_MC_AC3_DUAL_MONO_MODE_BOTH_CHANNEL = 0,
    OMF_MC_AC3_DUAL_MONO_MODE_1CHANNEL,
    OMF_MC_AC3_DUAL_MONO_MODE_2CHANNEL,
    OMF_MC_AC3_DUAL_MONO_MODE_MIX_CHANNEL
} OMF_MC_DUALMONO_MODETYPE;

/**************************/
/* Extended Color Format. */
/**************************/
#define OMF_MC_COLOR_FormatYUV411HV22  (OMX_COLOR_FormatYUV420Planar)
#define OMF_MC_COLOR_FormatYUV411HV41  (OMX_COLOR_FormatVendorStartUnused + 0)
#define OMF_MC_COLOR_FormatYUV422HV21  (OMX_COLOR_FormatVendorStartUnused + 1)
#define OMF_MC_COLOR_FormatYUV422HV12  (OMX_COLOR_FormatVendorStartUnused + 2)
#define OMF_MC_COLOR_FormatYUV444HV11  (OMX_COLOR_FormatVendorStartUnused + 3)

/*************************/
/* Extended Decode Mode. */
/*************************/
#define OMF_MC_DCT_DecodeMode8x8       (0x00000000)
#define OMF_MC_DCT_DecodeMode4x4       (0x00000002)
#define OMF_MC_DCT_DecodeMode2x2       (0x00000004)
#define OMF_MC_DCT_DecodeMode1x1       (0x00000008)

/*********************/
/* Port Information. */
/*********************/

/* Port Index Offset, */
#define OMF_MC_OFFSET_PORT_INPUT  (0)
#define OMF_MC_OFFSET_PORT_OUTPUT (1)


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
/***************************/
/* Extended PCM Mode Type. */
/***************************/
typedef struct tagOMF_MC_AUDIO_CONFIG_PCMMODETYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMF_MC_AUDIO_CHANNELMODETYPE eChannelMode;
    OMF_MC_AUDIO_DUALMONOTYPE    eDualMonoMode;
    OMX_S32                      nPlaySpeed;
} OMF_MC_AUDIO_CONFIG_PCMMODETYPE;

/*****************************/
/* Extended PostFilter Type. */
/*****************************/
/* This structure is used for only EM1 SDK. */
typedef struct tagOMF_MC_VIDEO_PARAM_POSTFILTERTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_BOOL                     bPostFilter;
} OMF_MC_VIDEO_PARAM_POSTFILTERTYPE;

/*****************************/
/* Decode Option Type.       */
/*****************************/
/* This structure is used for only EM1 SDK. */
typedef struct tagOMF_MC_VIDEO_PARAM_DECODEOPTIONTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_BOOL                     bDecodeOption;
}OMF_MC_VIDEO_PARAM_DECODEOPTIONTYPE;

/**************************/
/* Extended Reorder Type. */
/**************************/
typedef struct tagOMF_MC_VIDEO_PARAM_REORDERTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_BOOL                     bReorder;
} OMF_MC_VIDEO_PARAM_REORDERTYPE;

/**************************/
/* Extended Vc1 Type.     */
/**************************/
typedef struct tagOMF_MC_VIDEO_PARAM_VC1TYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMF_MC_VIDEO_VC1PROFILETYPE  eProfile;
    OMF_MC_VIDEO_VC1LEVELTYPE    eLevel;
} OMF_MC_VIDEO_PARAM_VC1TYPE;

/************************************/
/* Audio Decoder Output Delay Type. */
/************************************/
typedef struct tagOMF_MC_AUDIO_OUTPUT_DELAYTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32 sec;
    OMX_U32 usec;
} OMF_MC_AUDIO_OUTPUT_DELAYTYPE;

/*******************************/
/* Extended Color Format Type. */
/*******************************/
typedef struct tagOMF_MC_IMAGE_PARAM_CONVERT_COLOR_FORMATTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_U32                      eConvertColorFormat;
} OMF_MC_IMAGE_PARAM_CONVERT_COLOR_FORMATTYPE;

/**********************************/
/* Extended DCT Decode Mode Type. */
/**********************************/
typedef struct tagOMF_MC_IMAGE_CONFIG_DCT_DECODE_MODETYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_U32                      eDctDecodeMode;
} OMF_MC_IMAGE_CONFIG_DCT_DECODE_MODETYPE;

/*************************************/
/* Extended Huffman Table Mode Type. */
/*************************************/
typedef struct tagOMF_MC_IMAGE_CONFIG_HUFFMAN_TABLE_MODETYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_U32                      nPortIndex;
    OMX_BOOL                     bHuffmanTableMode;
} OMF_MC_IMAGE_CONFIG_HUFFMAN_TABLE_MODETYPE;

/******************************/
/* Video Decoder Result Type. */
/******************************/
typedef struct tagOMF_MC_VIDEO_DECODERESULTTYPE {
    OMX_U32                      nSize;
    OMX_VERSIONTYPE              nVersion;
    OMX_PTR pvPhysImageAddressY;
    OMX_PTR pvPhysImageAddressUV;
    OMX_PTR pvVirtImageAddressY;
    OMX_PTR pvVirtImageAddressUV;
    OMX_U16 u16ImageHeight;
    OMX_U16 u16ImageWidth;
    OMX_U16 u16DecodedImageHeight;
    OMX_U16 u16DecodedImageWidth;
    OMX_U16 u16StreamError;
    OMX_U16 u16Profile;
    OMX_U16 u16Level;
    OMX_U16 u16FrameType;
    OMX_U32 u32FrameNumber;
    OMX_U16 u16SplitFlag;
    OMX_U16 u16Reserved1;
    OMX_U16 u16AspectRatioPresentFlag;
    OMX_U16 u16AspectRatioInfo;
    OMX_U16 u16AspectRatioWidth;
    OMX_U16 u16AspectRatioHeight;
    OMX_PTR pVenderPrivate;
} OMF_MC_VIDEO_DECODERESULTTYPE;

/* H.264 specific decode result */
/* This structure is used for only EM1 SDK. */
typedef struct OMF_MC_H264D_DECODERESULTTYPE_ {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U32                     u32ErrorMacroBlockNumber;
    OMX_U32                     u32CorrectMacroBlockNumber;
    OMX_U16                     u16TimingInformationFlag;
    OMX_U16                     u16Reserved1;
    OMX_U32                     u32NumUnitInTick;
    OMX_U32                     u32TimeScale;
    OMX_U16                     u16FixedFrameRateFlag;
    OMX_U16                     u16Reserved2;
    OMX_U32                     u32DecodedPictureCount;
    OMX_U16                     u16CpbDpbDelaysPresentFlag;
    OMX_U16                     u16CpbRemovalDelay;
} OMF_MC_H264D_DECODERESULTTYPE;

/* Mpeg4 specific decode result */
/* This structure is used for only EM1 SDK. */
typedef struct OMF_MC_M4VD_DECODERESULTTYPE_ {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U16                     u16Format;
    OMX_U16                     u16Mpeg4Option;
    OMX_U32                     u32TimeSecond;
    OMX_U16                     u16TimeFrac;
    OMX_U16                     u16TimeResolution;
    OMX_U16                     u16ResyncMarkerDisable;
    OMX_U16                     u16Reserved;
} OMF_MC_M4VD_DECODERESULTTYPE;

/* VC1 specific decode result */
/* This structure is used for only EM1 SDK. */
typedef struct OMF_MC_VC1D_DECODERESULTTYPE_ {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U16                     u16RcvVersion;
    OMX_U16                     u16CodecVersion;
    OMX_U32                     u32NumFrames;
    OMX_U32                     u32FrameRate;
    OMX_U32                     u32TimeStamp;
    OMX_U16                     u16KeyFrame;
    OMX_U16                     u16Reserved1;
} OMF_MC_VC1D_DECODERESULTTYPE;
/* This structure is used for only EM/EV. */
typedef struct OMF_MC_VC1D_V2_DECODERESULTTYPE_ {
	OMX_U32						nSize;
	OMX_VERSIONTYPE				nVersion;
	OMX_U16						u16MultiResolution;
	OMX_U16						u16Reserved1;
} OMF_MC_VC1D_V2_DECODERESULTTYPE;

/******************************/
/* JPEG Decoder Result Type.  */
/******************************/
typedef struct tagOMF_MC_JPEGD_DECODERESULTTYPE {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_PTR         pvPhysImageAddressY;
    OMX_PTR         pvPhysImageAddressCb;
    OMX_PTR         pvPhysImageAddressCr;
    OMX_PTR         pvVirtImageAddressY;
    OMX_PTR         pvVirtImageAddressCb;
    OMX_PTR         pvVirtImageAddressCr;
    OMX_U16         u16ImageHeight;
    OMX_U16         u16ImageWidth;
    OMX_U16         u16DecodeImageHeight;
    OMX_U16         u16DecodeImageWidth;
    OMX_U16         u16StreamError;
    OMX_U16         u16SplitFlags;
    OMX_U16         u16Format;
    OMX_U16         u16SampleRatio;
    OMX_U16         u16Component;
    OMX_U16         u16Reserved1;
    OMX_U16         u16OriginalImageHeight;
    OMX_U16         u16OriginalImageWidth;
    OMX_U16         u16OriginalSampleRatio;
    OMX_U16         u16FilterProcFlag;
} OMF_MC_JPEGD_DECODERESULTTYPE;

/******************************/
/* AC3 Decoder Param Type.    */
/******************************/
typedef struct tagOMF_MC_AUDIO_PARAM_AC3TYPE {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U32                     nPortIndex;
    OMF_MC_KCAPABLE_MODETYPE    eKcapableMode;
    OMF_MC_COMPRESSION_MODETYPE eCompressionMode;
    OMF_MC_OUTPUT_MODETYPE      eOutputMode;
    OMF_MC_STEREO_MODETYPE      eStereoMode;
    OMX_BOOL                    bLfeMode;
    OMF_MC_DUALMONO_MODETYPE    eDualMonoMode;
    OMX_U32                     nPcmScaleFactor;
    OMX_U32                     nDynamicRangeScaleFactorHi;
    OMX_U32                     nDynamicRangeScaleFactorLow;
} OMF_MC_AUDIO_PARAM_AC3TYPE;


/****************************************************************************/
/*    Function prototypes (private)                                         */
/****************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMF_SetLogMode(OMX_IN OMX_U32 u32LogMode);
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMF_GetConfiguration(OMX_OUT OMX_STRING cConfigName, OMX_U32* pu32Length);
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMF_SetConfiguration(OMX_IN OMX_STRING cConfigName);


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OMF_EXTENSION_H__ */
