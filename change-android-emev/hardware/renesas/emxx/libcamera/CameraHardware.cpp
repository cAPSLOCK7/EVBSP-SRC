/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "CameraHardware"
#include <utils/Log.h>

#include "CameraHardware.h"
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ui/Overlay.h>

#define CAMERA_DEVICE       "/dev/v4l/video1"

// The default value of camera parameters
#define PREVIEW_SIZE_WIDTH  640
#define PREVIEW_SIZE_HEIGHT 480
#define PREVIEW_SIZE_VALUES "640x480"
#define PREVIEW_FRAMERATE   30
#define PICTURE_SIZE_VALUES "640x480"
#define PICTURE_SIZE_WIDTH  640
#define PICTURE_SIZE_HEIGHT 480
#define PICTURE_FORMAT      "jpeg"

/*
 * Select the pixel format for preview from here
 * Buffers will be registered to the Surface flinger
 * with this format only.
 */
#define PREVIEW_PIX_FORMAT   "yuv420p"

//************debug macro**********
#define DEBUG_CAM 0
#if DEBUG_CAM
#define CAMHW_DEBUG_LOGD(FMT, ARG...) \
	LOGD("[debug] CameraHardware::%s L:%d "FMT, \
		__FUNCTION__,__LINE__, ##ARG)
#define PRINT_DEBUG_MSG CAMHW_DEBUG_LOGD("")
#else
#define CAMHW_DEBUG_LOGD(FMT, ARG...)
#define PRINT_DEBUG_MSG
#endif
//*********************************
namespace android {
CameraHardware::CameraHardware()
		: mParameters(),
		  mPreviewHeap(0),
		  mRawHeap(0),
		  mNotifyCb(0),
		  mDataCb(0),
		  mDataCbTimestamp(0),
		  mCallbackCookie(0),
		  mMsgEnabled(0),
		  mPreviewFrameSize(0),
		  mCurrentPreviewFrame(0),
		  mCurrentEncodeFrame(0),
		  mPreviewStopped(true),
		  mOverlay(0),
		  mUseOverlay(1)
		  //mUseOverlay(0)
{

	for (int i = 0; i < kBufferCount; i++) {
		mOverlayBuffer[i].vaddr = 0;
	}

	initDefaultParameters();
}

void CameraHardware::initDefaultParameters()
{
	CameraParameters p;

	p.setPreviewFormat(PREVIEW_PIX_FORMAT);

	p.set("preview-size-values", PREVIEW_SIZE_VALUES);
	p.setPreviewSize(PREVIEW_SIZE_WIDTH, PREVIEW_SIZE_HEIGHT);
	p.setPreviewFrameRate(PREVIEW_FRAMERATE);

	p.set("picture-size-values", PICTURE_SIZE_VALUES);
	p.setPictureSize(PICTURE_SIZE_WIDTH, PICTURE_SIZE_HEIGHT);
	p.setPictureFormat(PICTURE_FORMAT);

	p.set("jpeg-quality", "85");
	if (setParameters(p) != NO_ERROR) {
		LOGE("Failed to set default parameters?!");
	}

	mVideoEncodeFormat = HAL_PIXEL_FORMAT_YCbCr_420_P;
}

void CameraHardware::initHeapLocked()
{
	// Create raw heap.
	int picture_width = 0, picture_height = 0;
	mParameters.getPictureSize(&picture_width, &picture_height);
	mRawHeap = new MemoryHeapBase(picture_width * 2 * picture_height);

	int preview_width = 0, preview_height = 0;
	mParameters.getPreviewSize(&preview_width, &preview_height);
	LOGI("initHeapLocked: preview size=%dx%d", preview_width, preview_height);

	// Note that we enforce yuv422 in setParameters().
	int how_big = preview_width * preview_height * 2;

	// If we are being reinitialized to the same size as before, no
	// work needs to be done.
	if (how_big == mPreviewFrameSize)
		return;

	mPreviewFrameSize = how_big;
	if (!mUseOverlay) {
		// Make a new mmap'ed heap that can be shared across processes.
		// use code below to test with pmem
		mPreviewHeap = new MemoryHeapBase(mPreviewFrameSize * kBufferCount);
		// Make an IMemory for each frame so that we can reuse them in callbacks.
		for (int i = 0; i < kBufferCount; i++) {
			mBuffers[i] = new MemoryBase(mPreviewHeap, i * mPreviewFrameSize, mPreviewFrameSize);
		}
	}

	mEncodeFrameSize = preview_width * preview_height * 2;
	mEncodeHeap = new MemoryHeapBase(mEncodeFrameSize * kBufferCount);
	// Make an IMemory for each frame so that we can reuse them in callbacks.
	for (int i = 0; i < kBufferCount; i++) {
		mEncodeBuffers[i] = new MemoryBase(mEncodeHeap, i * mEncodeFrameSize,mEncodeFrameSize);
	}

}

CameraHardware::~CameraHardware()
{
	if (mPreviewThread != 0) {
		LOGI("mPreviewThread is not NULL\n");
		sp<PreviewThread> previewThread;
		previewThread = mPreviewThread;
		previewThread->requestExitAndWait();
		mPreviewThread.clear();
		mPreviewThread = 0;
	}

	if (mPreviewHeap != 0) {
		LOGI("mPreviewHeap is not NULL\n");
		mRawHeap.clear();
		for (int i = 0; i < kBufferCount; i++)
			mBuffers[i].clear();
		mPreviewHeap.clear();
	}

	if (mEncodeHeap != 0) {
		LOGI("mEncodeHeap is not NULL\n");
		for (int i = 0; i < kBufferCount; i++)
			mEncodeBuffers[i].clear();
		mEncodeHeap.clear();
	}

	singleton.clear();
}

sp<IMemoryHeap> CameraHardware::getPreviewHeap() const
{
	return mPreviewHeap;
}

sp<IMemoryHeap> CameraHardware::getRawHeap() const
{
	return mRawHeap;
}

void CameraHardware::setCallbacks(notify_callback notify_cb,
				   data_callback data_cb,
				   data_callback_timestamp data_cb_timestamp,
				   void* user)
{
	Mutex::Autolock lock(mLock);
	mNotifyCb = notify_cb;
	mDataCb = data_cb;
	mDataCbTimestamp = data_cb_timestamp;
	mCallbackCookie = user;
}

void CameraHardware::enableMsgType(int32_t msgType)
{
	Mutex::Autolock lock(mLock);
#if DEBUG_CAM
	CAMHW_DEBUG_LOGD("enableMsgType msgType = 0x%X\n", msgType);
#else
	LOGI("enableMsgType msgType = 0x%X\n",msgType);
#endif
	mMsgEnabled |= msgType;
}

void CameraHardware::disableMsgType(int32_t msgType)
{
	Mutex::Autolock lock(mLock);
#if DEBUG_CAM
	CAMHW_DEBUG_LOGD("disableMsgType msgType = 0x%X\n ", msgType);
#else
	LOGI("disableMsgType msgType = 0x%X\n",msgType);
#endif
	mMsgEnabled &= ~msgType;
}

bool CameraHardware::msgTypeEnabled(int32_t msgType)
{
	Mutex::Autolock lock(mLock);
#if DEBUG_CAM
	CAMHW_DEBUG_LOGD("msgTypeEnabled msgType = 0x%X\n ", msgType);
#else
	LOGI("msgTypeEnabled msgType = 0x%X\n",msgType);
#endif
	return (mMsgEnabled & msgType);
}


int CameraHardware::cameraRestart()
{
	status_t  retValue = NO_ERROR;
	/* Close camera */
	mCamera.StopStreaming();
	mCamera.Uninit();
	mCamera.Close();

	/* Restart camera */
	if (!mCamera.Open(CAMERA_DEVICE)) {
		LOGI("startPreview : mCamera.Open done\n");
	} else {
		LOGE("startPreview : mCamera.Open ERROR\n");
		return INVALID_OPERATION;
	}

	retValue = mCamera.Init();
	if (retValue != 0) {
		LOGE("-startPreview : mCamera.Init Failed\n");
		mCamera.Close();
		return retValue;
	}

	retValue = mCamera.StartStreaming();
	if (retValue != 0) {
		LOGE("-startPreview : mCamera.StartStreaming Failed\n");
		mCamera.Uninit();
		mCamera.Close();
		return retValue;
	}
	return retValue;
}


int CameraHardware::GrabCamFrame (unsigned char **previewBuffer, int *previewSize)
{
	int ret = mCamera.GrabCamFrame(previewBuffer, previewSize);

	if (ret != 0) {
		LOGI("-----CAMERA IS UNPLUGGED------");
		mVideorecordingStopped = true;
		mPreviewStopped = true;
		mCamera.StopStreaming();
		mCamera.Uninit();
		mCamera.Close();
	}

	return ret;
}

int CameraHardware::ReleaseCamFrame ()
{
	return mCamera.ReleaseCamFrame();
}

int CameraHardware::previewOverlay(unsigned char *previewBuffer, int previewSize)
{
	uint8_t* buffer = (uint8_t*)mOverlayBuffer[mCurrentPreviewFrame].vaddr;

#if DEBUG_CAM
	LOGI("Use overlay preview");
	CAMHW_DEBUG_LOGD("buffer=%p", buffer);
#endif

	if (buffer != 0) {
		overlay_buffer_t buf = (overlay_buffer_t)&mOverlayBuffer[mCurrentPreviewFrame];

		if (!mPreviewStopped) {
			CAMHW_DEBUG_LOGD("mOverlayBuffer[%d].vaddr=%p",
				mCurrentPreviewFrame,mOverlayBuffer[mCurrentPreviewFrame].vaddr);

			mOverlayBuffer[mCurrentPreviewFrame].vaddr = previewBuffer;

			// Notify the client of a new frame.
			CAMHW_DEBUG_LOGD("mMsgEnabled =%d,CAMERA_MSG_PREVIEW_FRAME=%d",
					mMsgEnabled , CAMERA_MSG_PREVIEW_FRAME);

			mOverlay->queueBuffer(buf);
			mOverlay->dequeueBuffer(&buf);

			// Advance the buffer pointer.
			mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % kBufferCount;
		}
	}

	return 0;
}

int CameraHardware::previewNoOverlay(unsigned char *previewBuffer, int previewSize)
{
#if DEBUG_CAM
	LOGI("Not use overlay preview");
#endif
	// Find the offset within the heap of the current buffer.
	ssize_t offset = mCurrentPreviewFrame * mPreviewFrameSize;

	sp<MemoryHeapBase> heap = mPreviewHeap;
	sp<MemoryBase> buffer = mBuffers[mCurrentPreviewFrame];

	if (buffer != 0) {
		// This is always valid, even if the client died -- the memory
		// is still mapped in our process.
		void *base = heap->base();

		// Fill the current frame with the v4l2 camera.
		uint8_t *frame = ((uint8_t *)base) + offset;
		if (!mPreviewStopped) {
			memcpy((void *)frame, (void *)previewBuffer, (size_t) previewSize);
			// Notify the client of a new frame.
			if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
				mDataCb(CAMERA_MSG_PREVIEW_FRAME, buffer, mCallbackCookie);
			}
			// Advance the buffer pointer.
			mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % kBufferCount;
		}
	}
	return 0;
}

int CameraHardware::previewRecord(unsigned char *previewBuffer, int previewSize)
{
	unsigned long long int nsTimeStamp = 0;
	//	unsigned long long int timeStamp = 0;
#if DEBUG_CAM
	LOGD("---------videoRecordingThread---");
	LOGD("[debug] %s: %d",__FUNCTION__,__LINE__);
#endif

	// Find the offset within the heap of the current buffer.
	ssize_t offset = mCurrentEncodeFrame * mEncodeFrameSize;
	sp<MemoryHeapBase> heap = mEncodeHeap;
	sp<MemoryBase> buffer = mEncodeBuffers[mCurrentEncodeFrame];

#if DEBUG_CAM
	bool isZero = true;
	LOGD("[debug] %s:%d : buffer is zero=%d",__FUNCTION__,__LINE__,isZero= (buffer==0)?true:false);
#endif
	if (buffer != 0) {
		// This is always valid, even if the client died -- the memory
		// is still mapped in our process.
		void *base = heap->base();
		// Fill the current frame with the v4l2 camera.
		uint8_t *frame = ((uint8_t *)base) + offset;

		if (mVideorecordingStopped == false) {
			memcpy((void *)frame, (void *)previewBuffer, (size_t) previewSize);
			nsecs_t timestamp = systemTime();
			#if DEBUG_CAM
				LOGD("-----------timestamp=%d------------", (int)timestamp);
			#endif
			// Notify the client of a new frame.
			//timeStamp = GetTimeUS() * 1000;

			if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
			#if DEBUG_CAM
				LOGD(" %s:%d,--------------call back mDataCbTimestamp ------",__FUNCTION__,__LINE__);
			#endif
				mDataCbTimestamp(timestamp, CAMERA_MSG_VIDEO_FRAME, buffer, mCallbackCookie);
			}
			// Advance the buffer pointer.
			mCurrentEncodeFrame = (mCurrentEncodeFrame + 1) % kBufferCount;
		}
	}

	return 0;
}
// ---------------------------------------------------------------------------

