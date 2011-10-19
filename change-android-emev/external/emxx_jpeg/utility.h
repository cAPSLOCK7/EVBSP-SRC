#ifndef __UTILITY_H__
#define __UTILITY_H__
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include "OMF_Extension.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define EXTENDED_SEEK_METHOD
#define MODE_OUTPUT_RENDER      (0)
#define MODE_OUTPUT_DECODER   (1)
#define MODE_INPUT_ENCODER        (2)


#define CODEC_AUDIO             (0)
#define CODEC_VIDEO             (1)
#define CODEC_NONE              (2)

#define PARAM_UDINV             (1)
#define PARAM_RLINV             (2)

#ifdef MULTI_PROCESS

typedef enum _PIPE_FIND {
	PIPE0_MAIN2SUB = 0,
	PIPE0_SUB2MAIN,
	PIPE1_MAIN2SUB,
	PIPE1_SUB2MAIN,
	PIPE2_MAIN2SUB,
	PIPE2_SUB2MAIN,
	PIPE_FIND_MAX,
}PIPE_FIND;
#endif

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef struct tagBUFFERCTRLTYPE {
	OMX_BUFFERHEADERTYPE* psQueTop;
	OMX_BUFFERHEADERTYPE* psQueEnd;
	pthread_mutex_t       BuffMutex;
	pthread_cond_t        BuffCond;
	int                   nNumOfQueing;
} BUFFERCTRLTYPE;

typedef struct tagVIDEODECFUNCPARAMTYPE {
	char						VideoFileName[256];
	int							Mode;
	int							FrameInterval;
	int							resize;
	int							rotate;
	int							inv;
	OMX_U32						nFrameWidth;
	OMX_U32						nFrameHeight;
	OMX_U32						nBitrate;
	OMX_U32						xFramerate;
	OMX_U32						nPFrames;
	OMX_VIDEO_AVCPROFILETYPE	eProfile;
	OMX_VIDEO_AVCLEVELTYPE		eLevel;
	OMX_BOOL					bconstIpred;
	OMX_VIDEO_AVCLOOPFILTERTYPE	eLoopFilterMode;
	OMX_COLOR_FORMATTYPE		eColorFormat;
	OMX_U32						eConvertColorFormat;
	OMX_U32						eDctDecodeMode;
	int							useFb;
	int							outputMonitor;
	OMX_U32						nQFactor;		/* use JPEG-Enc */
	OMX_U32						ResizeWidth;
	OMX_U32						ResizeHeight;
	OMX_S32						CropTop;
	OMX_S32						CropLeft;
	OMX_U32						CropWidth;
	OMX_U32						CropHeight;
	int							segNum;		/* use H.264 */
	int							segErr;		/* use H.264 */
	int							noSps;		/* use H.264 */
	int							fillType;	/* use H.264/MPEG2V/MPEG4V */
	int							frameSkip;	/* use decoder */
	int							nextEos;	/* use decoder */
	int							oneBuff;	/* use decoder */
	int							n2DivBuffFlag;
	int							h264NalMax;
	int							nFilledLengthFlag;
	int 						eoframe;
	int							eos;
	int 						outOrder; /* h264 only*/
	int 						compBuff;
	int							jpgeloop;/* for jpg encoding loop only*/
	int							dropdci;
	int							nDataLossFlg;
	int							loadedDly;
	int							disableEOF;
 
	int							pluFrame; /* use VC-1 */
	int							noSeq;    /* use VC-1 */
	int							framErr;  /* use VC-1 */
	int							nFilledLen_1frame; /* use VC-1 */

	int							hdmi; //display by hdmi
	int 						saveYUV;
	int						halfman;
	int						fbdisp;
} VIDEODECFUNCPARAMTYPE;

