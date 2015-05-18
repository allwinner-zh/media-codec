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

CDX_INTERFACE void __CdxListAdd(struct CdxListNodeS *new,
                        struct CdxListNodeS *prev, struct CdxListNodeS *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

CDX_INTERFACE void CdxListAdd(struct CdxListNodeS *new, struct CdxListS *list)
{
	__CdxListAdd(new, (struct CdxListNodeS *)list, list->head);
}

CDX_INTERFACE void CdxListAddBefore(struct CdxListNodeS *new, 
                                    struct CdxListNodeS *pos)
{
	__CdxListAdd(new, pos->prev, pos);
}

CDX_INTERFACE void CdxListAddAfter(struct CdxListNodeS *new, 
                                    struct CdxListNodeS *pos)
{
	__CdxListAdd(new, pos, pos->next);
}

CDX_INTERFACE void CdxListAddTail(struct CdxListNodeS *new, struct CdxListS *list)
{
	__CdxListAdd(new, list->tail, (struct CdxListNodeS *)list);
}

CDX_INTERFACE void __CdxListDel(struct CdxListNodeS *prev, struct CdxListNodeS *next)
{
	next->prev = prev;
	prev->next = next;
}

CDX_INTERFACE void CdxListDel(struct CdxListNodeS *node)
{
	__CdxListDel(node->prev, node->next);
	node->next = CDX_LIST_POISON1;
	node->prev = CDX_LIST_POISON2;
}

CDX_INTERFACE int CdxListEmpty(const struct CdxListS *list)
{
	return (list->head == (struct CdxListNodeS *)list) 
	       && (list->tail == (struct CdxListNodeS *)list);
}