int CameraHardware::previewThread()
{
	int ret=0;
	unsigned char *previewBuffer = NULL;
	int previewSize;
	CAMHW_DEBUG_LOGD("previewThread start\n");
	CAMHW_DEBUG_LOGD("mVideorecordingStopped=%d, mVideorecordingStopped = %d\n",
		mVideorecordingStopped,mVideorecordingStopped);

	mLock.lock();
	ret = GrabCamFrame(&previewBuffer, &previewSize);
	if (ret!=0) {
		mLock.unlock();
		return ret;
	}

	if ( mUseOverlay ) {  //user overlay && need not data callback
		previewOverlay(previewBuffer, previewSize);
	} else { //not use overlay
		previewNoOverlay(previewBuffer, previewSize);
	}
	if (mVideorecordingStopped == false) {
		previewRecord(previewBuffer, previewSize);
	}
	ReleaseCamFrame();
	mLock.unlock();

	CAMHW_DEBUG_LOGD("mPreviewFrameDelay = %d",__FUNCTION__,__LINE__,mPreviewFrameDelay);

	usleep(mPreviewFrameDelay);
	return NO_ERROR;
}

status_t CameraHardware::startPreview()
{
	LOGI("----------startPreview----------\n");

	status_t  retValue = NO_ERROR;

	Mutex::Autolock lock(mLock);
	if (mPreviewThread != 0) {
		// already running
		LOGI("-startPreview: mPreviewThread !=0 that means already running\n");
		return INVALID_OPERATION;
	}
	// Calculate how long to wait between frames.
	mPreviewFrameDelay = (int)(1000000.0f / float(mPreviewFrameRate));

	if (!mCamera.Open(CAMERA_DEVICE)) {
		LOGI("startPreview : mCamera.Open done\n");
	} else {
		LOGE("startPreview : mCamera.Open ERROR\n");
		return INVALID_OPERATION;
	}

	mCamera.QueryCapabilities();

	if (mCamera.IsVideoCaptureSupported())
		mVideoCaptureSupported = true;
	else
		LOGE("VideoCapture Not Supported");

	if (mCamera.IsStreamingSupported())
		mStreamingSupported = true;
	else
		LOGE("Streaming Not Supported");


	mCamera.SetCameraParameters(mPreviewWidth, mPreviewHeight,
				mCameraCapturePixelFormat);

	retValue = mCamera.Init();
	if (retValue != 0) {
		LOGI("-startPreview : mCamera.Init Failed\n");
		mCamera.Close();
		return retValue;
	}

	retValue = mCamera.StartStreaming();
	if (retValue != 0) {
		LOGI("-startPreview : mCamera.StartStreaming Failed\n");
		mCamera.Close();
		return retValue;
	}

	mPreviewStopped = false;
	mVideorecordingStopped = true;
	mPreviewThread = new PreviewThread(this);
	LOGI("-startPreview : new PreviewThread DONE\n");
	return NO_ERROR;
}

