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

#ifndef _VENC_DEVICE_H_
#define _VENC_DEVICE_H_

#endif //_VENC_DEVICE_H_

#include "vencoder.h"

typedef struct VENC_DEVICE
{
	const char *codecType;
	void*      (*open)();
	int        (*init)(void *handle,VencBaseConfig* pBaseConfig);
	int        (*uninit)(void *handle);
	void       (*close)(void *handle);
	int        (*encode)(void *handle, VencInputBuffer* pInBuffer);
	int        (*GetParameter)(void *handle, int indexType, void* param);
	int        (*SetParameter)(void *handle, int indexType, void* param);
	int        (*ValidBitStreamFrameNum)(void *handle);
    int        (*GetOneBitStreamFrame)(void *handle, VencOutputBuffer *pOutBuffer);
    int        (*FreeOneBitStreamFrame)(void *handle, VencOutputBuffer *pOutBuffer);
}VENC_DEVICE;


VENC_DEVICE *VencoderDeviceCreate(VENC_CODEC_TYPE type);
void VencoderDeviceDestroy(void *handle);

int VEncoderRegister(VENC_CODEC_TYPE type, char *desc, VENC_DEVICE *device);


#ifdef __cplusplus
}
#endif /* __cplusplus */


