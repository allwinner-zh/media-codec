/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
*
* This file is part of Cedarx.
*
* Cedarx is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*/
#ifndef MPEG4_CONFIG_H
#define MPEG4_CONFIG_H

#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
//#include "typedef.h"
#include "videoengine.h"
#include "veregister.h"


#ifdef __cplusplus
extern "C" {
#endif

	#define VALID   1
	#define INVALID 0

	#define VE_MODE_MPEG124	0
	

	typedef enum
	{
    	PIA_FID_UNDEFINED=0,
    	PIA_FID_RGB4,
    	PIA_FID_CLUT8,
    	PIA_FID_XRGB16_1555,
    	PIA_FID_RGB16_565,
    	PIA_FID_RGB16_655,
    	PIA_FID_RGB16_664,
    	PIA_FID_RGB24,
    	PIA_FID_XRGB32,
    	PIA_FID_MPEG2V,
    	PIA_FID_YVU9,
    	PIA_FID_YUV12,
    	PIA_FID_IYUV,
    	PIA_FID_YV12,
    	PIA_FID_YUY2,
    	PIA_FID_BITMAP16,
    	PIA_FID_H261,
    	PIA_FID_H263,
    	PIA_FID_H263PLUS,
    	PIA_FID_TROMSO,
    	PIA_FID_ILVR,
    	PIA_FID_REALVIDEO21,
    	PIA_FID_REALVIDEO22,
    	PIA_FID_REALVIDEO30,
    	PIA_FID_

	}SubIDVersion;
	
	#define MPEG_REGS_BASE 	((uint32_t)ve_get_reglist(REG_GROUP_MPEG_DECODER))
    #define VC1_REGS_BASE   ((uint32_t)ve_get_reglist(REG_GROUP_VC1_DECODER))

#ifdef __cplusplus
}
#endif

#endif

