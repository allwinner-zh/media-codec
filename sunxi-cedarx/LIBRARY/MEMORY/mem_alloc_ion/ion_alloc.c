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

#include "config.h"
#include "log.h"
#include "ion_alloc.h"
#include "ion_alloc_list.h"

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
#define ION_ALLOC_ALIGN (SZ_1M)
#else
#define ION_ALLOC_ALIGN	SZ_1k
#endif

#define DEV_NAME 			"/dev/ion"

typedef struct BUFFER_NODE
{
	struct list_head i_list;
	unsigned long phy;		//phisical address
	unsigned long vir;		//virtual address
	unsigned int size;		//buffer size
#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
	int dmabuf_fd;	//dma_buffer fd
	ion_user_handle_t handle;		//alloc data handle
#else
	struct ion_fd_data fd_data;
#endif
}buffer_node;

typedef struct ION_ALLOC_CONTEXT
{
	int					fd;			// driver handle
	struct list_head	list;		// buffer list
	int					ref_cnt;	// reference count
}ion_alloc_context;

ion_alloc_context	*	g_alloc_context = NULL;
pthread_mutex_t			g_mutex_alloc = PTHREAD_MUTEX_INITIALIZER;

/*funciton begin*/
int ion_alloc_open()
{
	logv("ion_alloc_open \n");
	
	pthread_mutex_lock(&g_mutex_alloc);
	if (g_alloc_context != NULL)
	{
		logv("ion allocator has already been created \n");
		goto SUCCEED_OUT;
	}

	g_alloc_context = (ion_alloc_context*)malloc(sizeof(ion_alloc_context));
	if (g_alloc_context == NULL)
	{
		logv("create ion allocator failed, out of memory \n");
		goto ERROR_OUT;
	}
	else
	{
		logv("pid: %d, g_alloc_context = %p \n", getpid(), g_alloc_context);
	}

	memset((void*)g_alloc_context, 0, sizeof(ion_alloc_context));
#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
	g_alloc_context->fd = open(DEV_NAME, O_RDWR, 0);
#else
	g_alloc_context->fd = open(DEV_NAME, /*O_RDWR */O_RDONLY, 0);
#endif
	if (g_alloc_context->fd <= 0)
	{
		logv("open %s failed \n", DEV_NAME);
		goto ERROR_OUT;
	}

	INIT_LIST_HEAD(&g_alloc_context->list);

SUCCEED_OUT:
	g_alloc_context->ref_cnt++;
	pthread_mutex_unlock(&g_mutex_alloc);
	return 0;

ERROR_OUT:
	if (g_alloc_context != NULL
		&& g_alloc_context->fd > 0)
	{
		close(g_alloc_context->fd);
		g_alloc_context->fd = 0;
	}
	
	if (g_alloc_context != NULL)
	{
		free(g_alloc_context);
		g_alloc_context = NULL;
	}
	
	pthread_mutex_unlock(&g_mutex_alloc);
	return -1;
}

