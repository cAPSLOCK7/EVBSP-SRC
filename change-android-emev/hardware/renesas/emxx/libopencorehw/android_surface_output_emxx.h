/* ------------------------------------------------------------------
 * Copyright (C) 2008 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#ifndef ANDROID_SURFACE_OUTPUT_EMXX_H_INCLUDED
#define ANDROID_SURFACE_OUTPUT_EMXX_H_INCLUDED

#include "android_surface_output.h"

// support for shared contiguous physical memory
#include <ui/Overlay.h>
#include <sys/time.h>

#define USE_BUF_COUNT	2

class AndroidSurfaceOutputEmxx : public AndroidSurfaceOutput
{
public:
	AndroidSurfaceOutputEmxx();

	virtual status_t set(PVPlayer* pvPlayer, const sp<ISurface>& surface, bool emulation);

	virtual bool initCheck();
	virtual PVMFStatus writeFrameBuf(uint8* aData, uint32 aDataLen, const PvmiMediaXferHeader& data_header_info);
	virtual void postLastFrame();
	virtual void closeFrameBuf();

	OSCL_IMPORT_REF ~AndroidSurfaceOutputEmxx();

private:
	bool		mUseOverlay;
	sp<Overlay> 	mOverlay; 
	int 		mBuffer_count;
	struct overlay_buffer_emxx mOverlayBuffer[USE_BUF_COUNT];
	PVMFTimestamp	mTimeStamp;
	struct timeval	mSysTime;
	int mPostFlag;

	void GetScreenTime(PVMFTimestamp msec);
};

#endif // ANDROID_SURFACE_OUTPUT_EMXX_H_INCLUDED
