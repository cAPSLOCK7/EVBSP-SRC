
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <sys/types.h>                  /* for lstat() */
#include <sys/stat.h>                   /* ,,          */
#include <unistd.h>                     /* ,,          */
#include <ctype.h>                      /* for tolower() */
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <OMF_Extension.h>
#include "emxx_jpeglib.h"
#include "utility.h"

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define MAX_STR               256
#define FLAG_DEC_ERROR_DECODE 0x02

/* Progressive Jpeg Decode Settting */
#define IMAGE_MAX_STREAM_SIZE ((512*1024) * 2)

/****************************************************************************/
/*    Function prototypes (private)                                         */
/****************************************************************************/
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_EVENTTYPE eEvent, OMX_U32 u32Data1,
        OMX_U32 u32Data2, OMX_PTR pvEventData);
static OMX_ERRORTYPE empty_buffer_done(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_BUFFERHEADERTYPE* psBuffer);
static OMX_ERRORTYPE fill_buffer_done(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_BUFFERHEADERTYPE* psBuffer);
static void ParamInit (VIDEODECFUNCPARAMTYPE *VideoParam);
static int yuv_framesize(OMX_U32 width, OMX_U32 height, OMX_U32 ColorFormat);
static void FILE_DBG(char* pMsg);
static void FILE_DBG1(char* pMsg, int a)
{
    FILE_DBG(pMsg);
}

static void FILE_DBG2(char* pMsg, int a, int b)
{
    FILE_DBG(pMsg);
}
static void FILE_DBG_CLOSE(void);
/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/
static APP_DATA appData;
static int output_mode;
static OMX_U32 nFrameWidth, nFrameHeight;
static OMX_COLOR_FORMATTYPE eColorFormat;
static int jpegeLoop;
static char* yuvData = NULL;
static FILE*  jpgFile = NULL;
static int frameSize = 0;
int ef, ff;
/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
APP_DATA* g_pJPGAppData = &appData;
int g_IsJpegPlaying = 0;

int emxx_yuv_to_jpeg(unsigned char *srcYUVData, int width, int height, FILE *dstJPGfp, 
        int quality,int yuvType)
{
    VIDEODECFUNCPARAMTYPE  VideoParam;
    int ret;

    //Initialize
    ParamInit(&VideoParam);
    yuvData = NULL; //save yuv data
    jpgFile = NULL;

    yuvData = srcYUVData;
    jpgFile = dstJPGfp;

    switch (yuvType)
    {
        case OMF_MC_JPEGD_SAMPLING11:
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV444HV11;
            break;
        case OMF_MC_JPEGD_SAMPLING21:
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV422HV21;
            break;
        case OMF_MC_JPEGD_SAMPLING12:
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV422HV12;
            break;
        case OMF_MC_JPEGD_SAMPLING22:
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV411HV22;
            break;
        case OMF_MC_JPEGD_SAMPLING41:
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV411HV41;
            break;
        default:
            FILE_DBG("Invalid H:V Ratio, use HV22 instead");
            VideoParam.eColorFormat = (OMX_COLOR_FORMATTYPE)OMF_MC_COLOR_FormatYUV411HV22;
            break;

    }

    //width setting
    VideoParam.nFrameWidth = width;
    VideoParam.resize = 1;

    //height setting
    VideoParam.nFrameHeight = height;
    VideoParam.resize = 1;

    VideoParam.nQFactor = quality; 

    OMX_Init();
    printf("start encoding...\n");
    FILE_DBG("start encoding...");
    return jpege_main(&VideoParam);

}

