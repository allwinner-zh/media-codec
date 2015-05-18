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
#ifndef CDX_QUEUE_H
#define CDX_QUEUE_H
#include <CdxTypes.h>
#include <CdxLog.h>
#include <AwPool.h>

typedef struct CdxQueueS CdxQueueT;
typedef cdx_void * CdxQueueDataT;

struct CdxQueueOpsS
{
    CdxQueueDataT (*pop)(CdxQueueT *);
    cdx_err (*push)(CdxQueueT *, CdxQueueDataT);
    cdx_bool (*empty)(CdxQueueT *);
    
};

struct CdxQueueS
{
    struct CdxQueueOpsS *ops;
};

static inline CdxQueueDataT CdxQueuePop(CdxQueueT *queue)
{
    CDX_ASSERT(queue);
    CDX_ASSERT(queue->ops);
    CDX_ASSERT(queue->ops->pop);
    return queue->ops->pop(queue);
}

static inline cdx_err CdxQueuePush(CdxQueueT *queue, CdxQueueDataT data)
{
    CDX_ASSERT(queue);
    CDX_ASSERT(queue->ops);
    CDX_ASSERT(queue->ops->push);
    return queue->ops->push(queue, data);
}


static inline cdx_bool CdxQueueEmpty(CdxQueueT *queue)
{
    CDX_ASSERT(queue);
    CDX_ASSERT(queue->ops);
    CDX_ASSERT(queue->ops->empty);
    return queue->ops->empty(queue);
}

#ifdef __cplusplus
extern "C"
{
#endif
/*this is a look free queue*/
CdxQueueT *CdxQueueCreate(AwPoolT *pool);

cdx_void CdxQueueDestroy(CdxQueueT *queue);

#ifdef __cplusplus
}
#endif

#endif