typedef struct tagAUDIODECFUNCPARAMTYPE {
	char							AudioFileName[256];
	int								Mode;
	int								Volume;
	int								PauseTime;
	int								ResumeTime;
	int								BitRate;          /* use MP3-Enc, AAC-Enc */
	int								verbose;
	int								InputSampleRate;  /* use */
	int								InputChannel;     /* use */
	int								OutputSampleRate; /* use MP3-Enc, AAC-Enc */
	int								OutputChannel;    /* use MP3-Enc, AAC-Enc */
	int								nBitPerSample;
	int								MicMode;          /* use Encoder */
	OMF_MC_AUDIO_CHANNELMODETYPE	eChannelMode;
	OMX_AUDIO_CHANNELMODETYPE		eEChannelMode;     /* use MP3-Enc */
	OMX_AUDIO_MP3STREAMFORMATTYPE	eFormat;
	OMX_AUDIO_AACPROFILETYPE		eAACProfile;
	OMX_AUDIO_AACSTREAMFORMATTYPE	eAACStreamFormat;
	OMF_MC_AUDIO_DUALMONOTYPE		eDualMonoMode;    /* use */
	OMF_MC_KCAPABLE_MODETYPE		eAC3KcapableMode;				/* use AC3-Dec */
	OMF_MC_COMPRESSION_MODETYPE		eAC3CompressionMode;			/* use AC3-Dec */
	OMF_MC_OUTPUT_MODETYPE			eAC3OutputMode;					/* use AC3-Dec */
	OMF_MC_STEREO_MODETYPE			eAC3StereoMode;					/* use AC3-Dec */
	OMX_BOOL						bAC3LfeMode;					/* use AC3-Dec */
	OMF_MC_DUALMONO_MODETYPE		eAC3DualMonoMode;				/* use AC3-Dec */
	OMX_U32							nAC3PcmScaleFactor;				/* use AC3-Dec */
	OMX_U32							nAC3DynamicRangeScaleFactorHi;	/* use AC3-Dec */
	OMX_U32							nAC3DynamicRangeScaleFactorLow;	/* use AC3-Dec */
	int								oneBuff;						/* use Decoder */
	int								eos;
	int								nextEos;
	int								nFilledLengthFlag;
	int			loadedDly;
	int			outbuffsize;
	int			n2DivBuffFlag;
	int			compBuff;
} AUDIODECFUNCPARAMTYPE;

typedef struct tagUTILITY_INFO {
	int AvFlag;
	int Mode;
#ifdef MULTI_PROCESS
	int ProcNo;
#endif
} UTILITY_INFO;

typedef struct _APP_DATA {
	char*        pInList;
	char*        pInJpeg;
	FILE*        fpList;
	unsigned int fileNumber;
	int          endFlag;
	int          optFlag;
	int          decFlag;
} APP_DATA ;

/***************************************************************************/
/*    extern                                                               */
/***************************************************************************/
extern BUFFERCTRLTYPE  *sBuffCtrl[2];
extern OMX_U32         nPortIndex[2];

extern FILE *pTrace;
extern sem_t eos_sem_id;

/********
* Audio *
********/
extern OMX_HANDLETYPE  Audio_hComponent;
extern FILE *AudioFp;

extern OMX_BUFFERHEADERTYPE* apsAudioBuffer[2][32];
extern OMX_U32 nAudioPortIndex[2];

extern pthread_mutex_t AudioMutex;
extern pthread_cond_t  AudioCond;

extern pthread_t AudioInputThreadId;
extern pthread_t AudioOutputThreadId;
extern pthread_t AudioOutputDataThreadId;

extern sem_t AudioCommandThreadStartSemId;
extern sem_t AudioCommandThreadEndSemId;
extern sem_t AudioInputSemId;
extern sem_t AudioOutputSemId;
extern sem_t AudioOutputDataSemId;

extern int   AudioInputPortThreadLoopFlag;
extern int   AudioOutputPortThreadLoopFlag;
extern int   AudioInputBuffNum;
extern int   AudioOutputBuffNum;
extern int   AudioInputBuffSize;
extern int   AudioOutputBuffSize;

extern void (*AudioInputPortThread)(void);
extern void (*AudioOutputPortThread)(void);
extern void (*AudioCodecReinit)(void);
extern void (*AudioOutputDataThread)(void);

/********
* Video *
********/
extern OMX_HANDLETYPE  Video_hComponent;
extern FILE *VideoFp;

extern OMX_BUFFERHEADERTYPE* apsVideoBuffer[2][32];
extern OMX_U32 nVideoPortIndex[2];

extern pthread_mutex_t VideoMutex;
extern pthread_cond_t  VideoCond;

extern pthread_t VideoInputThreadId;
extern pthread_t VideoOutputThreadId;
extern pthread_t VideoOutputDataThreadId;

extern sem_t VideoCommandThreadStartSemId;
extern sem_t VideoCommandThreadEndSemId;
extern sem_t VideoInputSemId;
extern sem_t VideoOutputSemId;
extern sem_t VideoOutputDataSemId;

extern int VideoInputPortThreadLoopFlag;
extern int VideoOutputPortThreadLoopFlag;
extern int VideoInputBuffNum;
extern int VideoOutputBuffNum;
extern int VideoInputBuffSize;

extern void (*VideoInputPortThread)(void);
extern void (*VideoOutputPortThread)(void);
extern void (*VideoOutputDataThread)(void);
extern void (*VideoCodecReinit)(void);

extern int RenderingStartFlag;
extern int RenderingInterval;
extern int EndOfStreamFlag;
extern int QueNum;
extern int DeQueNum;
extern int counter;
extern int prev_width;
extern int prev_height;
extern int divLengthType;

extern struct timeval sRenderingTimeInfo;
#ifdef MULTI_PROCESS
extern int pipefd[PIPE_FIND_MAX][2];
#endif

#ifdef EXTENDED_SEEK_METHOD
extern int nSpeedEx;
#endif


