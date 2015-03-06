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
#ifndef MEMORY_ADAPTER_H
#define MEMORY_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

//* open the memory adater.
int MemAdapterOpen(void);

//* close the memory adapter.
void MemAdapterClose(void);

//* allocate memory that is physically continue.
void* MemAdapterPalloc(int nSize);

//* free memory allocated by AdapterMemPalloc()
int  MemAdapterPfree(void* pMem);

//* synchronize dram and cpu cache.
void  MemAdapterFlushCache(void* pMem, int nSize);

//* get physic address of a memory block allocated by AdapterMemPalloc().
void* MemAdapterGetPhysicAddress(void* pVirtualAddress);

//* get virtual address with a memory block's physic address.
void* MemAdapterGetVirtualAddress(void* pPhysicAddress);

//* get cpu physic address of a memory block allocated by AdapterMemPalloc().
//* 'cpu physic address' means the physic address the cpu saw, it is different 
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetPhysicAddressCpu(void* pVirtualAddress);

//* get virtual address with a memory block's cpu physic address,
//* 'cpu physic address' means the physic address the cpu saw, it is different 
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetVirtualAddressCpu(void* pCpuPhysicAddress);


#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ADAPTER_H */
