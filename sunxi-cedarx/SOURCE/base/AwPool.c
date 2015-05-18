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
#include <AwPool.h>
#include <CdxLog.h>
#include <CdxList.h>
#include <CdxAtomic.h>
//#include <CdxLock.h>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#define POOL_BLOCK_SIZE 8192
#define POOL_ALIGNMENT 8
#define POOL_LARGE_SIZE 4095

#define AwAlign(d, a) (((d) + (a - 1)) & ~(a - 1)) /*ÏòÉÏ¶ÔÆë*/

struct PoolDataS
{
    char *last;
    char *end;
    CdxListNodeT node;
    CdxListT pmList;
    cdx_atomic_t ref;
    int failed;
};

struct AwPoolS
{
    struct PoolDataS *current;
    CdxListT largeList; /* large PoolMemoryS list */
    CdxListT pdList; /* PoolDataS list */
    CdxListT childList; /* child pool list */
    CdxListNodeT node; /* in father's child list */
    pthread_mutex_t mutex;
    char *file;
    int line;
};

struct PoolMemoryS
{
    struct PoolDataS *owner;
    char *file;
    int line;
    CdxListNodeT node;
    int size;
};

static AwPoolT *gGolbalPool = NULL;

static inline struct PoolDataS *PoolDataIncRef(struct PoolDataS *poolData)
{
    CdxAtomicInc(&poolData->ref);
    return poolData;
}

static inline void PoolDataDecRef(struct PoolDataS *poolData)
{
    if (CdxAtomicDec(&poolData->ref) == 0)
    {
        CdxListDel(&poolData->node);
        free(poolData);
    }
}

static void *PallocBlock(AwPoolT *pool, int size, char *file, int line)
{
    struct PoolDataS *pd = NULL, *newPd = NULL, *currentPd = NULL;
    CdxListNodeT *pbNode = NULL, *nPbNode = NULL;   
    struct PoolMemoryS *pm = NULL;

    newPd = malloc(POOL_BLOCK_SIZE);
//    newPd = memalign(POOL_ALIGNMENT, POOL_BLOCK_SIZE);
    if (newPd == NULL)
    {
        CDX_LOGE("memalign alloc %d failure errno(%d)", POOL_BLOCK_SIZE, errno);
        return NULL;
    }

    newPd->end = ((char *)newPd) + POOL_BLOCK_SIZE;
    newPd->last = ((char *)newPd) + AwAlign(sizeof(struct PoolDataS), POOL_ALIGNMENT);
    newPd->failed = 0;
    CdxListInit(&newPd->pmList);
    CdxAtomicSet(&newPd->ref, 1);

    for (pbNode = &pool->current->node, nPbNode = pbNode->next;
         pbNode != (CdxListNodeT *)&pool->pdList; 
         pbNode = nPbNode, nPbNode = pbNode->next)
    {
        pd = CdxListEntry(pbNode, struct PoolDataS, node);
        if (pd->failed++ > 4)
        {
            if (pd->node.next != (CdxListNodeT *)&pool->pdList)
            {
                currentPd = CdxListEntry(pd->node.next, struct PoolDataS, node);
            }
            else
            {
                currentPd = newPd;
            }
            PoolDataDecRef(pd);
        }
    }

    CdxListAddTail(&newPd->node, &pool->pdList);
    pool->current = currentPd ? currentPd : pool->current;

    pm = (struct PoolMemoryS *)newPd->last;
    pm->owner = newPd;
    pm->file = file;
    pm->line = line;
    pm->size = size;
    
    newPd->last += AwAlign(sizeof(*pm) + size, POOL_ALIGNMENT);
    CdxListAddTail(&pm->node, &newPd->pmList);
    PoolDataIncRef(newPd);
    
    return pm + 1;
}

static void *PallocLarge(AwPoolT *pool, int size, char *file, int line)
{
    struct PoolMemoryS *pm;

    pm = malloc(size + sizeof(*pm));
    if (pm == NULL) 
    {
        CDX_LOGE("malloc size(%d) failure, errno(%d)", size, errno);
        return NULL;
    }

    pm->owner = NULL;
    pm->file = file;
    pm->line = line;
    pm->size = size;
    CdxListAdd(&pm->node, &pool->largeList);
    return pm + 1;
}

