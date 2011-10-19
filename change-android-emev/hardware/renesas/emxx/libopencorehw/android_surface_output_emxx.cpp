/* ------------------------------------------------------------------
 * Copyright (C) 2008 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#define LOG_NDEBUG 0
#define LOG_TAG "VideoMioEMXX"
#include <utils/Log.h>

#include "android_surface_output_emxx.h"
#include <media/PVPlayer.h>

using namespace android;

OSCL_EXPORT_REF AndroidSurfaceOutputEmxx::AndroidSurfaceOutputEmxx():
	AndroidSurfaceOutput()
{
	mUseOverlay = true;
	mNumberOfFramesToHold = 2;
	mOverlay = NULL;
}

OSCL_EXPORT_REF AndroidSurfaceOutputEmxx::~AndroidSurfaceOutputEmxx()
{
	mUseOverlay = false;
}

status_t AndroidSurfaceOutputEmxx::set(PVPlayer* pvPlayer, const sp<ISurface>& surface, bool emulation)
{
	mPvPlayer = pvPlayer;
	mEmulation = emulation;
	mSurface = surface;
	return NO_ERROR;
}

OSCL_EXPORT_REF bool AndroidSurfaceOutputEmxx::initCheck()
{
	// reset flags in case display format changes in the middle of a stream
	resetVideoParameterFlags();

	// release resources if previously initialized
	closeFrameBuf();

	// copy parameters in case we need to adjust them
	int displayWidth = iVideoDisplayWidth;
	int displayHeight = iVideoDisplayHeight;
	int frameWidth = iVideoWidth;
	int frameHeight = iVideoHeight;
	int frameSize;
	int overlay_format;

	if (mUseOverlay) {
		frameSize = (frameWidth * frameHeight * 3) / 2;
		if (iVideoSubFormat ==PVMF_MIME_YUV420_SEMIPLANAR) {
			overlay_format = OVERLAY_FORMAT_YCbCr_420_SP;
		} else {
			overlay_format = OVERLAY_FORMAT_YCbCr_420_P;
		}
		sp<OverlayRef> ref = mSurface->createOverlay(displayWidth, displayHeight, overlay_format, 0);
		mOverlay = new Overlay(ref);

		overlay_buffer_t buf;
		for (int i = 0; i < USE_BUF_COUNT; i++) {
			buf = (overlay_buffer_t)&mOverlayBuffer[i];
			if (mOverlay->dequeueBuffer(&buf)) {
				return false;
			}
			mOverlayBuffer[i].w_src = frameWidth;
			mOverlayBuffer[i].h_src = frameHeight;
		}
	}

	mSysTime.tv_usec = 0;
	mSysTime.tv_sec = 0;
	mTimeStamp = 0;
	mBuffer_count = 0;
	mInitialized = true;
	mPostFlag = 1;
	mPvPlayer->sendEvent(MEDIA_SET_VIDEO_SIZE, iVideoDisplayWidth, iVideoDisplayHeight);

	return mInitialized;
}

void AndroidSurfaceOutputEmxx::GetScreenTime(PVMFTimestamp msec)
{
	struct timeval tmp;

	if (msec > 100)
		msec = 0;

	mSysTime.tv_usec += (msec * 1000);
	if (mSysTime.tv_usec >= 1000000) {
		mSysTime.tv_usec -= 1000000;
		mSysTime.tv_sec++;
	}

	gettimeofday(&tmp, NULL);

	tmp.tv_usec += (16 * 1000);
	if (tmp.tv_usec >= 1000000) {
		tmp.tv_usec -= 1000000;
		tmp.tv_sec++;
	}

	if (mPostFlag == 0) {
		if (mSysTime.tv_sec > tmp.tv_sec)
			return;

		if (mSysTime.tv_sec == tmp.tv_sec) {
			if (mSysTime.tv_usec > tmp.tv_usec)
				return;
		}
	}

	mSysTime.tv_sec  = tmp.tv_sec;
	mSysTime.tv_usec = tmp.tv_usec + (17 * 1000);
	if (mSysTime.tv_usec >= 1000000) {
		mSysTime.tv_usec -= 1000000;
		mSysTime.tv_sec++;
	}
}

PVMFStatus AndroidSurfaceOutputEmxx::writeFrameBuf(uint8* aData, uint32 aDataLen, const PvmiMediaXferHeader& data_header_info)
{
	if (mSurface == 0) return PVMFFailure;

	if (mUseOverlay) {
		overlay_buffer_t buf = (overlay_buffer_t)&mOverlayBuffer[mBuffer_count];

		mOverlayBuffer[mBuffer_count].vaddr = aData;

		GetScreenTime(data_header_info.timestamp - mTimeStamp);
		if (mPostFlag == 0) {
			mOverlayBuffer[mBuffer_count].time.tv_sec = mSysTime.tv_sec;
			mOverlayBuffer[mBuffer_count].time.tv_usec = mSysTime.tv_usec;
		} else {
			mOverlayBuffer[mBuffer_count].time.tv_sec = -1;
			mOverlayBuffer[mBuffer_count].time.tv_usec = -1;
			mPostFlag = 0;
		}

		mTimeStamp = data_header_info.timestamp;

		mOverlay->queueBuffer(buf);
		mOverlay->dequeueBuffer(&buf);
		if (++mBuffer_count == USE_BUF_COUNT) mBuffer_count = 0;
	}
	return PVMFSuccess;
}

// post the last video frame to refresh screen after pause
void AndroidSurfaceOutputEmxx::postLastFrame()
{
	mPostFlag = 1;
}

void AndroidSurfaceOutputEmxx::closeFrameBuf()
{
	if (!mInitialized) return;

	mInitialized = false;

	//Free the buffers
	if (mOverlay != NULL) {
		mOverlay->destroy();
		mOverlay.clear();
	}
}

// factory function for playerdriver linkage
extern "C" AndroidSurfaceOutputEmxx* createVideoMio()
{
	LOGD("Creating Vendor(EMXX) Specific MIO component");
	return new AndroidSurfaceOutputEmxx();
}

