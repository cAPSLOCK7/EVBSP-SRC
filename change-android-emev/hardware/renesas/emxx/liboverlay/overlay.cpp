/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Overlay"

#include <hardware/hardware.h>
#include <hardware/overlay.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <cutils/log.h>
#include <cutils/ashmem.h>
#include <cutils/atomic.h>

#define CONFIG_VIDEO_EMXX
#define CONFIG_VIDEO_EMXX_FILTER
#include <linux/videodev2.h>
#include <linux/fbcommon.h>
#include <linux/emxx_mem.h>
#include <linux/fb.h>

#define V4L_MAX_BUF	3

#define SHARED_DATA_MARKER	(0x78912345)

/* This will be modified based on the actual length. 
 * FIXME
 */
#define OVERLAY_BUFFER_SIZE 0x00200000
#define OVERLAY_BUFFER_ADDR (EMXX_PMEM_BASE + EMXX_PMEM_SIZE - OVERLAY_BUFFER_SIZE)

/*****************************************************************************/

struct overlay_shared_t {
	unsigned int marker;
	int size;
	volatile int streamEn;
};

struct overlay_control_context_t {
	struct overlay_control_device_t device;
	int ctl_fd;

	int dispW;
	int dispH;

	int width;
	int height;
	int format;
	int x;
	int y;
	int dst_w;
	int dst_h;
};

struct overlay_data_context_t {
	struct overlay_data_device_t device;
	int ctl_fd;

	int shared_fd;
	struct overlay_shared_t  *shared;

	int mmap_fd;
	int map_size;
	unsigned long paddr[V4L_MAX_BUF];
	void *vaddr[V4L_MAX_BUF];

	int buf_num;
	int index;
	int first;
	int num;
	int direct;

	int width;
	int height;
	int size;
	int format;
};

static int overlay_device_open(const struct hw_module_t* module, const char* name,
		struct hw_device_t** device);

static struct hw_module_methods_t overlay_module_methods = {
	open: overlay_device_open
};

struct overlay_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		version_major: 1,
		version_minor: 0,
		id: OVERLAY_HARDWARE_MODULE_ID,
		name: "Sample Overlay module",
		author: "The Android Open Source Project",
		methods: &overlay_module_methods,
	}
};

/*****************************************************************************/

/*
 * This is the overlay_t object, it is returned to the user and represents
 * an overlay.
 * This handles will be passed across processes and possibly given to other
 * HAL modules (for instance video decode modules).
 */

struct handle_t : public native_handle {
	/* add the data fields we need here, for instance: */
	int ctl_fd;
	int shared_fd;
	int width;
	int height;
	int format;
	int buf_num;
	int buf_size;
};

class overlay_object : public overlay_t {

	handle_t mHandle;
	struct overlay_shared_t *mShared;

	static overlay_handle_t getHandleRef(struct overlay_t* overlay) {
		/* returns a reference to the handle, caller doesn't take ownership */
		return &(static_cast<overlay_object *>(overlay)->mHandle);
	}

public:
	overlay_object(int fd, int shared_fd, int w, int h,
			int format, int buf_num, int buf_size) {
		this->overlay_t::getHandleRef = getHandleRef;
		mHandle.version = sizeof(native_handle);
		mHandle.numFds    = 2;
		mHandle.numInts   = 5; // extra ints we have in  our handle
		mHandle.ctl_fd = fd;
		mHandle.shared_fd = shared_fd;
		mHandle.width = w;
		mHandle.height = h;
		mHandle.format = format;
		mHandle.buf_num = buf_num;
		mHandle.buf_size = buf_size;
		this->w = w;
		this->h = h;
		this->format = format;
		this->w_stride = w;
		this->h_stride = h;
	}

	int ctl_fd() { return mHandle.ctl_fd; }
	int shared_fd() { return mHandle.shared_fd; }
	overlay_shared_t* getShared() { return mShared; }
	void              setShared( overlay_shared_t *p ) { mShared = p; }
};


static int handle_fd(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->ctl_fd;
}
static int handle_shared_fd(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->shared_fd;
}
static int handle_width(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->width;
}
static int handle_height(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->height;
}
static int handle_format(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->format;
}
static int handle_buf_num(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->buf_num;
}
static int handle_buf_size(const overlay_handle_t overlay)
{
	return static_cast<const struct handle_t *>(overlay)->buf_size;
}

