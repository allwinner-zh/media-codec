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

#include <stdlib.h>
#include<string.h>
#include "sbm.h"
#include "adapter.h"

#include "log.h"

#define SBM_FRAME_FIFO_SIZE (2048)  //* store 2048 frames of bitstream data at maximum.

static int lock(Sbm *pSbm);
static void unlock(Sbm *pSbm);

/*
**********************************************************************
*                             SbmCreate
*
*Description: Create Stream Buffer Manager module.
*
*Arguments  : nBufferSize     the size of pStreamBuffer, to store stream info.
*
*Return     : result
*               = NULL;     failed;
*              != NULL;     Sbm handler.
*               
*Summary    : nBufferSize is between 4MB and 12MB.
*
**********************************************************************
*/
Sbm *SbmCreate(int nBufferSize)
{
    Sbm *pSbm;
    char *pSbmBuf;
    int i;
    int ret;
    
    if(nBufferSize <= 0)
        return NULL;

    pSbmBuf = (char*)AdapterMemPalloc(nBufferSize);    //*
    if(pSbmBuf == NULL)
    {
        loge("pSbmBuf == NULL.");
        return NULL;
    }

    pSbm = (Sbm *)malloc(sizeof(Sbm));
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        AdapterMemPfree(pSbmBuf); //*
        return NULL;
    }
    memset(pSbm, 0, sizeof(Sbm));

    pSbm->frameFifo.pFrames = (VideoStreamDataInfo *)malloc(SBM_FRAME_FIFO_SIZE 
                                                 * sizeof(VideoStreamDataInfo));
    if(pSbm->frameFifo.pFrames == NULL)
    {
        loge("sbm->frameFifo.pFrames == NULL.");
        free(pSbm);
        AdapterMemPfree(pSbmBuf);
        return NULL;
    }
    memset(pSbm->frameFifo.pFrames, 0,  SBM_FRAME_FIFO_SIZE * sizeof(VideoStreamDataInfo));
    for(i = 0; i < SBM_FRAME_FIFO_SIZE; i++)
    {
        pSbm->frameFifo.pFrames[i].nID = i;
    }

    ret = pthread_mutex_init(&pSbm->mutex, NULL);
    if(ret != 0)
    {
        loge("pthread_mutex_init failed.");
        free(pSbm->frameFifo.pFrames);
        free(pSbm);
        AdapterMemPfree(pSbmBuf);
        return NULL;
    }
    pSbm->pStreamBuffer      = pSbmBuf;
    pSbm->pStreamBufferEnd   = pSbmBuf + nBufferSize -1;
    pSbm->nStreamBufferSize  = nBufferSize;
    pSbm->pWriteAddr         = pSbmBuf;
    pSbm->nValidDataSize     = 0;

    pSbm->frameFifo.nMaxFrameNum     = SBM_FRAME_FIFO_SIZE;
    pSbm->frameFifo.nValidFrameNum   = 0;
    pSbm->frameFifo.nUnReadFrameNum  = 0;
    pSbm->frameFifo.nReadPos         = 0;
    pSbm->frameFifo.nWritePos        = 0;
    pSbm->frameFifo.nFlushPos        = 0;

    return pSbm;
}

/*
**********************************************************************
*                             SbmDestroy
*
*Description: Destroy Stream Buffer Manager module, free resource.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : NULL
*               
*Summary    :
*
**********************************************************************
*/
void SbmDestroy(Sbm *pSbm)
{    
    if(pSbm != NULL)
    {
        pthread_mutex_destroy(&pSbm->mutex);

        if(pSbm->pStreamBuffer != NULL)
        {
            AdapterMemPfree(pSbm->pStreamBuffer);
            pSbm->pStreamBuffer = NULL;
        }

        if(pSbm->frameFifo.pFrames != NULL)
        {
            free(pSbm->frameFifo.pFrames);
            pSbm->frameFifo.pFrames = NULL;
        }

        free(pSbm);
    }

    return;
}

/*
**********************************************************************
*                             SbmReset
*
*Description: Reset Stream Buffer Manager module.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : NULL
*               
*Summary    : If succeed, Stream Buffer Manager module will be resumed to initial state, 
*             stream data will be discarded.
*
**********************************************************************
*/
void SbmReset(Sbm *pSbm)
{
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return;
    }

    if(lock(pSbm) != 0)
        return;

    pSbm->pWriteAddr                 = pSbm->pStreamBuffer;
    pSbm->nValidDataSize             = 0;

    pSbm->frameFifo.nReadPos         = 0;
    pSbm->frameFifo.nWritePos        = 0;
    pSbm->frameFifo.nFlushPos        = 0;
    pSbm->frameFifo.nValidFrameNum   = 0;
    pSbm->frameFifo.nUnReadFrameNum  = 0;
    unlock(pSbm);

    return;
}

