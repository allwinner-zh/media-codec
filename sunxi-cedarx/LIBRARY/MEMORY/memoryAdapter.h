
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
