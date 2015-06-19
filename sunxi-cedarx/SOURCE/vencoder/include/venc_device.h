/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd. 
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 * 
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from 
 * Softwinner Products. 
 *
 * The entire notice above must be reproduced on all copies 
 * and should not be removed. 
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


