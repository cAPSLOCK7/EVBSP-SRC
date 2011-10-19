
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/select.h>
#include <unistd.h>
#include "utility.h"
#include <fcntl.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define FLAG_DEC_ERROR_INPUT  0x04
#define FLAG_DEC_DECODING     0x01
#define FLAG_DEC_LAST_DECODE  0x08
#define FLAG_END_OFF          0
#define FLAG_END_ON           1
#define FLAG_DEC_FINISH       0x10

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/****************************************************************************/
/*    Function prototypes (private)                                         */
/****************************************************************************/
static void DisplayInputCommand(void);
void StopCommand(int AvFlag, int Mode);
static void PlayPauseCommand(int AvFlag, int Mode);
static void RepeatCommand(void);
static void ChangeChannelCommand(int AvFlag, int Mode);
static void ChangeSpeedCommand(int AvFlag, int Mode);
static void GetAllStream(int AvFlag, int Mode);
static int  SendnDividedBuff(OMX_BUFFERHEADERTYPE* pBuffArr[], int maxCount, int*pIndex, int* counter, int n2DivBuffFlag);

/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
BUFFERCTRLTYPE  *sBuffCtrl[2];

sem_t eos_sem_id;
FILE *pTrace = NULL ;

pthread_mutex_t mutex_eos = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_eos  = PTHREAD_COND_INITIALIZER;

/********
* Audio *
********/
OMX_HANDLETYPE  Audio_hComponent;
OMX_U32         nAudioPortIndex[2];

FILE *AudioFp;

OMX_BUFFERHEADERTYPE* apsAudioBuffer[2][32];

pthread_mutex_t AudioMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  AudioCond  = PTHREAD_COND_INITIALIZER;

pthread_t AudioInputThreadId;
pthread_t AudioOutputThreadId;
pthread_t AudioOutputDataThreadId;

sem_t AudioCommandThreadStartSemId;
sem_t AudioCommandThreadEndSemId;
sem_t AudioInputSemId;
sem_t AudioOutputSemId;
sem_t AudioOutputDataSemId;


int AudioInputPortThreadLoopFlag  = 1;
int AudioOutputPortThreadLoopFlag = 1;
int AudioInputBuffNum;
int AudioOutputBuffNum;
int AudioInputBuffSize;
int AudioOutputBuffSize;

void (*AudioInputPortThread)(void);
void (*AudioOutputPortThread)(void);
void (*AudioCodecReinit)(void);
void (*AudioOutputDataThread)(void);

/********
* Video *
********/
OMX_HANDLETYPE  Video_hComponent;
OMX_U32         nVideoPortIndex[2];

FILE *VideoFp;

OMX_BUFFERHEADERTYPE* apsVideoBuffer[2][32];

pthread_mutex_t VideoMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  VideoCond  = PTHREAD_COND_INITIALIZER;

pthread_t VideoInputThreadId;
pthread_t VideoOutputThreadId;
pthread_t VideoOutputDataThreadId;

void	*inputRet, *outputRet, *outdataRet;

sem_t VideoCommandThreadStartSemId;
sem_t VideoCommandThreadEndSemId;
sem_t VideoInputSemId;
sem_t VideoOutputSemId;
sem_t VideoOutputDataSemId;

int VideoInputPortThreadLoopFlag  = 1;
int VideoOutputPortThreadLoopFlag = 1;
int VideoInputBuffNum;
int VideoOutputBuffNum;
int VideoInputBuffSize;

void (*VideoInputPortThread)(void);
void (*VideoOutputPortThread)(void);
void (*VideoOutputDataThread)(void);
void (*VideoCodecReinit)(void);

int RenderingStartFlag = 0;
int RenderingInterval  = 33333;
int EndOfStreamFlag    = 0;
int QueNum             = 1;
int DeQueNum           = 1;
int counter            = 0;
int prev_width         = 0;
int prev_height        = 0;
int divLengthType = 0;
int    end_flag = 1;

#ifdef EXTENDED_SEEK_METHOD
int nSpeedEx = 0;
#endif

#ifdef MULTI_PROCESS
int pipefd[PIPE_FIND_MAX][2];
#endif
struct timeval sRenderingTimeInfo;


void PRINT_TIME(char* msg);
static void CLOSE_PRINT_TIME();
/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/
static int RepertFlag = 0;

/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void AddTime(struct timeval *pTimeInfo, int addtime)
{
	pTimeInfo->tv_usec += addtime;

	if(1000000 <= pTimeInfo->tv_usec){
		pTimeInfo->tv_usec -= 1000000;
		pTimeInfo->tv_sec++;
	}
}


/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void AdjustTime(struct timeval *pTimeInfo)
{
	struct timeval        sCurrentTimeInfo;

	memset(&sCurrentTimeInfo, 0, sizeof(sCurrentTimeInfo));

	gettimeofday(&sCurrentTimeInfo, NULL);

	if(pTimeInfo->tv_sec > sCurrentTimeInfo.tv_sec)
	{
		return;
	}

	if(pTimeInfo->tv_usec > sCurrentTimeInfo.tv_usec)
	{
		return;
	}

	pTimeInfo->tv_usec = sCurrentTimeInfo.tv_usec;
	pTimeInfo->tv_sec  = sCurrentTimeInfo.tv_sec;
}