// ****************************************************************************
// Local Functions
// ****************************************************************************
static int create_shared_data(struct overlay_shared_t **shared)
{
	int fd;
	int size = getpagesize();
	struct overlay_shared_t *p;

	if ((fd = ashmem_create_region("overlay_data", size)) < 0) {
		LOGE("Failed to Create Overlay Shared Data!\n");
		return fd;
	}

	p = (struct overlay_shared_t*)mmap(NULL, size, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		LOGE("Failed to Map Overlay Shared Data!\n");
		close(fd);
		return -1;
	}

	memset(p, 0, size);
	p->marker = SHARED_DATA_MARKER;
	p->size = size;

	*shared = p;
	return fd;
}

static void destroy_shared_data(int shared_fd,
		struct overlay_shared_t *shared, bool closefd )
{
	if (shared == NULL) {
		return;
	}

	if (munmap(shared, shared->size)) {
		LOGE("Failed to Unmap Overlay Shared Data!\n");
	}

	if (closefd && close(shared_fd)) {
		LOGE("Failed to Close Overlay Shared Data!\n");
	}
}

static int open_shared_data( overlay_data_context_t *dctx )
{
	int rc   = -1;
	int mode = PROT_READ | PROT_WRITE;
	int fd   = dctx->shared_fd;
	int size = getpagesize();

	if (dctx->shared != NULL) {
		LOGI("Overlay Shared Data Already Open\n");
		return 0;
	}

	dctx->shared = (struct overlay_shared_t*)mmap(0, size, mode, MAP_SHARED, fd, 0);
	if (dctx->shared == MAP_FAILED) {
		LOGE("Failed to Map Overlay Shared Data!\n");
	} else if ( dctx->shared->marker != SHARED_DATA_MARKER ) {
		LOGE("Invalid Overlay Shared Marker!\n");
		munmap( dctx->shared, size);
	} else if ( (int)dctx->shared->size != size ) {
		LOGE("Invalid Overlay Shared Size!\n");
		munmap(dctx->shared, size);
	} else {
		rc = 0;
	}
	return rc;
}

static void close_shared_data(overlay_data_context_t *dctx)
{
	destroy_shared_data(dctx->shared_fd, dctx->shared, false);
	dctx->shared = NULL;
}

// ****************************************************************************
// V4L2 function
// ****************************************************************************
static int v4l2_set_fb(struct overlay_control_context_t* cctx, int colorkey)
{
	int fb;
	int err;
	scrn_modes screen;
	unsigned int colorkeyTmp;

	colorkeyTmp = (unsigned int) colorkey;
	struct fb_var_screeninfo info;
	if ((fb = open("/dev/graphics/fb0", O_RDWR)) < 0) {
		LOGE("fb open error");
		return -1;
	}

	if (ioctl(fb, FBIOGET_VSCREENINFO, &info) == -1){
		LOGE("FBIOGET_VSCREENINFO error: %s", strerror(errno));
		return -1;
	}

	switch (info.red.offset) {
	case 11: // RGB565
		break;
	case 0:  // BGR888
	case 16: // RGB888
		colorkeyTmp =
			(((((colorkeyTmp & 0xF800) >> 11) << 3) +
			((colorkeyTmp & 0x8000) >> 15) +
			((colorkeyTmp & 0x8000) >> 14) +
			((colorkeyTmp & 0x8000) >> 13)) << 16) |
			(((((colorkeyTmp & 0x07E0) >>  5) << 2) +
			((colorkeyTmp & 0x0400) >> 10) +
			((colorkeyTmp & 0x0400) >>  9)) << 8) |
			(((((colorkeyTmp & 0x001F) >>  0) << 3) +
			((colorkeyTmp & 0x0010) >>  4) +
			((colorkeyTmp & 0x0010) >>  3) +
			((colorkeyTmp & 0x0010) >>  2)) << 0);
		break;
	default:
		break;
	}

	cctx->dispW = info.xres;
	cctx->dispH = info.yres;

	/* Update screen. */
	screen.tc_enable = 0;
	screen.t_color   = (int)colorkeyTmp;
	screen.alpha     = 255;
	screen.page      = 0;
	screen.rot       = 0;
	screen.update    = 1;
	err = ioctl(fb, EMXX_FB_SET_MODES, &screen);
	if (err < 0) {
		LOGE("EMXX_FB_SET_MODES error");
		return -1;
	}
	screen.page      = 1;
	err = ioctl(fb, EMXX_FB_SET_MODES, &screen);
	if (err < 0) {
		LOGE("EMXX_FB_SET_MODES error");
		return -1;
	}

	close(fb);
	return 0;
}

