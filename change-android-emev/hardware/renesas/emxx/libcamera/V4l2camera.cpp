/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.00000
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "V4L2Camera"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "V4l2camera.h"

#define USE_HW_JPG_LIB            1
#define ENCODE_NO_SUPPORT_YUV420P 1   //encodec don't support yuv420p

//************debug macro**********
#define DEBUG_CAM 0
#if DEBUG_CAM
	#define PRINT_DEBUG_MSG LOGD("[debug] V4L2Camera::%s,%d",__FUNCTION__,__LINE__)
#else
	#define PRINT_DEBUG_MSG
#endif
#define CAMERA_DUMP_PREVIEW_TO_FILE 0
#define CAMERA_DUMP_ENCODE_TO_FILE  0
#define PREVIEW_FILE_NAME                  "/data/campreview.422"
#define PREVIEW_TAKE_PICTURE_FILE_NAME     "/data/camptakepicture.422"
#define ENCODE_FILE_NAME                   "/data/camenc420_convert_after.yuv"
#define ENCODE_CONVERT_BEFORE_FILE_NAME    "/data/camenc420_convert_before.yuv"
#define CAMERA_TEMP_JPG_FILE_NAME          "/data/camtemp.jpg"
#define CAMERA_JPEG_COMPRESSION_QUALITY 100

//*********************************
#define  RGB888_TO_RGB565(a, b, c, out)\
		 {\
			 unsigned char r = a & 0xF8; \
			 unsigned char g = b & 0xFC; \
			 unsigned char b = c & 0xF8; \
			 out = ((unsigned short )r << 8) | ((unsigned short )g << 3) | b >> 3; \
		 }

