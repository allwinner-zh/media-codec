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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include "memoryAdapter.h"

#include "config.h"
#include "log.h"

#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM

extern int  sunxi_alloc_open(void);
extern int  sunxi_alloc_close(void);
extern int  sunxi_alloc_alloc(int size);
extern int sunxi_alloc_free(void * pbuf);
extern int  sunxi_alloc_vir2phy(void * pbuf);
extern int  sunxi_alloc_phy2vir(void * pbuf);
extern void sunxi_flush_cache(void* startAddr, int size);

#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)

extern int  ion_alloc_open();
extern int  ion_alloc_close();
extern long ion_alloc_alloc(int size);
extern int  ion_alloc_free(void * pbuf);
extern long ion_alloc_vir2phy(void * pbuf);
extern long ion_alloc_phy2vir(void * pbuf);
extern void ion_flush_cache(void* startAddr, int size);

#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)

extern int  ve_alloc_open();
extern int  ve_alloc_close();
extern long ve_alloc_alloc(int size);
extern int  ve_alloc_free(void * pbuf);
extern long ve_alloc_vir2phy(void * pbuf);
extern long ve_alloc_phy2vir(void * pbuf);
extern void ve_flush_cache(void* startAddr, int size);

#else

#error "invalid configuration of memory driver."

#endif

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif


static pthread_mutex_t gMutexMemAdater = PTHREAD_MUTEX_INITIALIZER;
static int gMemAdapterRefCount = 0;

//* open the memory adater.
int MemAdapterOpen(void)
{
    pthread_mutex_lock(&gMutexMemAdater);
    
    if(gMemAdapterRefCount == 0)
    {
        //* open memory allocate module.
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
        sunxi_alloc_open();
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
        ion_alloc_open();
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
        ve_alloc_open();
#else
    #error "invalid configuration of memory driver."
#endif
    }
    
    gMemAdapterRefCount++;
    
    pthread_mutex_unlock(&gMutexMemAdater);
    
    return 0;    
}


//* close the memory adapter.
void MemAdapterClose(void)
{
    pthread_mutex_lock(&gMutexMemAdater);
    
    if(gMemAdapterRefCount <= 0)
    {
        pthread_mutex_unlock(&gMutexMemAdater);
        return;
    }
    
    gMemAdapterRefCount--;
    
    if(gMemAdapterRefCount == 0)
    {
        //* close memory alloc module.
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
        sunxi_alloc_close();
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
        ion_alloc_close();
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
        ve_alloc_close();
#else
    #error "invalid configuration of memory driver."
#endif
    }
    
    pthread_mutex_unlock(&gMutexMemAdater);
    
    return;
}


//* allocate memory that is physically continue.
void* MemAdapterPalloc(int nSize)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_alloc(nSize);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return (void*)ion_alloc_alloc(nSize);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return (void*)ve_alloc_alloc(nSize);    
#else
    #error "invalid configuration of memory driver."
#endif
}


//* free memory allocated by AdapterMemPalloc()
int  MemAdapterPfree(void* pMem)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return sunxi_alloc_free(pMem);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return ion_alloc_free(pMem);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return ve_alloc_free(pMem);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* synchronize dram and cpu cache.
void  MemAdapterFlushCache(void* pMem, int nSize)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return sunxi_flush_cache(pMem, nSize);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return ion_flush_cache(pMem, nSize);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return ve_flush_cache(pMem, nSize);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* get physic address of a memory block allocated by AdapterMemPalloc().
void* MemAdapterGetPhysicAddress(void* pVirtualAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)(sunxi_alloc_vir2phy(pVirtualAddress) - PHY_OFFSET);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return (void*)(ion_alloc_vir2phy(pVirtualAddress) - PHY_OFFSET);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return (void *)(ve_alloc_vir2phy(pVirtualAddress));
#else
    #error "invalid configuration of memory driver."
#endif
}


//* get virtual address with a memory block's physic address.
void* MemAdapterGetVirtualAddress(void* pPhysicAddress)
{
    //* transform the physic address for modules to physic address for cpu.
    long nPhysicAddressForCpu = (long)pPhysicAddress + PHY_OFFSET;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_phy2vir((void*)nPhysicAddressForCpu);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return (void*)ve_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    #error "invalid configuration of memory driver."
#endif
}

//* get cpu physic address of a memory block allocated by AdapterMemPalloc().
//* 'cpu physic address' means the physic address the cpu saw, it is different 
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetPhysicAddressCpu(void* pVirtualAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_vir2phy(pVirtualAddress);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return (void*)ion_alloc_vir2phy(pVirtualAddress);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return (void*)ve_alloc_vir2phy(pVirtualAddress);
#else
    #error "invalid configuration of memory driver."
#endif
}

//* get virtual address with a memory block's cpu physic address,
//* 'cpu physic address' means the physic address the cpu saw, it is different 
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetVirtualAddressCpu(void* pCpuPhysicAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_phy2vir(pCpuPhysicAddress);
#elif ((CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION) || CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION_LINUX_3_10)
    return (void*)ion_alloc_phy2vir(pCpuPhysicAddress);
#elif (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
    return (void*)ve_alloc_phy2vir(pCpuPhysicAddress);
#else
    #error "invalid configuration of memory driver."
#endif
}
