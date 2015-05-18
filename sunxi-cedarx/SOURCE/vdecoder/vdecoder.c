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
#include <pthread.h>
#include <memory.h>
#include "vdecoder.h"
#include "sbm.h"
#include "fbm.h"
#include "videoengine.h"
#include "adapter.h"
#include "log.h"
#include <stdio.h>

#include "sunxi_tr.h"
#include<sys/types.h>  
#include<sys/stat.h>
#include<fcntl.h>


const char* strCodecFormat[] = 
{   "MPEG1",        "MPEG2", "XVID",
    "H263",  "H264","MJPEG",
};

const char* strDecodeResult[] = 
{
    "UNSUPPORTED",
    "OK",
    "FRAME_DECODED",
    "CONTINU",
    "KEYFRAME_DECODED",
    "NO_FRAME_BUFFER",
    "NO_BITSTREAM",
    "RESOLUTION_CHANGE"
};

const char* strPixelFormat[] = 
{
    "DEFAULT",          "YUV_PLANER_420",           "YUV_PLANER_420",
    "YUV_PLANER_444",   "YV12",                     "NV1",
    "YUV_MB32_420",     "MB32_422",                 "MB32_444",
};

typedef struct VideoDecoderContext
{
    VConfig             vconfig;
    VideoStreamInfo     videoStreamInfo;
    VideoEngine*        pVideoEngine;
    Fbm*                pFbm;
    Sbm*                pSbm;
    VideoStreamDataInfo partialStreamDataInfo;
}VideoDecoderContext;


static int  DecideStreamBufferSize(VideoDecoderContext* p);