int jpege_main(void *Param) {
    OMX_PARAM_PORTDEFINITIONTYPE asPortDefinition[2];
    OMX_PORT_PARAM_TYPE sImageInit;
    OMX_IMAGE_PARAM_PORTFORMATTYPE sImageParamPortformatType;
    OMX_CALLBACKTYPE sCallbacks;
    OMX_IMAGE_PARAM_QFACTORTYPE sImageParamQFacttype;
    int i = 0, j = 0;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_STATETYPE state = OMX_StateMax;
    OMX_U32 nQFactor = 0;
    OMX_S8 **cName;
    OMX_U32 numComp;
    char ConfigFileName[256];
    OMX_S8 componentName[256];
    OMX_VERSIONTYPE componentVer;
    OMX_VERSIONTYPE specVer;
    OMX_UUIDTYPE componentUUID;
    void *InputPortRet, *OutputPortRet, *OutputDataRet;

    memset(asPortDefinition, 0, sizeof(asPortDefinition));
    memset(&sImageInit, 0, sizeof(sImageInit));
    memset(&sImageParamPortformatType, 0, sizeof(sImageParamPortformatType));
    memset(&sCallbacks, 0, sizeof(sCallbacks));

    output_mode = ((VIDEODECFUNCPARAMTYPE *) Param)->Mode;
    nFrameWidth = ((VIDEODECFUNCPARAMTYPE *) Param)->nFrameWidth;
    nFrameHeight = ((VIDEODECFUNCPARAMTYPE *) Param)->nFrameHeight;
    eColorFormat = ((VIDEODECFUNCPARAMTYPE *) Param)->eColorFormat;
    nQFactor = ((VIDEODECFUNCPARAMTYPE *) Param)->nQFactor;
    int ret = 0, hasHandle = 0, commandValue = 0, hasNoWait = 1; /* */

    ef = ff = 0;

    if ((frameSize = yuv_framesize(nFrameWidth, nFrameHeight, eColorFormat)) <= 0) {
        FILE_DBG1("[emxx_jpeglib][%d] yuv_init\n", __LINE__);
        return -1;
    }

    strcpy(ConfigFileName, AV_CODEC_CONFIG_FILE_NAME);

    err = OMF_SetLogMode(0x00000001);
    if (err != OMX_ErrorNone)
    {
        FILE_DBG1("[emxx_jpeglib] Can't set log mode! error code = 0x%08x\n", err);
        goto jpege_end;
    }

    err = OMF_SetConfiguration(ConfigFileName);
    if (err != OMX_ErrorNone)
    {
        FILE_DBG1("[emxx_jpeglib] Can't set configuration file! error code = 0x%08x\n", err);
        goto jpege_end;
    }

    err = OMX_Init();

    if (err != OMX_ErrorNone) {
        FILE_DBG("can't initialize OMX LIb, exit\n");
        goto jpege_end;
    }

    /* Get MC Name */
    cName = (OMX_U8 **) malloc(sizeof(OMX_U8 *));
    cName[0] = (OMX_U8 *) malloc(128 * sizeof(OMX_U8));
    numComp = 1;
    err = OMX_GetComponentsOfRole(JPEGE_ROLE, &numComp, cName);
    if (err != OMX_ErrorNone) {
        FILE_DBG1("[emxx_jpeglib] Can't get component name! error code = 0x%8x\n", err);
    } else {
        FILE_DBG1("OMF Name = %s\n", cName[0]);
    }

    /* Get Media Component handle. */
    sCallbacks.EventHandler = event_handler;
    sCallbacks.EmptyBufferDone = empty_buffer_done;
    sCallbacks.FillBufferDone = fill_buffer_done;
    err = OMX_GetHandle(&Video_hComponent, cName[0], NULL, &sCallbacks);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't get handle! erro code = 0x%08x\n", __LINE__,
                err);
        free(cName[0]);
        free(cName);
        ret = -1;
        goto jpege_end;
    }
    hasHandle = 1;
    free(cName[0]);
    free(cName);

    /* GetComponentVersion */
    err = OMX_GetComponentVersion(Video_hComponent, componentName,
            &componentVer, &specVer, &componentUUID);
    if (err != OMX_ErrorNone) {
        FILE_DBG2(
                "[emxx_jpeglib][%d] Can't get component version! error code = 0x%08x\n",
                __LINE__, err);
        return -1;
    } else {
        printf("Component Version = 0x%08x\n", componentVer);
        printf("Spec      Version = 0x%08x\n", specVer);
    }

    /* Get OMX_IndexParamImageInit. */
    sImageInit.nSize = sizeof(OMX_PORT_PARAM_TYPE);
    err = OMX_GetParameter(Video_hComponent, OMX_IndexParamImageInit,
            &sImageInit);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't get parameter! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }

    /* Get & Set OMX_IndexParamPortDefinition. */
    for (i = 0; i < sImageInit.nPorts; i++) {
        asPortDefinition[i].nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
        asPortDefinition[i].nPortIndex = sImageInit.nStartPortNumber + i;
        err = OMX_GetParameter(Video_hComponent, OMX_IndexParamPortDefinition,
                &asPortDefinition[i]);
        if (err != OMX_ErrorNone) {
            FILE_DBG2("[emxx_jpeglib][%d] Can't get parameter! erro code = 0x%08x\n",
                    __LINE__, err);
            ret = -1;
            goto jpege_end;
        }

        if (i == PORT_OFFSET_INPUT) /* VPB+0 */
        {
            asPortDefinition[PORT_OFFSET_INPUT].format.image.nFrameWidth
                    = nFrameWidth;
            asPortDefinition[PORT_OFFSET_INPUT].format.image.nFrameHeight
                    = nFrameHeight;
            asPortDefinition[PORT_OFFSET_INPUT].format.image.eColorFormat
                    = eColorFormat;
            err = OMX_SetParameter(Video_hComponent,
                    OMX_IndexParamPortDefinition,
                    &asPortDefinition[PORT_OFFSET_INPUT]);
            if (err != OMX_ErrorNone) {
                extern int end_flag;
                printf(
                        "[emxx_jpeglib][%d] Can't set parameter! erro code = 0x%08x\n",
                        __LINE__, err);
                ret = -1;
                goto jpege_end;
            }
        }
    }

    nVideoPortIndex[PORT_OFFSET_INPUT]
            = asPortDefinition[PORT_OFFSET_INPUT].nPortIndex;
    nVideoPortIndex[PORT_OFFSET_OUTPUT]
            = asPortDefinition[PORT_OFFSET_OUTPUT].nPortIndex;

    VideoInputBuffNum = asPortDefinition[PORT_OFFSET_INPUT].nBufferCountActual;
    VideoOutputBuffNum
            = asPortDefinition[PORT_OFFSET_OUTPUT].nBufferCountActual;

    printf("VideoInputBuffNum, %d    VideoOutputBuffNum, %d\n", VideoInputBuffNum, VideoOutputBuffNum);
    /* Get & Set OMX_IndexParamImagePortFormat. */
    sImageParamPortformatType.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
    sImageParamPortformatType.nPortIndex = PORT_OFFSET_INPUT;
    sImageParamPortformatType.nIndex = nVideoPortIndex[PORT_OFFSET_INPUT];
    err = OMX_GetParameter(Video_hComponent, OMX_IndexParamImagePortFormat,
            &sImageParamPortformatType);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't get parameter! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }

    sImageParamPortformatType.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
    sImageParamPortformatType.eColorFormat = eColorFormat;
    err = OMX_SetParameter(Video_hComponent, OMX_IndexParamImagePortFormat,
            &sImageParamPortformatType);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't get parameter! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }
    /* Change state to OMX_StateIdle from OMX_StateLoaded. */
    pthread_mutex_lock(&VideoMutex);
    err = OMX_SendCommand(Video_hComponent, OMX_CommandStateSet, OMX_StateIdle,
            NULL);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't send command! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }

    /* Allocate stream buffer. */
    for (i = 0; i < sImageInit.nPorts; i++) {
        for (j = 0; j < asPortDefinition[i].nBufferCountActual; j++) {
            err = OMX_AllocateBuffer(Video_hComponent, &apsVideoBuffer[i][j],
                    asPortDefinition[i].nPortIndex, NULL,
                    asPortDefinition[i].nBufferSize);
            if (err != OMX_ErrorNone) {
                FILE_DBG2(
                        "[emxx_jpeglib][%d] Can't allocate buffer! erro code = 0x%08x\n",
                        __LINE__, err);
                ret = -1;
                goto jpege_end;
            }

            FILE_DBG2("[%d][%d] allocated\n", i, j);
        }
    }

    /* Complete state transition. */
    pthread_cond_wait(&VideoCond, &VideoMutex);
    pthread_mutex_unlock(&VideoMutex);

    semaphore_init(CODEC_VIDEO, output_mode);
