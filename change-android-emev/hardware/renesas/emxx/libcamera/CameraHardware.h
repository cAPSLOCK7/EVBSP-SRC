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

#ifndef CAMERA_HARDWARE_H
#define CAMERA_HARDWARE_H

#include <utils/threads.h>
#include <camera/CameraHardwareInterface.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <hardware/overlay.h>
#include <sys/ioctl.h>
#include "V4l2camera.h"

namespace android {

#define CHECK_FPS 0

typedef struct {
	int width;
	    int height;
} preview_size_type;

static preview_size_type preview_sizes[] = {
	{ 640, 480 }, // VGA
	{ 352, 288 }, // CIF
	{ 320, 240 }, // QVGA
};
#define PREVIEW_SIZE_COUNT (sizeof(preview_sizes)/sizeof(preview_size_type))
static preview_size_type picture_sizes[] = {
	{ 640, 480 }, // VGA
	{ 352, 288 }, // CIF
	{ 320, 240 }, // QVGA
};
#define PICTURE_SIZE_COUNT (sizeof(picture_sizes)/sizeof(preview_size_type))

class CameraHardware : public CameraHardwareInterface {
public:
	virtual sp<IMemoryHeap> getPreviewHeap() const;
	virtual sp<IMemoryHeap> getRawHeap() const;

	virtual void        setCallbacks(notify_callback notify_cb,
					 data_callback data_cb,
					 data_callback_timestamp data_cb_timestamp,
					 void* user);

	virtual void        enableMsgType(int32_t msgType);
	virtual void        disableMsgType(int32_t msgType);
	virtual bool        msgTypeEnabled(int32_t msgType);

	virtual status_t    startPreview();
	virtual void        stopPreview();
	virtual bool        previewEnabled();

	virtual status_t    startRecording();
	virtual void        stopRecording();
	virtual bool        recordingEnabled();
	virtual void        releaseRecordingFrame(const sp<IMemory>& mem);

	virtual status_t    autoFocus();
	virtual status_t    cancelAutoFocus();
	virtual status_t    takePicture();
	virtual status_t    cancelPicture();
	virtual status_t    dump(int fd, const Vector<String16>& args) const;
	virtual status_t    setParameters(const CameraParameters& params);
	virtual CameraParameters  getParameters() const;
	virtual status_t    sendCommand(int32_t command, int32_t arg1,
					                int32_t arg2);
	virtual void release();

	static sp<CameraHardwareInterface> createInstance();
	virtual bool useOverlay();
	status_t setOverlay(const sp<Overlay> &overlay);

private:
		CameraHardware();
	virtual ~CameraHardware();

	static wp<CameraHardwareInterface> singleton;

	static const int kBufferCount = 3;

	class PreviewThread : public Thread {
		CameraHardware* mHardware;
	public:
		PreviewThread(CameraHardware* hw) :
#ifdef SINGLE_PROCESS
			// In single process mode this thread needs to be a java thread,
			// since we won't be calling through the binder.
			Thread(true),
#else
			Thread(false),
#endif
			  mHardware(hw) { }
		virtual void onFirstRef() {
			run("CameraPreviewThread", PRIORITY_URGENT_DISPLAY);
		}
		virtual bool threadLoop() {
			mHardware->previewThread();
			// loop until we need to quit
			return true;
		}
	};

	void initDefaultParameters();
	void initHeapLocked();

	int GrabCamFrame (unsigned char **previewBuffer, int *previewSize);
	int ReleaseCamFrame ();
	int previewOverlay(unsigned char *previewBuffer, int previewSize);
	int previewNoOverlay(unsigned char *previewBuffer, int previewSize);
	int previewRecord(unsigned char *previewBuffer, int previewSize);
	int previewThread();
#if CHECK_FPS
	int f_num;
#endif
	static int beginAutoFocusThread(void *cookie);
	int autoFocusThread();

	static int beginPictureThread(void *cookie);
	int pictureThread();

	int cameraRestart();

	mutable Mutex       mLock;

	CameraParameters    mParameters;

	sp<MemoryHeapBase>  mPreviewHeap;
	sp<MemoryHeapBase>  mEncodeHeap;
	sp<MemoryHeapBase>  mRawHeap;

	int                 mEncodeFrameSize;
	int mPreviewFrameRate;
	int mPreviewHeight;
	int mPreviewWidth;
	int mRawHeight;
	int mRawWidth;

	notify_callback     mNotifyCb;
	data_callback       mDataCb;
	data_callback_timestamp mDataCbTimestamp;
	void               *mCallbackCookie;

	int32_t             mMsgEnabled;

	// only used from PreviewThread
	int                 mPreviewFrameSize;
	int                 mCurrentPreviewFrame;
	int                 mCurrentEncodeFrame;
	bool                mPreviewStopped;
	int                 mNQueued;
	int                 mNDequeued;
	sp<Overlay>         mOverlay;
	struct overlay_buffer_emxx mOverlayBuffer[kBufferCount];
	int                 mUseOverlay;
	sp<MemoryBase>      mBuffers[kBufferCount];
	sp<MemoryBase>      mEncodeBuffers[kBufferCount];

	bool                mPreviewRunning;

	// protected by mLock
	sp<PreviewThread>   mPreviewThread;
	bool                mStreamingSupported;
	bool                mVideoCaptureSupported;
	int                 mCameraCapturePixelFormat;
	int                 mPreviewPixelFormat;
	int                 mVideoEncodeFormat;
	int                 mPreviewFrameDelay;
	bool                mVideorecordingStopped;
	V4L2Camera          mCamera;
};

}; // namespace android

#endif