int ion_alloc_close()
{
	struct list_head * pos, *q;
	buffer_node * tmp;

	logv("ion_alloc_close \n");
	
	pthread_mutex_lock(&g_mutex_alloc);
	if (--g_alloc_context->ref_cnt <= 0)
	{
		logv("pid: %d, release g_alloc_context = %p \n", getpid(), g_alloc_context);
		
		list_for_each_safe(pos, q, &g_alloc_context->list)
		{
			tmp = list_entry(pos, buffer_node, i_list);
			logv("ion_alloc_close del item phy= 0x%08x vir= 0x%08x, size= %d \n", tmp->phy, tmp->vir, tmp->size);
			list_del(pos);
			free(tmp);
		}
		
		close(g_alloc_context->fd);
		g_alloc_context->fd = 0;

		free(g_alloc_context);
		g_alloc_context = NULL;
	}
	else
	{
		logv("ref cnt: %d > 0, do not free \n", g_alloc_context->ref_cnt);
	}
	pthread_mutex_unlock(&g_mutex_alloc);
	
	return 0;
}

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
// return virtual address: 0 failed
long ion_alloc_alloc(int size)
{
	int rest_size = 0;
	long addr_phy = 0;
	long addr_vir = 0;
	buffer_node * alloc_buffer = NULL;

	ion_allocation_data_t alloc_data;
	ion_handle_data_t handle_data;
	ion_custom_data_t custom_data;
	ion_fd_data_t fd_data;
	sunxi_phys_data phys_data;
	int fd, ret = 0;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}
	
	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}
	
	/*alloc buffer*/
	alloc_data.len = size;
	alloc_data.align = ION_ALLOC_ALIGN ;
	alloc_data.heap_id_mask = ION_HEAP_CARVEOUT_MASK;
	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logv("ION_IOC_ALLOC error \n");
		goto ALLOC_OUT;
	}

	/*get physical address*/
	phys_data.handle = alloc_data.handle;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;
	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if (ret)
	{
		logv("ION_IOC_CUSTOM failed \n");
		goto out1;
	}
	addr_phy = phys_data.phys_addr;

	/*get dma buffer fd*/
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if (ret)
	{
		logv("ION_IOC_MAP failed \n");
		goto out1;
	}

	/*mmap to user space*/
	addr_vir = (int)mmap(NULL, alloc_data.len, PROT_READ | PROT_WRITE, MAP_SHARED, 
					fd_data.fd, 0);
	if ((int)MAP_FAILED == addr_vir)
	{
		logv("mmap fialed \n");
		goto out2;
	}

	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		logv("malloc buffer node failed \n");
		goto out3;
	}
	alloc_buffer->size	    = size;
	alloc_buffer->phy 	    = addr_phy;
	alloc_buffer->vir 	    = addr_vir;
	alloc_buffer->handle    = alloc_data.handle;
	alloc_buffer->dmabuf_fd = fd_data.fd;

	logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d \n", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

	goto ALLOC_OUT;
out3:
	/* unmmap */
	ret = munmap((void*)addr_vir, alloc_data.len);
	if(ret) printf("munmap err, ret %d\n", ret);
	printf("munmap succes\n");
	
out2:
	/* close dmabuf fd */
	close(fd_data.fd);
	printf("close dmabuf fd succes\n");

out1:
	/* free buffer */
	handle_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);
	if(ret)
		printf("ION_IOC_FREE err, ret %d\n", ret);
	printf("ION_IOC_FREE succes\n");

ALLOC_OUT:
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

#else
// return virtual address: 0 failed
int ion_alloc_alloc(int size)
{
	struct ion_allocation_data alloc_data;
	struct ion_fd_data fd_data;
	struct ion_handle_data handle_data;
	struct ion_custom_data custom_data;
	sunxi_phys_data   phys_data;
	
	int rest_size = 0;
	int addr_phy = 0;
	int addr_vir = 0;
	buffer_node * alloc_buffer = NULL;
    int ret = 0;
	
	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}
	
	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}
	
	/*alloc buffer*/
	alloc_data.len = size;
	alloc_data.align = ION_ALLOC_ALIGN ;

#if (CONFIG_PRODUCT == OPTION_PRODUCT_PAD && CONFIG_CHIP == OPTION_CHIP_1639)
	alloc_data.heap_id_mask = ION_HEAP_DMA_MASK;
#else
	alloc_data.heap_id_mask = ION_HEAP_CARVEOUT_MASK;
