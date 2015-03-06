/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 Ning Fang <fangning@allwinnertech.com>
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

#ifndef __OMX_TSEM_H__
#define __OMX_TSEM_H__

#include <pthread.h>

typedef struct omx_sem_t
{
	pthread_cond_t 	condition;
	pthread_mutex_t mutex;
	unsigned int 	semval;
}omx_sem_t;

int omx_sem_init(omx_sem_t* tsem, unsigned int val);
void omx_sem_deinit(omx_sem_t* tsem);
void omx_sem_down(omx_sem_t* tsem);
void omx_sem_up(omx_sem_t* tsem);
void omx_sem_reset(omx_sem_t* tsem);
void omx_sem_wait(omx_sem_t* tsem);
void omx_sem_signal(omx_sem_t* tsem);


#endif //__OMX_TSEM_H__

#ifdef __cplusplus
}
#endif /* __cplusplus */

