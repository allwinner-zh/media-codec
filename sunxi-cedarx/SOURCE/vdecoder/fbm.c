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
//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
#define LOG_TAG "fbm.c"

#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include "fbm.h"
#include "adapter.h"
#include "log.h"

#define SUPPORT_REQUEST_MEMORY_DYNAMIC 0
extern const char* strPixelFormat[];

static void FbmEnqueue(FrameNode** ppHead, FrameNode* p)
{
    FrameNode* pCur;

    pCur = *ppHead;

    if(pCur == NULL)
    {
        *ppHead  = p;
        p->pNext = NULL;
        return;
    }
    else
    {
        while(pCur->pNext != NULL)
            pCur = pCur->pNext;

        pCur->pNext = p;
        p->pNext    = NULL;

        return;
    }
}

#if 0
static void FbmEnqueueToHead(FrameNode ** ppHead, FrameNode* p)
{
    FrameNode* pCur;

    pCur     = *ppHead;
    *ppHead  = p;
    p->pNext = pCur;
    return;
}
#endif

static FrameNode* FbmDequeue(FrameNode** ppHead)
{
    FrameNode* pHead;

    pHead = *ppHead;

    if(pHead == NULL)
        return NULL;
    else
    {
        *ppHead = pHead->pNext;
        pHead->pNext = NULL;
        return pHead;
    }
}

Fbm* FbmCreate(FbmCreateInfo* pFbmCreateInfo)
{
    Fbm*       p;
    int        i;
    FrameNode* pFrameNode;
	
    int nCreateFrameBuNum = 0;
    int nAlignStride = 0;
    FbmBufInfo fbmBufferInfo;
    
    int nFrameNum        = pFbmCreateInfo->nFrameNum;
    int nWidth           = pFbmCreateInfo->nWidth;
    int nHeight          = pFbmCreateInfo->nHeight;
    int ePixelFormat     = pFbmCreateInfo->ePixelFormat;
    int nBufferType      = pFbmCreateInfo->nBufferType;
    int bProgressiveFlag = pFbmCreateInfo->bProgressiveFlag;


    p = (Fbm*)malloc(sizeof(Fbm));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(Fbm));


    //* allocate frame nodes.
    pFrameNode = (FrameNode*)malloc(nFrameNum*sizeof(FrameNode));
    if(pFrameNode == NULL)
    {
        loge("memory alloc fail, alloc %d bytes.", nFrameNum*sizeof(FrameNode));
        free(p);
        return NULL;
    }
    memset(pFrameNode, 0, sizeof(nFrameNum*sizeof(FrameNode)));

    pthread_mutex_init(&p->mutex, NULL);
    p->nMaxFrameNum       = nFrameNum;
    p->pEmptyBufferQueue  = NULL;
    p->pValidPictureQueue = NULL;
    p->pFrames            = pFrameNode;

    nCreateFrameBuNum = nFrameNum;

    for(i=0; i<nFrameNum; i++)
    {
        memset((void*)(&pFrameNode->vpicture), 0, sizeof(VideoPicture));
        pFrameNode->bUsedByDecoder       = 0;
        pFrameNode->bUsedByRender        = 0;
        pFrameNode->bInValidPictureQueue = 0;
        pFrameNode->bAlreadyDisplayed    = 0;
        pFrameNode->pNext                = NULL;

        pFrameNode->vpicture.nID          = i;
        pFrameNode->vpicture.ePixelFormat = ePixelFormat;

        pFrameNode->vpicture.nWidth       = nWidth;
        pFrameNode->vpicture.nHeight      = nHeight;
        pFrameNode->vpicture.nLineStride  = pFrameNode->vpicture.nWidth;
        pFrameNode->vpicture.bIsProgressive = 1;
        pFrameNode->vpicture.nLeftOffset   = 0;
        pFrameNode->vpicture.nTopOffset    = 0;
        pFrameNode->vpicture.nRightOffset  = nWidth;
        pFrameNode->vpicture.nBottomOffset = nHeight;

        if(FbmAllocatePictureBuffer(&pFrameNode->vpicture, &nAlignStride, nWidth, nHeight) != 0)
        	break;
        pFrameNode++;
    }
    p->nAlignValue = nAlignStride;
	fbmBufferInfo.nAlignValue = nAlignStride;

    //* check whether all picture buffer allocated.
    if(i < nFrameNum)
    {
        //* not all picture buffer allocated, abort the Fbm creating.
        int j;
    	loge("memory alloc fail, only %d frames allocated, we need %d frames.", i, nFrameNum);
        for(j = 0; j < i; j++)
        	FbmFreePictureBuffer(&p->pFrames[j].vpicture);
        pthread_mutex_destroy(&p->mutex);
        free(p->pFrames);
        free(p);
        return NULL;
    }

    //* put all frame to the empty frame queue.
    for(i=0; i<nFrameNum; i++)
    {
        pFrameNode = &p->pFrames[i];
        FbmEnqueue(&p->pEmptyBufferQueue, pFrameNode);
    }

    p->nEmptyBufferNum = nFrameNum;
    return p;
}