namespace android {

V4L2Camera::V4L2Camera ()
	: nQueued(0), nDequeued(0)
{
	PRINT_DEBUG_MSG;
	videoIn = (struct vdIn *) calloc (1, sizeof (struct vdIn));
	instance = 0;
}

V4L2Camera::~V4L2Camera()
{
	PRINT_DEBUG_MSG;
	free(videoIn);
}

int V4L2Camera::Open (const char *device)
{
	PRINT_DEBUG_MSG;
#if DEBUG_CAM
	LOGD("[debug] V4L2Camera::%s %d device path = %s",__FUNCTION__,__LINE__,device);
#endif

	if ((fd = open(device, O_RDWR)) == -1) {
		LOGE("ERROR opening V4L interface: %s", strerror(errno));
		return -1;
	}
	instance++;
	return 0;
}

int V4L2Camera::QueryCapabilities()
{
	PRINT_DEBUG_MSG;

	int ret = 0;
	ret = ioctl (fd, VIDIOC_QUERYCAP, &videoIn->cap);
	if (ret < 0) {
		LOGE("Error opening device: unable to query device: %s",strerror(errno));
		return ret;
	}

	return 0;
}

bool V4L2Camera::IsVideoCaptureSupported()
{
	PRINT_DEBUG_MSG;

	if (videoIn->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		return true;
	else
		return false;
}

bool V4L2Camera::IsStreamingSupported()
{
	PRINT_DEBUG_MSG;

	if (videoIn->cap.capabilities & V4L2_CAP_STREAMING)
		return true;
	else
		return false;
}

int V4L2Camera::GetCameraParameters(int *width, int *height, int *pixelFormat)
{
	PRINT_DEBUG_MSG;

	int ret = 0;
	videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_FMT, &videoIn->format);
	if (ret < 0) {
		LOGE("VIDIOC_G_FMT Failed: %s", strerror(errno));
		return ret;
	}

#if DEBUG_CAM
	LOGD("[debug] videoIn->format.type: %d\n", videoIn->format.type);
	LOGD("[debug] videoIn->format.fmt.pix.width: %d\n",
		  videoIn->format.fmt.pix.width);
	LOGD("[debug] videoIn->format.fmt.pix.height: %d\n",
		  videoIn->format.fmt.pix.height);
	LOGD("[debug] videoIn->format.fmt.pix.pixelformat: 0x%x\n",
		  videoIn->format.fmt.pix.pixelformat);
	LOGD("[debug] videoIn->format.fmt.pix.field: %d\n",
		  videoIn->format.fmt.pix.field);
	LOGD("[debug] videoIn->format.fmt.pix.bytesperline: %d\n",
		  videoIn->format.fmt.pix.bytesperline);
	LOGD("[debug] videoIn->format.fmt.pix.sizeimage: 0x%x\n",
		  videoIn->format.fmt.pix.sizeimage);
	LOGD("[debug] videoIn->format.fmt.pix.colorspace: %d\n",
		  videoIn->format.fmt.pix.colorspace);
#endif

	*width = videoIn->format.fmt.pix.width;
	*height = videoIn->format.fmt.pix.height;
	*pixelFormat = videoIn->format.fmt.pix.pixelformat;
	return 0;
}

int V4L2Camera::SetCameraParameters(int width, int height, int pixelFormat)
{
	PRINT_DEBUG_MSG;
#if DEBUG_CAM
	LOGD("[debug] V4L2Camera::%s:%d width=%d, height=%d\n",__FUNCTION__,__LINE__, width, height);
#endif

	int ret = 0;
	videoIn->width = width;
	videoIn->height = height;
	videoIn->framesizeIn = (width * height << 1);
	videoIn->formatIn = pixelFormat;

	videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->format.fmt.pix.width = width;
	videoIn->format.fmt.pix.height = height;
	videoIn->format.fmt.pix.pixelformat = pixelFormat;

#if DEBUG_CAM
	LOGD("[debug] videoIn->format.fmt.pix.width: %d\n",
		  videoIn->format.fmt.pix.width);
	LOGD("[debug] videoIn->format.fmt.pix.height: %d\n",
		  videoIn->format.fmt.pix.height);
	LOGD("[debug] videoIn->format.fmt.pix.pixelformat: 0x%x\n",
		  videoIn->format.fmt.pix.pixelformat);
#endif

	ret = ioctl(fd, VIDIOC_S_FMT, &videoIn->format);
	if (ret < 0) {
		LOGE("Open: VIDIOC_S_FMT Failed: %s", strerror(errno));
		return ret;
	}

	return 0;
}

void V4L2Camera::Close ()
{
	PRINT_DEBUG_MSG;

	close(fd);
	instance--;
}

int V4L2Camera::Init()
{
	PRINT_DEBUG_MSG;
	LOGI("V4L2Camera: Init");
	int ret = 0;

	/* Check if camera can handle NB_BUFFER buffers */
	videoIn->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->rb.memory = V4L2_MEMORY_MMAP;
	videoIn->rb.count = NB_BUFFER;

	ret = ioctl(fd, VIDIOC_REQBUFS, &videoIn->rb);
	if (ret < 0) {
		LOGE("Init: VIDIOC_REQBUFS failed: %s", strerror(errno));
		return ret;
	}
#if DEBUG_CAM
	LOGD("[debug] V4L2Camera::%s:%d: VIDIOC_QUERYBUF = %x ",__FUNCTION__,__LINE__,VIDIOC_QUERYBUF);
#endif

	for (int i = 0; i < NB_BUFFER; i++) {
		memset (&videoIn->buf, 0, sizeof (struct v4l2_buffer));

		videoIn->buf.index = i;
		videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		videoIn->buf.memory = V4L2_MEMORY_MMAP;

		ret = ioctl (fd, VIDIOC_QUERYBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("Init: Unable to query buffer-index=%d (%s)", i,strerror(errno));
			return ret;
		}

		videoIn->mem[i] = mmap (0, videoIn->buf.length,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, videoIn->buf.m.offset);

		if (videoIn->mem[i] == MAP_FAILED) {
			LOGE("Init: Unable to map buffer: videoIn->buf.length = %d,videoIn->buf.m.offset = %d (%s)",
			videoIn->buf.length,videoIn->buf.m.offset,strerror(errno));
			return -1;
		}

		ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("Init: VIDIOC_QBUF Failed(%s)",strerror(errno));
			return -1;
		}
		nQueued++;
	}

	for (int i = 0; i < PREVIEW_BUFFER; i++)
		nPreviewBuffer[i] = -1;

	nPreviewIndex = 1;
	nReleaseIndex = 0;

	return 0;
}

void V4L2Camera::Uninit ()
{
	PRINT_DEBUG_MSG;
	LOGI("V4L2Camera::Uninit");

	int ret;
	videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;
#if 0
	/* Dequeue everything */
	int DQcount = nQueued - nDequeued;

	for (int i = 0; i < DQcount-1; i++) {
	ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("Uninit: VIDIOC_DQBUF Failed (%s)",strerror(errno));
			break;
		}
	}
	nQueued = 0;
	nDequeued = 0;
#endif
	/* Unmap buffers */
	for (int i = 0; i < NB_BUFFER; i++)
		if (munmap(videoIn->mem[i], videoIn->buf.length) < 0)
			LOGE("Uninit: Unmap failed ");
}

