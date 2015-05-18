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
#ifndef JPEG_DEC_LIB_H
#define JPEG_DEC_LIB_H

#include "mjpeg.h"

#ifdef __cplusplus
extern "C" {
#endif

	//JPEG Define
	
	#define MAX(a,b)	((a) > (b) ? (a) : (b))
	#define MIN(a,b)	((a) < (b) ? (a) : (b))
	
	typedef enum 	/* JPEG marker codes */
	{
		M_SOF0  = (unsigned)0xc0,
		M_SOF1  = (unsigned)0xc1,
		M_SOF2  = (unsigned)0xc2,
		M_SOF3  = (unsigned)0xc3,

		M_SOF5  = (unsigned)0xc5,
		M_SOF6  = (unsigned)0xc6,
		M_SOF7  = (unsigned)0xc7,

		M_JPG   = (unsigned)0xc8,
		M_SOF9  = (unsigned)0xc9,
		M_SOF10 = (unsigned)0xca,
		M_SOF11 = (unsigned)0xcb,

		M_SOF13 = (unsigned)0xcd,
		M_SOF14 = (unsigned)0xce,
		M_SOF15 = (unsigned)0xcf,

		M_DHT   = (unsigned)0xc4,

		M_DAC   = (unsigned)0xcc,

		M_RST0  = (unsigned)0xd0,
		M_RST1  = (unsigned)0xd1,
		M_RST2  = (unsigned)0xd2,
		M_RST3  = (unsigned)0xd3,
		M_RST4  = (unsigned)0xd4,
		M_RST5  = (unsigned)0xd5,
		M_RST6  = (unsigned)0xd6,
		M_RST7  = (unsigned)0xd7,

		M_SOI   = (unsigned)0xd8,
		M_EOI   = (unsigned)0xd9,
		M_SOS   = (unsigned)0xda,
		M_DQT   = (unsigned)0xdb,
		M_DNL   = (unsigned)0xdc,
		M_DRI   = (unsigned)0xdd,
		M_DHP   = (unsigned)0xde,
		M_EXP   = (unsigned)0xdf,

		M_APP0  = (unsigned)0xe0,
		M_APP1  = (unsigned)0xe1,
		M_APP2  = (unsigned)0xe2,
		M_APP3  = (unsigned)0xe3,
		M_APP4  = (unsigned)0xe4,
		M_APP5  = (unsigned)0xe5,
		M_APP6  = (unsigned)0xe6,
		M_APP7  = (unsigned)0xe7,
		M_APP8  = (unsigned)0xe8,
		M_APP9  = (unsigned)0xe9,
		M_APP10 = (unsigned)0xea,
		M_APP11 = (unsigned)0xeb,
		M_APP12 = (unsigned)0xec,
		M_APP13 = (unsigned)0xed,
		M_APP14 = (unsigned)0xee,
		M_APP15 = (unsigned)0xef,

		M_JPG0  = (unsigned)0xf0,
		M_JPG13 = (unsigned)0xfd,
		M_COM   = (unsigned)0xfe,

		M_TEM   = (unsigned)0x01,

		M_ERROR = (unsigned)0x100
	}JPEG_MARKER;
	
	typedef enum
	{
		MJPEG_DEC_INIT           = 0,
		MJPEG_DEC_GET_FBM_BUFFER = 1,
		MJPEG_DEC_GET_STREAM     = 2,
		MJPEG_DEC_STREAM_DATA    = 3,
		MJPEG_DEC_PROCESS_ROTATE = 4,
		MJPEG_DEC_PROCESS_RESULT = 5,
		MJPEG_DEC_Step_
	}MjpegDecStep;

	typedef enum 
	{
		JPEG420 = 0,
		JPEG411 ,
		JPEG422	,
		JPEG444	,
		JPEG422T,
		JPEG400	,
		JPEGERR
	}JpegPaserFormat;
	
	typedef enum
	{
    	JPEG_MARKER_ERROR   =0,
    	JPEG_FORMAT_UNSUPPORT,
		JPEG_PARSER_OK
	}JpegParserState;

	#define MAX_COMPONENTS    3
	#define BITS_IN_JSAMPLE   8
	#define DCTSIZE2		  64
	#define NUM_QUANT_TBLS	  4
	#define NUM_HUFF_TBLS     2
	#define MAX_COMPS_IN_SCAN 4
	
	typedef struct 
	{
		uint16_t startCode[16];
		uint8_t  offset[16];
		uint8_t  symbol[256];
	}JHuffTable;

	typedef struct 
	{
		int32_t   componentId;
		int32_t   componentIndex;
		int16_t   hSampFactor;
		int16_t   vSampFactor;
		int32_t   quantTblNo;

		int32_t   dcTblNo;
		int32_t   acTblNo;
	}JpegComponentInfo;
	


	typedef struct JPEG_DECODER
	{
	    uint8_t                  nDecStramIndex;
	    uint8_t                  nDecStep;

		Sbm*                pSbm;
        uint8_t*             	pVbvBase;
        uint32_t             	nVbvSize;
        Fbm*                pFbm;

        uint8_t*					data;
        uint32_t					nFrameSize;
        
		int32_t					nWidth;
		int32_t 				nHeight;
		int32_t 				nNumComponents;
		int32_t 				nCompsInScan;
		int32_t 				nNextRestartNum;
		int32_t 				nRestartInterval;
		JpegComponentInfo   curCompInfo[MAX_COMPONENTS];

		uint8_t  				sosNbBlocks[MAX_COMPS_IN_SCAN];
		uint8_t  				sosHCount[MAX_COMPS_IN_SCAN];
		uint8_t  				sosVCount[MAX_COMPS_IN_SCAN];
		uint8_t  				seqCompId[MAX_COMPS_IN_SCAN];

		int32_t 				maxHSampFactor;
		int32_t 				maxVSampFactor;
    	    				
		uint16_t 				eoiReached;
		int32_t 				jpegFormat;
		uint16_t 				mcuWidth;
		uint16_t 				mcuHeight;
		int32_t 				isHuffSramA;
		
		uint16_t 				QTab[4][DCTSIZE2];	/* quantization step for each coefficient */
		JHuffTable 			dcHuffTbl[NUM_HUFF_TBLS];
		JHuffTable 			acHuffTbl[NUM_HUFF_TBLS];

		uint8_t  				unreadMarker;
		uint8_t  				sawSOI;
		uint8_t  				sawSOF;
    	    				
		int32_t 				bufferFillOver;
		int64_t  				lastPts;
		
		uint16_t 				hasDht;
		uint16_t 				hasConfigDht;
    	
    	int8_t  				bodyfrmFlag; 		// 0-first frame, !0-not the first frame,so call body frame
    	int8_t    				syncstatFlag;		//if set 1, indicate play mode change, need sync with parser,
    	                  						//and vdrv will clear frame buffer and output frame queue.
    	                  						//It is used for FF, RR, NLY(normal play) switch

		int64_t                 nPts;
		VideoPicture*       pRefPicture;
		uint32_t                 nRefPicLumaAddr;
		uint32_t                 nRefPicChromaAddr;
		uint32_t                 nReconPicLumaAddr;
		uint32_t                 nReconPicChromaAddr;
	}JpegDec;
	
	extern int32_t JpegDecoderMain(MjpegDecodeContext* pMjpegContext,  JpegDec* jpgctx);
	extern int32_t JpegDecodeRaw(JpegDec* p);

#ifdef __cplusplus
}
#endif

#endif

