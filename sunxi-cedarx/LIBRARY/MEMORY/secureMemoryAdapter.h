/*
 * secureos_adapter.h
 *
 *  Created on: 2014-8-18
 *      Author: wei
 */

#ifndef SECURE_MEMORY_ADAPTER_H_
#define SECURE_MEMORY_ADAPTER_H_
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Allocate secure memory.
 *
 * @param size: memory size to allocate
 *
 * @param align: memory should be aligned with how many bytes.
 *
 * @return: returns secure memory address allocated on success or NULL
 * on error.
 */
void *SecureMemAdapterAlloc(size_t size);

/**
 * Release secure memory.
 *
 * @param ptr: memory to release.
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterFree(void *ptr);

/**
 * Copy n bytes from secure memory area src to secure memory area dest
 *
 * @param dest: secure memory to copy data to.
 *
 * @param src: secure memory to copy data from.
 *
 * @param n: bytes to be copied.
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterCopy(void *dest, void *src, size_t n);

/**
 * Fills the first n bytes of the memory area pointed to by s with the constant byte c
 *
 * @param s: secure memory to set.
 *
 * @param c: value to fill s with.
 *
 * @param n: bytes to be filled.
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterSet(void *s, int c, size_t n);

/**
 * Get physical address accessed by VE of secure memory from virtual address virt.
 *
 * @param virt: secure memory virtual address.
 *
 * @return: returns physical address of memory virt.
 */
void* SecureMemAdapterGetPhysicAddress(void *virt);

/**
 * Get virtual address of secure memory from physical address phy.
 *
 * @param phy: secure memory physical address accessed by VE.
 *
 * @return: returns physical address of memory phy.
 */
void* SecureMemAdapterGetVirtualAddress(void *phy);

/**
 * Get physical address accessed by CPU of secure memory from virtual address virt.
 *
 * @param virt: secure memory virtual address.
 *
 * @return: returns physical address of memory virt.
 */
void* SecureMemAdapterGetPhysicAddressCpu(void *virt);

/**
 * Get virtual address of secure memory from physical address phy.
 *
 * @param phy: secure memory physical address accessed by CPU.
 *
 * @return: returns physical address of memory phy.
 */
void* SecureMemAdapterGetVirtualAddressCpu(void *phy);
/**
 * Flush cache from ptr to ptr + size -1.
 *
 * @param ptr: secure memory physical address.
 *
 * @param size: secure memory size to flush
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterFlushCache(void *ptr, size_t size);

/**
 * Read n bytes data from secure memory src to non-secure memory dest.
 *
 * @param src: secure memory address to read data.
 *
 * @param dest: non-secure memory address to store data.
 *
 * @param n: size to read
 *
 * @return: returns bytes read on success and errorno on error.
 */
int SecureMemAdapterRead(void *src, void *dest, size_t n);

/**
 * Write n bytes data from non-secure memory dest to non-secure memory src.
 *
 * @param src: non-secure memory address.
 *
 * @param dest: secure memory address to write data.
 *
 * @param n: size to write
 *
 * @return: returns bytes written on success and errorno on error.
 */
int SecureMemAdapterWrite(void *src, void *dest, size_t n);

/**
 * Debug secure os
 *
 * @param s: if s is not zero, secure os will stay in a dead loop.
 *
 * @return: none
 */
int SecureMemAdapterDebug(int s);

/**
 * Dump secure memory
 *
 * @param ptr: secure memory address to dump.
 *
 * @param size: size to dump
 *
 * @return: none
 */
int SecureMemAdapterDump(void *ptr, size_t size);

/**
 * Open secure memory adapter, make sure memoryAdapter has been
 * opened before opening secureMemoryAdapter.
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterOpen();

/**
 * Close secure memory adapter.
 *
 * @return: returns 0 on success or errorno on error.
 */
int SecureMemAdapterClose();
#ifdef __cplusplus
}
#endif
#endif /* SECURE_MEMORY_ADAPTER_H_ */