void CameraHardware::stopPreview()
{

	LOGI("ENTER stopPreview\n");
	if (mPreviewStopped == false) {
		mPreviewStopped = true;
		sp<PreviewThread> previewThread;

		{ // scope for the lock
			Mutex::Autolock lock(mLock);
			previewThread = mPreviewThread;
		}

		// don't hold the lock while waiting for the thread to quit
		if (previewThread != 0) {
			LOGI("previewThread != 0\n");
			previewThread->requestExitAndWait();
		}

		Mutex::Autolock lock(mLock);
		mPreviewThread.clear();
		mPreviewThread = 0;
		LOGI("stopPreview: closing camera...\n");
		mCamera.StopStreaming();
		mCamera.Uninit();
		mCamera.Close();
	}
	LOGI("EXIT stopPreview\n");
}

bool CameraHardware::previewEnabled()
{
	return mPreviewThread != 0;
}

status_t CameraHardware::startRecording()
{
	LOGI("startRecording\n");
	int preview_width = 0, preview_height = 0;
	mParameters.getPreviewSize(&preview_width, &preview_height);
	char videoSize[8] = {0};
	sprintf(videoSize, "%dx%d", preview_width, preview_height);
	LOGI("videoSize=%s\n", videoSize);
	mParameters.set("video-size", videoSize);
	mVideorecordingStopped = false;
	return NO_ERROR;
}

