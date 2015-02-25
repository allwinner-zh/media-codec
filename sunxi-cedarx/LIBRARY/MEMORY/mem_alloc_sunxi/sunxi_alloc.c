
/*
 * sunxi_alloc.c
 *
 * xiaoshujun@allwinnertech.com
 *
 * sunxi memory allocate
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>

//#include <linux/sunxi_physmem.h>

#include "sunxi_alloc_list.h"

#ifdef VERBOSE
    #ifdef ANDROID_OS
        #define LOG_NDEBUG 0
        #define LOG_TAG "sunxi_alloc.c"
        #include <utils/Log.h>
        #define logv    ALOGV
    #else
        #define logv    printf
    #endif
#else
    #define logv(...)
#endif

/*
 * io cmd, must be same as  <linux/sunxi_physmem.h> 
 */
#define SUNXI_MEM_ALLOC 		1
#define SUNXI_MEM_FREE 			3 /* cannot be 2, which reserved in linux */
#define SUNXI_MEM_GET_REST_SZ 	4
#define SUNXI_MEM_FLUSH_CACHE   5
#define SUNXI_MEM_FLUSH_CACHE_ALL	6

#define DEV_NAME 			"/dev/sunxi_mem"

typedef struct BUFFER_NODE
{
	struct list_head i_list; 
	unsigned int phy;
	unsigned int vir;
	unsigned int size;
}buffer_node;

typedef struct SUNXI_ALLOC_CONTEXT
{
	int					fd;			// driver handle
	struct list_head	list;		// buffer list
	int					ref_cnt;	// reference count
}sunxi_alloc_context;

sunxi_alloc_context	*	g_alloc_context = NULL;
pthread_mutex_t			g_mutex_alloc = PTHREAD_MUTEX_INITIALIZER;
void sunxi_alloc_debug()
{
    int         ret;
	buffer_node * tmp;
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		logv("sunxi_alloc_debug phy= 0x%08x vir= 0x%08x, size= %d \n", tmp->phy, tmp->vir, (int)tmp->size);
	}
	
	int rest_size = ioctl(g_alloc_context->fd, SUNXI_MEM_GET_REST_SZ, 0);
	logv("sunxi_alloc_debug rest size: 0x%08x \n", rest_size);

	ret = system("cat /proc/sunxi_mem");

	usleep(1000000);	// sleep 1s
}