VideoDecoder* CreateVideoDecoder(void)
{
    VideoDecoderContext* p;
    
    logv("CreateVideoDecoder");
    
    p = (VideoDecoderContext*)malloc(sizeof(VideoDecoderContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        AdpaterRelease(0);
        return NULL;
    }
    
    memset(p, 0, sizeof(VideoDecoderContext));
    
    //* the partialStreamDataInfo is for data submit status recording.
    p->partialStreamDataInfo.nStreamIndex = 0;
    p->partialStreamDataInfo.nPts         = -1;
    p->partialStreamDataInfo.nStreamIndex = 1;
    p->partialStreamDataInfo.nPts         = -1;

    logv("create a video decoder with handle=%p", p);
    
    return (VideoDecoder*)p;
}


void DestroyVideoDecoder(VideoDecoder* pDecoder)
{
    VideoDecoderContext* p;
    
    logv("DestroyVideoDecoder, pDecoder=%p", pDecoder);
    
    p = (VideoDecoderContext*)pDecoder;
    

    //* Destroy the video engine first.
    if(p->pVideoEngine != NULL)
    {
        AdapterLockVideoEngine();
        VideoEngineDestroy(p->pVideoEngine);
        AdapterUnLockVideoEngine();
    }
        
    //* the memory space for codec specific data was allocated in InitializeVideoDecoder.
    if(p->videoStreamInfo.pCodecSpecificData != NULL)
        free(p->videoStreamInfo.pCodecSpecificData);
    
    //* Destroy the stream buffer manager.
    if(p->pSbm != NULL)
        SbmDestroy(p->pSbm);

    if(p->pSbm != NULL)
        SbmDestroy(p->pSbm);
    
    AdpaterRelease();
    
    free(p);
    return;
}


int InitializeVideoDecoder(VideoDecoder* pDecoder, VideoStreamInfo* pVideoInfo,  VConfig* pVconfig)
{
    VideoDecoderContext* p;
    int                  nStreamBufferSize;
    int                  i = 0;
    
    logv("InitializeVideoDecoder, pDecoder=%p, pVideoInfo=%p", pDecoder, pVideoInfo);
    
    p = (VideoDecoderContext*)pDecoder;

    //* check codec format.
    if(pVideoInfo->eCodecFormat > VIDEO_CODEC_FORMAT_MAX ||
        pVideoInfo->eCodecFormat <  VIDEO_CODEC_FORMAT_MIN)
    {
        loge("codec format(0x%x) invalid.", pVideoInfo->eCodecFormat);
        return -1;
    }

//    SecureMemAdapterOpen();
//    SecureMemAdapterClose();
    
	if(AdapterInitialize() != 0)
	{
		loge("can not set up video engine runtime environment.");
		return 0;
	}
    //* print video stream information.
    {
        logv("Video Stream Information:");
        logv("     codec          = %s",        strCodecFormat[pVideoInfo->eCodecFormat - VIDEO_CODEC_FORMAT_MIN]);
        logv("     width          = %d pixels", pVideoInfo->nWidth);
        logv("     height         = %d pixels", pVideoInfo->nHeight);
        logv("     frame rate     = %d",        pVideoInfo->nFrameRate);
        logv("     frame duration = %d us",     pVideoInfo->nFrameDuration);
        logv("     aspect ratio   = %d",        pVideoInfo->nAspectRatio);
        logv("     csd data len   = %d",        pVideoInfo->nCodecSpecificDataLen);
    }
    
    //* save the video stream information.
    p->videoStreamInfo.eCodecFormat          = pVideoInfo->eCodecFormat;
    p->videoStreamInfo.nWidth                = pVideoInfo->nWidth;
    p->videoStreamInfo.nHeight               = pVideoInfo->nHeight;
    p->videoStreamInfo.nFrameRate            = pVideoInfo->nFrameRate;
    p->videoStreamInfo.nFrameDuration        = pVideoInfo->nFrameDuration;
    p->videoStreamInfo.nAspectRatio          = pVideoInfo->nAspectRatio;
    p->videoStreamInfo.nCodecSpecificDataLen = pVideoInfo->nCodecSpecificDataLen;

    if(p->videoStreamInfo.nCodecSpecificDataLen > 0)
    {
        int   nSize = p->videoStreamInfo.nCodecSpecificDataLen;
        char* pMem  = (char*)malloc(nSize);
        
        if(pMem == NULL)
        {
            p->videoStreamInfo.nCodecSpecificDataLen = 0;
            loge("memory alloc fail.");
            return -1;
        }
        
        memcpy(pMem, pVideoInfo->pCodecSpecificData, nSize);
        p->videoStreamInfo.pCodecSpecificData = pMem;
    }
    
    memcpy(&p->vconfig, pVconfig, sizeof(VConfig));

    //* create stream buffer.
    nStreamBufferSize = DecideStreamBufferSize(p);

    p->pSbm = SbmCreate(nStreamBufferSize);

    if(p->pSbm == NULL)
    {
        loge("create stream buffer fail.");
        return -1;
    }

    //* check and fix the configuration for decoder.
//    CheckConfiguration(p);
    
    //* create video engine.
    AdapterLockVideoEngine();
    p->pVideoEngine = VideoEngineCreate(&p->vconfig, &p->videoStreamInfo);
    AdapterUnLockVideoEngine();
    if(p->pVideoEngine == NULL)
    {
        loge("create video engine fail.");
        return -1;
    }
    
    //* set stream buffer to video engine.
    AdapterLockVideoEngine();
    VideoEngineSetSbm(p->pVideoEngine, p->pSbm, i);
    AdapterUnLockVideoEngine();

    return 0;
}

void ResetVideoDecoder(VideoDecoder* pDecoder)
{
    int                  i;
    VideoDecoderContext* p;
    
    logv("ResetVideoDecoder, pDecoder=%p", pDecoder);
    
    p = (VideoDecoderContext*)pDecoder;
    
    //* reset the video engine.
    if(p->pVideoEngine)
    {
        AdapterLockVideoEngine();
        VideoEngineReset(p->pVideoEngine);
        AdapterUnLockVideoEngine();
    }
        
    if(p->pFbm==NULL && p->pVideoEngine != NULL)
    {
    	AdapterLockVideoEngine();
    	p->pFbm = VideoEngineGetFbm(p->pVideoEngine, 0);
    	AdapterUnLockVideoEngine();
    }
    if(p->pFbm != NULL)
    	FbmFlush(p->pFbm);
    
    //* flush stream data.
    if(p->pSbm != NULL)
    	SbmReset(p->pSbm);
    //* clear the partialStreamDataInfo.
    memset(&p->partialStreamDataInfo, 0, sizeof(VideoStreamDataInfo));
    
    return;
}


int DecodeVideoStream(VideoDecoder* pDecoder, 
                      int           bEndOfStream,
                      int           bDecodeKeyFrameOnly,
                      int           bDropBFrameIfDelay,
                      int64_t       nCurrentTimeUs)
{
    int                  ret;
    VideoDecoderContext* p;
    
    logi("DecodeVideoStream, pDecoder=%p, bEndOfStream=%d, bDropBFrameIfDelay=%d, nCurrentTimeUs=%lld",
            pDecoder, bEndOfStream, bDropBFrameIfDelay, nCurrentTimeUs);
    
    p = (VideoDecoderContext*)pDecoder;
    
    if(p->pVideoEngine == NULL)
        return VDECODE_RESULT_UNSUPPORTED;


    AdapterLockVideoEngine();
    ret = VideoEngineDecode(p->pVideoEngine, 
                            bEndOfStream, 
                            bDecodeKeyFrameOnly, 
                            bDropBFrameIfDelay, 
                            nCurrentTimeUs);
    AdapterUnLockVideoEngine();
    logi("decode method return %s", strDecodeResult[ret-VDECODE_RESULT_MIN]);

    if(bEndOfStream && ret == VDECODE_RESULT_NO_BITSTREAM)
    {
        //* stream end, let the low level decoder return all decoded pictures.
        AdapterLockVideoEngine();
        VideoEngineReset(p->pVideoEngine);
        AdapterUnLockVideoEngine();
    }
    return ret;
}


int GetVideoStreamInfo(VideoDecoder* pDecoder, VideoStreamInfo* pVideoInfo)
{
    VideoDecoderContext* p;
    
    logv("GetVideoStreamInfo, pDecoder=%p, pVideoInfo=%p", pDecoder, pVideoInfo);
    
    p = (VideoDecoderContext*)pDecoder;
    
    //* set the video stream information.
    pVideoInfo->eCodecFormat   = p->videoStreamInfo.eCodecFormat;
    pVideoInfo->nWidth         = p->videoStreamInfo.nWidth;
    pVideoInfo->nHeight        = p->videoStreamInfo.nHeight;
    pVideoInfo->nFrameRate     = p->videoStreamInfo.nFrameRate;
    pVideoInfo->nFrameDuration = p->videoStreamInfo.nFrameDuration;
    pVideoInfo->nAspectRatio   = p->videoStreamInfo.nAspectRatio;
    
    //* print video stream information.
    {
        logi("Video Stream Information:");
        logi("     codec          = %s",        strCodecFormat[pVideoInfo->eCodecFormat - VIDEO_CODEC_FORMAT_MIN]);
        logi("     width          = %d pixels", pVideoInfo->nWidth);
        logi("     height         = %d pixels", pVideoInfo->nHeight);
        logi("     frame rate     = %d",        pVideoInfo->nFrameRate);
        logi("     frame duration = %d us",     pVideoInfo->nFrameDuration);
        logi("     aspect ratio   = %d",        pVideoInfo->nAspectRatio);
    }
    
    return 0;
}


int RequestVideoStreamBuffer(VideoDecoder* pDecoder,
                             int           nRequireSize,
                             char**        ppBuf,
                             int*          pBufSize,
                             char**        ppRingBuf,
                             int*          pRingBufSize,
                             int           nStreamBufIndex)
{
    char*                pStart;
    char*                pStreamBufEnd;
    char*                pMem;
    int                  nFreeSize;
    Sbm*                 pSbm;
    VideoDecoderContext* p;
    
    logi("RequestVideoStreamBuffer, pDecoder=%p, nRequireSize=%d, nStreamBufIndex=%d", 
            pDecoder, nRequireSize, nStreamBufIndex);
    
    p = (VideoDecoderContext*)pDecoder;
    
    *ppBuf        = NULL;
    *ppRingBuf    = NULL;
    *pBufSize     = 0;
    *pRingBufSize = 0;
    
    pSbm          = p->pSbm;
    
    if(pSbm == NULL)
    {
        logw("pSbm of video stream %d is NULL, RequestVideoStreamBuffer fail.", nStreamBufIndex);
        return -1;
    }

    //* sometimes AVI parser will pass empty stream frame to help pts calculation.
    //* in this case give four bytes even the parser does not need.
    if(nRequireSize == 0)
        nRequireSize = 4;
    
    //* we've filled partial frame data but not added to the SBM before, 
    //* we need to calculate the actual buffer pointer by self.
    nRequireSize += p->partialStreamDataInfo.nLength;
    if(SbmRequestBuffer(pSbm, nRequireSize, &pMem, &nFreeSize) < 0)
    {
        logi("request stream buffer fail, %d bytes valid data in SBM[%d], total buffer size is %d bytes.",
                SbmStreamDataSize(pSbm), 
                nStreamBufIndex, 
                SbmBufferSize(pSbm));
                
        return -1;
    }
    
    //* check the free buffer is larger than the partial data we filled before.
    if(nFreeSize <= p->partialStreamDataInfo.nLength)
    {
        logi("require stream buffer get %d bytes, but this buffer has been filled with partial \
                frame data of %d bytes before, nStreamBufIndex=%d.",
                nFreeSize, p->partialStreamDataInfo.nLength, nStreamBufIndex);
        
        return -1;
    }
    
    //* calculate the output buffer pos.
    pStreamBufEnd = (char*)SbmBufferAddress(pSbm) + SbmBufferSize(pSbm);
    pStart        = pMem + p->partialStreamDataInfo.nLength;
    if(pStart >= pStreamBufEnd)
        pStart -= SbmBufferSize(pSbm);
    nFreeSize -= p->partialStreamDataInfo.nLength;
    
    if(pStart + nFreeSize <= pStreamBufEnd) //* check if buffer ring back.
    {
        *ppBuf    = pStart;
        *pBufSize = nFreeSize;
    }
    else
    {
        //* the buffer ring back.
        *ppBuf        = pStart;
        *pBufSize     = pStreamBufEnd - pStart;
        *ppRingBuf    = SbmBufferAddress(pSbm);
        *pRingBufSize = nFreeSize - *pBufSize;
        logi("stream buffer %d ring back.", nStreamBufIndex);
    }

	return 0;
}


int SubmitVideoStreamData(VideoDecoder*        pDecoder,
                          VideoStreamDataInfo* pDataInfo,
                          int                  nStreamBufIndex)
{
    Sbm*                 pSbm;
    VideoDecoderContext* p;
    VideoStreamDataInfo* pPartialStreamDataInfo;
    
    logi("SubmitVideoStreamData, pDecoder=%p, pDataInfo=%p, nStreamBufIndex=%d", 
            pDecoder, pDataInfo, nStreamBufIndex);
    
    p = (VideoDecoderContext*)pDecoder;
    
    pSbm = p->pSbm;
    
    if(pSbm == NULL)
    {
        logw("pSbm of video stream %d is NULL, SubmitVideoStreamData fail.", nStreamBufIndex);
        return -1;
    }
    
    pPartialStreamDataInfo = &p->partialStreamDataInfo;
    
    //* chech wheter a new stream frame.
    if(pDataInfo->bIsFirstPart)
    {
        if(pPartialStreamDataInfo->nLength != 0)    //* last frame is not complete yet.
        {
            logw("stream data frame uncomplete.");
            SbmAddStream(pSbm, pPartialStreamDataInfo);
        }
        
        //* set the data address and pts.
        pPartialStreamDataInfo->pData        = pDataInfo->pData;
        pPartialStreamDataInfo->nLength      = pDataInfo->nLength;
        pPartialStreamDataInfo->nPts         = pDataInfo->nPts;
        pPartialStreamDataInfo->nPcr         = pDataInfo->nPcr;
        pPartialStreamDataInfo->bIsFirstPart = pDataInfo->bIsFirstPart;
        pPartialStreamDataInfo->bIsLastPart  = 0;
    }
    else
    {
        pPartialStreamDataInfo->nLength += pDataInfo->nLength;
        if(pPartialStreamDataInfo->nPts == -1 && pDataInfo->nPts != -1)
            pPartialStreamDataInfo->nPts = pDataInfo->nPts;
    }
    
    //* check whether a stream frame complete.
    if(pDataInfo->bIsLastPart)
    {
        if(pPartialStreamDataInfo->pData != NULL && pPartialStreamDataInfo->nLength != 0)
        {
            //* we need to flush data from cache to memory for the VE hardware module.
            char* pStreamBufEnd = (char*)SbmBufferAddress(pSbm) + SbmBufferSize(pSbm);
            if(pPartialStreamDataInfo->pData + pPartialStreamDataInfo->nLength <= pStreamBufEnd)
            {
            	AdapterMemFlushCache(pPartialStreamDataInfo->pData, pPartialStreamDataInfo->nLength);
            }
            else
            {
                //* buffer ring back.
                int nPartialLen = pStreamBufEnd - pPartialStreamDataInfo->pData;
                AdapterMemFlushCache(pPartialStreamDataInfo->pData, nPartialLen);
                AdapterMemFlushCache(SbmBufferAddress(pSbm), pPartialStreamDataInfo->nLength - nPartialLen);
            }
        }
        else
        {
            //* maybe it is a empty frame for MPEG4 decoder from AVI parser.
            logw("empty stream data frame submitted, pData=%p, nLength=%d",
                pPartialStreamDataInfo->pData, pPartialStreamDataInfo->nLength);
        }
        
        //* submit stream frame to the SBM.
        SbmAddStream(pSbm, pPartialStreamDataInfo);
        
        //* clear status of stream data info.
        pPartialStreamDataInfo->pData        = NULL;
        pPartialStreamDataInfo->nLength      = 0;
        pPartialStreamDataInfo->nPts         = -1;
        pPartialStreamDataInfo->nPcr         = -1;
        pPartialStreamDataInfo->bIsLastPart  = 0;
        pPartialStreamDataInfo->bIsFirstPart = 0;
    }

    return 0;
}


static void UpdateVideoStreamInfo(VideoDecoderContext* p, VideoPicture* pPicture)
{
    //* update frame rate.
    if(p->videoStreamInfo.nFrameRate != pPicture->nFrameRate && pPicture->nFrameRate != 0)
        p->videoStreamInfo.nFrameRate = pPicture->nFrameRate;

    return;
}

VideoPicture* RequestPicture(VideoDecoder* pDecoder, int nStreamIndex)
{
    VideoDecoderContext* p;
    Fbm*                 pFbm;
    VideoPicture*        pPicture;
    
    logi("RequestPicture, nStreamIndex=%d", nStreamIndex);
    
    p = (VideoDecoderContext*)pDecoder;
    
    pFbm = p->pFbm;
    if(pFbm == NULL)
    {
        //* get FBM handle from video engine.
        if(p->pVideoEngine != NULL)
            pFbm = p->pFbm = VideoEngineGetFbm(p->pVideoEngine, nStreamIndex);
        
        if(pFbm == NULL)
        {
        	logi("Fbm module of video stream %d not create yet, RequestPicture fail.", nStreamIndex);
        	return NULL;
        }
    }
    
    //* request picture.
    pPicture = FbmRequestPicture(pFbm);
    
    if(pPicture != NULL)
    {
        //* set stream index to the picture, it will be useful when picture returned.
        pPicture->nStreamIndex = nStreamIndex;
        //* update video stream information.
        UpdateVideoStreamInfo(p, pPicture);
        logv("picture w: %d,  h: %d, addr: %x, top: %d, bottom: %d, left: %d, right: %d\n",
        		pPicture->nWidth, pPicture->nHeight, (unsigned int)pPicture,
        		pPicture->nTopOffset, pPicture->nBottomOffset,
        		pPicture->nLeftOffset, pPicture->nRightOffset);
    }
    
    return pPicture;
}

int ReturnPicture(VideoDecoder* pDecoder, VideoPicture* pPicture)
{
    VideoDecoderContext* p;
    Fbm*                 pFbm;
    int                  nStreamIndex;
    int                  ret;
    
    logi("ReturnPicture, pPicture=%p", pPicture);
    
    p = (VideoDecoderContext*)pDecoder;
    
    nStreamIndex = pPicture->nStreamIndex;
    if(nStreamIndex < 0 || nStreamIndex > 2)
    {
        loge("invalid stream index(%d), pPicture->nStreamIndex must had been \
                changed by someone incorrectly, ReturnPicture fail.", nStreamIndex);
        return -1;
    }
    
    pFbm = p->pFbm;
    if(pFbm == NULL)
    {
        logw("pFbm is NULL when returning a picture, ReturnPicture fail.");
        return -1;
    }
    
    ret = FbmReturnPicture(pFbm, pPicture);
    if(ret != 0)
    {
        logw("FbmReturnPicture return fail, it means the picture being returned it not one of this FBM.");
    }
    
    return ret;
}


VideoPicture* NextPictureInfo(VideoDecoder* pDecoder, int nStreamIndex)
{
    VideoDecoderContext* p;
    Fbm*                 pFbm;
    VideoPicture*        pPicture;
    
    logi("RequestPicture, nStreamIndex=%d", nStreamIndex);
    
    p = (VideoDecoderContext*)pDecoder;
    
    pFbm = p->pFbm;
    if(pFbm == NULL)
    {
        //* get FBM handle from video engine.
        if(p->pVideoEngine != NULL)
            pFbm = p->pFbm = VideoEngineGetFbm(p->pVideoEngine, nStreamIndex);
        
        logi("Fbm module of video stream %d not create yet, NextPictureInfo() fail.", nStreamIndex);
        return NULL;
    }
    
    //* request picture.
    pPicture = FbmNextPictureInfo(pFbm);
    
    if(pPicture != NULL)
    {
        //* set stream index to the picture, it will be useful when picture returned.
        pPicture->nStreamIndex = nStreamIndex;
        
        //* update video stream information.
        UpdateVideoStreamInfo(p, pPicture);
    }
    
    return pPicture;
}
int ValidPictureNum(VideoDecoder* pDecoder, int nStreamIndex)
{
    VideoDecoderContext* p;
    Fbm*                 pFbm;
    
//    logv("ValidPictureNum, nStreamIndex=%d", nStreamIndex);
    
    p    = (VideoDecoderContext*)pDecoder;
    pFbm = p->pFbm;
    if(pFbm == NULL)
    {
        //* get FBM handle from video engine.
        if(p->pVideoEngine != NULL)
            pFbm = p->pFbm = VideoEngineGetFbm(p->pVideoEngine, nStreamIndex);
        
        logi("Fbm module of video stream %d not create yet, ValidPictureNum() fail.", nStreamIndex);
        return 0;
    }
    
    return FbmValidPictureNum(pFbm);
}
int ReopenVideoEngine(VideoDecoder* pDecoder)
{
    int                  i;
    VideoDecoderContext* p;
    
    logv("ReopenVideoEngine");
    
    p = (VideoDecoderContext*)pDecoder;
    if(p->pVideoEngine == NULL)
    {
        logw("video decoder is not initialized yet, ReopenVideoEngine() fail.");
        return -1;
    }
    
    //* destroy the video engine.
    AdapterLockVideoEngine();
    VideoEngineDestroy(p->pVideoEngine);
    AdapterUnLockVideoEngine();
    
    //* FBM is destroyed when video engine is destroyed, so clear the handles.
    p->pFbm = NULL;
//    p->nFbmNum = 0;
    p->pVideoEngine = NULL;
    
    //* create a video engine again, 
    //* before that, we should clear the stream information except codec format.
    p->videoStreamInfo.nWidth                = 0;
    p->videoStreamInfo.nHeight               = 0;
    p->videoStreamInfo.nFrameRate            = 0;
    p->videoStreamInfo.nFrameDuration        = 0;
    p->videoStreamInfo.nAspectRatio          = 0;
    p->videoStreamInfo.nCodecSpecificDataLen = 0;
    if(p->videoStreamInfo.pCodecSpecificData != NULL)
    {
        free(p->videoStreamInfo.pCodecSpecificData);
        p->videoStreamInfo.pCodecSpecificData = NULL;
    }
    
    AdapterLockVideoEngine();
    p->pVideoEngine = VideoEngineCreate(&p->vconfig, &p->videoStreamInfo);
    AdapterUnLockVideoEngine();
    if(p->pVideoEngine == NULL)
    {
        loge("VideoEngineCreate fail, can not reopen the video engine.");
        return -1;
    }
    
    //* set sbm module to the video engine.
    AdapterLockVideoEngine();
    VideoEngineSetSbm(p->pVideoEngine, p->pSbm, 0);
    AdapterUnLockVideoEngine();
    
    return 0;
}
static int DecideStreamBufferSize(VideoDecoderContext* p)
{
    int nVideoHeight;
    int eCodecFormat;
    int nBufferSize;

    //* we decide stream buffer size by resolution and codec format.
    nVideoHeight = p->videoStreamInfo.nHeight;
    eCodecFormat = p->videoStreamInfo.eCodecFormat;
    
    //* if resolution is unknown, treat it as full HD source.
    if(nVideoHeight == 0)
        nVideoHeight = 1080;
        
    if(nVideoHeight < 480)
        nBufferSize = 2*1024*1024;
    else if (nVideoHeight < 720)
        nBufferSize = 4*1024*1024;
    else if(nVideoHeight < 1080)
        nBufferSize = 6*1024*1024;
    else if(nVideoHeight < 2160)
        nBufferSize = 8*1024*1024;
    else
    	nBufferSize = 12*1024*1024;
    
    if(eCodecFormat == VIDEO_CODEC_FORMAT_MPEG1 ||
       eCodecFormat == VIDEO_CODEC_FORMAT_MPEG2)
    {
        nBufferSize += 2*1024*1024; //* for old codec format, compress rate is low.
    }
    return nBufferSize;
}
