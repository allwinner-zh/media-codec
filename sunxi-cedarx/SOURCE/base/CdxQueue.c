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
#include <CdxTypes.h>
#include <CdxAtomic.h>
//#include <CdxMemory.h>
//#include <CdxBaseErrno.h>
#include <CdxLog.h>

#include <CdxQueue.h>

#define CdxAtomicBoolCAS(ptr, oldVal, newVal) \
            __sync_bool_compare_and_swap(ptr, oldVal, newVal)

struct CdxQueueImplS
{
    struct CdxQueueS base;
    struct CdxQueueNodeEntityS *front;
    struct CdxQueueNodeEntityS *rear;
    cdx_bool enablePop;
    cdx_bool enablePush;
    AwPoolT *pool;
};

struct CdxQueueNodeEntityS
{
    struct CdxQueueNodeEntityS *next;
    cdx_atomic_t ref;
    CdxQueueDataT data;
};

static inline struct CdxQueueNodeEntityS *
        QueueNodeEntityIncRef(struct CdxQueueNodeEntityS *entity)
{
    CdxAtomicInc(&entity->ref);
    return entity;
}

static inline cdx_void QueueNodeEntityDecRef(AwPoolT *pool, struct CdxQueueNodeEntityS *entity)
{
    if (CdxAtomicDec(&entity->ref) == 0)
    {
        Pfree(pool, entity);
    }
}


static CdxQueueDataT __CdxQueuePop(CdxQueueT *queue)
{
    struct CdxQueueImplS *impl;
    struct CdxQueueNodeEntityS *entity = NULL;
    CdxQueueDataT data;
    CDX_ASSERT(queue);
    impl = CdxContainerOf(queue, struct CdxQueueImplS, base);

    if (!impl->enablePop)
    {
        return NULL;
    }
    
    do
    {
        if (entity)
        {
            QueueNodeEntityDecRef(impl->pool, entity);
        }
        entity = QueueNodeEntityIncRef(impl->front);
        if (entity->next == NULL)
        {
            QueueNodeEntityDecRef(impl->pool, entity);
            return NULL;
        }
        data = entity->next->data; 
        /*
              *先把数据保存下来，
              *避免取到entity之后，它的next被释放了
              */
    }
    while (!CdxAtomicBoolCAS(&impl->front, entity, entity->next));
    QueueNodeEntityDecRef(impl->pool, entity); /*对应上面取entity的时候+1*/
    QueueNodeEntityDecRef(impl->pool, entity); /*再-1 才能释放内存*/
    
    return data;
}

static cdx_err __CdxQueuePush(CdxQueueT *queue, CdxQueueDataT data)
{
    struct CdxQueueImplS *impl;
    struct CdxQueueNodeEntityS *entity, *tmpEntity;
    cdx_bool ret;
    
    CDX_ASSERT(queue);
    impl = CdxContainerOf(queue, struct CdxQueueImplS, base);
    
    if (!impl->enablePush)
    {
        return -1;
    }
    entity = Palloc(impl->pool, sizeof(*entity));
    CDX_ASSERT(entity);
    CDX_ASSERT(data);/*不希望有为0的data*/
    entity->data = data;
    entity->next = NULL;
    CdxAtomicSet(&entity->ref, 1);

    do
    {
        tmpEntity = impl->rear;
    }
    while (!CdxAtomicBoolCAS(&tmpEntity->next, NULL, entity));

    ret = CdxAtomicBoolCAS(&impl->rear, tmpEntity, entity);
    CDX_ASSERT(CDX_TRUE == ret);

    return CDX_SUCCESS;
}

static cdx_bool __CdxQueueEmpty(CdxQueueT *queue)
{
    struct CdxQueueImplS *impl;
        
    CDX_ASSERT(queue);
    impl = CdxContainerOf(queue, struct CdxQueueImplS, base);
    
    return impl->front == impl->rear;
}

static struct CdxQueueOpsS gQueueOps =
{
    .pop = __CdxQueuePop,
    .push = __CdxQueuePush,
    .empty = __CdxQueueEmpty
};

CdxQueueT *CdxQueueCreate(AwPoolT *pool)
{
    struct CdxQueueImplS *impl;
    struct CdxQueueNodeEntityS *dummy;
    impl = Palloc(pool, sizeof(struct CdxQueueImplS));
    CDX_ASSERT(impl);
    memset(impl, 0x00, sizeof(struct CdxQueueImplS));

    impl->pool = pool;
    dummy = Palloc(impl->pool, sizeof(struct CdxQueueNodeEntityS));
    CDX_ASSERT(dummy);
    dummy->next = NULL;
    dummy->data = NULL;

    impl->front = dummy;
    impl->rear = dummy;
    impl->base.ops = &gQueueOps;
    impl->enablePop = CDX_TRUE;
    impl->enablePush = CDX_TRUE;
    return &impl->base;
}

cdx_void CdxQueueDestroy(CdxQueueT *queue)
{
    struct CdxQueueImplS *impl;
//    struct CdxQueueNodeEntityS *dummy;
    
    CDX_ASSERT(queue);
    impl = CdxContainerOf(queue, struct CdxQueueImplS, base);

    impl->enablePush = CDX_FALSE;
    
    impl->enablePop = CDX_FALSE;
    CDX_LOG_ASSERT(impl->front == impl->rear, "queue not empty");

    QueueNodeEntityDecRef(impl->pool, impl->front);
    Pfree(impl->pool, impl);

    return ;
}

