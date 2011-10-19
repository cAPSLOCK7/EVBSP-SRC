/*
**
** Copyright 2009, The Android Open Source Project
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

#ifndef V4L2_CAMERA_H
#define V4L2_CAMERA_H

#define NB_BUFFER 4
#define PREVIEW_BUFFER 3
#define CONFIG_VIDEO_EMXX
#define CONFIG_VIDEO_EMXX_FILTER

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <linux/videodev2.h>

#include <pixelflinger/format.h>
#include <camera/CameraHardwareInterface.h>


extern "C"{
#include <jpeglib.h>
#include <emxx_jpeglib.h>
}
namespace android {

struct vdIn {
	struct v4l2_capability cap;
	struct v4l2_format format;
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers rb;
	void *mem[NB_BUFFER];
	bool isStreaming;
	int width;
	int height;
	int formatIn;
	int framesizeIn;
};

class V4L2Camera {

public:
	int    JpegQuality;
	V4L2Camera();
	~V4L2Camera();
	int Open(const char *device);
	void Close();
	int Init();
	void Uninit();

	int QueryCapabilities();
	bool IsVideoCaptureSupported();
	bool IsStreamingSupported();
	int GetCameraParameters(int *width, int *height, int *pixelFormat);
	int SetCameraParameters(int width, int height, int pixelFormat);

	int StartStreaming ();
	int StopStreaming ();

	void Camera_dump_to_file(const char *fname, uint8_t *buf, uint32_t size);

	int GrabCamFrame (unsigned char **previewBuffer, int *previewSize);
	int ReleaseCamFrame();
	sp<IMemory> GrabRawFrame (int CamPixFormat);
	sp<IMemory> GrabJpegFrame (int CamPixFormat);
	void dump(int fd) const;

private:
	int    instance;
	struct vdIn *videoIn;
	int    fd;

	int    nQueued;
	int    nDequeued;
	int    nPreviewBuffer[PREVIEW_BUFFER];
	int    nPreviewIndex;
	int    nReleaseIndex;
	
	int    mWidth, mHeight;
	int    mCounter;
	int    mCheckX, mCheckY;

	int    save_YUYV_to_JPEG (unsigned char *inputBuffer, int width,
					          int height, FILE *file, int quality);

	int    save_MJPEG_to_JPEG (unsigned char *inputBuffer, int width,
				   int height, FILE *file, int quality); 

	int    yuv420_to_jpeg(unsigned char *data, int width, int height, FILE *fp, int quality);
	void   convert_yuv420p_to_yuv420sp(unsigned char *InBuff,
					   unsigned char *OutBuff,
					   int width, int height);
};

}; // namespace android

#endif /* V4L2_CAMERA_H */

