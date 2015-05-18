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
#ifndef CDX_ATOMIC_H
#define CDX_ATOMIC_H

#include <CdxTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    volatile cdx_ssize counter;
} cdx_atomic_t;

/*引用计数+ 1 ，返回结果的数值*/
static inline cdx_int32 CdxAtomicInc(cdx_atomic_t *ref) 
{
    return __sync_add_and_fetch(&ref->counter, 1L);
}

static inline cdx_int32 CdxAtomicAdd(cdx_atomic_t *ref, cdx_ssize val) 
{
    return __sync_add_and_fetch(&ref->counter, val);
}

/*引用计数- 1 ，返回结果的数值*/
static inline cdx_int32 CdxAtomicDec(cdx_atomic_t *ref)
{
    return __sync_sub_and_fetch(&ref->counter, 1L);
}

static inline cdx_int32 CdxAtomicSub(cdx_atomic_t *ref, cdx_ssize val)
{
    return __sync_sub_and_fetch(&ref->counter, val);
}

/*设置引用计数为val，返回设置前数值*/
static inline cdx_int32 CdxAtomicSet(cdx_atomic_t *ref, cdx_ssize val)
{
    return __sync_lock_test_and_set(&ref->counter, val);
}

/*读取引用计数数值*/
static inline cdx_int32 CdxAtomicRead(cdx_atomic_t *ref)
{
    return __sync_or_and_fetch(&ref->counter, 0L);
}

static inline cdx_bool CdxAtomicCAS(cdx_atomic_t *ref, cdx_ssize oldVal, cdx_ssize newVal)
{
    return __sync_bool_compare_and_swap(&ref->counter, oldVal, newVal);
}



#ifdef __cplusplus
}
#endif

#endif