/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
int semaphore_init (
	int AvFlag,
	int Mode
)
{
	if (AvFlag == CODEC_AUDIO)
	{
		if ((sem_init(&AudioInputSemId,  0, AudioInputBuffNum)) == -1)
		{
			printf("[TP err] Error audio input sem_init\n");
			return -1;
		}

		if ((sem_init(&AudioOutputSemId, 0, AudioOutputBuffNum)) == -1)
		{
			printf("[TP err] Error audio output sem_init\n");
			return -1;
		}

		if ((sem_init(&AudioOutputDataSemId, 0, 0)) == -1)
		{
			printf("[TP err] Error audio output data sem_init\n");
			return -1;
		}
	}
	else if (AvFlag == CODEC_VIDEO)
	{
		if ((sem_init(&VideoInputSemId,  0, VideoInputBuffNum)) == -1)
		{
			printf("[TP err] Error video input sem_init\n");
			return -1;
		}

		if ((sem_init(&VideoOutputSemId, 0, VideoOutputBuffNum)) == -1)
		{
			printf("[TP err] Error video output sem_init\n");
			return -1;
		}

		if ((sem_init(&VideoOutputDataSemId, 0, 0)) == -1)
		{
			printf("[TP err] Error video output data sem_init\n");
			return -1;
		}
	}
	else
	{
		;
	}

	if (Mode == MODE_INPUT_ENCODER)
	{
		if ((sem_init(&eos_sem_id, 0, 0)) == -1)
		{
			printf("[TP err] Error sem_init\n");
			return -1;
		}
	}

	return 0;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
int semaphore_deinit (
	int AvFlag,
	int Mode
)
{
	if (AvFlag == CODEC_AUDIO)
	{
		if (sem_destroy(&AudioInputSemId) == -1)
		{
			printf("[TP err] Error audio input sem_deinit\n");
			return -1;
		}

		if (sem_destroy(&AudioOutputSemId) == -1)
		{
			printf("[TP err] Error audio output sem_deinit\n");
			return -1;
		}

		sem_post(&AudioOutputDataSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
		if (sem_destroy(&AudioOutputDataSemId) == -1)
		{
			printf("[TP err] Error audio output data sem_deinit\n");
			return -1;
		}
	}
	else if (AvFlag == CODEC_VIDEO)
	{
		if (sem_destroy(&VideoInputSemId) == -1)
		{
			printf("[TP err] Error video input sem_deinit\n");
			return -1;
		}

		if (sem_destroy(&VideoOutputSemId) == -1)
		{
			printf("[TP err] Error video output sem_deinit\n");
			return -1;
		}

		sem_post(&VideoOutputDataSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
		if (sem_destroy(&VideoOutputDataSemId) == -1)
		{
			printf("[TP err] Error video output data sem_deinit\n");
			return -1;
		}
	}
	else
	{
		;
	}

	if(Mode == MODE_INPUT_ENCODER)
	{
		sem_post(&eos_sem_id);	/* sem_destroy is fail because semVal is 0 (Android only) */
		if ((sem_destroy(&eos_sem_id)) == -1)
		{
			printf("[TP err] Error EOS sem_deinit\n");
			return -1;
		}
	}

	return 0;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
int CommandSemaphoreInit (
	void
)
{
	if ((sem_init(&AudioCommandThreadStartSemId,  0, 0)) == -1)
	{
		printf("[TP err] Error audio command start sem_init\n");
		return -1;
	}

	if ((sem_init(&AudioCommandThreadEndSemId, 0, 0)) == -1)
	{
		printf("[TP err] Error audio command end sem_init\n");
		return -1;
	}

	if ((sem_init(&VideoCommandThreadStartSemId,  0, 0)) == -1)
	{
		printf("[TP err] Error video command start sem_init\n");
		return -1;
	}

	if ((sem_init(&VideoCommandThreadEndSemId, 0, 0)) == -1)
	{
		printf("[TP err] Error video command end sem_init\n");
		return -1;
	}

	return 0;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
int CommandSemaphoreDeinit (
	void
)
{
	sem_post(&AudioCommandThreadStartSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
	if (sem_destroy(&AudioCommandThreadStartSemId) == -1)
	{
		printf("[TP err] Error audio command start sem_deinit\n");
	}

	sem_post(&AudioCommandThreadEndSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
	if (sem_destroy(&AudioCommandThreadEndSemId) == -1)
	{
		printf("[TP err] Error audio command end sem_deinit\n");
	}

	sem_post(&VideoCommandThreadStartSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
	if (sem_destroy(&VideoCommandThreadStartSemId) == -1)
	{
		printf("[TP err] Error video command start sem_deinit\n");
	}

	sem_post(&VideoCommandThreadEndSemId);	/* sem_destroy is fail because semVal is 0 (Android only) */
	if (sem_destroy(&VideoCommandThreadEndSemId) == -1)
	{
		printf("[TP err] Error video command end sem_deinit\n");
	}

	return 0;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void InputCommandThreadInit(void)
{
	AudioInputPortThreadLoopFlag  = 1;
	AudioOutputPortThreadLoopFlag = 1;
	VideoInputPortThreadLoopFlag  = 1;
	VideoOutputPortThreadLoopFlag = 1;


	return;
}

#ifndef MULTI_PROCESS
/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void InputCommandThread (
	void *arg
)
{
	char InputCommand[256];
	int  end_flag = 1;
	int  i = 0;
	int  AvFlag[2];
	int  Mode[2];

	memset(InputCommand, 0, sizeof(InputCommand));
	memset(AvFlag,       0, sizeof(AvFlag));
	memset(Mode,         0, sizeof(Mode));

	InputCommandThreadInit();

	for (i=0; i<2; i++)
	{
		AvFlag[i] = (((UTILITY_INFO*)arg)[i]).AvFlag;
		Mode[i]   = (((UTILITY_INFO*)arg)[i]).Mode;
	}

	if ((AvFlag[0] == CODEC_AUDIO) || (AvFlag[1] == CODEC_AUDIO))
	{
		sem_wait(&AudioCommandThreadStartSemId);
	}

	if ((AvFlag[0] == CODEC_VIDEO) || (AvFlag[1] == CODEC_VIDEO))
	{
		sem_wait(&VideoCommandThreadStartSemId);
	}

	while (end_flag)
	{
#ifdef BACKGROUND_RUN_VER
		continue;
#endif
		DisplayInputCommand();
		fgets(InputCommand, 255, stdin);

		switch (InputCommand[0])
		{
			/* Stop */
			case 's':
				StopCommand(AvFlag[0], Mode[0]);
				StopCommand(AvFlag[1], Mode[1]);
				break;

			/* Play/Pause */
			case 'p':
				PlayPauseCommand(AvFlag[0], Mode[0]);
				PlayPauseCommand(AvFlag[1], Mode[1]);
				break;

			/* Repeat */
			case 'r':
				RepeatCommand();
				break;

			/* Change channel */
			case 'c':
				ChangeChannelCommand(AvFlag[0], Mode[0]);
				ChangeChannelCommand(AvFlag[1], Mode[1]);
				break;

			/* Change speed */
			case 't':
				ChangeSpeedCommand(AvFlag[0], Mode[0]);
#ifndef EXTENDED_SEEK_METHOD
				ChangeSpeedCommand(AvFlag[1], Mode[1]);
#endif
				break;

			/* Quit */
			case 'q':
				StopCommand(AvFlag[0], Mode[0]);
				StopCommand(AvFlag[1], Mode[1]);
				semaphore_deinit(AvFlag[0], Mode[0]);
				semaphore_deinit(AvFlag[1], Mode[1]);
				end_flag = 0;
				break;

			default:
				/* Nothing */
				break;
		}
	}

	if ((AvFlag[0] == CODEC_AUDIO) || (AvFlag[1] == CODEC_AUDIO))
	{
		sem_post(&AudioCommandThreadEndSemId);
	}

	if ((AvFlag[0] == CODEC_VIDEO) || (AvFlag[1] == CODEC_VIDEO))
	{
		sem_post(&VideoCommandThreadEndSemId);
	}

	pthread_exit(NULL);
}
#else
/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void InputCommandThreadMain (
	void *arg
)
{
	char   InputCommand[256];
	char   SndCommand[256];
	char   RcvCommand[256];
	int    i = 0;
	int    AvFlag[2];
	int    Mode[2];
	fd_set rfds;
	int    fd = 0;

	memset(InputCommand, 0, sizeof(InputCommand));
	memset(SndCommand,   0, sizeof(SndCommand));
	memset(RcvCommand,   0, sizeof(RcvCommand));
	memset(AvFlag,       0, sizeof(AvFlag));
	memset(Mode,         0, sizeof(Mode));

	for (i=0; i<2; i++)
	{
		AvFlag[i] = (((UTILITY_INFO*)arg)[i]).AvFlag;
		Mode[i]   = (((UTILITY_INFO*)arg)[i]).Mode;
	}

	while (end_flag)
	{
		DisplayInputCommand();
		fgets(InputCommand, 255, stdin);

		memset(SndCommand,   0, sizeof(SndCommand));

		switch (InputCommand[0])
		{
			/* Stop */
			case 's':
				SndCommand[0] = 's';
				break;

			/* Play/Pause */
			case 'p':
				SndCommand[0] = 'p';
				break;

			/* Repeat */
			case 'r':
				SndCommand[0] = 'r';
				break;

			/* Change channel */
			case 'c':
				SndCommand[0] = 'c';
				break;

			/* Change speed */
			case 't':
				SndCommand[0] = 't';
				break;

			/* Quit */
			case 'q':
				SndCommand[0] = 'q';
				end_flag = 0;
				break;

			default:
				/* Nothing */
				break;
		}

		if ((AvFlag[0] == CODEC_AUDIO) || (AvFlag[0] == CODEC_VIDEO))
		{
			write(pipefd[PIPE0_MAIN2SUB][1], SndCommand, 1);
			read(pipefd[PIPE0_SUB2MAIN][0],  RcvCommand, 1);
		}
		if ((AvFlag[1] == CODEC_AUDIO) || (AvFlag[1] == CODEC_VIDEO))
		{
			write(pipefd[PIPE1_MAIN2SUB][1], SndCommand, 1);
			read(pipefd[PIPE1_SUB2MAIN][0],  RcvCommand, 1);
		}
	}
	CLOSE_PRINT_TIME();//peng for debug msg
	pthread_exit(NULL);
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void InputCommandThreadSub (
	void *arg
)
{
	char RcvCommand[256];
	char SndCommand[256];
	int  end_flag = 1;
	int  i        = 0;
	int  AvFlag   = 0;
	int  Mode     = 0;
	int  ProcNo   = 0;
	PIPE_FIND PipeMain2Sub = PIPE_FIND_MAX;
	PIPE_FIND PipeSub2Main = PIPE_FIND_MAX;
	fd_set    rfds;
	int       fd = 0;

	memset(RcvCommand, 0, sizeof(RcvCommand));
	memset(SndCommand, 0, sizeof(SndCommand));

	AvFlag = ((UTILITY_INFO*)arg)->AvFlag;
	Mode   = ((UTILITY_INFO*)arg)->Mode;
	ProcNo = ((UTILITY_INFO*)arg)->ProcNo;

	if (AvFlag == CODEC_AUDIO)
	{
		sem_wait(&AudioCommandThreadStartSemId);
	}
	else if (AvFlag == CODEC_VIDEO)
	{
		sem_wait(&VideoCommandThreadStartSemId);
	}
	else
	{
		/* Nothing */
		printf("[TP err] invalid AvFlag\n");
		pthread_exit(NULL);
	}

	if (ProcNo == 0)
	{
		PipeMain2Sub = PIPE0_MAIN2SUB;
		PipeSub2Main = PIPE0_SUB2MAIN;
	}
	else if (ProcNo == 1)
	{
		PipeMain2Sub = PIPE1_MAIN2SUB;
		PipeSub2Main = PIPE1_SUB2MAIN;
	}
	else
	{
		printf("[TP err] invalid ProcNo\n");
		pthread_exit(NULL);
	}

	while (end_flag)
	{
		read(pipefd[PipeMain2Sub][0], RcvCommand, 1);

		switch (RcvCommand[0])
		{
			/* Stop */
			case 's':
				StopCommand(AvFlag, Mode);
				break;

			/* Play/Pause */
			case 'p':
				PlayPauseCommand(AvFlag, Mode);
				break;

			/* Repeat */
			case 'r':
				RepeatCommand();
				break;

			/* Change channel */
			case 'c':
				ChangeChannelCommand(AvFlag, Mode);
				break;

			/* Change speed */
			case 't':
				ChangeSpeedCommand(AvFlag, Mode);
				break;

			/* Quit */
			case 'q':
				StopCommand(AvFlag, Mode);
				semaphore_deinit(AvFlag, Mode);
				end_flag = 0;
				break;

			default:
				/* Nothing */
				break;
		}

		write(pipefd[PipeSub2Main][1], SndCommand, 1);
	}

	if (AvFlag == CODEC_AUDIO)
	{
		sem_post(&AudioCommandThreadEndSemId);
	}
	else if (AvFlag == CODEC_VIDEO)
	{
		sem_post(&VideoCommandThreadEndSemId);
	}

	pthread_exit(NULL);
}
#endif

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void EosThread (
	void *arg
)
{
	int AvFlag = 0;
	int Mode   = 0;

	pthread_mutex_lock(&mutex_eos);

	AvFlag = ((UTILITY_INFO*)arg)->AvFlag;
	Mode   = ((UTILITY_INFO*)arg)->Mode;

	if(Mode != MODE_INPUT_ENCODER)
	{
		StopCommand(AvFlag, Mode);
	}
	else
	{
		GetAllStream(AvFlag, Mode);
		sem_wait(&eos_sem_id);
		printf("\n --- Encode Complete. ---\n");
		if (semaphore_deinit(AvFlag, Mode) != 0)
		{
			printf("[TP err] semaphore_deinit\n");
		}
		if (semaphore_init(AvFlag, Mode) != 0)
		{
			printf("[TP err] semaphore_init\n");
		}
		EndOfStreamFlag = 0;
		pthread_cond_signal(&cond_eos);
	}

	if (RepertFlag == 1)
	{
		PlayPauseCommand(AvFlag, Mode);
	}

	if (AvFlag == CODEC_AUDIO)
	{
		PRINT_TIME("-- Audio Complete --");
		printf(" -- Audio Complete --\n");
	}
	else if (AvFlag == CODEC_VIDEO)
	{
		PRINT_TIME("-- Video Complete --");
		printf(" -- Video Complete --\n");
	}
	else
	{
		printf("[TP err] AvFlag Error\n");
	}

	pthread_mutex_unlock(&mutex_eos);

	pthread_exit(NULL);
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
static void DisplayInputCommand (
	void
)
{
	printf("\n");
	printf("Please input command\n");
	printf(" s:stop\n");
	printf(" p:play/pause\n");
	printf(" r:repert\n");
	printf(" c:change channel\n");
	printf(" t:change play speed\n");
	printf(" q:quit\n");

	return;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
void StopCommand (
	int AvFlag,
	int Mode
)
{
	OMX_ERRORTYPE eErr   = OMX_ErrorNone;
	OMX_STATETYPE eState = OMX_StateMax;
	void                 *InputThreadRet = NULL, *OutputThreadRet = NULL, *OutputDataThreadRet = NULL;
	int InputVal = 0, OutputVal = 0, OutputDataVal = 0;


	OMX_HANDLETYPE  hComponent          = NULL;
	pthread_mutex_t *mutex              = NULL;
	pthread_cond_t  *cond               = NULL;
	pthread_t       *InputThreadId      = NULL;
	pthread_t       *OutputThreadId     = NULL;
	pthread_t       *OutputDataThreadId = NULL;
	sem_t           *InputSemId         = NULL;
	sem_t           *OutputSemId        = NULL;
	sem_t           *OutputDataSemId    = NULL;
	int             InputBuffNum        = 0;
	int             OutputBuffNum       = 0;

	/* Audio */
	if (AvFlag == CODEC_AUDIO)
	{
		hComponent         = Audio_hComponent;
		mutex              = &AudioMutex;
		cond               = &AudioCond;
		InputThreadId      = &AudioInputThreadId;
		OutputThreadId     = &AudioOutputThreadId;
		OutputDataThreadId = &AudioOutputDataThreadId;
		InputSemId         = &AudioInputSemId;
		OutputSemId        = &AudioOutputSemId;
		OutputDataSemId    = &AudioOutputDataSemId;
		InputBuffNum       = AudioInputBuffNum;
		OutputBuffNum      = AudioOutputBuffNum;
	}
	/* Video(Image) */
	else if (AvFlag == CODEC_VIDEO)
	{
		hComponent         = Video_hComponent;
		mutex              = &VideoMutex;
		cond               = &VideoCond;
		InputThreadId      = &VideoInputThreadId;
		OutputThreadId     = &VideoOutputThreadId;
		OutputDataThreadId = &VideoOutputDataThreadId;
		InputSemId         = &VideoInputSemId;
		OutputSemId        = &VideoOutputSemId;
		OutputDataSemId    = &VideoOutputDataSemId;
		InputBuffNum       = VideoInputBuffNum;
		OutputBuffNum      = VideoOutputBuffNum;
	}
	/* Othrer */
	else
	{
		goto EXIT;
	}

	/* mutex lock */
	pthread_mutex_lock(mutex);

	eErr = OMX_GetState(hComponent, &eState);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] Error OMX_GetState\n");
	}

	/* Stop (Executing or Pause -> Idle) */
	if ((eErr == OMX_ErrorNone) && ((eState == OMX_StateExecuting) || (eState == OMX_StatePause)))
	{
		if (AvFlag == CODEC_AUDIO)
		{
			AudioInputPortThreadLoopFlag  = 0;
			AudioOutputPortThreadLoopFlag = 0;
		}
		else
		{
			VideoInputPortThreadLoopFlag  = 0;
			VideoOutputPortThreadLoopFlag = 0;
		}

		sem_getvalue(InputSemId, &InputVal);
		if (InputVal <= 0)
		{
			sem_post(InputSemId);
		}

		sem_getvalue(OutputSemId, &OutputVal);
		if (OutputVal <= 0)
		{
			sem_post(OutputSemId);
		}

		sem_getvalue(OutputDataSemId, &OutputDataVal);
		if (OutputDataVal <= 0)
		{
			sem_post(OutputDataSemId);
		}

		/* Wait thread finish */
		if (pthread_join(*InputThreadId, &InputThreadRet) != 0)
		{
			printf("[TP err] input pthread_join\n");
		}

		if (pthread_join(*OutputThreadId, &OutputThreadRet) != 0)
		{
			printf("[TP err] output pthread_join\n");
		}

		if (pthread_join(*OutputDataThreadId, &OutputDataThreadRet) != 0)
		{
			printf("[TP err] output data pthread_join\n");
		}

		eErr = OMX_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
		if (eErr != OMX_ErrorNone)
		{
			printf("[TP err] Error OMX_SendCommand\n");
		}
		pthread_cond_wait(cond, mutex);


		if (semaphore_deinit(AvFlag, Mode) != 0)
		{
			printf("[TP err] semaphore_deinit\n");
		}
		if (semaphore_init(AvFlag, Mode) != 0)
		{
			printf("[TP err] semaphore_init\n");
		}

		if (AvFlag == CODEC_AUDIO)
		{
			AudioCodecReinit();
		}
		else
		{
			VideoCodecReinit();
		}
	}
	else
	{
		/* Nothing */
	}

	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:
	return;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static void GetAllStream(
	int AvFlag,
	int Mode
)
{
	OMX_ERRORTYPE eErr   = OMX_ErrorNone;
	OMX_STATETYPE eState = OMX_StateMax;
	void                 *InputThreadRet      = NULL;
	void                 *OutputThreadRet     = NULL;
	void                 *OutputDataThreadRet = NULL;
	int                   InputVal      = 0;
	int                   OutputVal     = 0;
	int                   OutputDataVal = 0;

	OMX_HANDLETYPE  hComponent           = NULL;
	pthread_mutex_t *mutex               = NULL;
	pthread_cond_t  *cond                = NULL;
	pthread_t       *InputThreadId       = NULL;
	pthread_t       *OutputThreadId      = NULL;
	pthread_t       *OutputDataThreadId  = NULL;
	sem_t           *InputSemId          = NULL;
	sem_t           *OutputSemId         = NULL;
	sem_t           *OutputDataSemId     = NULL;
	int             InputBuffNum         = 0;

	/* Audio */
	if (AvFlag == CODEC_AUDIO)
	{
		hComponent         = Audio_hComponent;
		mutex              = &AudioMutex;
		cond               = &AudioCond;
		InputThreadId      = &AudioInputThreadId;
		OutputThreadId     = &AudioOutputThreadId;
		OutputDataThreadId = &AudioOutputDataThreadId;
		InputSemId         = &AudioInputSemId;
		OutputSemId        = &AudioOutputSemId;
		OutputDataSemId    = &AudioOutputDataSemId;
		InputBuffNum       = AudioInputBuffNum;
	}
	/* Video(Image) */
	else if (AvFlag == CODEC_VIDEO)
	{
		hComponent         = Video_hComponent;
		mutex              = &VideoMutex;
		cond               = &VideoCond;
		InputThreadId      = &VideoInputThreadId;
		OutputThreadId     = &VideoOutputThreadId;
		OutputDataThreadId = &VideoOutputDataThreadId;
		InputSemId         = &VideoInputSemId;
		OutputSemId        = &VideoOutputSemId;
		OutputDataSemId    = &VideoOutputDataSemId;
		InputBuffNum       = VideoInputBuffNum;
	}
	/* Othrer */
	else
	{
		goto EXIT;
	}


	/* mutex lock */
	pthread_mutex_lock(mutex);

	eErr = OMX_GetState(hComponent, &eState);

	if ((eErr == OMX_ErrorNone) && ((eState == OMX_StateExecuting) || (eState == OMX_StatePause)))
	{
		/* Wait thread finish */
		if (pthread_join(*InputThreadId, &InputThreadRet) != 0) {
			printf("[TP err] input pthread_join\n");
		}

		if (pthread_join(*OutputThreadId, &OutputThreadRet) != 0) {
			printf("[TP err] output pthread_join\n");
		}

		if (pthread_join(*OutputDataThreadId, &OutputDataThreadRet) != 0) {
			printf("[TP err] output data pthread_join\n");
		}

		sem_getvalue(InputSemId,  &InputVal);
		sem_getvalue(OutputSemId, &OutputVal);
		sem_getvalue(OutputDataSemId, &OutputDataVal);

		eErr = OMX_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
		if (eErr != OMX_ErrorNone)
		{
			printf("[TP err] Error OMX_SendCommand\n");
		}
		pthread_cond_wait(cond, mutex);


		if (AvFlag == CODEC_AUDIO)
		{
			AudioCodecReinit();
		}
		else
		{
			VideoCodecReinit();
		}
	}
	else {
		/* Nothing */
	}
	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:
	return ;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static void PlayPauseCommand (
	int AvFlag,
	int Mode
)
{
	OMX_ERRORTYPE eErr   = OMX_ErrorNone;
	OMX_STATETYPE eState = OMX_StateMax;

	OMX_HANDLETYPE  hComponent      = NULL;
	pthread_mutex_t *mutex          = NULL;
	pthread_cond_t  *cond           = NULL;
	pthread_t       *InputThreadId  = NULL;
	pthread_t       *OutputThreadId = NULL;

	/* Audio */
	if (AvFlag == CODEC_AUDIO)
	{
		hComponent     = Audio_hComponent;
		mutex          = &AudioMutex;
		cond           = &AudioCond;
		InputThreadId  = &AudioInputThreadId;
		OutputThreadId = &AudioOutputThreadId;
	}
	/* Video(Image) */
	else if (AvFlag == CODEC_VIDEO)
	{
		hComponent     = Video_hComponent;
		mutex          = &VideoMutex;
		cond           = &VideoCond;
		InputThreadId  = &VideoInputThreadId;
		OutputThreadId = &VideoOutputThreadId;
	}
	/* Othrer */
	else
	{
		goto EXIT;
	}


	/* mutex lock */
	pthread_mutex_lock(mutex);

	eErr = OMX_GetState(hComponent, &eState);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] Error OMX_GetState\n");
	}

	/* Pause (Executing -> Pause) */
	if ((eErr == OMX_ErrorNone) && (eState == OMX_StateExecuting))
	{
		eErr = OMX_SendCommand(hComponent, OMX_CommandStateSet, OMX_StatePause, NULL);
		if (eErr != OMX_ErrorNone)
		{
			printf("[TP err] Error OMX_SendCommand\n");
		}
		pthread_cond_wait(cond, mutex);
	}
	/* Play (Pause -> Executing) */
	else if ((eErr == OMX_ErrorNone) && (eState == OMX_StatePause))
	{
		if (AvFlag == CODEC_VIDEO)
		{
			/* Get start time */
			gettimeofday(&sRenderingTimeInfo, NULL);
			AddTime(&sRenderingTimeInfo, 500*1000);
		}

		eErr = OMX_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		if (eErr != OMX_ErrorNone)
		{
			printf("[TP err] Error OMX_SendCommand\n");
		}
		pthread_cond_wait(cond, mutex);
	}
	/* Play (Idle -> Executing) */
	else if ((eErr == OMX_ErrorNone) && (eState == OMX_StateIdle))
	{
		if (AvFlag == CODEC_AUDIO)
		{
			AudioInputPortThreadLoopFlag  = 1;
			AudioOutputPortThreadLoopFlag = 1;

			if (pthread_create(InputThreadId,  NULL, (void*)AudioInputPortThread,  NULL) != 0)
			{
				printf("[TP err] pthread_create AudioInputPortThread\n");
			}

			if (pthread_create(OutputThreadId, NULL, (void*)AudioOutputPortThread, NULL) != 0)
			{
				printf("[TP err] pthread_create AudioOutputPortThread\n");
			}
			if (pthread_create(&AudioOutputDataThreadId, NULL, (void*)AudioOutputDataThread, NULL) != 0)
			{
				printf("[TP err] pthread_create AudioOutputDataThread\n");
			}
		}
		else
		{
			VideoInputPortThreadLoopFlag  = 1;
			VideoOutputPortThreadLoopFlag = 1;

			if (pthread_create(InputThreadId,  NULL, (void*)VideoInputPortThread,  NULL) != 0)
			{
				printf("[TP err] pthread_create VideoInputPortThread\n");
			}

			if (pthread_create(OutputThreadId, NULL, (void*)VideoOutputPortThread, NULL) != 0)
			{
				printf("[TP err] pthread_create VideoOutputPortThread\n");
			}
			if (pthread_create(&VideoOutputDataThreadId, NULL, (void*)VideoOutputDataThread, NULL) != 0)
			{
				printf("[TP err] pthread_create VideoOutputDataThread\n");
			}

			gettimeofday(&sRenderingTimeInfo, NULL);
			AddTime(&sRenderingTimeInfo, 500*1000);
		}

		eErr = OMX_SendCommand(hComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		if (eErr != OMX_ErrorNone)
		{
			printf("[TP err] Error OMX_SendCommand\n");
		}
		pthread_cond_wait(cond, mutex);
	}
	else
	{
		/* Nothing */
	}

	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:

	return;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static void RepeatCommand (
	void
)
{
	if (RepertFlag == 0)
	{
		RepertFlag = 1;
		printf("Repeat ON\n");
	}
	else
	{
		RepertFlag = 0;
		printf("Repeat OFF\n");
	}

	return;
}

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static void ChangeChannelCommand (
	int AvFlag,
	int Mode
)
{
	OMX_ERRORTYPE                   eErr = OMX_ErrorNone;
	OMF_MC_AUDIO_CONFIG_PCMMODETYPE sConfig;
	char InputCommandChannelMode[256];
	char InputCommandDualMono[256];

	OMX_HANDLETYPE  hComponent = NULL;
	pthread_mutex_t *mutex     = NULL;
	pthread_cond_t  *cond      = NULL;

	memset(&sConfig,                0, sizeof(sConfig));
	memset(InputCommandChannelMode, 0, sizeof(InputCommandChannelMode));
	memset(InputCommandDualMono,    0, sizeof(InputCommandDualMono));

	if (AvFlag != CODEC_AUDIO)
	{
			goto EXIT;
	}

	/* Audio Only */
	hComponent = Audio_hComponent;
	mutex      = &AudioMutex;
	cond       = &AudioCond;

	/* mutex lock */
	pthread_mutex_lock(mutex);

	sConfig.nSize      = sizeof(OMF_MC_AUDIO_CONFIG_PCMMODETYPE);
	sConfig.nPortIndex = nAudioPortIndex[1];
	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_GetConfig\n");
	}

	printf("\n");
	printf("Select channel mode\n");
	printf(" 0:Mono\n");
	printf(" 1:Stereo\n");
	printf(" 2:MonoR\n");
	printf(" 3:MonoL\n");
	printf(" 4:Not change\n");
	fgets(InputCommandChannelMode, 255, stdin);

	switch (InputCommandChannelMode[0])
	{
		case '0':
			sConfig.eChannelMode = OMF_MC_AUDIO_ChannelModeMono;
			break;

		case '1':
			sConfig.eChannelMode = OMF_MC_AUDIO_ChannelModeStereo;
			break;

		case '2':
			sConfig.eChannelMode = OMF_MC_AUDIO_ChannelModeMonoR;
			break;

		case '3':
			sConfig.eChannelMode = OMF_MC_AUDIO_ChannelModeMonoL;
			break;

		default:
			/* Nothing */
			break;
	}

	printf("\n");
	printf("Select DualMono\n");
	printf(" 0:Main\n");
	printf(" 1:Sub\n");
	printf(" 2:MainSub\n");
	printf(" 3:Not change\n");
	fgets(InputCommandDualMono, 255, stdin);

	switch (InputCommandDualMono[0])
	{
		case '0':
			sConfig.eDualMonoMode = OMF_MC_AUDIO_DualMonoModeMain;
			break;

		case '1':
			sConfig.eDualMonoMode = OMF_MC_AUDIO_DualMonoModeSub;
			break;

		case '2':
			sConfig.eDualMonoMode = OMF_MC_AUDIO_DualMonoModeMainSub;
			break;

		default:
			/* Nothing */
			break;
	}

	sConfig.nPortIndex = nAudioPortIndex[1];

	eErr = OMX_SetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	printf("sConfig.eChannelMode  = %d\n", sConfig.eChannelMode);
	printf("sConfig.eDualMonoMode = %d\n", sConfig.eDualMonoMode - 1);

	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:
	return;
}

void SetChannelCommand (OMF_MC_AUDIO_CHANNELMODETYPE channelMode)
{
	OMX_ERRORTYPE                   eErr = OMX_ErrorNone;
	OMF_MC_AUDIO_CONFIG_PCMMODETYPE sConfig;
	char InputCommandChannelMode[256];
	char InputCommandDualMono[256];

	OMX_HANDLETYPE  hComponent = NULL;
	pthread_mutex_t *mutex     = NULL;
	pthread_cond_t  *cond      = NULL;

	memset(&sConfig,                0, sizeof(sConfig));
	memset(InputCommandChannelMode, 0, sizeof(InputCommandChannelMode));
	memset(InputCommandDualMono,    0, sizeof(InputCommandDualMono));



	/* Audio Only */
	hComponent = Audio_hComponent;
	mutex      = &AudioMutex;
	cond       = &AudioCond;

	/* mutex lock */
	pthread_mutex_lock(mutex);

	sConfig.nSize      = sizeof(OMF_MC_AUDIO_CONFIG_PCMMODETYPE);
	sConfig.nPortIndex = nAudioPortIndex[1];
	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_GetConfig\n");
	}

	sConfig.eChannelMode = channelMode;
	

	sConfig.nPortIndex = nAudioPortIndex[1];

	eErr = OMX_SetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	printf("sConfig.eChannelMode  = %d\n", sConfig.eChannelMode);
	printf("sConfig.eDualMonoMode = %d\n", sConfig.eDualMonoMode - 1);

	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:
	return;
}



#ifdef EXTENDED_SEEK_METHOD
static void ChangeSpeedCommandEX(int speed)
{
	nSpeedEx = speed;
	printf("discard %d in %d frame(s) every time\n", nSpeedEx, nSpeedEx+1);
}
#endif

/***************************************************************************
 * SUMMARY:
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static void ChangeSpeedCommand (
	int AvFlag,
	int Mode
)
{
	OMX_ERRORTYPE                   eErr = OMX_ErrorNone;
	OMF_MC_AUDIO_CONFIG_PCMMODETYPE sConfig;
	char InputCommandPlaySpeed[256];
	int  iPlaySpeed;

	OMX_HANDLETYPE  hComponent = NULL;
	pthread_mutex_t *mutex     = NULL;
	pthread_cond_t  *cond      = NULL;

#ifdef EXTENDED_SEEK_METHOD
	printf("please input frames to seek every time\n");
	scanf("%d", &iPlaySpeed);
	ChangeSpeedCommandEX(iPlaySpeed);
	return ;
#endif

	memset(&sConfig,                0, sizeof(sConfig));
	memset(InputCommandPlaySpeed,   0, sizeof(InputCommandPlaySpeed));

	if (AvFlag != CODEC_AUDIO)
	{
		goto EXIT;
	}

	/* Audio Only */
	hComponent = Audio_hComponent;
	mutex      = &AudioMutex;
	cond       = &AudioCond;

	/* mutex lock */
	pthread_mutex_lock(mutex);

	sConfig.nSize      = sizeof(OMF_MC_AUDIO_CONFIG_PCMMODETYPE);
	sConfig.nPortIndex = nAudioPortIndex[1];
	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_GetConfig\n");
	}

	printf("\n");
	printf("Input play speed(-103 to +103):");
	fgets(InputCommandPlaySpeed, 255, stdin);
	iPlaySpeed = atoi(InputCommandPlaySpeed);
	if((iPlaySpeed >= -103) || (iPlaySpeed <= 103))
	{
		sConfig.nPlaySpeed = iPlaySpeed;
	}else
	{
		printf("Invalid Value:%d", iPlaySpeed);
	}
	printf("Set Play speed = %ld\n", sConfig.nPlaySpeed);

	eErr = OMX_SetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	eErr = OMX_GetConfig(hComponent, OMF_MC_IndexConfigAudioPcm, &sConfig);
	if (eErr != OMX_ErrorNone)
	{
		printf("[TP err] OMX_SetConfig\n");
	}

	printf("Config state\n");
	printf("sConfig.nPlaySpeed    = %ld\n", sConfig.nPlaySpeed);

	/* mutex unlock */
	pthread_mutex_unlock(mutex);

EXIT:
	return;
}

/***************************************************************************
 * SUMMARY:
 *
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
int SendDividedBuff(OMX_BUFFERHEADERTYPE* pBuffArr[], int maxCount, int*pIndex, int* counter, int n2DivBuffFlag)
{
	int ret = 0;
	int bufflen = pBuffArr[*pIndex]->nFilledLen;

	printf("divbuff value=%d\n",n2DivBuffFlag );
	if (n2DivBuffFlag <= 1)
	{
		return -1;
	}

	switch (n2DivBuffFlag)
	{
		//case 12345: // 2 bytes buff sequencies
			//n2DivBuffFlag = bufflen/2;
			//	break;
		default: //n div, each time 2 bytes length, except the last time
			ret = SendnDividedBuff(pBuffArr, maxCount, pIndex, counter, n2DivBuffFlag);
			break;
		//default:
		//	ret = -1;
		//	break;
	}

	return ret;
}

/***************************************************************************
 * SUMMARY:
 *
 * PARAMETERS:
 * RETURNS:
 * DESCRIPTION:
 * NOTES:
 ***************************************************************************/
static int SendnDividedBuff(OMX_BUFFERHEADERTYPE* pBuffArr[], int maxCount, int*pIndex, int* counter, int n2DivBuffFlag)
{
	int                   index      = *pIndex;
	unsigned char*        pBuffTemp  = pBuffArr[index]->pBuffer;
	int                   nFilledLen = pBuffArr[index]->nFilledLen;
	char*                 pBuff;
	OMX_ERRORTYPE         err        = OMX_ErrorNone;
	unsigned char         nflags     = 0;
	int                   i          = 0;
	unsigned char*        pVisit;
	int                   div_send_buff_size;
	OMX_BUFFERHEADERTYPE* psBuffer   = NULL;
	extern int            g_IsJpegPlaying;
	extern APP_DATA*      g_pJPGAppData;

	printf("use %d  divided buff\n", n2DivBuffFlag);

	nflags = pBuffArr[index]->nFlags;
	printf("nFilled len=%d\n", nFilledLen);
	pBuff = malloc(nFilledLen);

	if (pBuff == 0)
	{
		printf("not enough memory for buffering,  turn to normal way\n");
		return -1;
	}

	if (n2DivBuffFlag != 12345)
	{
		div_send_buff_size = nFilledLen/n2DivBuffFlag;
		if (div_send_buff_size <= 0)
		{
			div_send_buff_size = 2;
		}

		if ((g_IsJpegPlaying == 1) && ((div_send_buff_size % 2) != 0))
		{
			div_send_buff_size += 1;
		}
	}
	else
	{
		div_send_buff_size = 2;
	}
	memcpy(pBuff,  pBuffTemp, nFilledLen); //firstly backup

	//add odd&even support
	if (n2DivBuffFlag != 12345)
	{
		if (divLengthType == 1) //odd
		{
			if (div_send_buff_size %2 == 0)
			{
				div_send_buff_size++;
			}
		}
		else if (divLengthType == 2) //even
		{
			if (div_send_buff_size %2 == 1)
			{
				div_send_buff_size++;
			}
		}
	}

	pVisit = pBuff;
    printf("\n\n-----------------------------------------------------\n");


	while ((i < (nFilledLen - div_send_buff_size))  && (VideoInputPortThreadLoopFlag || g_IsJpegPlaying))//nFilledLen-2))
	{
		pBuffArr[index]->nFlags = 0;
		pBuffArr[index]->nFilledLen = div_send_buff_size;

		memcpy(pBuffArr[index]->pBuffer, pVisit, div_send_buff_size);
		pVisit = pVisit + div_send_buff_size;

		printf("send %d bytes buff\n", div_send_buff_size);
		err = OMX_EmptyThisBuffer(Video_hComponent,pBuffArr[index]);
		if (err != OMX_ErrorNone)
		{
			printf("[TP err][%d] OMX_EmptyThisBuffer erro code = 0x%08x\n", __LINE__, err);
		}
		psBuffer = &pBuffArr[index];

		index++;
		if (index >= maxCount)
		{
			index = 0;
		}
		*pIndex = index;

		//the following if case is for jpeg only, because jpeg needs "Appdata" for calculation, different from others.
		if (g_IsJpegPlaying)
		{
			if (g_pJPGAppData->decFlag != FLAG_DEC_ERROR_INPUT)
			{
				g_pJPGAppData->decFlag = FLAG_DEC_DECODING;
			}

			while (((psBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS) && (g_pJPGAppData->decFlag == FLAG_DEC_DECODING))
			{
				usleep(10000);
			}
			if (g_pJPGAppData->decFlag == FLAG_DEC_LAST_DECODE)
			{
				g_pJPGAppData->endFlag = FLAG_END_ON;
			}
			g_pJPGAppData->decFlag = FLAG_DEC_FINISH;
		}
		sem_wait(&VideoInputSemId);

		i += div_send_buff_size;
	}

	pBuffArr[index]->nFlags = nflags;
	pBuffArr[index]->nFilledLen = nFilledLen - i;
	memcpy(pBuffArr[index]->pBuffer, pVisit, pBuffArr[index]->nFilledLen);

	err = OMX_EmptyThisBuffer(Video_hComponent,pBuffArr[index]);
	if (err != OMX_ErrorNone)
	{
		printf("[TP err][%d] OMX_EmptyThisBuffer erro code = 0x%08x\n", __LINE__, err);
	}

	free(pBuff);
	printf("send the last buff\n");
	printf("--------------------------------------------\n");

	return 0;
}

/***************************************************************************
 ** SUMMARY:     Use to suspend and resume the thread at OMX_StateLoaded state
 ** PARAMETERS:  no
 ** RETURNS:	 void
 ** DESCRIPTION:
 ** NOTES:
 ****************************************************************************/

void LoadedDly()
{
	sleep(3);
	printf("sleep 3 seconds!\n");
}

#include <time.h>
#include <sys/timeb.h>
static FILE* s_fp = NULL;
void PRINT_TIME(char* msg)
{
	struct timeb tp;
	struct tm *tm;
	char buff[256] = {0};
	int len;

	ftime(&tp);
	tm = localtime(&tp.time);

	printf("[%02d:%02d:%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
	sprintf(buff, "[%02d:%02d:%02d] %s \r\n", tm->tm_hour, tm->tm_min, tm->tm_sec, msg);
	len = strlen(buff);

	if (s_fp == NULL)
		s_fp = fopen("log.dat", "wb");

	if (s_fp <= 0)printf("err write the file\n");

	if (s_fp > 0)
	{
		fwrite(buff, 1, len, s_fp);
		fflush(s_fp);
	}

}

void CLOSE_PRINT_TIME()
{
	if (s_fp > 0)
		fclose(s_fp);

	s_fp = 0;
}


// hdmi test --->
static int hdmi_fd = 0;
int OpenHDMI(void)
{
	hdmi_fd = open("/dev/hdmi", O_RDWR);
	if(hdmi_fd == -1)
	{
		printf("Failed open /dev/hdmi\n");
		return OMX_ErrorUndefined;
	}
	printf(" **HDMI opend**\n");

	return 0;
}

void CloseHDMI(void)
{
	if (hdmi_fd > 0)
	{
		close (hdmi_fd);
	}
}

void SetRpeatON()
{
	RepertFlag = 1;
}

void SetRpeatOFF()
{
	RepertFlag = 0;
}

// hdmi test ---<

int check_video_frame_type(unsigned   char   *data)
{
	unsigned   char   vt_byte;
	unsigned   int   head;


	head   =   *((unsigned   int   *)data);
//	printf("data:%08lx\n", head);
	if(head   ==   0xb3010000   ||   head   ==   0x00010000)
		return   i_frame;
	else   if(head   ==   0xb6010000)
	{
		vt_byte   =   data[4];
		vt_byte   &=   0xC0;
		vt_byte   =   vt_byte   >>   6;
		switch(vt_byte)
		{
			case   0:
			return   i_frame;
			break;
			case   1:
			return   p_frame;
			break;
			case   2:
			return   b_frame;
			break;
			case   3:
			return   d_frame;
			break;
		}
	}

	return   unknow_frame;
}

int check_h264_frame_type(unsigned   char   *data)
{
	char ch = data[0];

	ch = ch & 0x1F;

	//printf("ch = %d\n", ch);
	if (ch == 5) return i_frame;

	if (ch == 1)return p_frame;

	return unknow_frame;
}

/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
