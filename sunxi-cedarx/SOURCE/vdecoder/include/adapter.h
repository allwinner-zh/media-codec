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
#ifndef ADAPTER_H
#define ADAPTER_H

#include "ve.h"
#include "memoryAdapter.h"

int   AdapterInitialize();

void  AdpaterRelease();

int   AdapterLockVideoEngine(void);

void  AdapterUnLockVideoEngine(void);

void* AdapterMemPalloc(int nSize);

void  AdapterMemPfree(void* pMem);

void  AdapterMemFlushCache(void* pMem, int nSize);

void* AdapterMemGetPhysicAddress(void* pVirtualAddress);

void* AdapterMemGetVirtualAddress(void* pPhysicAddress);

void  AdapterVeReset(void);

int   AdapterVeWaitInterrupt(void);

void* AdapterVeGetBaseAddress(void);

int   AdapterMemGetDramType(void);

void AdapterMemSet(void* pMem, int nValue, int nSize);

void AdapterMemCopy(void* pMemDst, void* pMemSrc, int nSize);

int AdapterMemRead(void* pMemSrc, void* pMemDst, int nSize);

int AdapterMemWrite(void* pMemSrc, void* pMemDst, int nSize);


#endif