int V4L2Camera::StartStreaming ()
{
	PRINT_DEBUG_MSG;

	enum v4l2_buf_type type;
	int ret;

	LOGI("V4L2Camera::StartStreaming");

	if (!videoIn->isStreaming) {
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		ret = ioctl (fd, VIDIOC_STREAMON, &type);
		if (ret < 0) {
			LOGE("StartStreaming: Unable to start capture: %s", strerror(errno));
			return ret;
		}

		videoIn->isStreaming = true;
	}

#if CAMERA_DUMP_PREVIEW_TO_FILE  //use for debug
	int fd2 = open(PREVIEW_FILE_NAME, O_RDWR | O_CREAT);
	if (fd2 < 0) {
		LOGE("[debug] failed to create file [%s]: %s", PREVIEW_FILE_NAME, strerror(errno));
	}
	close(fd2);
#endif

#if CAMERA_DUMP_ENCODE_TO_FILE //use for debug
	int fd3 = open(ENCODE_FILE_NAME, O_RDWR | O_CREAT);
	if (fd3 < 0) {
		LOGE("[debug] failed to create file [%s]: %s", ENCODE_FILE_NAME, strerror(errno));
	}
	close(fd3);
#endif
	return 0;
}

int V4L2Camera::StopStreaming ()
{
	PRINT_DEBUG_MSG;

	enum v4l2_buf_type type;
	int ret;

	LOGI("V4L2Camera::StopStreaming");

	if (videoIn->isStreaming) {
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		ret = ioctl (fd, VIDIOC_STREAMOFF, &type);
		if (ret < 0) {
			LOGE("StopStreaming: Unable to stop capture: %s", strerror(errno));
			return ret;
		}

		videoIn->isStreaming = false;
	}

	return 0;
}

void V4L2Camera::Camera_dump_to_file(const char *fname,
					     uint8_t *buf, uint32_t size)
{
	int nw, cnt = 0;
	uint32_t written = 0;

	LOGD("[debug] V4L2Camera::%s opening file [%s]\n",__FUNCTION__, fname);
	int fd2 = open(fname, O_RDWR | O_APPEND);
	if (fd2 < 0) {
		LOGE("failed to create file [%s]: %s", fname, strerror(errno));
		return;
	}

	LOGD("[debug] V4L2Camera::%s writing %d bytes to file [%s]\n", __FUNCTION__,size, fname);
	while (written < size) {
		nw = ::write(fd2, buf + written, size - written);
		if (nw < 0) {
			LOGE("failed to write to file [%s]: %s",
				 fname, strerror(errno));
			break;
		}
		written += nw;
		cnt++;
	}
	LOGD("[debug] V4L2Camera::%s done writing %d bytes to file [%s] in %d passes\n",__FUNCTION__,
		 size, fname, cnt);
	::close(fd2);
}

int V4L2Camera::GrabCamFrame (unsigned char **previewBuffer, int *previewSize)
{
	PRINT_DEBUG_MSG;

#if DEBUG_CAM
	LOGD("[debug] V4L2Camera::GrabCamFrame");
	LOGD("[debug] V4L2Camera::%s:%d:CamPixFormat = %x,PreviewPixFormat = %x",__FUNCTION__,__LINE__,CamPixFormat ,PreviewPixFormat);
#endif

	unsigned char *tmpBuffer;
	int ret = 0;

	videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;

	while(1) {
		fd_set fds;
		struct  timeval tv;
		int  r;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select(fd + 1, &fds, NULL, NULL, &tv);
		if  (-1 == r) {
			LOGE("Select error");
			return -1;
		}
		if  (0 == r) {
			LOGE("Time out");
			return 1;
		}

		/* DQ */
		ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("GrabPreviewFrame: VIDIOC_DQBUF Failed: %s", strerror(errno));
			return ret;
		}
		nDequeued++;

		tmpBuffer = (unsigned char *)videoIn->mem[videoIn->buf.index];
		nPreviewBuffer[nPreviewIndex] = videoIn->buf.index;
		if (++nPreviewIndex >= PREVIEW_BUFFER)
			nPreviewIndex = 0;


	#if CAMERA_DUMP_PREVIEW_TO_FILE
		Camera_dump_to_file(PREVIEW_FILE_NAME,(uint8_t *)tmpBuffer, (size_t) videoIn->buf.bytesused);
	#endif

		*previewBuffer = tmpBuffer;
		*previewSize = videoIn->buf.bytesused;
		break;
	}
	PRINT_DEBUG_MSG;
	return 0;
}