static int v4l2_set_source(int fd, int w, int h, int format)
{
	struct v4l2_format src_format;

	/* Set output format. */
	memset(&src_format, 0, sizeof(struct v4l2_format));
	src_format.type                = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	src_format.fmt.pix.width       = w;
	src_format.fmt.pix.height      = h;
	src_format.fmt.pix.pixelformat = format;
	src_format.fmt.pix.bytesperline= w;
	if (ioctl(fd, VIDIOC_S_FMT, &src_format)) {
		LOGE("VIDIOC_S_FMT error");
		return -1;
	}
	return 0;
}

static int v4l2_set_crop(int fd, int x, int y, int w, int h)
{
	struct v4l2_crop crop;

	memset(&crop, 0, sizeof(struct v4l2_crop));
	crop.type     = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	crop.c.left   = x;
	crop.c.top    = y;
	crop.c.width  = w;
	crop.c.height = h;
	if (ioctl(fd, VIDIOC_S_CROP, &crop)) {
		LOGE("VIDIOC_S_CROP error");
		return -1;
	}
	return 0;
}

static int v4l2_set_rotation(int fd, int angle)
{
	struct v4l2_control ctrl;

	/* Set rotate mode. */
	memset(&ctrl, 0, sizeof(struct v4l2_control));
	ctrl.id    = V4L2_MOVIE_DISPLAY_ANGLE;
	ctrl.value = angle;
	if (ioctl(fd, VIDIOC_S_CTRL, &ctrl)) {
		LOGE("VIDIOC_S_CTRL error");
		return -1;
	}
	return 0;
}

static int v4l2_set_position(int fd, int x, int y, int w, int h)
{
	struct v4l2_format format;

	/* Set overlay format. */
	memset(&format, 0, sizeof(struct v4l2_format));
	format.type             = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	format.fmt.win.w.left   = x;
	format.fmt.win.w.top    = y;
	format.fmt.win.w.width  = w;
	format.fmt.win.w.height = h;
	if (ioctl(fd, VIDIOC_S_FMT, &format)) {
		LOGE("VIDIOC_S_FMT error");
		return -1;
	}
	return 0;
}

static int v4l2_stream_on(int fd)
{
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	if (ioctl(fd, VIDIOC_STREAMON, &type)) {
		LOGE("VIDIOC_STREAMON error\n");
		return -1;
	}
	return 0;
}

static int v4l2_stream_off(int fd)
{
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	if (ioctl(fd, VIDIOC_STREAMOFF, &type)) {
		LOGE("VIDIOC_STREAMOFF error\n");
		return -1;
	}
	return 0;
}

static int v4l2_set_output(int fd, int output)
{
	/* Set V4L2 Output(LCD) */
	if (ioctl(fd, VIDIOC_S_OUTPUT, &output)) {
		LOGE("VIDIOC_S_OUTPUT error");
		return -1;
	}
	return 0;
}

static int v4l2_req_buf(int fd, int buf_num)
{
	struct v4l2_requestbuffers reqbuf;

	/* Set request buffers. */
	reqbuf.count  = buf_num;
	reqbuf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_USERPTR; //The orignal is physical address type
	if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) {
		LOGE("VIDIOC_REQBUFS error");
		return -1;
	}
	return 0;
}

static int v4l2_que_buffer(int fd, int w, int h,
		unsigned long vaddr, int buf_num, struct timeval *t)
{
	struct v4l2_buffer buf;
	int err;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type              = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.timestamp.tv_sec  = t->tv_sec;;
	buf.timestamp.tv_usec = t->tv_usec;
	buf.width             = w;
	buf.height            = h;
	buf.d_width           = w;
	buf.memory            = V4L2_MEMORY_USERPTR; //The original is physical address type
	buf.m.phys_add.PhysAddr_Y  = vaddr;
	buf.m.phys_add.PhysAddr_UV = vaddr + w * h;
	buf.m.phys_add.PhysAddr_V  = vaddr + w * h + (w * h)/4;

	buf.filter.filter_option = SIZ_FILTER_DEFAULT;

	if ((err = ioctl(fd, VIDIOC_QBUF, &buf))) {
		LOGE("VIDIOC_QBUF error: %s", strerror(errno));
		return -1;
	}
	return 0;
}

