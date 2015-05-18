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
#ifndef CDX_LIST_H
#define CDX_LIST_H
#include <CdxTypes.h>

#define CDX_LIST_POISON1  ((void *) 0x00700700) 
#define CDX_LIST_POISON2  ((void *) 0x00900900) 

struct CdxListNodeS
{
    struct CdxListNodeS *next;
    struct CdxListNodeS *prev;
};

struct CdxListS
{
    struct CdxListNodeS *head;
    struct CdxListNodeS *tail;
};

#define CdxListInit(list) do { \
    (list)->head = (list)->tail = (struct CdxListNodeS *)(list);\
    }while (0)
    
#define CdxListNodeInit(node) do { \
    (node)->next = (node)->prev = (node);\
    }while (0)

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef AWP_DEBUG

void CdxListAdd(struct CdxListNodeS *new, struct CdxListS *list);

void CdxListAddBefore(struct CdxListNodeS *new, struct CdxListNodeS *pos);

void CdxListAddAfter(struct CdxListNodeS *new, struct CdxListNodeS *pos);

void CdxListAddTail(struct CdxListNodeS *new, struct CdxListS *list);

void CdxListDel(struct CdxListNodeS *node);

int CdxListEmpty(const struct CdxListS *list);

#else
#include <CdxList.i>
#endif

#ifdef __cplusplus
}
#endif

#define CdxListEntry(ptr, type, member) \
	CdxContainerOf(ptr, type, member)

#define CdxListFirstEntry(ptr, type, member) \
	CdxListEntry((ptr)->head, type, member)

#define CdxListForEach(pos, list) \
	for (pos = (list)->head; \
	        pos != (struct CdxListNodeS *)(list);\
	        pos = pos->next)

#define CdxListForEachPrev(pos, list) \
	for (pos = (list)->tail; \
	    pos != (struct CdxListNodeS *)(list); \
	    pos = pos->prev)

#define CdxListForEachSafe(pos, n, list) \
	for (pos = (list)->head, n = pos->next; \
	    pos != (struct CdxListNodeS *)(list); \
		pos = n, n = pos->next)

#define CdxListForEachPrevSafe(pos, n, list) \
	for (pos = (list)->tail, n = pos->prev; \
	     pos != (struct CdxListNodeS *)(list); \
	     pos = n, n = pos->prev)

#define CdxListForEachEntry(pos, list, member)				\
	for (pos = CdxListEntry((list)->head, typeof(*pos), member);	\
	     &pos->member != (struct CdxListNodeS *)(list); 	\
	     pos = CdxListEntry(pos->member.next, typeof(*pos), member))

#define CdxListForEachEntryReverse(pos, list, member)			\
	for (pos = CdxListEntry((list)->tail, typeof(*pos), member);	\
	     &pos->member != (struct CdxListNodeS *)(list); 	\
	     pos = CdxListEntry(pos->member.prev, typeof(*pos), member))

#define CdxListForEachEntrySafe(pos, n, list, member)			\
	for (pos = CdxListEntry((list)->head, typeof(*pos), member),	\
		n = CdxListEntry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (struct CdxListNodeS *)(list); 					\
	     pos = n, n = CdxListEntry(n->member.next, typeof(*n), member))

#define CdxListForEachEntrySafeReverse(pos, n, list, member)		\
	for (pos = CdxListEntry((list)->prev, typeof(*pos), member),	\
		n = CdxListEntry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (struct CdxListNodeS *)(list); 					\
	     pos = n, n = CdxListEntry(n->member.prev, typeof(*n), member))

#endif