void FbmDestroy(Fbm* pFbm)
{
    logv("FbmDestroy");
    int i = 0;
    pthread_mutex_destroy(&pFbm->mutex);

    for(i=0; i<pFbm->nMaxFrameNum; i++)
    	FbmFreePictureBuffer(&pFbm->pFrames[i].vpicture);
    free(pFbm->pFrames);
    free(pFbm);

    return;
}


VideoPicture* FbmRequestBuffer(Fbm* pFbm)
{
    FrameNode* pFrameNode;
    VideoPicture fbmPicBufInfo;
    int ret = 0;

    logi("FbmRequestBuffer");

    pFrameNode = NULL;
    memset(&fbmPicBufInfo, 0 , sizeof(VideoPicture));

    pthread_mutex_lock(&pFbm->mutex);
    
    pFrameNode = FbmDequeue(&pFbm->pEmptyBufferQueue);
    if(pFrameNode != NULL)
    {
        //* check the picture status.
        if(pFrameNode->bUsedByDecoder ||
            pFrameNode->bInValidPictureQueue ||
            pFrameNode->bUsedByRender ||
            pFrameNode->bAlreadyDisplayed)
        {
            //* the picture is in the pEmptyBufferQueue, these four flags
            //* shouldn't be set.
            loge("invalid frame status, a picture is just pick out from the pEmptyBufferQueue, \
                    but bUsedByDecoder=%d, bInValidPictureQueue=%d, bUsedByRender=%d,\
                    bAlreadyDisplayed=%d.",
                    pFrameNode->bUsedByDecoder, pFrameNode->bInValidPictureQueue,
                    pFrameNode->bUsedByRender, pFrameNode->bAlreadyDisplayed);
            abort();
        }
        pFbm->nEmptyBufferNum--;
        pFrameNode->bUsedByDecoder = 1;
    }

    pthread_mutex_unlock(&pFbm->mutex);
    return (pFrameNode==NULL)? NULL: &pFrameNode->vpicture;
}


