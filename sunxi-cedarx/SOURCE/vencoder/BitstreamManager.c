/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Author: Ning Fang <fangning@allwinnertech.com>
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
 
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <log.h>
#include <stdlib.h>
#include <string.h>
#include "EncAdapter.h"
#include "BitstreamManager.h"


#define BITSTREAM_FRAME_FIFO_SIZE 256

#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_64B(x) (((x) + (63)) & ~(63))


BitStreamManager* BitStreamCreate(int nBufferSize)
{
    BitStreamManager *handle;
    char *buffer;
    int i;
    int ret;
    
    if(nBufferSize <= 0)
        return NULL;

    buffer = (char*)EncAdapterMemPalloc(nBufferSize);
    if(buffer == NULL)
    {
        loge("pSbmBuf == NULL.");
        return NULL;
    }

	EncAdapterMemFlushCache(buffer, nBufferSize);
	
    handle = (BitStreamManager *)malloc(sizeof(BitStreamManager));
    if(handle == NULL)
    {
        loge("pSbm == NULL.");
        EncAdapterMemPfree(buffer);
        return NULL;
    }
	
    memset(handle, 0, sizeof(BitStreamManager));

    handle->nBSListQ.pStreamInfos = (StreamInfo *)malloc(BITSTREAM_FRAME_FIFO_SIZE * sizeof(StreamInfo));
                                                 
    if(handle->nBSListQ.pStreamInfos == NULL)
    {
        loge("context->nBSListQ.pStreamInfo == NULL.");
        free(handle);
        EncAdapterMemPfree(buffer);
        return NULL;
    }
	
    memset(handle->nBSListQ.pStreamInfos, 0, BITSTREAM_FRAME_FIFO_SIZE * sizeof(StreamInfo));

#if 0
    for(i = 0; i < BITSTREAM_FRAME_FIFO_SIZE; i++)
    {
		handle->nBSListQ.pStreamInfos[i].nID = i;
    }
#endif

    pthread_mutex_init(&handle->mutex, NULL);

    handle->pStreamBuffer      = buffer;

	handle->pStreamBufferPhyAddr = EncAdapterMemGetPhysicAddress(handle->pStreamBuffer);
	handle->pStreamBufferPhyAddrEnd   = handle->pStreamBufferPhyAddr + nBufferSize -1;
	
    handle->nStreamBufferSize  = nBufferSize;
	handle->nWriteOffset       = 0;
    handle->nValidDataSize     = 0;

    handle->nBSListQ.nMaxFrameNum     = BITSTREAM_FRAME_FIFO_SIZE;
    handle->nBSListQ.nValidFrameNum   = 0;
    handle->nBSListQ.nUnReadFrameNum  = 0;
    handle->nBSListQ.nReadPos         = 0;
    handle->nBSListQ.nWritePos        = 0;

	logd("BitStreamCreate OK");
    return handle;
}	

void BitStreamDestroy(BitStreamManager* handle)
{
	if(handle != NULL)
    {
        pthread_mutex_destroy(&handle->mutex);

        if(handle->pStreamBuffer != NULL)
        {
            EncAdapterMemPfree(handle->pStreamBuffer);
            handle->pStreamBuffer = NULL;
        }

        if(handle->nBSListQ.pStreamInfos != NULL)
        {
            free(handle->nBSListQ.pStreamInfos);
            handle->nBSListQ.pStreamInfos = NULL;
        }

        free(handle);
    }
}

void* BitStreamBaseAddress(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return NULL;
    }
	
    return (void *)handle->pStreamBuffer;
}


void* BitStreamBasePhyAddress(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return NULL;
    }
	
    return (void *)handle->pStreamBufferPhyAddr;
}

void* BitStreamEndPhyAddress(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return NULL;
    }
	
    return (void *)handle->pStreamBufferPhyAddrEnd;
}

int BitStreamBufferSize(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return 0;
    }
	
    return handle->nStreamBufferSize;
}

int BitStreamFreeBufferSize(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return 0;
    }
	
    return (handle->nStreamBufferSize - handle->nValidDataSize);
}

int BitStreamFrameNum(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return -1;
    }
	
    return handle->nBSListQ.nValidFrameNum;
}

int BitStreamWriteOffset(BitStreamManager* handle)
{
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return -1;
    }
	
    return handle->nWriteOffset;
}

