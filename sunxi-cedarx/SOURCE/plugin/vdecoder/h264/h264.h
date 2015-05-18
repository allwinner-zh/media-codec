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
#ifndef h264_H
#define h264_H

#include "h264_config.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct H264_DECODER_CONTEXT
{   
    DecoderInterface    interface;
    VideoEngine*        pVideoEngine;

    VConfig             vconfig;            //* video engine configuration;
    VideoStreamInfo     videoStreamInfo;    //* video stream information;

    void*              pH264Dec;         //* h264 decoder handle

} H264DecCtx;
	
DecoderInterface* CreateH264Decoder(VideoEngine* p);
    
#ifdef __cplusplus
}
#endif

#endif