static int v4l2_deque_buffer(int fd, int buf_num)
{
	struct v4l2_buffer buf;
	int err;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type              = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if ((err = ioctl(fd, VIDIOC_DQBUF, &buf))) {
//		LOGE("VIDIOC_DQBUF error:%s", strerror(errno));
		return -1;
	}
	return 0;
}

static int v4l2_slect_wait(int fd, int time)
{
	int ret;
	struct timeval tval;
	fd_set         fds;

	tval.tv_sec  = 0;
	tval.tv_usec = time;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	ret = select(fd+1, &fds, NULL, NULL, &tval);

	if (ret == 0) {
		LOGE("%s: timeout", __FUNCTION__);
		return -1;
	} else if (ret == -1) {
		LOGE("%s: error", __FUNCTION__);
		return -1;
	} else if (FD_ISSET(fd, &fds)) {
		return 0;
	} else {
		LOGE("%s: error", __FUNCTION__);
		return -1;
	}
}

// ****************************************************************************
// Control module
// ****************************************************************************

static int overlay_get(struct overlay_control_device_t *dev, int name)
{
	int result = -1;

	switch (name) {
	case OVERLAY_MINIFICATION_LIMIT:
		result = 0; // 0 = no limit
		break;
	case OVERLAY_MAGNIFICATION_LIMIT:
		result = 0; // 0 = no limit
		break;
	case OVERLAY_SCALING_FRAC_BITS:
		result = 0; // 0 = infinite
		break;
	case OVERLAY_ROTATION_STEP_DEG:
		result = 90; // 90 rotation steps (for instance)
		break;
	case OVERLAY_HORIZONTAL_ALIGNMENT:
		result = 1; // 1-pixel alignment
		break;
	case OVERLAY_VERTICAL_ALIGNMENT:
		result = 1; // 1-pixel alignment
		break;
	case OVERLAY_WIDTH_ALIGNMENT:
		result = 2; // 2-pixel alignment
		break;
	case OVERLAY_HEIGHT_ALIGNMENT:
		result = 1; // 1-pixel alignment
		break;
	}
	return result;
}

static overlay_t* overlay_createOverlay(struct overlay_control_device_t *dev,
		 uint32_t w, uint32_t h, int32_t format)
{
	overlay_object *overlay;
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;
	struct overlay_shared_t *shared;
	int fd;
	int shared_fd;
	int size, buf_num;
	int tmp_w, tmp_h, tmp;

	if (cctx->ctl_fd != 0) {
		LOGE("Overlays already in use");
		return NULL;
	}

	shared_fd = create_shared_data(&shared);
	if (shared_fd < 0) {
		LOGE("Failed to create shared data");
		return NULL;
	}

	/* Calculate 16-pixel boundary */
	tmp_w = w & 0xfffffff0;
	tmp = w % 16;
	if (tmp)
		tmp_w += 16;
	tmp_h = h & 0xfffffff0;
	tmp = h % 16;
	if (tmp)
		tmp_h += 16;

	switch (format) {
	case OVERLAY_FORMAT_YCbCr_420_SP:
		format = V4L2_PIX_FMT_NV12;
		size = (tmp_w * tmp_h * 3) / 2;
		break;
	case OVERLAY_FORMAT_YCbCr_420_P:
		format = V4L2_PIX_FMT_YUV420;
		size = (tmp_w * tmp_h * 3) / 2;
		break;
	case OVERLAY_FORMAT_YCbCr_422_SP:
		format = V4L2_PIX_FMT_NV422;
		size = tmp_w * tmp_h * 2;
		break;
	case OVERLAY_FORMAT_YCbCr_422_P:
	case OVERLAY_FORMAT_RGBA_8888:
	case OVERLAY_FORMAT_RGB_565:
	case OVERLAY_FORMAT_BGRA_8888:
	default:
		goto error;
	}

	if ((fd = open("/dev/v4l/video0", O_RDWR | O_NONBLOCK)) < 0) {
		LOGE("v4l2 open error %d", errno);
		goto error;
	}

	buf_num = OVERLAY_BUFFER_SIZE / size;
	if (buf_num > V4L_MAX_BUF) {
		buf_num = V4L_MAX_BUF;
	} else if (buf_num == 0){
		buf_num = 1;
	}

	v4l2_set_source(fd, w, h, format);
	v4l2_set_rotation(fd, 0);
	v4l2_req_buf(fd, V4L_MAX_BUF);

	v4l2_set_fb(cctx, OVERLAY_KEY_COLOR);

	if (cctx->dispW == FRONT_WIDTH_1080I && cctx->dispH == FRONT_HEIGHT_1080I) {
		v4l2_set_output(fd, V4L2_OUTPUT_HDMI_1080I);
	} else if (cctx->dispW == FRONT_WIDTH_720P && cctx->dispH == FRONT_HEIGHT_720P) {
		v4l2_set_output(fd, V4L2_OUTPUT_HDMI_720P);
	} else {
		v4l2_set_output(fd, V4L2_OUTPUT_LCD);
	}

	cctx->ctl_fd  = fd;
	cctx->width  = w;
	cctx->height = h;
	cctx->format = format;

	overlay = new overlay_object(fd, shared_fd, w, h, format, buf_num, size);
	if (overlay == NULL) {
		LOGE("Failed to create overlay object\n");
		goto error1;
	}

	overlay->setShared(shared);
	shared->streamEn = 0;

	return overlay;

error1:
	close(fd);
error:
	destroy_shared_data(shared_fd, shared, true);
	return NULL;
}