void CameraHardware::stopRecording()
{
	LOGI("ENTER stopRecording\n");
	mVideorecordingStopped = true;
}

bool CameraHardware::recordingEnabled()
{
	LOGI("recordingEnabled---\n");
	return (mVideorecordingStopped == false);
}

void CameraHardware::releaseRecordingFrame(const sp<IMemory>& mem)
{
	LOGI("releaseRecordingFrame\n");
}

// ---------------------------------------------------------------------------

int CameraHardware::beginAutoFocusThread(void *cookie)
{
	CameraHardware *c = (CameraHardware *)cookie;
	LOGI("beginAutoFocusThread\n");
	return c->autoFocusThread();
}

int CameraHardware::autoFocusThread()
{
	LOGI("autoFocusThread\n");
	if (mMsgEnabled & CAMERA_MSG_FOCUS) {
		mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
	}
	return NO_ERROR;
}

status_t CameraHardware::autoFocus()
{
	LOGI("autoFocus\n");
	Mutex::Autolock lock(mLock);
	if (createThread(beginAutoFocusThread, this) == false) {
		return UNKNOWN_ERROR;
	}
	return NO_ERROR;
}

status_t CameraHardware::cancelAutoFocus()
{
	LOGI("cancelAutoFocus\n");
	return NO_ERROR;
}

