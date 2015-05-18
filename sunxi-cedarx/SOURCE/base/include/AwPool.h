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
#ifndef AW_POOL_H
#define AW_POOL_H

#include <CdxTypes.h>
#include <string.h>

typedef struct AwPoolS AwPoolT;

#define Palloc(pool, size) \
    AwPalloc(pool, size, __FILE__, __LINE__)

#define Pfree(pool, p) \
    AwPfree(pool, p)

#define Pstrdup(pool, str)   \
    AwStrDup(pool, str, __FILE__, __LINE__)

#define Prealloc(pool, p, size) \
    AwRealloc(pool, p, size, __FILE__, __LINE__)
    
#ifdef __cplusplus
extern "C"
{
#endif

AwPoolT *__AwPoolCreate(AwPoolT *father, char *file, int line);

#define AwPoolCreate(father) __AwPoolCreate(father, __FILE__, __LINE__)

void AwPoolDestroy(AwPoolT *pool);

void *AwPalloc(AwPoolT *pool, int size, char *file, int line);

void AwPfree(AwPoolT *pool, void *p);

void AwPoolReset(void);

static inline char *AwStrDup(AwPoolT *pool, const char *str, char *file, int line)
{
    char *ret = NULL;
    int strLen = 0;
//    CDX_ASSERT(str);

    strLen = strlen(str);
    
    ret = (char *)AwPalloc(pool, strLen + 1, file, line);
    strcpy(ret, str);
    ret[strLen] = 0;
    return ret;
}

void *AwRealloc(AwPoolT *pool, void *p, int size, char *file, int line);

#ifdef __cplusplus
}
#endif

#endif /* AW_POOL_H */

