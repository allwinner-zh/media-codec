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
#ifndef FBM_H
#define FBM_H

#include "vdecoder.h"
#include <pthread.h>

enum BUFFER_TYPE
{
    BUF_TYPE_REFERENCE_DISP = 0,
    BUF_TYPE_ONLY_REFERENCE,
    BUF_TYPE_ONLY_DISP,
};

typedef struct FRAMENODE FrameNode;
struct FRAMENODE
{
    int          bUsedByDecoder;
    int          bUsedByRender;
    int          bInValidPictureQueue;
    int          bAlreadyDisplayed;
    VideoPicture vpicture;
    FrameNode*   pNext;
};


typedef struct FRAMEBUFFERMANAGER
{
    pthread_mutex_t mutex;
    int             nMaxFrameNum;
    int             nEmptyBufferNum;
    int             nValidPictureNum;
    FrameNode*      pEmptyBufferQueue;
    FrameNode*      pValidPictureQueue;
    FrameNode*      pFrames;
    int             nAlignValue;
}Fbm;

typedef struct FBMCREATEINFO
{
    int nFrameNum; 
    int nWidth;
    int nHeight;
    int ePixelFormat; 
    int nBufferType;
    int bProgressiveFlag;
}FbmCreateInfo;


Fbm* FbmCreate(FbmCreateInfo* pFbmCreateInfo);

void FbmDestroy(Fbm* pFbm);

void FbmFlush(Fbm* pFbm);

int FbmGetBufferInfo(Fbm* pFbm, VideoPicture* pVPicture);

int FbmTotalBufferNum(Fbm* pFbm);

int FbmEmptyBufferNum(Fbm* pFbm);

int FbmValidPictureNum(Fbm* pFbm);

VideoPicture* FbmRequestBuffer(Fbm* pFbm);

void FbmReturnBuffer(Fbm* pFbm, VideoPicture* pVPicture, int bValidPicture);

void FbmShareBuffer(Fbm* pFbm, VideoPicture* pVPicture);

VideoPicture* FbmRequestPicture(Fbm* pFbm);

int FbmReturnPicture(Fbm* pFbm, VideoPicture* pPicture);

VideoPicture* FbmNextPictureInfo(Fbm* pFbm);


int FbmAllocatePictureBuffer(VideoPicture* pPicture, int* nAlignValue, int nWidth, int nHeight);

int FbmFreePictureBuffer(VideoPicture* pPicture);
int FbmGetAlignValue(Fbm* pFbm);

#endif