#endif
	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logv("ION_IOC_ALLOC error \n");
		goto ALLOC_OUT;
	}

	/* get dmabuf fd */
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if(ret)
	{
		//loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.fd);
		goto ALLOC_OUT;
	}


	/* mmap to user */
	addr_vir = (int)mmap(NULL, alloc_data.len, PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
	if((int)MAP_FAILED == addr_vir)
	{
		//loge("mmap err, ret %d\n", (unsigned int)addr_vir);
		addr_vir = 0;
		goto ALLOC_OUT;
	}
//	logd("mmap succes, get user_addr 0x%08x\n", (unsigned int)addr_vir);

    /* get phy address */
	memset(&phys_data, 0, sizeof(phys_data));
	phys_data.handle = alloc_data.handle;
	phys_data.size = size;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if(ret) {
		//loge("ION_IOC_CUSTOM err, ret %d\n", ret);
		addr_phy = 0;
		addr_vir = 0;
		goto ALLOC_OUT;
	}

//	logv("get phys_data.phys_addr: %x\n", phys_data.phys_addr);

	addr_phy = phys_data.phys_addr;
	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		//loge("malloc buffer node failed");

		/* unmmap */
		ret = munmap((void*)addr_vir, alloc_data.len);
		if(ret) {
			//loge("munmap err, ret %d\n", ret);
		}

		/* close dmabuf fd */
		close(fd_data.fd);

		/* free buffer */
		handle_data.handle = alloc_data.handle;
		ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);
		
		if(ret) {
			//loge("ION_IOC_FREE err, ret %d\n", ret);
		}

		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0
		
		goto ALLOC_OUT;
	}
	alloc_buffer->phy 	= addr_phy;
	alloc_buffer->vir 	= addr_vir;
	alloc_buffer->size	= size;
	alloc_buffer->fd_data.handle = fd_data.handle;
	alloc_buffer->fd_data.fd = fd_data.fd;

	//logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

ALLOC_OUT:
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

#endif

int ion_alloc_free(void * pbuf)
{
	int flag = 0;
	unsigned long addr_vir = (unsigned long)pbuf;
	buffer_node * tmp;
	int ret;
	struct ion_handle_data handle_data;
	int nFreeSize = 0;

	if (0 == pbuf)
	{
		loge("can not free NULL buffer \n");
		return 0;
	}
	
	pthread_mutex_lock(&g_mutex_alloc);
	
	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		return 0;
	}
	
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (tmp->vir == addr_vir)
		{
			logv("ion_alloc_free item phy= 0x%08x vir= 0x%08x, size= %d \n", tmp->phy, tmp->vir, tmp->size);
			/*unmap user space*/
			if (munmap(pbuf, tmp->size) < 0)
			{
				loge("munmap 0x%p, size: %d failed \n", (void*)addr_vir, tmp->size);
			}
			nFreeSize = tmp->size;

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
			/*close dma buffer fd*/
			close(tmp->dmabuf_fd);
			
			/*free memory handle*/
			handle_data.handle = tmp->handle;
#else
			/*close dma buffer fd*/
			close(tmp->fd_data.fd);
			
			/* free buffer */
			handle_data.handle = tmp->fd_data.handle;
#endif
			
			ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);
			if (ret) 
			{
				logv("TON_IOC_FREE failed \n");
			}
		
			list_del(&tmp->i_list);
			free(tmp);

			flag = 1;
			break;
		}
	}

	if (0 == flag)
	{
		logv("ion_alloc_free failed, do not find virtual address: 0x%08x \n", addr_vir);
	}

	pthread_mutex_unlock(&g_mutex_alloc);
	return nFreeSize;
}

unsigned long ion_alloc_vir2phy(void * pbuf)
{
	int flag = 0;
	unsigned long addr_vir = (unsigned long)pbuf;
	unsigned long addr_phy = 0;
	buffer_node * tmp;
	
	if (0 == pbuf)
	{
		// logv("can not vir2phy NULL buffer \n");
		return 0;
	}
	
	pthread_mutex_lock(&g_mutex_alloc);
	
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (addr_vir >= tmp->vir
			&& addr_vir < tmp->vir + tmp->size)
		{
			addr_phy = tmp->phy + addr_vir - tmp->vir;
			// logv("ion_alloc_vir2phy phy= 0x%08x vir= 0x%08x \n", addr_phy, addr_vir);
			flag = 1;
			break;
		}
	}
	
	if (0 == flag)
	{
		logv("ion_alloc_vir2phy failed, do not find virtual address: 0x%08x \n", addr_vir);
	}
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_phy;
}