#if 1 
    /* Create data input thread. */
    if (pthread_create(&VideoInputThreadId, NULL,
            (void*) jpeg_input_port_thread, NULL) != 0) {
        FILE_DBG1("[emxx_jpeglib][%d] failed pthread_create input_port_thread\n",
                __LINE__);
        ret = -1;
        goto jpege_end;
    }

    /* Create data output thread. */
    if (pthread_create(&VideoOutputThreadId, NULL,
            (void*) jpeg_output_port_thread, NULL) != 0) {
        FILE_DBG1("[emxx_jpeglib][%d] failed output_port_thread\n", __LINE__);
        ret = -1;
        goto jpege_end;
    }

    /* Create data output thread */
    if (pthread_create(&VideoOutputDataThreadId, NULL,
            (void*) jpeg_output_data_thread, NULL) != 0) {
        FILE_DBG1("[emxx_jpeglib][%d] pthread_create output_data_thread\n", __LINE__);
        ret = -1;
        goto jpege_end;
    }
#endif
    /* Change state to OMX_StateExecuting form OMX_StateIdle. */
    pthread_mutex_lock(&VideoMutex);
    err = OMX_SendCommand(Video_hComponent, OMX_CommandStateSet,
            OMX_StateExecuting, NULL);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't send command! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }

    /* Complete state transition. */
    pthread_cond_wait(&VideoCond, &VideoMutex);
    pthread_mutex_unlock(&VideoMutex);

    FILE_DBG("state has been changed to executing\n");
    VideoOutputPortThreadLoopFlag = 1;

    while (!ef || !ff)
    {
        printf(".");
    }

    FILE_DBG("start close OMX\n");
    hasNoWait = 0;

    VideoOutputPortThreadLoopFlag = 0;

    pthread_mutex_lock(&VideoMutex);
    err = OMX_SendCommand(Video_hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
        FILE_DBG("can't send command\n");
        ret = -1;
        goto jpege_end;
    }

    pthread_cond_wait(&VideoCond, &VideoMutex);
    pthread_mutex_unlock(&VideoMutex);

    /* Change state to OMX_StateLoaded from OMX_StateIdle. */
    pthread_mutex_lock(&VideoMutex);
    err = OMX_SendCommand(Video_hComponent, OMX_CommandStateSet,OMX_StateLoaded, NULL);
    if (err != OMX_ErrorNone) {
        FILE_DBG2("[emxx_jpeglib][%d] Can't send command! erro code = 0x%08x\n",
                __LINE__, err);
        ret = -1;
        goto jpege_end;
    }

    FILE_DBG("changed to loaded state\n");

    /* Free buffer. */
    for (i = 0; i < sImageInit.nPorts; i++) {
        for (j = 0; j < asPortDefinition[i].nBufferCountActual; j++) {
            err = OMX_FreeBuffer(Video_hComponent,
                    asPortDefinition[i].nPortIndex, apsVideoBuffer[i][j]);
            if (err != OMX_ErrorNone) {
                FILE_DBG2("[emxx_jpeglib][%d] Can't free buffer! erro code = 0x%08x\n",
                        __LINE__, err);
                ret = -1;
                goto jpege_end;
            }
        }
    }

    /* Complete state transition. */
    pthread_cond_wait(&VideoCond, &VideoMutex);
    pthread_mutex_unlock(&VideoMutex);

    jpege_end:
    FILE_DBG("free buffer\n");
    /* Free JPEG Media Component handle. */
    if (hasHandle) {
        err = OMX_FreeHandle(Video_hComponent);
        if (err != OMX_ErrorNone) {
            FILE_DBG2("[emxx_jpeglib][%d] Can't free handle! erro code = 0x%08x\n",
                    __LINE__, err);
            ret = -1;
        }
    }

    semaphore_deinit(CODEC_VIDEO, output_mode);
    FILE_DBG("close debug file");
    FILE_DBG_CLOSE();
    return 0;
}