/*static*/ int CameraHardware::beginPictureThread(void *cookie)
{
	CameraHardware *c = (CameraHardware *)cookie;
	LOGI("beginPictureThread\n");
	return c->pictureThread();
}

int CameraHardware::pictureThread()
{
	//int picture_width = 0, picture_height = 0;
	int ret = 0;
	LOGI("---------------pictureThread------------\n");
	if (mMsgEnabled & CAMERA_MSG_SHUTTER) {
		LOGI("---CAMERA_MSG_SHUTTER---\n");
		mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
	}

	if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
		LOGI("pictureThread:---CAMERA_MSG_RAW_IMAGE---\n");

		//mParameters.getPictureSize(&picture_width, &picture_height);
		//LOGI("Requested:Picture Size: Width = %d \t Height = %d", picture_width, picture_height);
		if (!mCamera.Open(CAMERA_DEVICE)) {
			LOGI("pictureThread : mCamera.Open done\n");
		} else {
			LOGE("pictureThread : mCamera.Open ERROR\n");
			return INVALID_OPERATION;
		}
		mCamera.SetCameraParameters(mRawWidth, mRawHeight,
						mCameraCapturePixelFormat);

		CAMHW_DEBUG_LOGD("mCameraCapturePixelFormat = 0x%x",
			mCameraCapturePixelFormat);

		ret = mCamera.Init();
		if (ret != 0) {
			LOGE("pictureThread:mCamera.Init Failed\n");
			return NO_INIT;
		}

		mCamera.StartStreaming();
		mDataCb(CAMERA_MSG_RAW_IMAGE, mCamera.GrabRawFrame(mCameraCapturePixelFormat), mCallbackCookie);
	}

	if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
		LOGI("---CAMERA_MSG_COMPRESSED_IMAGE---\n");
		mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mCamera.GrabJpegFrame(mCameraCapturePixelFormat),
				mCallbackCookie);
	}

	mCamera.StopStreaming();
	mCamera.Uninit();
	mCamera.Close();
	return NO_ERROR;
}