void FbmReturnBuffer(Fbm* pFbm, VideoPicture* pVPicture, int bValidPicture)
{
    int        index;
    FrameNode* pFrameNode;
    VideoPicture fbmPicBufInfo;
    int  bNeedDispBuffer = 0;
    int  bNeedReturnBuffer = 0;

    logv("FbmReturnBuffer pVPicture=%p, bValidPicture=%d", pVPicture, bValidPicture);

    index = pVPicture->nID;

    //* check the index is valid.
    if(index < 0 || index >= pFbm->nMaxFrameNum)
    {
        logd("FbmReturnBuffer: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                pVPicture, pVPicture->nID);
        return;
    }
    else
    {
        if(pVPicture != &pFbm->pFrames[index].vpicture)
        {
            logd("FbmReturnBuffer: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                    pVPicture, pVPicture->nID);
            return;
        }
    }

    pFrameNode = &pFbm->pFrames[index];

    pthread_mutex_lock(&pFbm->mutex);

    if(pFrameNode->bUsedByDecoder == 0)
    {
        loge("invalid picture status, bUsedByDecoder=0 when picture buffer is returned.");
        abort();
    }
    pFrameNode->bUsedByDecoder = 0;

    if(pFrameNode->bInValidPictureQueue)
    {
        //* picture is in pValidPictureQueue, it was shared before.

        //* check status.
        if(pFrameNode->bUsedByRender || pFrameNode->bAlreadyDisplayed)
        {
            loge("invalid frame status, a picture in pValidPictureQueue, \
                    but bUsedByRender=%d and bAlreadyDisplayed=%d",
                    pFrameNode->bUsedByRender, pFrameNode->bAlreadyDisplayed);
            abort();
        }
    }
    else
    {
        //* picture not in pValidPictureQueue.
        if(pFrameNode->bUsedByRender == 0)
        {
            //* picture not used by render.
            if(pFrameNode->bAlreadyDisplayed == 0)
            {
                //* picture not been displayed yet.
                if(bValidPicture)
                {
                    FbmEnqueue(&pFbm->pValidPictureQueue, pFrameNode);
                    pFbm->nValidPictureNum++;
                    pFrameNode->bInValidPictureQueue = 1;
                }
                else
                {
                    FbmEnqueue(&pFbm->pEmptyBufferQueue, pFrameNode);
                    pFbm->nEmptyBufferNum++;
                }
            }
            else
            {
                //* picture had been shared before and had been displayed,
                //* put it to pEmptyBufferQueue.
                pFrameNode->bAlreadyDisplayed = 0;
                FbmEnqueue(&pFbm->pEmptyBufferQueue, pFrameNode);
                pFbm->nEmptyBufferNum++;
            }
        }
    }

    pthread_mutex_unlock(&pFbm->mutex);
    return;
}


void FbmShareBuffer(Fbm* pFbm, VideoPicture* pVPicture)
{
    int        index;
    FrameNode* pFrameNode;
    VideoPicture* pDispVPicture = NULL;
    VideoPicture fbmPicBufInfo;
    int   bNeedDispBuffer = 0;

    logv("FbmShareBuffer pVPicture=%p", pVPicture);

    index = pVPicture->nID;

    //* check the index is valid.
    if(index < 0 || index >= pFbm->nMaxFrameNum)
    {
        logd("FbmShareBuffer: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                pVPicture, pVPicture->nID);
        return;
    }
    else
    {
        if(pVPicture != &pFbm->pFrames[index].vpicture)
        {
            logw("FbmShareBuffer: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                    pVPicture, pVPicture->nID);
            return;
        }
    }

    pFrameNode = &pFbm->pFrames[index];

    pthread_mutex_lock(&pFbm->mutex);

    //* check status.
    if(pFrameNode->bUsedByDecoder == 0 ||
        pFrameNode->bInValidPictureQueue == 1 ||
        pFrameNode->bAlreadyDisplayed == 1)
    {
        loge("invalid frame status, a picture is shared but bUsedByDecoder=%d, \
                bInValidPictureQueue=%d, bUsedByDecoder=%d, bAlreadyDisplayed=%d.",
                pFrameNode->bUsedByDecoder, pFrameNode->bInValidPictureQueue,
                pFrameNode->bUsedByDecoder, pFrameNode->bAlreadyDisplayed);
        abort();
    }

    //* put the picture to pValidPictureQueue.
    pFrameNode->bInValidPictureQueue = 1;
    FbmEnqueue(&pFbm->pValidPictureQueue, pFrameNode);
    pFbm->nValidPictureNum++;
    pthread_mutex_unlock(&pFbm->mutex);
    return;
}


VideoPicture* FbmRequestPicture(Fbm* pFbm)
{
    VideoPicture* pVPicture;
    FrameNode*    pFrameNode;


    pthread_mutex_lock(&pFbm->mutex);

   // logi("FbmRequestPicture");

    pVPicture  = NULL;
    pFrameNode = FbmDequeue(&pFbm->pValidPictureQueue);

    if(pFrameNode != NULL)
    {
        if(pFrameNode->bInValidPictureQueue == 0 ||
            pFrameNode->bUsedByRender == 1 ||
            pFrameNode->bAlreadyDisplayed == 1)
        {
            //* the picture is in the pValidPictureQueue, one of these three flags is invalid.
            loge("invalid frame status, a picture is just pick out from the pValidPictureQueue, \
                    but bInValidPictureQueue=%d, bUsedByRender=%d, bAlreadyDisplayed=%d.",
                    pFrameNode->bInValidPictureQueue,
                    pFrameNode->bUsedByRender,
                    pFrameNode->bAlreadyDisplayed);
            abort();
        }

        pFbm->nValidPictureNum--;
        pFrameNode->bInValidPictureQueue = 0;
        pVPicture = &pFrameNode->vpicture;
        pFrameNode->bUsedByRender = 1;
    }

    pthread_mutex_unlock(&pFbm->mutex);

    return (pFrameNode==NULL)? NULL: pVPicture;
}


int FbmReturnPicture(Fbm* pFbm, VideoPicture* pVPicture)
{
    int        index;
    FrameNode* pFrameNode;

   // logi("FbmReturnPicture pVPicture=%p", pVPicture);

    index = pVPicture->nID;

    //* check the index is valid.
    if(index < 0 || index >= pFbm->nMaxFrameNum)
    {
        logw("FbmReturnPicture: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                pVPicture, pVPicture->nID);
        return -1;
    }
    else
    {
        if(pVPicture != &pFbm->pFrames[index].vpicture)
        {
            logw("FbmReturnPicture: the picture id is invalid, pVPicture=%p, pVPicture->nID=%d",
                    pVPicture, pVPicture->nID);
            return -1;
        }
    }

    pFrameNode = &pFbm->pFrames[index];

    pthread_mutex_lock(&pFbm->mutex);

    if(pFrameNode->bUsedByRender == 0 ||
        pFrameNode->bInValidPictureQueue == 1 ||
        pFrameNode->bAlreadyDisplayed == 1)
    {
        //* the picture is being returned, but one of these three flags is invalid.
        loge("invalid frame status, a picture being returned, \
                but bUsedByRender=%d, bInValidPictureQueue=%d, bAlreadyDisplayed=%d.",
                    pFrameNode->bUsedByRender,
                    pFrameNode->bInValidPictureQueue,
                    pFrameNode->bAlreadyDisplayed);
        abort();
    }

    pFrameNode->bUsedByRender = 0;
    if(pFrameNode->bUsedByDecoder)
        pFrameNode->bAlreadyDisplayed = 1;
    else
    {
        FbmEnqueue(&pFbm->pEmptyBufferQueue, pFrameNode);
        pFbm->nEmptyBufferNum++;
    }

    pthread_mutex_unlock(&pFbm->mutex);

    return 0;
}


VideoPicture* FbmNextPictureInfo(Fbm* pFbm)
{
    FrameNode* pFrameNode;

   // logi("FbmNextPictureInfo");
    if(pFbm == NULL)
    {
    	return NULL;
    }
    if(pFbm->pValidPictureQueue != NULL)
    {
        pFrameNode = pFbm->pValidPictureQueue;
        return &pFrameNode->vpicture;
    }
    else
        return NULL;
}


void FbmFlush(Fbm* pFbm)
{
    FrameNode* pFrameNode;
    VideoPicture fbmPicBufInfo;

    pthread_mutex_lock(&pFbm->mutex);

    logv("FbmFlush");

    while((pFrameNode = FbmDequeue(&pFbm->pValidPictureQueue)) != NULL)
    {
        pFbm->nValidPictureNum--;

        //* check the picture status.
        if(pFrameNode->bUsedByRender || pFrameNode->bAlreadyDisplayed)
        {
            //* the picture is in the pValidPictureQueue, these two flags
            //* shouldn't be set.
            loge("invalid frame status, a picture is just pick out from the pValidPictureQueue, \
                    but bUsedByRender=%d and bAlreadyDisplayed=%d.",
                    pFrameNode->bUsedByRender, pFrameNode->bAlreadyDisplayed);
            abort();
        }

        pFrameNode->bInValidPictureQueue = 0;
        if(pFrameNode->bUsedByDecoder == 0)
        {
            //* the picture is not used, put it into pEmptyBufferQueue.
            FbmEnqueue(&pFbm->pEmptyBufferQueue, pFrameNode);
            pFbm->nEmptyBufferNum++;
        }
        else
        {
            //* the picture was shared by the video engine,
            //* set the bAlreadyDisplayed so the picture won't be put into
            //* the pValidPictureQueue when it is returned by the video engine.
            pFrameNode->bAlreadyDisplayed = 1;
        }
    }

    pthread_mutex_unlock(&pFbm->mutex);

    return;
}

int FbmGetAlignValue(Fbm* pFbm)
{
	return 32;
}

int FbmGetBufferInfo(Fbm* pFbm, VideoPicture* pVPicture)
{
    FrameNode* pFrameNode;

    logi("FbmGetBufferInfo");

    //* give the general information of the video pictures.
    pFrameNode = &pFbm->pFrames[0];
    pVPicture->ePixelFormat   = pFrameNode->vpicture.ePixelFormat;
    pVPicture->nWidth         = pFrameNode->vpicture.nWidth;
    pVPicture->nHeight        = pFrameNode->vpicture.nHeight;
    pVPicture->nLineStride    = pFrameNode->vpicture.nLineStride;
    pVPicture->nTopOffset     = pFrameNode->vpicture.nTopOffset;
    pVPicture->nLeftOffset    = pFrameNode->vpicture.nLeftOffset;
    pVPicture->nFrameRate     = pFrameNode->vpicture.nFrameRate;
    pVPicture->nAspectRatio   = pFrameNode->vpicture.nAspectRatio;
    pVPicture->bIsProgressive = pFrameNode->vpicture.bIsProgressive;

    return 0;
}


int FbmTotalBufferNum(Fbm* pFbm)
{
    logi("FbmTotalBufferNum");
    return pFbm->nMaxFrameNum;
}


int FbmEmptyBufferNum(Fbm* pFbm)
{
    return pFbm->nEmptyBufferNum;
}


int FbmValidPictureNum(Fbm* pFbm)
{
    logi("FbmValidPictureNum");
    return pFbm->nValidPictureNum;
}


int FbmAllocatePictureBuffer(VideoPicture* pPicture, int* nAlignValue, int nWidth, int nHeight)
{
    int   ePixelFormat;
    int   nHeight16Align;
    int   nHeight32Align;
    char* pMem;
    int   nMemSizeY;
    int   nMemSizeC;
    int   nLineStride;
    int   nHeight64Align;

    ePixelFormat   = pPicture->ePixelFormat;
    nHeight16Align = (nHeight+15) & ~15;
    nHeight32Align = (nHeight+31) & ~31;
    nHeight64Align = (nHeight+63) & ~63;

    pPicture->pData0 = NULL;
    pPicture->pData1 = NULL;
    pPicture->pData2 = NULL;
    pPicture->pData3 = NULL;

    switch(ePixelFormat)
    {
        case PIXEL_FORMAT_YUV_MB32_420:
        case PIXEL_FORMAT_YUV_MB32_422:
        case PIXEL_FORMAT_YUV_MB32_444:
            //* for decoder,
            //* height of Y component is required to be 32 aligned.
            //* height of UV component are both required to be 32 aligned.
            //* nLineStride should be 32 aligned.
            *nAlignValue = 32;

        	pPicture->nLineStride = (nWidth+31)&~31;
        	pPicture->nWidth = pPicture->nLineStride;
        	pPicture->nHeight = nHeight32Align;
        	nLineStride = (pPicture->nWidth+63) &~ 63;
        	nMemSizeY = nLineStride*nHeight64Align;

            if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
                nMemSizeC = nLineStride*nHeight64Align/4;
            else if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
                nMemSizeC = nLineStride*nHeight64Align/2;
            else
                nMemSizeC = nLineStride*nHeight64Align;

            pMem = (char*)AdapterMemPalloc(nMemSizeY);
            if(pMem == NULL)
            {
                loge("memory alloc fail, require %d bytes for picture buffer.", nMemSizeY);
                return -1;
            }
            pPicture->pData0 = pMem;

            pMem = (char*)AdapterMemPalloc(nMemSizeC*2);    //* UV is combined.
            if(pMem == NULL)
            {
                loge("memory alloc fail, require %d bytes for picture buffer.", nMemSizeC*2);
                AdapterMemPfree(pPicture->pData0);
                pPicture->pData0 = NULL;
                return -1;
            }
            pPicture->pData1 = pMem;
            pPicture->pData2 = NULL;
            pPicture->pData3 = NULL;

            break;

        default:
            loge("pixel format incorrect, ePixelFormat=%d", ePixelFormat);
            return -1;
    }

    if(pPicture->pData0 != NULL)
    {
    	pPicture->phyYBufAddr = (unsigned int)AdapterMemGetPhysicAddress(pPicture->pData0);
    }
    if(pPicture->pData1 != NULL)
    {
    	pPicture->phyCBufAddr = (unsigned int)AdapterMemGetPhysicAddress(pPicture->pData1);
    }
    return 0;
}


int FbmFreePictureBuffer(VideoPicture* pPicture)
{
    if(pPicture->pData0 != NULL)
    {
        AdapterMemPfree(pPicture->pData0);
        pPicture->pData0 = NULL;
    }
    if(pPicture->ePixelFormat < PIXEL_FORMAT_YUV_MB32_420)
    {
    	return 0;
    }
    if(pPicture->pData1 != NULL)
    {
        AdapterMemPfree(pPicture->pData1);
        pPicture->pData1 = NULL;
    }

    if(pPicture->pData2 != NULL)
    {
        AdapterMemPfree(pPicture->pData2);
        pPicture->pData2 = NULL;
    }

    if(pPicture->pData3 != NULL)
    {
        AdapterMemPfree(pPicture->pData3);
        pPicture->pData3 = NULL;
    }
    return 0;
}