AwPoolT *PoolNodeCreate(char *file, int line)
{
    AwPoolT *pool; 
    struct PoolDataS *poolData;

    pool = malloc(1024);
    poolData = malloc(POOL_BLOCK_SIZE);

//    pool = memalign(POOL_ALIGNMENT, 1024);
//    poolData = memalign(POOL_ALIGNMENT, POOL_BLOCK_SIZE);
    if ((!pool) || (!poolData)) 
    {
        CDX_LOGE("memalign alloc %d failure errno(%d)", POOL_BLOCK_SIZE, errno);
		if(pool)
			free(pool);
		if(poolData)
			free(poolData);
        return NULL;
    }

    poolData->last = (char *) poolData + sizeof(*poolData);
    poolData->end = (char *) poolData + POOL_BLOCK_SIZE;
    poolData->failed = 0;
    CdxListInit(&poolData->pmList);
    
    CdxListInit(&pool->largeList);
    CdxListInit(&pool->pdList);
    pool->current = poolData;
    CdxListInit(&pool->childList);

    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0)
    {
        CDX_LOGE("init thread mutex attr failure...");
        return NULL;
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
    {
        CDX_LOGE("pthread_mutexattr_settype failure...");
        return NULL;
    }
    pthread_mutex_init(&pool->mutex, &attr);

    CdxListAddTail(&poolData->node, &pool->pdList);
    CdxAtomicSet(&poolData->ref, 1); /*pool.data*/

    pool->file = file;
    pool->line = line;

    return pool;
}

AwPoolT *__AwPoolCreate(AwPoolT *father, char *file, int line)
{
    AwPoolT  *pool;
    if (!father)
    {
        if (!gGolbalPool)
        {
            gGolbalPool = PoolNodeCreate(__FILE__, __LINE__);
        }
        father = gGolbalPool;
    }

    pool = PoolNodeCreate(file, line);
    
    CdxListAdd(&pool->node, &father->childList);
    
    return pool;
}

void AwPoolDestroy(AwPoolT *pool)
{
    AwPoolT *p, *nP;
    struct PoolMemoryS *pm, *nPm;
    struct PoolDataS *pd, *nPd;

    /*destroy child  pool*/
    CdxListForEachEntrySafe(p, nP, &pool->childList, node)
    {
        CdxListDel(&p->node);
        AwPoolDestroy(p);
    }

    if (pool != gGolbalPool)
    {
        CdxListDel(&pool->node); /* cut from father's list */
    }
    
    CdxListForEachEntrySafe(pm, nPm, &pool->largeList, node)
    {
        CDX_LOGW("memory leak @<%s:%d>", strrchr(pm->file, '/') + 1, pm->line);        
        CdxListDel(&pm->node);
        free(pm);
    }

    CdxListForEachEntrySafe(pd, nPd, &pool->pdList, node)
    {
        if (pd->failed > 5)
        {
            PoolDataIncRef(pd);
        }
        
        if (CdxAtomicRead(&pd->ref) != 1)
        {
            struct PoolMemoryS *pm;
            CdxListForEachEntry(pm, &pd->pmList, node)
            {
                CDX_LOGW("memory leak @<%s:%d>", strrchr(pm->file, '/') + 1, pm->line);        
                PoolDataDecRef(pd);
            }
        }
        
        CDX_LOG_ASSERT(CdxAtomicRead(&pd->ref) == 1, "ref(%d), failed(%d)", 
                    CdxAtomicRead(&pd->ref), pd->failed);
        PoolDataDecRef(pd);
    }

    pthread_mutex_destroy(&pool->mutex);

    free(pool);
    return ;
}

