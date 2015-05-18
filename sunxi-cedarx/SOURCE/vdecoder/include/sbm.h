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
#ifndef SBM_H
#define SBM_H

#include "vdecoder.h"
#include <pthread.h>

typedef struct STREAMFRAMEFIFO
{
    VideoStreamDataInfo* pFrames;
    int                  nMaxFrameNum;
    int                  nValidFrameNum;
    int                  nUnReadFrameNum;
    int                  nReadPos;
    int                  nWritePos;
    int                  nFlushPos;
}StreamFrameFifo;


typedef struct STREAMBUFFERMANAGER
{
    pthread_mutex_t mutex;
    char*           pStreamBuffer;
    char*           pStreamBufferEnd;
    int             nStreamBufferSize;
    char*           pWriteAddr;
    int             nValidDataSize;
    StreamFrameFifo frameFifo;
}Sbm;


Sbm* SbmCreate(int nBufferSize);

void SbmDestroy(Sbm* pSbm);

void SbmReset(Sbm* pSbm);

void* SbmBufferAddress(Sbm* pSbm);

int SbmBufferSize(Sbm* pSbm);

int SbmStreamFrameNum(Sbm* pSbm);

int SbmStreamDataSize(Sbm* pSbm);

int SbmRequestBuffer(Sbm* pSbm, int nRequireSize, char** ppBuf, int* pBufSize);

int SbmAddStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);

VideoStreamDataInfo* SbmRequestStream(Sbm* pSbm);

int SbmReturnStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);

int SbmFlushStream(Sbm* pSbm, VideoStreamDataInfo* pDataInfo);
char* SbmBufferWritePointer(Sbm* pSbm);


#endif