int V4L2Camera::ReleaseCamFrame ()
{
	int ret;
	if (nPreviewBuffer[nReleaseIndex] != -1) {
		videoIn->buf.index = nPreviewBuffer[nReleaseIndex];
		videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		videoIn->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("GrabCamFrame: VIDIOC_QBUF Failed: ret=%x", ret);
			return ret;
		}
		nQueued++;
		nPreviewBuffer[nReleaseIndex] = -1;
		if (++nReleaseIndex >= PREVIEW_BUFFER)
			nReleaseIndex = 0;
	}
	return 0;
}

void V4L2Camera::convert_yuv420p_to_yuv420sp(unsigned char *InBuff,
                                             unsigned char *OutBuff,
                                             int width, int height)
{
	PRINT_DEBUG_MSG;
	unsigned char *pBuffer=NULL;
	unsigned char *pTmp=NULL;
	unsigned int *pCbCr=NULL;
	int i;
	int idx;
	int pixels;

	pixels = width * height;
	pBuffer = (unsigned char *)malloc(pixels);
	// Read Y
	memcpy(pBuffer, InBuff, (size_t)pixels);
	// Write Y
	memcpy(OutBuff, pBuffer, (size_t)pixels);

	// Read Cb
	memcpy(pBuffer, InBuff + pixels, (size_t)pixels/4);
	pTmp = pBuffer;
	// Write Cb
	for (idx = 0; idx <  (pixels/2); idx+=2) {
	    memcpy(OutBuff + pixels+idx, pTmp++, (size_t)1);
	}

	// Read Cr
	memcpy(pBuffer, InBuff + pixels*5/4, (size_t)pixels/4);

	pTmp = pBuffer;
	// Write Cr
	for (idx = 0; idx <  (pixels/2); idx+=2) {
	    memcpy(OutBuff+pixels+1+idx, pTmp++, (size_t)1);
	}
	free(pBuffer);
	return;
}

sp<IMemory> V4L2Camera::GrabRawFrame (int CamPixFormat)
{
	PRINT_DEBUG_MSG;

	unsigned char *tmpBuffer;
	int ret = 0;

	LOGD("GrabRawFrame");

	sp<MemoryHeapBase> memHeap = \
	new MemoryHeapBase(videoIn->width * videoIn->height * 2);

	sp<MemoryBase> memBase = \
	new MemoryBase(memHeap, 0, videoIn->width * videoIn->height * 2);

	void *RawFrameBase = memHeap->base();

	videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;

#define SKIP_ADJUST_FRAME 20
	/* Skip first 20 frames till camera adjusts autofocus */
	for (int i = 0; i < (SKIP_ADJUST_FRAME + 1); i++) {
		/* DQ */
		ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("GrabRawFrame: VIDIOC_DQBUF Failed %s", strerror(errno));
			goto Exit;
		}
		nDequeued++;

		ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("GrabRawFrame: VIDIOC_QBUF Failed %s", strerror(errno));
			goto Exit;
		}
		nQueued++;
	}

	tmpBuffer = (unsigned char*)videoIn->mem[videoIn->buf.index];

	if (CamPixFormat == V4L2_PIX_FMT_YUV420) {
		memcpy((void *)RawFrameBase, (void *)tmpBuffer, (size_t) videoIn->buf.bytesused);
	} else{
		LOGE("--Unsupported Camera Format");
	}

Exit:
	return memBase;
}

