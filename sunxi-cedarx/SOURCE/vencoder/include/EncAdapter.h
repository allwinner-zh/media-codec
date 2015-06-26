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

#ifndef _ENC_ADAPTER_H
#define _ENC_ADAPTER_H

#include "ve.h"
#include "memoryAdapter.h"
#include "config.h"

int   EncAdapterInitialize(void);

void  EncAdpaterRelease(void);

int   EncAdapterLockVideoEngine(void);

void  EncAdapterUnLockVideoEngine(void);

void* EncAdapterMemPalloc(int nSize);

void  EncAdapterMemPfree(void* pMem);

void  EncAdapterMemFlushCache(void* pMem, int nSize);

void* EncAdapterMemGetPhysicAddress(void* pVirtualAddress);

void* EncAdapterMemGetPhysicAddressCpu(void* pVirtualAddress);

void* EncAdapterMemGetVirtualAddress(void* pPhysicAddress);

void  EncAdapterVeReset(void);

int   EncAdapterVeWaitInterrupt(void);

void* EncAdapterVeGetBaseAddress(void);

int   EncAdapterMemGetDramType(void);

void EncAdapterEnableEncoder(void);

void EncAdapterDisableEncoder(void);

void EncAdapterResetEncoder(void);

void EncAdapterInitPerformance(int nMode);

void EncAdapterUninitPerformance(int nMode);

unsigned int EncAdapterGetICVersion(void);

void EncAdapterSetDramType(void);

void EncAdapterPrintTopVEReg(void);

void EncAdapterPrintEncReg(void);

void EncAdapterPrintIspReg(void);


#endif //_ENC_ADAPTER_H

#ifdef __cplusplus
}
#endif /* __cplusplus */