unsigned long ion_alloc_phy2vir(void * pbuf)
{
	int flag = 0;
	unsigned long addr_vir = 0;
	unsigned long addr_phy = (unsigned long)pbuf;
	buffer_node * tmp;
	
	if (0 == pbuf)
	{
		loge("can not phy2vir NULL buffer \n");
		return 0;
	}

	pthread_mutex_lock(&g_mutex_alloc);
	
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (addr_phy >= tmp->phy
			&& addr_phy < tmp->phy + tmp->size)
		{
			addr_vir = tmp->vir + addr_phy - tmp->phy;
			flag = 1;
			break;
		}
	}
	
	if (0 == flag)
	{
		logv("ion_alloc_phy2vir failed, do not find physical address: 0x%08x \n", addr_phy);
	}
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

void ion_flush_cache(void* startAddr, int size)
{
	sunxi_cache_range range;
	struct ion_custom_data custom_data;
	int ret;

	/* clean and invalid user cache */
	range.start = (unsigned long)startAddr;
	range.end = (unsigned long)startAddr + size;
	
	custom_data.cmd = ION_IOC_SUNXI_FLUSH_RANGE;
	custom_data.arg = (unsigned long)&range;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if (ret) 
	{
		logv("ION_IOC_CUSTOM failed \n");
	}

	return;
}

void ion_flush_cache_all()
{
	ioctl(g_alloc_context->fd, ION_IOC_SUNXI_FLUSH_ALL, 0);
}

long  ion_alloc_alloc_drm(int size)
{
	struct ion_allocation_data alloc_data;
	struct ion_fd_data fd_data;
	struct ion_handle_data handle_data;
	struct ion_custom_data custom_data;
	sunxi_phys_data   phys_data;

	int rest_size = 0;
	long addr_phy = 0;
	long addr_vir = 0;
	buffer_node * alloc_buffer = NULL;
	int ret = 0;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}

	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}

	/*alloc buffer*/
	alloc_data.len = size;
	alloc_data.align = ION_ALLOC_ALIGN ;
	alloc_data.heap_id_mask = ION_HEAP_SECURE_MASK;
	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logv("ION_IOC_ALLOC error %s \n", strerror(errno));
		goto ALLOC_OUT;
	}

	/* get dmabuf fd */
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if(ret)
	{
		//loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.fd);
		goto ALLOC_OUT;
	}


	/* mmap to user */
	addr_vir = (long)mmap(NULL, alloc_data.len, PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
	if((long)MAP_FAILED == addr_vir)
	{
		//loge("mmap err, ret %d\n", (unsigned int)addr_vir);
		addr_vir = 0;
		goto ALLOC_OUT;
	}
	//	logd("mmap succes, get user_addr 0x%08x\n", (unsigned int)addr_vir);

	/* get phy address */
	memset(&phys_data, 0, sizeof(phys_data));
	phys_data.handle = alloc_data.handle;
	phys_data.size = size;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if(ret) {
		//loge("ION_IOC_CUSTOM err, ret %d\n", ret);
		addr_phy = 0;
		addr_vir = 0;
		goto ALLOC_OUT;
	}

	//	logv("get phys_data.phys_addr: %x\n", phys_data.phys_addr);

	addr_phy = phys_data.phys_addr;
	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		//loge("malloc buffer node failed");

		/* unmmap */
		ret = munmap((void*)addr_vir, alloc_data.len);
		if(ret) {
			//loge("munmap err, ret %d\n", ret);
		}

		/* close dmabuf fd */
		close(fd_data.fd);

		/* free buffer */
		handle_data.handle = alloc_data.handle;
		ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);

		if(ret) {
			//loge("ION_IOC_FREE err, ret %d\n", ret);
		}

		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0

		goto ALLOC_OUT;
	}


	alloc_buffer->size	    = size;
	alloc_buffer->phy 	    = addr_phy;
	alloc_buffer->vir 	    = addr_vir;

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
	alloc_buffer->handle    = alloc_data.handle;
	alloc_buffer->dmabuf_fd = fd_data.fd;
#else
	alloc_buffer->fd_data.handle = fd_data.handle;
	alloc_buffer->fd_data.fd = fd_data.fd;
#endif

	//logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

	ALLOC_OUT:

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}