/****************************************************************************/
/*    Function prototypes (private)                                         */
/****************************************************************************/
/* Utility function. */
#ifdef MULTI_PROCESS
void InputCommandThreadMain(void *arg);
void InputCommandThreadSub(void *arg);
#else
void InputCommandThread(void *arg);
#endif
void EosThread(void *arg);

/* AAC (ADTS) parse function. */
int aac_analyze_header(FILE* fp, OMX_AUDIO_PARAM_AACPROFILETYPE* sAacParam);
int aac_get_1frame_data(FILE* fp, unsigned char* buff, unsigned long* frame_size, int* remain_size);

/* H.264 (RAW data format) parse function. */
int h264_analyze_header(FILE* fp);
int h264_get_frame_data(FILE* fp, unsigned char* buffer, unsigned long* size, int* remain_size, int *nFlags, int *start_code_prefix_size);

/* H.264 (PSU data format) parse function. */
int h264_psu_analyze_header(FILE* fp);
int h264_psu_get_frame_data(FILE* fp, unsigned char* buffer, unsigned long* size, int* remain_size);

/* VC1 (RCV V2m format) parse function. */
int  vc1_analyze_header(FILE* fp, int vc1es);
void vc1_set_max_buffer_size(unsigned long size);
int  vc1_get_frame_data(FILE* fp, unsigned char* buffer, unsigned long* size, int* remain_size, unsigned char* nflags, int vc1es);

/* MPEG4 (RAW data format) parse function. */
int mpeg4v_analyze_header(FILE* fp);
int mpeg4v_get_frame_data(FILE* fp, unsigned char* buffer, unsigned long* size, int* remain_size, unsigned char* nflags, int fillType);

/* MPEG2 (RAW data format) parse function. */
int mpeg2v_analyze_header(FILE* fp);
int mpeg2v_get_frame_data(FILE* fp, unsigned char* buffer, unsigned long* size, int* remain_size, unsigned char* nflags, int fillType);

/* Rendering timing control function. */
void AddTime(struct timeval *pTimeInfo, int addtime);
void AdjustTime(struct timeval *pTimeInfo);




/* AAC function */
void heaac_input_port_thread(void);
void heaac_output_port_thread(void);
void heaac_output_data_thread(void);
void heaac_reinit(void);

/* AC3 function */
void ac3_input_port_thread(void);
void ac3_output_port_thread(void);
void ac3_output_data_thread(void);
void ac3_reinit(void);

/* MPEG4 function*/
void mpeg4v_input_port_thread(void);
void mpeg4v_output_port_thread(void);
void mpeg4v_output_data_thread(void);
void mpeg4v_reinit(void);

/* MPEG2 function */
void mpeg2v_input_port_thread(void);
void mpeg2v_output_port_thread(void);
void mpeg2v_output_data_thread(void);
void mpeg2v_reinit(void);

/* H.264 function */
void h264_input_port_thread(void);
void h264_output_port_thread(void);
void h264_output_data_thread(void);
void h264_reinit(void);

/* JPEG function */
//void jpeg_input_port_thread(APP_DATA* pAppData);
//void jpeg_output_port_thread(APP_DATA* pAppData);
void jpeg_input_port_thread(void);
void jpeg_output_port_thread(void);
void jpeg_output_data_thread(void);
void jpeg_reinit(void);

/* MP3 function */
void mp3_input_port_thread(void);
void mp3_output_port_thread(void);
void mp3_output_data_thread(void);
void mp3_reinit(void);

/* VC1 function */
void vc1_input_port_thread(void);
void vc1_output_port_thread(void);
void vc1_output_data_thread(void);
void vc1_reinit(void);

/* WMA function */
void wma_input_port_thread(void);
void wma_output_port_thread(void);
void wma_output_data_thread(void);
void wma_reinit(void);


int semaphore_init(int AvFlag, int Mode);
int semaphore_deinit(int AvFlag, int Mode);
int CommandSemaphoreInit(void);
int CommandSemaphoreDeinit(void);

int SendDividedBuff(OMX_BUFFERHEADERTYPE* pBuffArr[], int maxCount, int*pIndex, int* counter, int n2DivBuffFlag);
void SetChannelCommand (OMF_MC_AUDIO_CHANNELMODETYPE channelMode);
void StopCommand (int AvFlag,int Mode);

void LoadedDly();//jian chun
extern void PRINT_TIME(char* msg);//peng

extern int OpenHDMI(void);//for hdmi test only
extern void CloseHDMI(void); //for hdmi test only
extern void SetRpeatON();
extern void SetRpeatOFF();

extern int check_video_frame_type(unsigned   char   *data);
extern int check_h264_frame_type(unsigned   char   *data);

/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
enum {
	i_frame = 0,
	p_frame,
	b_frame,
	d_frame,
	unknow_frame
};
// peng add --<


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
#endif /* __UTILITY_H__ */