int BitStreamAddOneBitstream(BitStreamManager* handle, StreamInfo* pStreamInfo)
{
    int nWritePos;
	int NewWriteOffset;
	
	if(handle == NULL || pStreamInfo == NULL)
    {
        loge("param error.");
        return -1;
	}

    if(pthread_mutex_lock(&handle->mutex) != 0)
        return -1;

    if(handle->nBSListQ.nValidFrameNum >= handle->nBSListQ.nMaxFrameNum)
    {
        loge("nValidFrameNum > nMaxFrameNum.");
        pthread_mutex_unlock(&handle->mutex);
        return -1;
    }

	if(pStreamInfo->nStreamLength > (handle->nStreamBufferSize - handle->nValidDataSize))
	{
		loge("pStreamInfo->nStreamLength > freebuffer");
		pthread_mutex_unlock(&handle->mutex);
        return -1;
	}

	
    nWritePos = handle->nBSListQ.nWritePos;
    memcpy(&handle->nBSListQ.pStreamInfos[nWritePos], pStreamInfo, sizeof(StreamInfo));
	
	handle->nBSListQ.pStreamInfos[nWritePos].nID = nWritePos;
    nWritePos++;
    if(nWritePos >= handle->nBSListQ.nMaxFrameNum)
    {
        nWritePos = 0;
    }

	handle->nBSListQ.nWritePos = nWritePos;
    handle->nBSListQ.nValidFrameNum++;
    handle->nBSListQ.nUnReadFrameNum++;
    handle->nValidDataSize += ALIGN_64B(pStreamInfo->nStreamLength); // encoder need 64 byte align
    NewWriteOffset = handle->nWriteOffset + ALIGN_64B(pStreamInfo->nStreamLength);
	
    if(NewWriteOffset >= handle->nStreamBufferSize)
    {
        NewWriteOffset -= handle->nStreamBufferSize;
    }

    handle->nWriteOffset = NewWriteOffset;

    pthread_mutex_unlock(&handle->mutex);
	
    return 0;
}

StreamInfo* BitStreamGetOneBitstream(BitStreamManager* handle)
{
	StreamInfo* pStreamInfo;

	if(handle == NULL)
    {
        loge("handle == NULL");
        return NULL;
	}

    if(pthread_mutex_lock(&handle->mutex) != 0)
    {
    	loge("pthread_mutex_lock failed.");
    	return NULL;
    }

    if(handle->nBSListQ.nUnReadFrameNum == 0)
    {
        loge("nUnReadFrameNum == 0.");
        pthread_mutex_unlock(&handle->mutex);
        return NULL;
    }

    pStreamInfo = &handle->nBSListQ.pStreamInfos[handle->nBSListQ.nReadPos];
    
    if(pStreamInfo == NULL)
    {
        loge("request failed.");
        pthread_mutex_unlock(&handle->mutex);
        return NULL;
    }
    
    handle->nBSListQ.nReadPos++;
    handle->nBSListQ.nUnReadFrameNum--;
    if(handle->nBSListQ.nReadPos >= handle->nBSListQ.nMaxFrameNum)
    {
        handle->nBSListQ.nReadPos = 0;
    }
    pthread_mutex_unlock(&handle->mutex);
	
    return pStreamInfo;
}

int BitStreamReturnOneBitstream(BitStreamManager* handle, StreamInfo* pStreamInfo)
{
	int stream_size;
	
	if(handle == NULL)
    {
        loge("BitStreamManager == NULL.");
        return 0;
    }

	if(pStreamInfo->nID < 0 || pStreamInfo->nID > handle->nBSListQ.nMaxFrameNum)
	{
		loge("pStreamInfo->nID is error");
	}

    if(pthread_mutex_lock(&handle->mutex) != 0)
    {
        return -1;
    }

    if(handle->nBSListQ.nValidFrameNum == 0)
    {
        loge("no valid frame.");
        pthread_mutex_unlock(&handle->mutex);
        return -1;
    }

	stream_size = handle->nBSListQ.pStreamInfos[pStreamInfo->nID].nStreamLength;
    handle->nBSListQ.nValidFrameNum--;
    handle->nValidDataSize -= ALIGN_64B(stream_size);
    pthread_mutex_unlock(&handle->mutex);
		
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