status_t CameraHardware::takePicture()
{
	LOGI("takePicture\n");

	stopPreview();
	if (createThread(beginPictureThread, this) == false)
		return -1;
	return NO_ERROR;
}

status_t CameraHardware::cancelPicture()
{
	LOGI("cancelPicture\n");
	return NO_ERROR;
}

status_t CameraHardware::dump(int fd, const Vector<String16>& args) const
{
#if 0
	const size_t SIZE = 256;
	char buffer[SIZE];
	String8 result;
	AutoMutex lock(&mLock);
	if (mFakeCamera != 0) {
		mFakeCamera->dump(fd);
		mParameters.dump(fd, args);
		snprintf(buffer, 255, " preview frame(%d), size (%d), running(%s)\n",
			mCurrentPreviewFrame, mPreviewFrameSize, mPreviewRunning?"true": "false");
		result.append(buffer);
	} else {
		result.append("No camera client yet.\n");
	}
	write(fd, result.string(), result.size());

#endif

	const size_t SIZE = 256;
	char buffer[SIZE] = {0};
	String8 result;
	LOGI("-----Camera::dump-----\n");
	AutoMutex lock(&mLock);
	mCamera.dump(fd);
	mParameters.dump(fd, args);
	snprintf(buffer, 255, " preview frame(%d), size (%d), running(%s)\n",
	mCurrentPreviewFrame, mPreviewFrameSize, mPreviewRunning?"true": "false");
	result.append(buffer);

	write(fd, result.string(), result.size());
	return NO_ERROR;
}

status_t CameraHardware::setParameters(const CameraParameters& params)
{
	LOGI("setParameters - init\n");
	Mutex::Autolock lock(mLock);
	// XXX verify params

	if (strcmp(params.getPreviewFormat(), "yuv422sp") == 0) {
	PRINT_DEBUG_MSG;
		mPreviewPixelFormat = HAL_PIXEL_FORMAT_YCbCr_422_SP;
		mCameraCapturePixelFormat = V4L2_PIX_FMT_NV16;
	} else if (strcmp(params.getPreviewFormat(), "yuv420sp") == 0) {
	PRINT_DEBUG_MSG;
		mPreviewPixelFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
		mCameraCapturePixelFormat = V4L2_PIX_FMT_NV12;
	} else if (strcmp(params.getPreviewFormat(), "yuv422p") == 0) {
	PRINT_DEBUG_MSG;
		mPreviewPixelFormat = HAL_PIXEL_FORMAT_YCbCr_422_P;
		mCameraCapturePixelFormat = V4L2_PIX_FMT_YUV422P;
	} else if (strcmp(params.getPreviewFormat(), "yuv420p") == 0) {
	PRINT_DEBUG_MSG;
		mPreviewPixelFormat = HAL_PIXEL_FORMAT_YCbCr_420_P;
		mCameraCapturePixelFormat = V4L2_PIX_FMT_YUV420;
	}else{
		LOGE("Only %s preview is supported",params.getPreviewFormat());
		return -1;
	}

#if DEBUG_CAM
LOGD("CameraHardware::%s:%d: mPreviewPixelFormat=0x%x,mPreviewPixelFormat=%d",
		__FUNCTION__,__LINE__,mPreviewPixelFormat,mPreviewPixelFormat);
#endif

	if (strcmp(params.getPictureFormat(), "jpeg") != 0) {
		LOGE("Only jpeg still pictures are supported");
		return -1;
	}

	mParameters = params;

	/* check size for preview */
	preview_size_type* ps = preview_sizes;
	preview_size_type* ps_pic = picture_sizes;
	size_t i;
	int framerate;
	params.getPreviewSize(&mPreviewWidth, &mPreviewHeight);
	LOGI("Preview size=%dx%d", mPreviewWidth, mPreviewHeight);
	for (i = 0; i < PREVIEW_SIZE_COUNT; ++i, ++ps) {
	    if (mPreviewWidth >= ps->width && mPreviewHeight >= ps->height) break;
	}
	// app requested smaller size than supported, use smallest size
	if (i == PREVIEW_SIZE_COUNT) ps--;
	LOGI("Preview Actual size %d x %d", ps->width, ps->height);
	mParameters.setPreviewSize(ps->width, ps->height);
	mParameters.getPreviewSize(&mPreviewWidth, &mPreviewHeight);

	mPreviewFrameRate = params.getPreviewFrameRate();
	LOGI("Preview frame rate=%d",mPreviewFrameRate);

	/* check size for picture */
	params.getPictureSize(&mRawWidth, &mRawHeight);
	LOGI("Picture size: %d x %d", mRawWidth, mRawHeight);
	for (i = 0; i < PICTURE_SIZE_COUNT; ++i, ++ps_pic) {
	    if (mRawWidth >= ps_pic->width && mRawHeight >= ps_pic->height) break;
	}
	// app requested smaller size than supported, use smallest size
	if (i == PICTURE_SIZE_COUNT) ps_pic--;
	LOGI("Picture Actual size %d x %d", ps_pic->width, ps_pic->height);
	mParameters.setPictureSize(ps_pic->width, ps_pic->height);
	mParameters.getPictureSize(&mRawWidth, &mRawHeight);

	mCamera.JpegQuality = params.getInt("jpeg-quality");
	LOGI("JpegQuality=%d", mCamera.JpegQuality);
	mParameters = params;
	LOGI("setParameters:Calling initHeapLocked\n");
	initHeapLocked();

	return NO_ERROR;
}