/*
**********************************************************************
*                             SbmBufferAddress
*
*Description: Get the base address of SBM buffer.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : The base address of SBM buffer.
*               
*Summary    : 
*
**********************************************************************
*/
void *SbmBufferAddress(Sbm *pSbm)
{
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return NULL;
    }

    return pSbm->pStreamBuffer;
}

/*
**********************************************************************
*                             SbmBufferSize
*
*Description: Get the sbm buffer size.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : The size of SBM buffer, in Bytes.
*               
*Summary    : The size is set when create SBM.
*
**********************************************************************
*/
int SbmBufferSize(Sbm* pSbm)
{
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return 0;
    }

    return pSbm->nStreamBufferSize;
}

/*
**********************************************************************
*                             SbmStreamFrameNum
*
*Description: Get the total frames of undecoded stream data.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : The frames of undecoded stream data.
*               
*Summary    : 
*
**********************************************************************
*/
int SbmStreamFrameNum(Sbm *pSbm)
{
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return 0;
    }

    return pSbm->frameFifo.nValidFrameNum;
}

/*
**********************************************************************
*                             SbmStreamDataSize
*
*Description: Get the total size of undecoded data.
*
*Arguments  : pSbm     Created by SbmCreate function.
*
*Return     : The total size of undecoded stream data, in bytes.
*               
*Summary    : 
*
**********************************************************************
*/
int SbmStreamDataSize(Sbm* pSbm)
{
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return 0;
    }

    return pSbm->nValidDataSize;
}

char* SbmBufferWritePointer(Sbm* pSbm)
{
    
    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return 0;
    }

    return pSbm->pWriteAddr;
}

/*
**********************************************************************
*                             SbmRequestBuffer
*
*Description: Request buffer from sbm module.
*
*Arguments  : pSbm              Created by SbmCreate function;
*             nRequireSize      the required size, in bytes;
*             ppBuf             store the requested buffer address;
*             pBufSize          store the requested buffer size.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*               
*Summary    : SBM buffer is cyclic, if the  buffer turns around, there will be 2 blocks.
*
**********************************************************************
*/
int SbmRequestBuffer(Sbm *pSbm, int nRequireSize, char **ppBuf, int *pBufSize)
{
    int nFreeSize;
    
    if(pSbm == NULL || ppBuf == NULL || pBufSize == NULL)
    {
        loge("input error.");
        return -1;
    }

    if(lock(pSbm) != 0)
        return -1;

    if(pSbm->frameFifo.nValidFrameNum >= pSbm->frameFifo.nMaxFrameNum)
    {
        loge("nValidFrameNum >= nMaxFrameNum.");
        unlock(pSbm);
        return -1;
    }

    if(pSbm->nValidDataSize < pSbm->nStreamBufferSize)
    {
        nFreeSize = pSbm->nStreamBufferSize - pSbm->nValidDataSize;
        if(nRequireSize > nFreeSize)
        {
            unlock(pSbm);
            return -1;
        }

        *ppBuf    = pSbm->pWriteAddr;
        *pBufSize = nRequireSize;
        
        unlock(pSbm);
        return 0;
    }
    else
    {
        loge("no free buffer.");
        unlock(pSbm);
        return -1;
    }
}

/*
**********************************************************************
*                             SbmAddStream
*
*Description: Add one frame stream to sbm module.
*
*Arguments  : pSbm              Created by SbmCreate function;
*             pDataInfo         the stream info need to be added.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*               
*Summary    : pDataInfo should contain Complete frame, bIsFirstPart=bIsLastPart=1.
*
**********************************************************************
*/
int SbmAddStream(Sbm *pSbm, VideoStreamDataInfo *pDataInfo)
{
    int nWritePos;
    char *pNewWriteAddr;
    
    if(pSbm == NULL || pDataInfo == NULL)
    {
        loge("input error.");
        return -1;
    }

    if(lock(pSbm) != 0)
        return -1;

    if(pSbm->frameFifo.nValidFrameNum >= pSbm->frameFifo.nMaxFrameNum)
    {
        loge("nValidFrameNum > nMaxFrameNum.");
        unlock(pSbm);
        return -1;
    }

    if(pDataInfo->nLength + pSbm->nValidDataSize > pSbm->nStreamBufferSize)
    {
        loge("no free buffer.");
        unlock(pSbm);
        return -1;
    }
    if(pDataInfo->bValid == 0)
    {
        pDataInfo->bValid = 1;
    }

    nWritePos = pSbm->frameFifo.nWritePos;
    memcpy(&pSbm->frameFifo.pFrames[nWritePos], pDataInfo, sizeof(VideoStreamDataInfo));
    nWritePos++;
    if(nWritePos >= pSbm->frameFifo.nMaxFrameNum)
    {
        nWritePos = 0;
    }

    pSbm->frameFifo.nWritePos = nWritePos;
    pSbm->frameFifo.nValidFrameNum++;
    pSbm->frameFifo.nUnReadFrameNum++;
    pSbm->nValidDataSize += pDataInfo->nLength;

    pNewWriteAddr = pSbm->pWriteAddr + pDataInfo->nLength;
    if(pNewWriteAddr > pSbm->pStreamBufferEnd)
    {
        pNewWriteAddr -= pSbm->nStreamBufferSize;
    }

    pSbm->pWriteAddr = pNewWriteAddr;

    unlock(pSbm);
    return 0;
}