static void overlay_destroyOverlay(struct overlay_control_device_t *dev, overlay_t* overlay)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;
	overlay_object *obj = static_cast<overlay_object *>(overlay);
	struct overlay_shared_t *shared = obj->getShared();

	if (shared == NULL) {
		LOGE("Overlay was already destroyed - nothing needs to be done\n");
		return;
	}

	destroy_shared_data(obj->shared_fd(), shared, true);
	obj->setShared(NULL);

	close(cctx->ctl_fd);
	cctx->ctl_fd = 0;

	if (overlay) {
		delete overlay;
		overlay = NULL;
	}
}

static int overlay_setPosition(struct overlay_control_device_t *dev,
		 overlay_t* overlay, int x, int y, uint32_t w, uint32_t h)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;

//	LOGE("%s: x=%d y=%d w=%d h=%d", __FUNCTION__, x, y, w, h);

	cctx->x     = x;
	cctx->y     = y;
	cctx->dst_w = w;
	cctx->dst_h = h;

	v4l2_set_position(cctx->ctl_fd, x, y, w, h);

	return 0;
}

static int overlay_getPosition(struct overlay_control_device_t *dev,
		 overlay_t* overlay, int* x, int* y, uint32_t* w, uint32_t* h)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;

	*x = cctx->x;
	*y = cctx->y;
	*w = cctx->dst_w;
	*h = cctx->dst_h;
	return 0;
}

static int overlay_setParameter(struct overlay_control_device_t *dev,
		 overlay_t* overlay, int param, int value)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;
	int result = 0;
	int angle;

//	LOGD("%s: param=%d val=%d", __FUNCTION__, param, value);

	switch (param) {
	case OVERLAY_DITHER:
		break;
	case OVERLAY_TRANSFORM:
		switch (value) {
		case OVERLAY_TRANSFORM_ROT_90:
			angle = 1;
			break;
		case OVERLAY_TRANSFORM_ROT_180:
			angle = 2;
			break;
		case OVERLAY_TRANSFORM_ROT_270:
			angle = 3;
			break;
		default:
			angle = 0;
			break;
		}
		v4l2_set_rotation(cctx->ctl_fd, angle);
		break;
	default:
		result = -EINVAL;
		break;
	}
	return result;
}

static int overlay_stage(struct overlay_control_device_t *dev,
			overlay_t* overlay)
{
	return 0;
}

static int overlay_commit(struct overlay_control_device_t *dev,
			overlay_t* overlay)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;
	overlay_object *obj = static_cast<overlay_object *>(overlay);
	struct overlay_shared_t *shared = obj->getShared();

	if (shared->streamEn == 0) {
		v4l2_stream_on(cctx->ctl_fd);
		shared->streamEn = 1;
	}
	return 0;
}

static int overlay_control_close(struct hw_device_t *dev)
{
	struct overlay_control_context_t* cctx = (struct overlay_control_context_t*)dev;

	if (cctx) {
		free(cctx);
	}
	return 0;
}
 
