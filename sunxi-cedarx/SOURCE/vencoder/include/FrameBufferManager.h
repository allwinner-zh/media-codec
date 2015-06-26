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
#ifndef _FRAME_BUFFER_MANAGER_H
#define _FRAME_BUFFER_MANAGER_H


typedef struct VencInputBufferInfo VencInputBufferInfo;

struct VencInputBufferInfo
{
    VencInputBuffer   		inputbuffer;
    VencInputBufferInfo*    next;
};

typedef struct AllocateBufferManage
{
	unsigned int  	   	   min_num;
	unsigned int  	   	   max_num;
    unsigned int  	   	   buffer_num;
	unsigned int		   quene_buffer_num;
    VencInputBufferInfo*   allocate_queue;
	VencInputBufferInfo*   allocate_buffer;
	pthread_mutex_t    	   mutex;
}AllocateBufferManage;

typedef struct InputBufferList
{
    unsigned int  	   	   size_of_list;
	unsigned int		   max_size;

	VencInputBufferInfo*   used_quene;
	VencInputBufferInfo*   input_quene;
	VencInputBufferInfo*   empty_quene;
	VencInputBufferInfo*   buffer_quene;
	pthread_mutex_t    	   mutex;
}InputBufferList;

typedef struct FrameBufferManager
{
   AllocateBufferManage ABM_inputbuffer;
   InputBufferList      inputbuffer_list;
   unsigned int         size_y;
   unsigned int         size_c;
}FrameBufferManager;


FrameBufferManager* FrameBufferManagerCreate(int num);
void FrameBufferManagerDestroy(FrameBufferManager* fbm);
int AddInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);
int GetInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);
int AddUsedInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);
int GetUsedInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);

int AllocateInputBuffer(FrameBufferManager* fbm, VencAllocateBufferParam *buffer_param);
int GetOneAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer* inputbuffer);
int FlushCacheAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);
int ReturnOneAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer);
void FreeAllocateInputBuffer(FrameBufferManager* fbm);


#endif //_FRAME_BUFFER_MANAGER_H


#ifdef __cplusplus
}
#endif /* __cplusplus */