CameraParameters CameraHardware::getParameters() const
{
	LOGI("getParameters\n");
	Mutex::Autolock lock(mLock);
	return mParameters;
}

status_t CameraHardware::sendCommand(int32_t command, int32_t arg1,
					int32_t arg2)
{
	LOGI("sendCommand\n");
	return BAD_VALUE;
}

void CameraHardware::release()
{
	LOGI("Camera:release\n");
}

bool CameraHardware::useOverlay()
{
	LOGI("useOveraly true\n");
	return (mUseOverlay) ? true : false;
}
status_t CameraHardware::setOverlay(const sp<Overlay> &overlay)
{
	LOGI("setOverlay ++\n");

	overlay_buffer_t buf;
	int ret;
	int preview_width, preview_height;

	//destroy the old overlay
	if (mOverlay != NULL) {
		int i;
		mOverlay->destroy();
		mOverlay.clear();
		mOverlay = 0;
		for (int i = 0; i < kBufferCount; i++) {
			mOverlayBuffer[i].vaddr = 0;
			CAMHW_DEBUG_LOGD("mOverlayBuffer[%d].vaddr=0x%p",
				i,mOverlayBuffer[i].vaddr);
		}

		LOGE("old overlay was destroyed");
		return NO_ERROR;
	}

	mOverlay = overlay;
	//set new overlay
	if ( mOverlay != NULL ) {
		mParameters.getPreviewSize(&preview_width, &preview_height);
		// Register Buffer
		for (int i = 0; i < kBufferCount; i++) {
			buf = (overlay_buffer_t)&mOverlayBuffer[i];
			ret = mOverlay->dequeueBuffer(&buf);

			CAMHW_DEBUG_LOGD("mOverlayBuffer[%d].vaddr=%p",
				i,mOverlayBuffer[i].vaddr);
			CAMHW_DEBUG_LOGD("preview_width=%d,preview_height=%d",
				preview_width,preview_height);

			if (ret != 0) {
				LOGE("Set overlay Error %d", ret);
				return ret;
			}
			mOverlayBuffer[i].w_src = preview_width;
			mOverlayBuffer[i].h_src = preview_height;
		}
		LOGV("setOverlay --\n");
	}

	return NO_ERROR;
}

wp<CameraHardwareInterface> CameraHardware::singleton;

sp<CameraHardwareInterface> CameraHardware::createInstance()
{
	LOGI("createInstance\n");
	if (singleton != 0) {
		sp<CameraHardwareInterface> hardware = singleton.promote();
		if (hardware != 0) {
			return hardware;
		}
	}


	sp<CameraHardwareInterface> hardware(new CameraHardware());
	singleton = hardware;
	return hardware;
}

extern "C" sp<CameraHardwareInterface> openCameraHardware()
{
	LOGI("*********** openCameraHardware ************\n");
	return CameraHardware::createInstance();
}

}; // namespace android