// ****************************************************************************
// Data module
// ****************************************************************************
int overlay_initialize(struct overlay_data_device_t *dev,
		overlay_handle_t handle)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;
	int fd;
	int i, size;
	struct stat stat;

	dctx->ctl_fd  = handle_fd(handle);
	dctx->shared_fd = handle_shared_fd(handle);
	dctx->width   = handle_width(handle);
	dctx->height  = handle_height(handle);
	dctx->format  = handle_format(handle);
	dctx->buf_num = handle_buf_num(handle);
	dctx->size      = handle_buf_size(handle);

	size = dctx->size;

//	LOGD("%s: fd=%d w=%d h=%d num=%d size=%d\n", __FUNCTION__, 
//		dctx->ctl_fd, dctx->width, dctx->height, dctx->buf_num, size);

	if (fstat(dctx->ctl_fd, &stat)) {
		LOGE("Error = %s from %s", strerror(errno), "overlay initialize");
		return -1;
	}

	if (open_shared_data(dctx)) {
		return -1;
	}

	fd = open("/dev/siz", O_RDWR | O_SYNC);
	if (fd < 0) {
		LOGE("/dev/siz open error");
		LOGE("[overlay] %s", strerror(errno));
		close_shared_data(dctx);
		return -1;
	}

	dctx->map_size = size * dctx->buf_num;

	//Don't use physical address anymore
	dctx->paddr[0] = OVERLAY_BUFFER_ADDR;

	/* FIXME, need to change according to the real mmap method */
	dctx->vaddr[0] = mmap(0, dctx->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OVERLAY_BUFFER_ADDR);
	if((int)dctx->vaddr[0] == -1 ){
		LOGE("[overlay]overlay initialize failed:%s", strerror(errno));
		close(fd);
		dctx->mmap_fd = 0;
		close_shared_data(dctx);
		return -1;
	}

	for (i = 1; i < dctx->buf_num; i++) {
		dctx->paddr[i] = dctx->paddr[0] + i*size;
		dctx->vaddr[i] = (void *)((unsigned int)dctx->vaddr[0] + i*size);
	}

	dctx->mmap_fd  = fd;
	dctx->index    = 0;
	dctx->first    = 0;
	dctx->num      = 0;
	dctx->direct   = 0;

	return 0;
}

static int overlay_resizeInput(struct overlay_data_device_t *dev, uint32_t w,
			uint32_t h)
{
	return 0;
}

static int overlay_data_setParameter(struct overlay_data_device_t *dev,
			int param, int value)
{
	return 0;
}

static int overlay_setCrop(struct overlay_data_device_t *dev, uint32_t x,
			uint32_t y, uint32_t w, uint32_t h)
{
	return 0;
}

static int overlay_getCrop(struct overlay_data_device_t *dev , uint32_t* x,
			uint32_t* y, uint32_t* w, uint32_t* h)
{
	return 0;
}

int overlay_dequeueBuffer(struct overlay_data_device_t *dev,
			  overlay_buffer_t* buf)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;
	struct overlay_buffer_emxx *buffer = (struct overlay_buffer_emxx *)*buf;

	if (dctx->num > 1) {
		if (dctx->num >= dctx->buf_num) {
			v4l2_slect_wait(dctx->ctl_fd, 500 * 1000);
			while (v4l2_deque_buffer(dctx->ctl_fd, dctx->buf_num) == 0) {
				dctx->num--;
			}
		}
	}

	buffer->index = dctx->index;
	buffer->time.tv_sec = -1;
	buffer->time.tv_usec = -1;
	buffer->vaddr = (void *)dctx->vaddr[dctx->index];
	buffer->paddr = (void *)dctx->paddr[dctx->index];
	if (++dctx->index >= dctx->buf_num) dctx->index = 0;

	return 0;
}

int overlay_queueBuffer(struct overlay_data_device_t *dev, overlay_buffer_t buf)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;
	struct overlay_buffer_emxx *buffer = (struct overlay_buffer_emxx*)buf;
	int index = buffer->index;
	int ret;
	int c_width = 0, c_height = 0;

