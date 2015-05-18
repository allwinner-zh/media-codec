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
#include "adapter.h"
#include "log.h"

//* ve control methods.
int AdapterInitialize(int bIsSecureVideoFlag)
{
    if(VeInitialize() < 0)
        return -1;
    
    if(MemAdapterOpen() < 0)
       return -1;
    return 0;    
}


void AdpaterRelease()
{
    VeRelease();
    MemAdapterClose();
    return;
}


int AdapterLockVideoEngine(void)
{
    return VeLock();
}


void AdapterUnLockVideoEngine(void)
{
    VeUnLock();
}


void AdapterVeReset(void)
{
    VeReset();
}


int AdapterVeWaitInterrupt(void)
{
    return VeWaitInterrupt();
}


void* AdapterVeGetBaseAddress(void)
{
    return VeGetRegisterBaseAddress();
}


int AdapterMemGetDramType(void)
{
    return VeGetDramType();
}


//* memory methods.

void* AdapterMemPalloc(int nSize)
{
	void* pPallocPtr = NULL;

	pPallocPtr = MemAdapterPalloc(nSize);
	if(pPallocPtr != NULL)
	{
		logv("palloc nPallocSize=%d\n", nSize);
	}
	return pPallocPtr;
}


void AdapterMemPfree(void* pMem)
{
	int nFreeSize = 0;

    nFreeSize = MemAdapterPfree(pMem);
    logv("free nFreeSize=%d\n", nFreeSize);
}


void AdapterMemFlushCache(void* pMem, int nSize)
{
    MemAdapterFlushCache(pMem, nSize);
}


void* AdapterMemGetPhysicAddress(void* pVirtualAddress)
{
    return MemAdapterGetPhysicAddress(pVirtualAddress);
}


void* AdapterMemGetVirtualAddress(void* pPhysicAddress)
{
    return MemAdapterGetVirtualAddress(pPhysicAddress);
}


void AdapterMemSet(void* pMem, int nValue, int nSize)
{
    memset(pMem, nValue, nSize);
}

void AdapterMemCopy(void* pMemDst, void* pMemSrc, int nSize)
{
    memcpy(pMemDst, pMemSrc, nSize);
}

int AdapterMemRead(void* pMemSrc, void* pMemDst, int nSize)
{
    memcpy(pMemDst, pMemSrc, nSize);
    return 0;
}

int AdapterMemWrite(void* pMemSrc, void* pMemDst, int nSize)
{
    memcpy(pMemDst, pMemSrc, nSize);
    return 0;
}