sp<IMemory> V4L2Camera::GrabJpegFrame (int CamPixFormat)
{
	PRINT_DEBUG_MSG;

	FILE *output;
	FILE *input;
	int fileSize = 0;
	int ret = 0;

	LOGD("GrabJpegFrame");

	videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;

	/* Dequeue buffer */
	ret = ioctl(fd, VIDIOC_DQBUF, &videoIn->buf);
	if (ret < 0) {
		LOGE("GrabJpegFrame: VIDIOC_DQBUF Failed %s", strerror(errno));
		return NULL;
	}
	nDequeued++;

#if DEBUG_CAM
	LOGD("GrabJpegFrame: Generated a frame from capture device");
	unsigned char *tmpBuffer;
	tmpBuffer = (unsigned char *)videoIn->mem[videoIn->buf.index];

#if CAMERA_DUMP_PREVIEW_TO_FILE
	Camera_dump_to_file(PREVIEW_TAKE_PICTURE_FILE_NAME,(uint8_t *)tmpBuffer, (size_t) videoIn->buf.bytesused);
#endif
#endif

	/* Enqueue buffer */
	ret = ioctl(fd, VIDIOC_QBUF, &videoIn->buf);
	if (ret < 0) {
		LOGE("GrabJpegFrame: VIDIOC_QBUF Failed %s", strerror(errno));
		return NULL;
	}
	nQueued++;

	output = fopen(CAMERA_TEMP_JPG_FILE_NAME, "wb");

	if (output == NULL) {
		LOGE("GrabJpegFrame: Ouput file == NULL");
		return NULL;
	}

	/* range is 1-100 */
	if (JpegQuality == 0 || JpegQuality > 100)
		JpegQuality = CAMERA_JPEG_COMPRESSION_QUALITY;

#if DEBUG_CAM
	LOGD("V4L2Camera::GrabJpegFrame JpegQuality=%d", JpegQuality);
#endif

	if (CamPixFormat == V4L2_PIX_FMT_YUV420) {
		fileSize = yuv420_to_jpeg((unsigned char *)videoIn->mem[videoIn->buf.index],
					videoIn->width, videoIn->height, output,
					JpegQuality);
		PRINT_DEBUG_MSG;
	} else if (CamPixFormat == V4L2_PIX_FMT_MJPEG) {
		fileSize = save_MJPEG_to_JPEG((unsigned char *)videoIn->mem[videoIn->buf.index],
			videoIn->width, videoIn->height, output,
			JpegQuality);
		PRINT_DEBUG_MSG;
	}
	fclose(output);

	input = fopen(CAMERA_TEMP_JPG_FILE_NAME, "rb");

	if (input == NULL)
		LOGE("GrabJpegFrame: Input file == NULL");
	else {
		sp<MemoryHeapBase> mjpegPictureHeap = \
			new MemoryHeapBase(fileSize);
		sp<MemoryBase> jpegmemBase = \
			new MemoryBase(mjpegPictureHeap, 0, fileSize);

		fread((uint8_t *)mjpegPictureHeap->base(), 1, fileSize, input);
		fclose(input);

		return jpegmemBase;
	}

	return NULL;
}


int V4L2Camera::save_MJPEG_to_JPEG (unsigned char *inputBuffer, int width,
					                int height, FILE *file, int quality)
{
	PRINT_DEBUG_MSG;

	int i = 0, counter = 0;
	while(height--) {
		for (i = 0; i < width*2; i++) {
			fprintf(file, "%c", *inputBuffer++);
			counter++;
		}
	}
	return counter;
}

int V4L2Camera:: yuv420_to_jpeg(unsigned char *data, int width, int height, FILE *fp, int quality)
{
#if USE_HW_JPG_LIB
	PRINT_DEBUG_MSG;
	int fileSize = 0;
	int ret = emxx_yuv_to_jpeg(data,width,height,fp,quality,OMF_MC_JPEGD_SAMPLING22);
	if (ret != 0) {
		LOGE("yuv file convert to jpeg file failed!!!!");
	}
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
#else
	PRINT_DEBUG_MSG;
	int fileSize = 0; 
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1]; 
	int row_stride;
	JSAMPIMAGE  buffer;
	unsigned char *pSrc,*pDst;
	int band,i,buf_width[3],buf_height[3];
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = width; 
	cinfo.image_height = height;
	cinfo.input_components = 3; 
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE );
	cinfo.raw_data_in = TRUE;
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;

	jpeg_start_compress(&cinfo, TRUE);
	buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY)); 

	for (band=0; band <3; band++) {
		buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
		buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
		buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo,
					JPOOL_IMAGE, buf_width[band], buf_height[band]);
	} 
	unsigned char *rawData[3];
	rawData[0]=data;
	rawData[1]=data+width*height;
	rawData[2]=data+width*height*5/4;

	int src_width[3],src_height[3];
	for (int i=0;i<3;i++){
		src_width[i]=(i==0)?width:width/2;
		src_height[i]=(i==0)?height:height/2;
	}
	int max_line = cinfo.max_v_samp_factor*DCTSIZE; 
	for (int counter=0; cinfo.next_scanline < cinfo.image_height; counter++) {
		//buffer image copy.
		for (band=0; band <3; band++){
			int mem_size = src_width[band];
			pDst = (unsigned char *) buffer[band][0];
			pSrc = (unsigned char *) rawData[band] + counter*buf_height[band] * src_width[band];
			for (i=0; i <buf_height[band]; i++){
				memcpy(pDst, pSrc, mem_size);
				pSrc += src_width[band];
				pDst += buf_width[band];
			}
		}
		jpeg_write_raw_data(&cinfo, buffer, max_line);
	}
	jpeg_finish_compress(&cinfo);
	fileSize = ftell(fp);
	jpeg_destroy_compress(&cinfo);
#endif
	return fileSize;
}

void V4L2Camera::dump(int fd) const
{
	PRINT_DEBUG_MSG;
}
}; // namespace android