//	LOGD("index=%d num=%d addr=%lx", index, dctx->num,  buffer->vaddr);

	if (dctx->shared->streamEn == 1) {
		switch (dctx->format) {
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV422:
			c_width = dctx->width + ((buffer->w_src - dctx->width) % 4);
			c_height = dctx->height + ((buffer->h_src - dctx->height) % 4);
			break;
		case V4L2_PIX_FMT_YUV420:
			c_width = dctx->width + ((buffer->w_src - dctx->width) % 8);
			c_height = dctx->height + ((buffer->h_src - dctx->height) % 8);
			break;
		}
		v4l2_set_crop(dctx->ctl_fd, 0, 0, c_width, c_height);

		if (dctx->first == 0) {
			dctx->first = 1;

			ret = v4l2_que_buffer(dctx->ctl_fd, buffer->w_src, buffer->h_src,
				(unsigned long)buffer->vaddr, dctx->buf_num, &buffer->time);
			if (ret == 0) {
				dctx->direct = 1;
				dctx->buf_num = V4L_MAX_BUF;
				dctx->num++;
			} else {
				dctx->direct = 0;
				memcpy(dctx->vaddr[index], buffer->vaddr, dctx->size);
				ret = v4l2_que_buffer(dctx->ctl_fd, buffer->w_src, buffer->h_src,
					(unsigned long)dctx->vaddr[index], dctx->buf_num, &buffer->time);
				if (ret == 0) {
					dctx->num++;
				}
			}
		} else {
			if (dctx->direct == 0) {
				memcpy(dctx->vaddr[index], buffer->vaddr, dctx->size);
				ret = v4l2_que_buffer(dctx->ctl_fd, buffer->w_src, buffer->h_src,
					(unsigned long)dctx->vaddr[index], dctx->buf_num, &buffer->time);
			} else {
				ret = v4l2_que_buffer(dctx->ctl_fd, buffer->w_src, buffer->h_src,
					(unsigned long)buffer->vaddr, dctx->buf_num, &buffer->time);
			}
			if (ret == 0) {
				dctx->num++;
			}
		}
	}
	return 0;
}

void *overlay_getBufferAddress(struct overlay_data_device_t *dev, overlay_buffer_t buf)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;
	struct overlay_buffer_emxx *buffer = (struct overlay_buffer_emxx*)buf;

	return (void *)dctx->vaddr[buffer->index];
}

int overlay_getBufferCount(struct overlay_data_device_t *dev)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;
	return dctx->buf_num;
}

static int overlay_data_close(struct hw_device_t *dev)
{
	struct overlay_data_context_t* dctx = (struct overlay_data_context_t*)dev;

	if (dctx) {
		if (dctx->mmap_fd > 0) {
			munmap(dctx->vaddr[0], dctx->map_size);
			close(dctx->mmap_fd);
		}
		close_shared_data(dctx);
		free(dctx);
	}
	return 0;
}

/*****************************************************************************/

static int overlay_device_open(const struct hw_module_t* module, const char* name,
		struct hw_device_t** device)
{
	int status = -EINVAL;

	if (!strcmp(name, OVERLAY_HARDWARE_CONTROL)) {
		struct overlay_control_context_t *dev;
		dev = (overlay_control_context_t*)malloc(sizeof(*dev));

		/* initialize our state here */
		memset(dev, 0, sizeof(*dev));

		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module = const_cast<hw_module_t*>(module);
		dev->device.common.close = overlay_control_close;
		
		dev->device.get = overlay_get;
		dev->device.createOverlay = overlay_createOverlay;
		dev->device.destroyOverlay = overlay_destroyOverlay;
		dev->device.setPosition = overlay_setPosition;
		dev->device.getPosition = overlay_getPosition;
		dev->device.setParameter = overlay_setParameter;
		dev->device.stage = overlay_stage;
		dev->device.commit = overlay_commit;

		*device = &dev->device.common;
		status = 0;
	} else if (!strcmp(name, OVERLAY_HARDWARE_DATA)) {
		struct overlay_data_context_t *dev;
		dev = (overlay_data_context_t*)malloc(sizeof(*dev));

		/* initialize our state here */
		memset(dev, 0, sizeof(*dev));

		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module = const_cast<hw_module_t*>(module);
		dev->device.common.close = overlay_data_close;

		dev->device.initialize = overlay_initialize;
		dev->device.resizeInput = overlay_resizeInput;
		dev->device.setCrop = overlay_setCrop;
		dev->device.getCrop = overlay_getCrop;
		 dev->device.setParameter = overlay_data_setParameter;
		dev->device.dequeueBuffer = overlay_dequeueBuffer;
		dev->device.queueBuffer = overlay_queueBuffer;
		dev->device.getBufferAddress = overlay_getBufferAddress;
		dev->device.getBufferCount = overlay_getBufferCount;

		*device = &dev->device.common;
		status = 0;
	}
	return status;
}
