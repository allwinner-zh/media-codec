/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 BZ Chen <bzchen@allwinnertech.com>
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
#ifndef VDECODER_H
#define VDECODER_H

#include <unistd.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*Callback)(void* pUserData, int eMessageId, void* param);

enum VIDEODECCALLBACKMG
{
    VIDEO_DEC_BUFFER_INFO = 0,
    VIDEO_DEC_REQUEST_BUFFER,
    VIDEO_DEC_DISPLAYER_BUFFER,
    VIDEO_DEC_RETURN_BUFFER,
};


enum EVIDEOCODECFORMAT
{
    VIDEO_CODEC_FORMAT_UNKNOWN          = 0,
    VIDEO_CODEC_FORMAT_MPEG1            = 0x101,
    VIDEO_CODEC_FORMAT_MPEG2            = 0x102,
    VIDEO_CODEC_FORMAT_XVID             = 0x103,
    VIDEO_CODEC_FORMAT_H263             = 0x104,
    VIDEO_CODEC_FORMAT_H264             = 0x105,
    VIDEO_CODEC_FORMAT_MJPEG            = 0x106,
    
    VIDEO_CODEC_FORMAT_WMV3             = 0x107,
    VIDEO_CODEC_FORMAT_VP8             = 0x108,

    VIDEO_CODEC_FORMAT_MAX              = VIDEO_CODEC_FORMAT_MJPEG,
    VIDEO_CODEC_FORMAT_MIN              = VIDEO_CODEC_FORMAT_MPEG1,
};

enum EPIXELFORMAT
{
    PIXEL_FORMAT_DEFAULT            = 0,
    
    PIXEL_FORMAT_YUV_PLANER_420     = 1,
    PIXEL_FORMAT_YUV_PLANER_422     = 2,
    PIXEL_FORMAT_YUV_PLANER_444     = 3,
    
    PIXEL_FORMAT_YV12               = 4,
    PIXEL_FORMAT_NV21               = 5,
    PIXEL_FORMAT_NV12               = 6,
    PIXEL_FORMAT_YUV_MB32_420       = 7,
    PIXEL_FORMAT_YUV_MB32_422       = 8,
    PIXEL_FORMAT_YUV_MB32_444       = 9,
    
    PIXEL_FORMAT_MIN = PIXEL_FORMAT_DEFAULT,
    PIXEL_FORMAT_MAX = PIXEL_FORMAT_YUV_MB32_444,
};

enum EVDECODERESULT
{
    VDECODE_RESULT_UNSUPPORTED       = -1,
    VDECODE_RESULT_OK                = 0,
    VDECODE_RESULT_FRAME_DECODED     = 1,
    VDECODE_RESULT_CONTINUE          = 2,
    VDECODE_RESULT_KEYFRAME_DECODED  = 3,
    VDECODE_RESULT_NO_FRAME_BUFFER   = 4,
    VDECODE_RESULT_NO_BITSTREAM      = 5,
    VDECODE_RESULT_RESOLUTION_CHANGE = 6,
    
    VDECODE_RESULT_MIN = VDECODE_RESULT_UNSUPPORTED,
    VDECODE_RESULT_MAX = VDECODE_RESULT_RESOLUTION_CHANGE,
};

//*for new display
typedef struct FBMBUFINFO
{
	int nBufNum;
	int nBufWidth;
	int nBufHeight;
	int nBufLeftOffset;
	int nBufRightOffset;
	int nBufTopOffset;
	int nBufBottomOffset;
	int ePixelFormat;
	int nAlignValue;
    int bProgressiveFlag;
}FbmBufInfo;

typedef struct VIDEOSTREAMINFO
{
    int   eCodecFormat;
    int   nWidth;
    int   nHeight;
    int   nFrameRate;
    int   nFrameDuration;
    int   nAspectRatio;
    int   nCodecSpecificDataLen;
    char* pCodecSpecificData;
}VideoStreamInfo;

typedef struct VCONFIG
{
    int eOutputPixelFormat;
    int nVbvBufferSize;
}VConfig;

typedef struct VIDEOSTREAMDATAINFO
{
    char*   pData;
    int     nLength;
    int64_t nPts;
    int64_t nPcr;
    int     bIsFirstPart;
    int     bIsLastPart;
    int     nID;
    int     nStreamIndex;
    int     bValid;
}VideoStreamDataInfo;


typedef struct VIDEOPICTURE
{
    int     nID;
    int     nStreamIndex;
    int     ePixelFormat;
    int     nWidth;
    int     nHeight;
    int     nLineStride;
    int     nTopOffset;
    int     nLeftOffset;
    int     nBottomOffset;
    int     nRightOffset;
    int     nFrameRate;
    int     nAspectRatio;
    int     bIsProgressive;
    int     bTopFieldFirst;
    int     bRepeatTopField;
    int64_t nPts;
    int64_t nPcr;
    char*   pData0;
    char*   pData1;
    char*   pData2;
    char*   pData3;
    unsigned int phyYBufAddr;
    unsigned int phyCBufAddr;
}VideoPicture;

typedef void* VideoDecoder;

VideoDecoder* CreateVideoDecoder(void);

void DestroyVideoDecoder(VideoDecoder* pDecoder);

int InitializeVideoDecoder(VideoDecoder* pDecoder, VideoStreamInfo* pVideoInfo, VConfig* pVconfig);

void ResetVideoDecoder(VideoDecoder* pDecoder);

int DecodeVideoStream(VideoDecoder* pDecoder, 
                      int           bEndOfStream,
                      int           bDecodeKeyFrameOnly,
                      int           bDropBFrameIfDelay,
                      int64_t       nCurrentTimeUs);

int GetVideoStreamInfo(VideoDecoder* pDecoder, VideoStreamInfo* pVideoInfo);

int RequestVideoStreamBuffer(VideoDecoder* pDecoder,
                             int           nRequireSize,
                             char**        ppBuf,
                             int*          pBufSize,
                             char**        ppRingBuf,
                             int*          pRingBufSize,
                             int           nStreamBufIndex);

int SubmitVideoStreamData(VideoDecoder*        pDecoder,
                          VideoStreamDataInfo* pDataInfo,
                          int                  nStreamBufIndex);

int VideoStreamBufferSize(VideoDecoder* pDecoder, int nStreamBufIndex);

int VideoStreamDataSize(VideoDecoder* pDecoder, int nStreamBufIndex);

int VideoStreamFrameNum(VideoDecoder* pDecoder, int nStreamBufIndex);

VideoPicture* RequestPicture(VideoDecoder* pDecoder, int nStreamIndex);

int ReturnPicture(VideoDecoder* pDecoder, VideoPicture* pPicture);

VideoPicture* NextPictureInfo(VideoDecoder* pDecoder, int nStreamIndex);

int ValidPictureNum(VideoDecoder* pDecoder, int nStreamIndex);

int ReopenVideoEngine(VideoDecoder* pDecoder);
#ifdef __cplusplus
}
#endif

#endif

