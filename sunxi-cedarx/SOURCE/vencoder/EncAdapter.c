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
 
#include "log.h"

#include "EncAdapter.h"

//* use method provide by libVE.so to process ve control methods.
//* use method provide by libMemAdapter.so to process physical continue memory allocation.

//* ve control methods.
int EncAdapterInitialize(void)
{
    if(VeInitialize() < 0)
        return -1;
    
    if(MemAdapterOpen() < 0)
        return -1;
	//If mediaserver died when playing protected media,
    //some hardware is in protected mode, shutdown.
#if(CONFIG_OS == OPTION_OS_ANDROID)
{
	SecureMemAdapterOpen();
	SecureMemAdapterClose();
}
#endif
    return 0;    
}

void EncAdpaterRelease(void)
{
    MemAdapterClose();
    VeRelease();
    return;
}


int EncAdapterLockVideoEngine(void)
{
    return VeEncoderLock();
}


void EncAdapterUnLockVideoEngine(void)
{
    VeEncoderUnLock();
}


void EncAdapterVeReset(void)
{
    VeReset();
}


int EncAdapterVeWaitInterrupt(void)
{
    return VeWaitEncoderInterrupt();
}


void* EncAdapterVeGetBaseAddress(void)
{
    return VeGetRegisterBaseAddress();
}

int EncAdapterMemGetDramType(void)
{
    return VeGetDramType();
}


//* memory methods.

void* EncAdapterMemPalloc(int nSize)
{
    return MemAdapterPalloc(nSize);
}

void EncAdapterMemPfree(void* pMem)
{
    MemAdapterPfree(pMem);
}


void EncAdapterMemFlushCache(void* pMem, int nSize)
{
    MemAdapterFlushCache(pMem, nSize);
}


void* EncAdapterMemGetPhysicAddress(void* pVirtualAddress)
{
    return MemAdapterGetPhysicAddress(pVirtualAddress);
}


void* EncAdapterMemGetPhysicAddressCpu(void* pVirtualAddress)
{
    return MemAdapterGetPhysicAddressCpu(pVirtualAddress);
}


void* EncAdapterMemGetVirtualAddress(void* pPhysicAddress)
{
    return MemAdapterGetVirtualAddress(pPhysicAddress);
}


void EncAdapterEnableEncoder(void)
{
	VeEnableEncoder();
}


void EncAdapterDisableEncoder(void)
{
	VeDisableEncoder();
}


void EncAdapterResetEncoder(void)
{
	VeResetEncoder();
}


void EncAdapterInitPerformance(int nMode)
{
	VeInitEncoderPerformance(nMode);
}


void EncAdapterUninitPerformance(int nMode)
{
	VeUninitEncoderPerformance(nMode);
}


unsigned int EncAdapterGetICVersion(void)
{
   volatile unsigned int value;
   value = *((unsigned int*)((char *)VeGetRegisterBaseAddress() + 0xf0));
   return (value>>16);
}


void EncAdapterSetDramType(void)
{
	VeSetDramType();
}


void EncAdapterPrintTopVEReg(void)
{
	int i;
	volatile int *ptr = (int*)VeGetRegisterBaseAddress();
	
	logd("--------- register of top level ve base:%p -----------\n",ptr);
	for(i=0;i<16;i++)
	{
		logd("row %2d: %08x %08x %08x %08x",i,ptr[0],ptr[1],ptr[2],ptr[3]);
		ptr += 4;
	}
}


void EncAdapterPrintEncReg(void)
{
	int i;
	volatile int *ptr = (int*)((unsigned long)VeGetRegisterBaseAddress() + 0xB00);
	
	logd("--------- register of ve encoder base:%p -----------\n",ptr);
	for(i=0;i<16;i++)
	{
		logd("row %2d: %08x %08x %08x %08x",i,ptr[0],ptr[1],ptr[2],ptr[3]);
		ptr += 4;
	}
}


void EncAdapterPrintIspReg(void)
{
	int i;
	volatile int *ptr = (int*)((unsigned long)VeGetRegisterBaseAddress() + 0xA00);
	
	logd("--------- register of ve isp base:%p -----------\n",ptr);
	for(i=0;i<16;i++)
	{
		logd("row %2d: %08x %08x %08x %08x",i,ptr[0],ptr[1],ptr[2],ptr[3]);
		ptr += 4;
	}
}


