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


int  ve_alloc_open()
{
    return 0;
}

int  ve_alloc_close()
{
    return 0;
}

long ve_alloc_alloc(int size)
{
    return VeMalloc(size);
}

int  ve_alloc_free(void * pbuf)
{
    return VeFree(pbuf);
}

long ve_alloc_vir2phy(void * pbuf)
{
    return VeVir2Phy(pbuf);
}

long ve_alloc_phy2vir(void * pbuf)
{
    return VePhy2Vir(pbuf);
}

void ve_flush_cache(void* startAddr, int size)
{
    VeFlushCache(startAddr, size);
}

