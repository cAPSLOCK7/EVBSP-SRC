
#ifndef _EMXX_JPEGLIB_H_
#define _EMXX_JPEGLIB_H_
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
/* SAMPLING */
#define OMF_MC_JPEGD_SAMPLING22             0x22 // 4:1:1(H:V=2:2)
#define OMF_MC_JPEGD_SAMPLING41             0x41 // 4:1:1(H:V=4:1)
#define OMF_MC_JPEGD_SAMPLING21             0x21 // 4:2:2(H:V=2:1)
#define OMF_MC_JPEGD_SAMPLING12             0x12 // 4:2:2(H:V=1:2)
#define OMF_MC_JPEGD_SAMPLING11             0x11 // 4:4:4(H:V=1:1)
#define JPEGE_ROLE   "image_encoder.jpeg"

/* Configration Name */
#define AV_CODEC_CONFIG_FILE_NAME		"/system/lib/omf/omx_av_codec.cfg"

/* MediaComponent Specification */
#define PORT_OFFSET_INPUT			    	(0)
#define PORT_OFFSET_OUTPUT				(1)

int jpege_main(void *Param);
int emxx_yuv_to_jpeg(unsigned char *srcYUVData, int width, int height, FILE *dstJPGfp, 
		                int quality,int yuvType);

#endif /* _EMXX_JPEGLIB_H_ */
