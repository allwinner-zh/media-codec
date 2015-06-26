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

#ifndef _BITSTREAM_MANAGER_H_
#define _BITSTREAM_MANAGER_H_

#include <pthread.h>

typedef struct StreamInfo
{
	int 		nStreamOffset;
	int 		nStreamLength;
	long long   nPts;
	int         nFlags;
	int         nID;
}StreamInfo;

typedef struct BSListQ
{
    StreamInfo* pStreamInfos;
    int         nMaxFrameNum;
    int         nValidFrameNum;
    int         nUnReadFrameNum;
    int         nReadPos;
    int         nWritePos;
}BSListQ;

typedef struct BitStreamManager
{
    pthread_mutex_t mutex;
    char*           pStreamBuffer;
	char*           pStreamBufferPhyAddrEnd;
	char*           pStreamBufferPhyAddr;
    int             nStreamBufferSize;
	int             nWriteOffset;
    int             nValidDataSize;
    BSListQ 		nBSListQ;
}BitStreamManager;

BitStreamManager* BitStreamCreate(int nBufferSize);
void BitStreamDestroy(BitStreamManager* handle);
void* BitStreamBaseAddress(BitStreamManager* handle);
void* BitStreamBasePhyAddress(BitStreamManager* handle);
void* BitStreamEndPhyAddress(BitStreamManager* handle);
int BitStreamBufferSize(BitStreamManager* handle);
int BitStreamFreeBufferSize(BitStreamManager* handle);
int BitStreamFrameNum(BitStreamManager* handle);
int BitStreamWriteOffset(BitStreamManager* handle);
int BitStreamAddOneBitstream(BitStreamManager* handle, StreamInfo* pStreamInfo);
StreamInfo* BitStreamGetOneBitstream(BitStreamManager* handle);
int BitStreamReturnOneBitstream(BitStreamManager* handle, StreamInfo* pStreamInfo);


#endif //_BITSTREAM_MANAGER_H_


#ifdef __cplusplus
}
#endif /* __cplusplus */