void *AwPalloc(AwPoolT *pool, int size, char *file, int line)
{
    struct PoolDataS *pd;
    int pmSize;
    void *ret;
        
    if (!pool)
    {
        if (!gGolbalPool)
        {
            gGolbalPool = PoolNodeCreate(file, line);
        }
        pool = gGolbalPool;
    }
    pthread_mutex_lock(&pool->mutex);
    
    pmSize = AwAlign(sizeof(struct PoolMemoryS) + size, POOL_ALIGNMENT);
    
    if (pmSize <= POOL_LARGE_SIZE) 
    {
        CdxListNodeT *pbNode = NULL;
        struct PoolMemoryS *pm = NULL;
    	for (pbNode = &pool->current->node;
             pbNode != (CdxListNodeT *)&pool->pdList; 
    	     pbNode = pbNode->next)
        {
            pd = CdxListEntry(pbNode, struct PoolDataS, node);
            if ((int)(pd->end - pd->last) >= pmSize)
            {
                pm = (struct PoolMemoryS *)pd->last; 
                pm->owner = pd;
                pm->file = file;
                pm->line = line;
                pm->size = size;
                PoolDataIncRef(pm->owner);
                CdxListAddTail(&pm->node, &pd->pmList);
                pd->last += pmSize;
                ret = pm + 1;
                goto out;
            }
        }

        ret = PallocBlock(pool, size, file, line);
        goto out;
    }

    ret = PallocLarge(pool, size, file, line);

out:
    pthread_mutex_unlock(&pool->mutex);
    return ret;
}

void *AwRealloc(AwPoolT *pool, void *p, int size, char *file, int line)
{
    struct PoolMemoryS *pm;
    int freeSize = 0;
    void *newP;
    
    if (!pool)
    {
        pool = gGolbalPool;
    }
    
    pthread_mutex_lock(&pool->mutex);

    pm = ((struct PoolMemoryS *)p) - 1;

    CDX_LOG_ASSERT(size > pm->size, "invalid size, (%d, %d)", size, pm->size);
    
    if (pm->size > POOL_LARGE_SIZE) /*in large memory*/
    {
        newP = PallocLarge(pool, size, file, line);
        if (!newP)
        {
            CDX_LOGE("realloc failure...");
            goto out;
        }
        memcpy(newP, p, pm->size);
        CdxListDel(&pm->node);
        free(pm);
        goto out;
    }

    if (size > POOL_LARGE_SIZE)
    {
        newP = PallocLarge(pool, size, file, line);
        if (!newP)
        {
            CDX_LOGE("realloc failure...");
            goto out;
        }
        memcpy(newP, p, pm->size);
        AwPfree(pool, p);
        goto out;
    }
    
    if (pm->node.next == (CdxListNodeT *)&pm->owner->pmList)
    {
        freeSize = ((char *)(pm->owner)) + POOL_BLOCK_SIZE - ((char *)p);
    }
    else
    {
        struct PoolMemoryS *nextPm;
        nextPm = CdxListEntry(pm->node.next, struct PoolMemoryS, node);
        freeSize = ((char *)nextPm) - ((char *)p);
    }
    
    if (freeSize >= size)
    {
        pm->size = size;
        pm->file = file;
        pm->line = line;
        newP = p;
        if (pm->node.next == (CdxListNodeT *)&pm->owner->pmList)
        {
            pm->owner->last = ((char *)pm) + AwAlign(sizeof(struct PoolMemoryS) + size, POOL_ALIGNMENT);
        }
        goto out;
    }

    newP = AwPalloc(pool, size, file, line);
    memcpy(newP, p, pm->size);
    AwPfree(pool, p);

out:
    pthread_mutex_unlock(&pool->mutex);    
    return newP;
}

void AwPfree(AwPoolT *pool, void *p)
{
    struct PoolMemoryS *pm;
    
    if (!pool)
    {
        pool = gGolbalPool;
    }

    pthread_mutex_lock(&pool->mutex);

    pm = ((struct PoolMemoryS *)p) - 1;
    if (pm->size > POOL_LARGE_SIZE)
    {
        CdxListDel(&pm->node);
        free(pm);
    }
    else
    {
        CdxListDel(&pm->node);
        PoolDataDecRef(pm->owner);
    }

    pthread_mutex_unlock(&pool->mutex);    
    return ;
}

void AwPoolReset(void)
{
    if (gGolbalPool)
    {
        AwPoolDestroy(gGolbalPool);
        gGolbalPool = NULL;
    }
    else
    {
        CDX_LOGW("global pool not initinal...");
    }
    return ;
}