/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_EVENTTYPE eEvent, OMX_U32 u32Data1,
        OMX_U32 u32Data2, OMX_PTR pvEventData) {
    OMX_ERRORTYPE err = OMX_ErrorNone;
    pthread_t EosThreadId = 0;
    static UTILITY_INFO stUtilityInfo;

    pthread_mutex_lock(&VideoMutex);

    switch (eEvent) {
    case OMX_EventCmdComplete:
        switch (u32Data1) {
        case OMX_CommandStateSet:
            FILE_DBG("state changed\n");
            pthread_cond_signal(&VideoCond);
            break;

        case OMX_CommandFlush: /* Fall through */
        case OMX_CommandPortDisable: /* Fall through */
        case OMX_CommandPortEnable: /* Fall through */
        case OMX_CommandMarkBuffer:
            printf("[emxx_jpeglib][%d]  %s : Un-supported event!\n", __LINE__,
                    __FUNCTION__);
            break;
        }
        break;

    case OMX_EventError:
        switch (u32Data1) {
        case OMX_ErrorUnderflow:
            printf("err under flow\n");
            break;

        default:
            if (output_mode != MODE_INPUT_ENCODER) {
                ((APP_DATA*) pvAppData)->decFlag = FLAG_DEC_ERROR_DECODE;
                VideoInputPortThreadLoopFlag = 0; // Add 10.10.28 KSK Suzuki
            }
            printf("[emxx_jpeglib][%d]  %s : OMX_EventError (0x%08lx)\n", __LINE__,
                    __FUNCTION__, u32Data1);
            break;
        }
        break;

    case OMX_EventBufferFlag:

        printf("buffer flag\n");
        break;

    case OMX_EventMark: /* Fall through */
    case OMX_EventPortSettingsChanged: /* Fall through */
    case OMX_EventResourcesAcquired: /* Fall through */
    default: /* Do nothing */
        printf("default msg\n");
        break;
    }

    pthread_mutex_unlock(&VideoMutex);

    return err;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE empty_buffer_done(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_BUFFERHEADERTYPE* psBuffer) {
    OMX_ERRORTYPE err = OMX_ErrorNone;

    printf("empty_buffer_done\n ");
    sem_post(&VideoInputSemId);

    ef = 1;
    return err;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static OMX_ERRORTYPE fill_buffer_done(OMX_HANDLETYPE Video_hComponent,
        OMX_PTR pvAppData, OMX_BUFFERHEADERTYPE* psBuffer) {
    OMX_ERRORTYPE err = OMX_ErrorNone;



    printf("fill this buffer done\n");

    sem_post(&VideoOutputDataSemId);

    return err;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void jpeg_input_port_thread(void) {
    OMX_BUFFERHEADERTYPE* psBuffer = NULL;
    struct stat sb;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int resAboutJpeg = 0;
    int counter = 0;
    int loopCounter = 0;
    char buff[MAX_STR];
    int i = 0;


    memset(&sb, 0, sizeof(sb));
    memset(buff, 0, sizeof(buff));
    
    sem_wait(&VideoInputSemId);
        psBuffer = apsVideoBuffer[0][i];

        if (psBuffer != NULL /*&& VideoOutputPortThreadLoopFlag == 1*/) {
            psBuffer->nFilledLen = 0;//yuvData
            memcpy(psBuffer->pBuffer, yuvData, frameSize);
            psBuffer->nFilledLen = frameSize;


            psBuffer->nFlags = OMX_BUFFERFLAG_EOS;

            if (psBuffer->nFilledLen ) {
                FILE_DBG("empty buffer\n");
                psBuffer->nTickCount = counter++;
                
                err = OMX_EmptyThisBuffer(Video_hComponent, psBuffer);
                if (err != OMX_ErrorNone) {
                    printf(
                            "[emxx_jpeglib][%d] OMX_EmptyThisBuffer erro code = 0x%08x\n",
                            __LINE__, err);
                }
            }
        }

    //pthread_exit(NULL);
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void jpeg_output_port_thread(void) {
    OMX_BUFFERHEADERTYPE* psBuffer = NULL;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int i = 0;
    psBuffer = apsVideoBuffer[1][i];
    sem_wait(&VideoOutputSemId);
   
     if(psBuffer != NULL ) {
        FILE_DBG("fill buffer\n");
        err = OMX_FillThisBuffer(Video_hComponent, psBuffer);
        if (err != OMX_ErrorNone) {
            printf(
                    "[emxx_jpeglib][%d] OMX_FillThisBuffer erro code = 0x%08x\n",
                    __LINE__, err);
        }

        i++;
        if (i >= VideoOutputBuffNum) {
            i = 0;
        }
    }
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void jpeg_output_data_thread(void) {
    OMF_MC_JPEGD_DECODERESULTTYPE* psJpegDec;
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_ERRORTYPE errv4l2 = OMX_ErrorNone;
    struct timeval renderingTime;

    OMX_U16* pDestY = NULL;
    OMX_U16* pDestUV = NULL;
    OMX_U16* pDestU = NULL, pDestV = NULL;
    OMX_U32 AddressY = 0, AddressCbCr = 0;
    int width = 0, height = 0;
    int i = 0;
    FILE *fp;

    memset(&psJpegDec, 0, sizeof(psJpegDec));
    memset(&renderingTime, 0, sizeof(renderingTime));

    APP_DATA* pAppInfo = &appData;
    sem_wait(&VideoOutputDataSemId);
    FILE_DBG("write into file...");
    fwrite(apsVideoBuffer[1][i]->pBuffer, 1, apsVideoBuffer[1][i]->nFilledLen, jpgFile);

    i++;
    if (i >= VideoOutputBuffNum) {
        i = 0;
    }
    ff = 1;
    return;

}


static void ParamInit (
    VIDEODECFUNCPARAMTYPE *VideoParam
)
{

    /* Set Video Param default value */
    VideoParam->Mode                            = MODE_OUTPUT_DECODER;
    VideoParam->FrameInterval              = 33333;
    VideoParam->resize                           = 0; /* Use resizer */
    VideoParam->rotate                            = 0; /* No rotate */
    VideoParam->nFrameWidth              = 176;
    VideoParam->nFrameHeight             = 144;
    VideoParam->nBitrate                       = 64000;
    VideoParam->xFramerate                 = 0x000F0000;//15fps
    VideoParam->nPFrames                   = 14;
    VideoParam->eProfile                       = OMX_VIDEO_AVCProfileBaseline;
    VideoParam->eLevel                         = OMX_VIDEO_AVCLevel1;
    VideoParam->bconstIpred                = OMX_FALSE;
    VideoParam->eLoopFilterMode      = OMX_VIDEO_AVCLoopFilterEnable;
    VideoParam->eColorFormat            = OMF_MC_COLOR_FormatYUV411HV22;
    VideoParam->eConvertColorFormat  = OMX_COLOR_FormatUnused;
    VideoParam->eDctDecodeMode       = OMF_MC_DCT_DecodeMode8x8;
    VideoParam->useFb                          = 0;
    VideoParam->outputMonitor              = 0;
    VideoParam->nQFactor                     = 75;
    VideoParam->ResizeWidth               = 0;
    VideoParam->ResizeHeight              = 0;
    VideoParam->CropTop                      = 0;
    VideoParam->CropLeft                      = 0;
    VideoParam->CropWidth                      = 32760;
    VideoParam->CropHeight                     = 32760;
    VideoParam->segNum                         = 1;    /* Not segment */
    VideoParam->segErr                         = 0;    /* No error */
    VideoParam->noSps                          = 0;    /* Input SPS/PPS Nul Unit first */
    VideoParam->fillType                       = 0;    /* Set SPS/PPS/DCI with picture data */
    VideoParam->frameSkip                      = 0;    /* V4L2 Normal */
    VideoParam->nextEos                        = 0;    /* Set EOS flag for last steam buffer */
    VideoParam->oneBuff                        = 0;    /* Use Full Buffer */
    VideoParam->n2DivBuffFlag                  = 0;
    VideoParam->h264NalMax                     = 1;
    VideoParam->nFilledLengthFlag              = 0;
    VideoParam->eoframe                        = 0;
    VideoParam->eos                            = 0;
    VideoParam->outOrder            =0;  /* default order */
    VideoParam->compBuff            =1;
    VideoParam->jpgeloop            =1;
    divLengthType                =0;
    VideoParam->dropdci            = 0; /*drop dci data flag*/
    VideoParam->nDataLossFlg        = 0; /* when set, the frame data will be lost while sending into omf inner */
    VideoParam->loadedDly            = 0;

    VideoParam->disableEOF            = 0;
    VideoParam->hdmi            = 0; //if set 1, enable display by hdmi
    VideoParam->saveYUV            = 0;
    VideoParam->halfman            = 0;
    VideoParam->fbdisp            = 0;


    VideoParam->pluFrame                       = 0; /* No plual frame input buffer for vc-1 */
    VideoParam->framErr                        = 0; /* No frame error for vc-1 */
    VideoParam->noSeq                          = 0; /* Have sequence in strear
    VideoParam->nFilledLen_1frame                     = 0; /* nFilledLen = 1 frame size */


    VideoParam->jpgeloop = 1;
    return;
}


int yuv_framesize(OMX_U32 width, OMX_U32 height, OMX_U32 ColorFormat)
{
    int Ysize, UVsize, Fsize = 0;

    switch (ColorFormat)
    {
        case OMF_MC_COLOR_FormatYUV411HV22:
        case OMF_MC_COLOR_FormatYUV411HV41:
            Ysize  = width*height;
            UVsize = width*height/2;
            Fsize  = Ysize+UVsize;

            break;

        case OMF_MC_COLOR_FormatYUV422HV21:
        case OMF_MC_COLOR_FormatYUV422HV12:
            Ysize  = width*height;
            UVsize = width*height;
            Fsize  = Ysize+UVsize;

            break;

        case OMF_MC_COLOR_FormatYUV444HV11:
            Ysize  = width*height ;
            UVsize = width*height*2 ;
            Fsize  = Ysize+UVsize;

            break;

        default:
            FILE_DBG("[emxx_jpeglib] Yuv color format");
            return -1;
            break;
    }
    FILE_DBG1("1FrameSize=%d[byte]\n",Fsize) ;

    return Fsize;
}

static FILE* pfile = NULL;
static int si = 0;
void FILE_DBG(char* pMsg)
{
    char buff[256] = {0};

    si++;
    sprintf(buff, "[%d]: %s \r\n", si, pMsg);


    printf("%s\n", pMsg);
    if (pfile == NULL)
    {
        pfile = fopen("/data/yuv2jpeg.log", "wb");
    }

    if (pfile != NULL)
    {
        fwrite(buff, 1, strlen(buff), pfile);
    }
}

void FILE_DBG_CLOSE(void)
{
    si = 0;
    if (pfile != NULL)
    {
        fclose(pfile);
    }
    pfile = NULL;
}

/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
