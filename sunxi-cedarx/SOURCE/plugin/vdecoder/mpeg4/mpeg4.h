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
#ifndef MPEG4_H
#define MPEG4_H

#include "mp4_vars.h"

#ifdef __cplusplus
extern "C" {
#endif

    
    typedef struct MPEG4_DECODER_CONTEXT
    {
		DecoderInterface    interface;
		VideoEngine*        pVideoEngine;
		

		VConfig             vconfig;            //* video engine configuration;
		VideoStreamInfo     videoStreamInfo;    //* video stream information;
		Sbm*                pSbm;
		uint8_t*                 pStreamBufferBase;
		uint32_t                 nStreamBufferSize;
		Fbm*                pFbm;
		
        MP4_STATE           s;           	//* decode handler
        
        //* memory for hardware using.
        
        VideoPicture*		m_for;
        VideoPicture*		m_bac;
        VideoPicture*		m_cur;
        
        uint32_t				    bframe_num;
        uint8_t				    first_IFrm_flag;
        uint8_t				    need_key_frame;
        
        int32_t				    hasExtraData;
        VideoStreamDataInfo* stream;
        int32_t				     data_len;
        uint8_t				     stream_data_format;		//* 0: contains four bytes length code in bitstream frame;
        										//* 1: not contains length code in bitstream frame;
        										
        uint32_t                  nDecFrmNum;

    }mp4_dec_ctx_t;

    #define VLD_TABLE_DIM 6
	struct item;
	struct mv_item;
	
	typedef struct item item_t;
	typedef struct mv_item mv_item_t;
	
	struct ShortVector
	{
	    short x;
	    short y;
	};
	
	struct item
	{
	    event_t value;
	    char length;
	};
	
	struct mv_item
	{
	    struct ShortVector value;
	    short length;
	};

extern uint32_t mp4_bitpos(MP4_STREAM * ld);
extern uint32_t mp4_showbits (MP4_STREAM * ld, int32_t n);
extern void mp4_flushbits (MP4_STREAM * ld, int32_t n);
extern uint32_t mp4_getbits (MP4_STREAM * ld, int32_t n);
extern uint32_t mp4_getbits1(MP4_STREAM * ld);

#ifdef __cplusplus
}
#endif

#endif

