


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