/*
**********************************************************************
*                             SbmRequestStream
*
*Description: Request one frame stream data from sbm module to decoder.
*
*Arguments  : pSbm      Created by SbmCreate function;
*
*Return     : The stream infomation.
*               
*Summary    : The stream data obeys FIFO rule. 
*
**********************************************************************
*/
VideoStreamDataInfo *SbmRequestStream(Sbm *pSbm)
{
    VideoStreamDataInfo *pDataInfo;

    if(pSbm == NULL )
    {
        loge("pSbm == NULL.");
        return NULL;
    }

    if(lock(pSbm) != 0)
    {
        return NULL;
    }

    if(pSbm->frameFifo.nUnReadFrameNum == 0)
    {
        logv("nUnReadFrameNum == 0.");
        unlock(pSbm);
        return NULL;
    }
    
    pDataInfo = &pSbm->frameFifo.pFrames[pSbm->frameFifo.nReadPos];
    
    if(pDataInfo == NULL)
    {
        loge("request failed.");
        unlock(pSbm);
        return NULL;
    }
    
    pSbm->frameFifo.nReadPos++;
    pSbm->frameFifo.nUnReadFrameNum--;
    if(pSbm->frameFifo.nReadPos >= pSbm->frameFifo.nMaxFrameNum)
    {
        pSbm->frameFifo.nReadPos = 0;
    }
    unlock(pSbm);

    return pDataInfo;
}

/*
**********************************************************************
*                             SbmReturnStream
*
*Description: Return one undecoded frame to sbm module.
*
*Arguments  : pSbm          Created by SbmCreate function;
*             pDataInfo     the stream info need to be returned.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*               
*Summary    : After returned, the stream data's sequence is the same as before.
*
**********************************************************************
*/
int SbmReturnStream(Sbm *pSbm, VideoStreamDataInfo *pDataInfo)
{
    int nReadPos;
        
    if(pSbm == NULL || pDataInfo == NULL)
    {
        loge("input error.");
        return -1;
    }

    if(lock(pSbm) != 0)
    {
        return -1;
    }

    if(pSbm->frameFifo.nValidFrameNum == 0)
    {
        loge("nValidFrameNum == 0.");
        unlock(pSbm);
        return -1;
    }
    nReadPos = pSbm->frameFifo.nReadPos;
    nReadPos--;
    if(nReadPos < 0)
    {
        nReadPos = pSbm->frameFifo.nMaxFrameNum - 1;
    }
    pSbm->frameFifo.nUnReadFrameNum++;
    if(pDataInfo != &pSbm->frameFifo.pFrames[nReadPos])
    {
        loge("wrong frame sequence.");
        abort();
    }
    
    pSbm->frameFifo.pFrames[nReadPos] = *pDataInfo;
    pSbm->frameFifo.nReadPos  = nReadPos;

    unlock(pSbm);
    return 0;
}

/*
**********************************************************************
*                             SbmFlushStream
*
*Description: Flush one frame which is requested from SBM.
*
*Arguments  : pSbm          Created by SbmCreate function;
*             pDataInfo     the stream info need to be flushed.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*               
*Summary    : After flushed, the buffer can be used to store new stream. 
*
**********************************************************************
*/
int SbmFlushStream(Sbm *pSbm, VideoStreamDataInfo *pDataInfo)
{
    int nFlushPos;

    if(pSbm == NULL)
    {
        loge("pSbm == NULL.");
        return -1;
    }
    
    if(lock(pSbm) != 0)
    {
        return -1;
    }

    if(pSbm->frameFifo.nValidFrameNum == 0)
    {
        loge("no valid frame.");
        unlock(pSbm);
        return -1;
    }

    nFlushPos = pSbm->frameFifo.nFlushPos;
    if(pDataInfo != &pSbm->frameFifo.pFrames[nFlushPos])
    {
        loge("not current nFlushPos.");
        unlock(pSbm);
        return -1;
    }

    nFlushPos++;
    if(nFlushPos >= pSbm->frameFifo.nMaxFrameNum)
    {
        nFlushPos = 0;
    }

    pSbm->frameFifo.nValidFrameNum--;
    pSbm->nValidDataSize     -= pDataInfo->nLength;
    pSbm->frameFifo.nFlushPos = nFlushPos;   //*
    unlock(pSbm);
    return 0;
}

static int lock(Sbm *pSbm)
{
    if(pthread_mutex_lock(&pSbm->mutex) != 0)
        return -1;
    return 0;
}

static void unlock(Sbm *pSbm)
{
    pthread_mutex_unlock(&pSbm->mutex);
    return;
}
