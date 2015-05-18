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
#ifndef VIDEOENGINE_H
#define VIDEOENGINE_H

#include "vdecoder.h"
#include "sbm.h"
#include "fbm.h"



typedef struct DECODERINTERFACE DecoderInterface;
struct DECODERINTERFACE
{
    int  (*Init)(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo);
    void (*Reset)(DecoderInterface* pSelf);
    int  (*SetSbm)(DecoderInterface* pSelf, Sbm* pSbm, int nIndex);
    int  (*GetFbmNum)(DecoderInterface* pSelf);
    Fbm* (*GetFbm)(DecoderInterface* pSelf, int nIndex);
    int  (*Decode)(DecoderInterface* pSelf, 
                   int               bEndOfStream, 
                   int               bDecodeKeyFrameOnly, 
                   int               bSkipBFrameIfDelay, 
                   int64_t           nCurrentTimeUs);
    void (*Destroy)(DecoderInterface* pSelf);
};

typedef struct VIDEOENGINE
{
    VConfig           vconfig;
    VideoStreamInfo   videoStreamInfo;
    DecoderInterface* pDecoderInterface;
}VideoEngine;

typedef DecoderInterface *VDecoderCreator(VideoEngine *);

int VDecoderRegister(enum EVIDEOCODECFORMAT format, char *desc, VDecoderCreator *creator);

VideoEngine* VideoEngineCreate(VConfig* pVConfig, VideoStreamInfo* pVideoInfo);

void VideoEngineDestroy(VideoEngine* pVideoEngine);

void VideoEngineReset(VideoEngine* pVideoEngine);

int VideoEngineSetSbm(VideoEngine* pVideoEngine, Sbm* pSbm, int nIndex);

int VideoEngineGetFbmNum(VideoEngine* pVideoEngine);

Fbm* VideoEngineGetFbm(VideoEngine* pVideoEngine, int nIndex);

int VideoEngineDecode(VideoEngine* pVideoEngine,
                      int          bEndOfStream,
                      int          bDecodeKeyFrameOnly,
                      int          bDropBFrameIfDelay,
                      int64_t      nCurrentTimeUs);
                      

void ResetVeInternal(VideoEngine* p);

void SetVeTopLevelRegisters(VideoEngine* p);

int GetBufferSize(int ePixelFormat, int nWidth, int nHeight, int*nYBufferSize, 
                int *nCBufferSize, int* nYLineStride, int* nCLineStride, int nAlignValue);

#endif

