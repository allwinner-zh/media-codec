#ifndef VE_ALLOC_H
#define VE_ALLOC_H

int  ve_alloc_open();
int  ve_alloc_close();
long ve_alloc_alloc(int size);
int  ve_alloc_free(void * pbuf);
long ve_alloc_vir2phy(void * pbuf);
long ve_alloc_phy2vir(void * pbuf);
void ve_flush_cache(void* startAddr, int size);

#endif