int sunxi_alloc_open(void)
{
	logv("sunxi_alloc_open \n");
	
	pthread_mutex_lock(&g_mutex_alloc);
	if (g_alloc_context != NULL)
	{
		logv("sunxi allocator has already been created \n");
		goto SUCCEED_OUT;
	}

	g_alloc_context = (sunxi_alloc_context*)malloc(sizeof(sunxi_alloc_context));
	if (g_alloc_context == NULL)
	{
		logv("create sunxi allocator failed, out of memory \n");
		goto ERROR_OUT;
	}
	else
	{
		logv("pid: %d, g_alloc_context = %p \n", getpid(), g_alloc_context);
	}

	memset((void*)g_alloc_context, 0, sizeof(sunxi_alloc_context));
	
	g_alloc_context->fd = open(DEV_NAME, O_RDWR, 0);
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

int sunxi_alloc_close(void)
{
	struct list_head * pos, *q;
	buffer_node * tmp;

	logv("sunxi_alloc_close \n");
	
	pthread_mutex_lock(&g_mutex_alloc);
	if (--g_alloc_context->ref_cnt <= 0)
	{
		logv("pid: %d, release g_alloc_context = %p \n", getpid(), g_alloc_context);
		
		list_for_each_safe(pos, q, &g_alloc_context->list)
		{
			tmp = list_entry(pos, buffer_node, i_list);
			logv("sunxi_alloc_close del item phy= 0x%08x vir= 0x%08x, size= %d \n", tmp->phy, tmp->vir, (int)tmp->size);
			list_del(pos);
			free(tmp);
		}
		
// #if (LOG_NDEBUG == 0)
// {
// 		int rest_size = ioctl(g_alloc_context->fd, SUNXI_MEM_GET_REST_SZ, 0);
// 		logv("before close allocator, rest size: 0x%08x \n", rest_size);
// }
// #endif

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

// return virtual address: 0 failed
unsigned int sunxi_alloc_alloc(unsigned int size)
{
	unsigned int rest_size = 0;
	unsigned int addr_phy = 0;
	unsigned int addr_vir = 0;
	buffer_node * alloc_buffer = NULL;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("sunxi_alloc do not opened, should call sunxi_alloc_open() before sunxi_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}
	
	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}
	
	rest_size = ioctl(g_alloc_context->fd, SUNXI_MEM_GET_REST_SZ, 0);
	logv("rest size: 0x%08x \n", rest_size);

	if (rest_size < size)
	{
		logv("not enough memory, rest: 0x%08x, request: 0x%08x \n", rest_size, size);
		goto ALLOC_OUT;
	}

	addr_phy = ioctl(g_alloc_context->fd, SUNXI_MEM_ALLOC, &size);
	if (0 == addr_phy)
	{
		logv("can not request physical buffer, size: %d \n", (int)size);
		goto ALLOC_OUT;
	}

	addr_vir = (unsigned int)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, 
					g_alloc_context->fd, (int)addr_phy);
	if ((unsigned int)MAP_FAILED == addr_vir)
	{
		ioctl(g_alloc_context->fd, SUNXI_MEM_FREE, &addr_phy);
		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0
		goto ALLOC_OUT;
	}

	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		logv("malloc buffer node failed \n");

		// should free and unmmapphy buffer
		ioctl(g_alloc_context->fd, SUNXI_MEM_FREE, &addr_phy);
		
		if (munmap((void*)addr_vir, size) < 0)
		{
			logv("munmap 0x%08x, size: %d failed \n", addr_vir, (int)size);
		}
		
		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0
		
		goto ALLOC_OUT;
	}
	alloc_buffer->phy 	= addr_phy;
	alloc_buffer->vir 	= addr_vir;
	alloc_buffer->size	= size;

	logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d \n", addr_phy, addr_vir, (int)size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

ALLOC_OUT:
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

int sunxi_alloc_free(void * pbuf)
{
	int flag = 0;
	unsigned int addr_vir = (unsigned int)pbuf;
	buffer_node * tmp;
	int nFreeSize = 0;

	if (0 == pbuf)
	{
		logv("can not free NULL buffer \n");
		return 0;
	}
	
	pthread_mutex_lock(&g_mutex_alloc);
	
	if (g_alloc_context == NULL)
	{
		logv("sunxi_alloc do not opened, should call sunxi_alloc_open() before sunxi_alloc_alloc(size) \n");
		return 0;
	}
	
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (tmp->vir == addr_vir)
		{
			logv("sunxi_alloc_free item phy= 0x%08x vir= 0x%08x, size= %d \n", tmp->phy, tmp->vir, (int)tmp->size);

			ioctl(g_alloc_context->fd, SUNXI_MEM_FREE, &tmp->phy);
			nFreeSize = tmp->size;
			
			if (munmap(pbuf, tmp->size) < 0)
			{
				logv("munmap 0x%08x, size: %d failed \n", addr_vir, (int)tmp->size);
			}
		
			list_del(&tmp->i_list);
			free(tmp);

// #if (LOG_NDEBUG == 0)
// {
// 			unsigned int rest_size = ioctl(g_alloc_context->fd, SUNXI_MEM_GET_REST_SZ, 0);
// 			logv("after free, rest size: 0x%08x \n", rest_size);
// }
// #endif

			flag = 1;
			break;
		}
	}

	if (0 == flag)
	{
		logv("sunxi_alloc_free failed, do not find virtual address: 0x%08x \n", addr_vir);
		sunxi_alloc_debug();
	}

	pthread_mutex_unlock(&g_mutex_alloc);
	return nFreeSize;
}

unsigned int sunxi_alloc_vir2phy(void * pbuf)
{
	int flag = 0;
	unsigned int addr_vir = (unsigned int)pbuf;
	unsigned int addr_phy = 0;
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
			// logv("sunxi_alloc_vir2phy phy= 0x%08x vir= 0x%08x \n", addr_phy, addr_vir);
			flag = 1;
			break;
		}
	}
	
	if (0 == flag)
	{
		logv("sunxi_alloc_vir2phy failed, do not find virtual address: 0x%08x \n", addr_vir);
		sunxi_alloc_debug();
	}
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_phy;
}

unsigned int sunxi_alloc_phy2vir(void * pbuf)
{
	int flag = 0;
	unsigned int addr_vir = 0;
	unsigned int addr_phy = (unsigned int)pbuf;
	buffer_node * tmp;
	
	if (0 == pbuf)
	{
		logv("can not phy2vir NULL buffer \n");
		return 0;
	}

	pthread_mutex_lock(&g_mutex_alloc);
	
	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (addr_phy >= tmp->phy
			&& addr_phy < tmp->phy + tmp->size)
		{
			addr_vir = tmp->vir + addr_phy - tmp->phy;
			// logv("sunxi_alloc_phy2vir phy= 0x%08x vir= 0x%08x \n", addr_phy, addr_vir);
			flag = 1;
			break;
		}
	}
	
	if (0 == flag)
	{
		logv("sunxi_alloc_phy2vir failed, do not find physical address: 0x%08x \n", addr_phy);
		sunxi_alloc_debug();
	}
	
	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

void sunxi_flush_cache(void* startAddr, unsigned int size)
{
	long arr[2];

	arr[0] = (long)startAddr;
	arr[1] = arr[0] + size - 1;

	ioctl(g_alloc_context->fd, SUNXI_MEM_FLUSH_CACHE, arr);

	return;
}

void sunxi_flush_cache_all()
{
	ioctl(g_alloc_context->fd, SUNXI_MEM_FLUSH_CACHE_ALL, 0);
}
