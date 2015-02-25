#include "log.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include "ve.h"
#include "vdecoder.h"


#if CONFIG_CHIP == OPTION_CHIP_1623 || \
    CONFIG_CHIP == OPTION_CHIP_1625
    #include "1623/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1633
    #include "1633/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1651
    #include "1651/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1650 || \
      CONFIG_CHIP == OPTION_CHIP_1661 || \
      CONFIG_CHIP == OPTION_CHIP_1667
    #include "1650/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1639
    #include "1639/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1673 
    #include "1673/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1680
    #include "1680/cedardev_api.h"
#elif CONFIG_CHIP == OPTION_CHIP_1681
    #include "1681/cedardev_api.h"
#else
    #error "invalid chip option"
#endif

#define PAGE_OFFSET (0xc0000000) // from kernel
#define PAGE_SIZE (4096)

static pthread_mutex_t  gVeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  gVeRegisterMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  gVeDecoderMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  gVeEncoderMutex = PTHREAD_MUTEX_INITIALIZER;

#if (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
static pthread_mutex_t  gVeMemoryMutex = PTHREAD_MUTEX_INITIALIZER;

struct MemChunkS
{
	unsigned int physAddr;
	int size;
	void *virtAddr;
	struct MemChunkS *next;
};

struct MemChunkS firstMemchunk;

struct CedarvCacheRangeS
{
    long start;
    long end;
};
#endif

#if CONFIG_CHIP == OPTION_CHIP_1681
static pthread_mutex_t  gVeJpegDecoderMutex = PTHREAD_MUTEX_INITIALIZER;
#endif


static int              gVeRefCount = 0;
int                     gVeDriverFd = -1;
cedarv_env_info_t       gVeEnvironmentInfo;

#define VE_MODE_SELECT 0x00
#define VE_RESET	   0x04
#if CONFIG_CHIP == OPTION_CHIP_1681
#define JPG_VE_RESET   0x2c
#endif

static int              gNomalEncRefCount = 0;
static int              gPerfEncRefCount = 0;


int VeInitialize(void)
{
    pthread_mutex_lock(&gVeMutex);
    
    if(gVeRefCount == 0)
    {
        //* open Ve driver.
        gVeDriverFd = open("/dev/cedar_dev", O_RDWR);
        if(gVeDriverFd < 0)
        {
            loge("open /dev/cedar_dev fail.");
            pthread_mutex_unlock(&gVeMutex);
            return -1;
        }
        
        //* set ve reference count to zero.
        //* we must reset it to zero to fix refcount error when process crash.
        ioctl(gVeDriverFd, IOCTL_SET_REFCOUNT, 0);
        //* request ve.
        ioctl(gVeDriverFd, IOCTL_ENGINE_REQ, 0);
        
#if 0    //* the ve speed is set by the driver to default value.
        //* set ve speed.
        int nVeSpeed = 320  //* 320MHz.
        ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, CONFIG_VE_SPPED_VALUE);
#endif


#if (CONFIG_CHIP == OPTION_CHIP_1639 || CONFIG_CHIP == OPTION_CHIP_1673)
		ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 600);
#else
	    //*the ve speed is set by the driver to default value
#endif

/* H3 Ve freq control by kernel
#if (CONFIG_CHIP == OPTION_CHIP_1680)
		ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 360);
#endif
*/

        //* map registers.
        ioctl(gVeDriverFd, IOCTL_GET_ENV_INFO, (unsigned long)&gVeEnvironmentInfo);
		gVeEnvironmentInfo.address_macc = (unsigned int)mmap(NULL, 
                                                             2048, 
                                                             PROT_READ | PROT_WRITE, MAP_SHARED,
                                                             gVeDriverFd, 
                                                             (int)gVeEnvironmentInfo.address_macc);

		//* reset ve.
        VeReset();
#if (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
        firstMemchunk.physAddr = gVeEnvironmentInfo.phymem_start - PAGE_OFFSET;
        firstMemchunk.size = gVeEnvironmentInfo.phymem_total_size;
        logd("xxxxxxx firstMemchunk.size(%d)", firstMemchunk.size);
#endif
    }
    
    gVeRefCount++;
    
    pthread_mutex_unlock(&gVeMutex);
    
    return 0;    
}


void VeRelease(void)
{
    pthread_mutex_lock(&gVeMutex);
    
    if(gVeRefCount <= 0)
    {
        loge("invalid status, gVeRefCount=%d at AdpaterRelease", gVeRefCount);
        pthread_mutex_unlock(&gVeMutex);
        return;
    }
    
    gVeRefCount--;
    
    if(gVeRefCount == 0)
    {
        if(gVeDriverFd != -1)
        {
            ioctl(gVeDriverFd, IOCTL_ENGINE_REL, 0);
            munmap((void *)gVeEnvironmentInfo.address_macc, 2048);
            close(gVeDriverFd);
            gVeDriverFd = -1;
        }
    }
    
    pthread_mutex_unlock(&gVeMutex);
    
    return;
}


int VeLock(void)
{
    return pthread_mutex_lock(&gVeDecoderMutex);
}


void VeUnLock(void)
{
    pthread_mutex_unlock(&gVeDecoderMutex);
}


int VeEncoderLock(void)
{
#if CONFIG_CHIP != OPTION_CHIP_1639
	return VeLock();
#else
    return pthread_mutex_lock(&gVeEncoderMutex);
#endif
}


void VeEncoderUnLock(void)
{
#if CONFIG_CHIP != OPTION_CHIP_1639
	VeUnLock();
#else
    pthread_mutex_unlock(&gVeEncoderMutex);
#endif
}

void VeSetDramType()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	switch (VeGetDramType())
	{
		case DDRTYPE_DDR1_16BITS:
			pVeModeSelect->ddr_mode = 0;
			break;
	
		case DDRTYPE_DDR1_32BITS:
		case DDRTYPE_DDR2_16BITS:
			pVeModeSelect->ddr_mode = 1;
			break;
	
		case DDRTYPE_DDR2_32BITS:
		case DDRTYPE_DDR3_16BITS:
			pVeModeSelect->ddr_mode = 2;
			break;
	
		case DDRTYPE_DDR3_32BITS:
		case DDRTYPE_DDR3_64BITS:
			pVeModeSelect->ddr_mode = 3;
			pVeModeSelect->rec_wr_mode = 1;
			break;
	
		default:
			break;
	}
	pthread_mutex_unlock(&gVeRegisterMutex);
}

void VeReset(void)
{
    ioctl(gVeDriverFd, IOCTL_RESET_VE, 0);
	VeSetDramType();
}

int VeWaitInterrupt(void)
{
    int ret;

#if CONFIG_CHIP == OPTION_CHIP_1681
    ret = ioctl(gVeDriverFd, IOCTL_WAIT_JPEG_DEC, 1);
#else
    ret = ioctl(gVeDriverFd, IOCTL_WAIT_VE_DE, 1);
#endif
    if(ret <= 0)
    {
        logw("wait ve interrupt timeout.");
        return -1;  //* wait ve interrupt fail.
    }
    else
        return 0;
}

int VeWaitEncoderInterrupt(void)
{
    int ret;
	
    ret = ioctl(gVeDriverFd, IOCTL_WAIT_VE_EN, 1);
    if(ret <= 0)
        return -1;  //* wait ve interrupt fail.
    else
        return 0;
}


void* VeGetRegisterBaseAddress(void)
{
    return (void*)gVeEnvironmentInfo.address_macc;
}


unsigned int VeGetIcVersion()
{
    if(gVeRefCount >0)
    {
		volatile unsigned int value;
   		value = *((unsigned int*)((char *)gVeEnvironmentInfo.address_macc + 0xf0));
		return (value>>16);
    }
	else
	{
		loge("must call VeGetIcVersion(), affer VeInitialize");
		return 0;
	}
}

int VeGetDramType(void)
{
    //* can we know memory type by some system api?
#if CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR1_16BITS
    return DDRTYPE_DDR1_16BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR1_32BITS
    return DDRTYPE_DDR1_32BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR2_16BITS
    return DDRTYPE_DDR2_16BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR2_32BITS
    return DDRTYPE_DDR2_32BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR3_16BITS
    return DDRTYPE_DDR3_16BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR3_32BITS
    return DDRTYPE_DDR3_32BITS;
#elif CONFIG_DRAM_INTERFACE == OPTION_DRAM_INTERFACE_DDR3_64BITS
    return DDRTYPE_DDR3_64BITS;
#else
    #error "invalid ddr type configuration."
#endif
}


int VeSetSpeed(int nSpeedMHz)
{
    return ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, nSpeedMHz);
}


void VeEnableEncoder()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	pVeModeSelect->enc_enable = 1;
	pVeModeSelect->enc_isp_enable = 1;
	pthread_mutex_unlock(&gVeRegisterMutex);
}

void VeDisableEncoder()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	pVeModeSelect->enc_enable = 0;
	pVeModeSelect->enc_isp_enable = 0;
	pthread_mutex_unlock(&gVeRegisterMutex);	
}

void VeEnableDecoder(int nDecoderMode)
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	
	switch(nDecoderMode)
	{
		case VIDEO_CODEC_FORMAT_H264:
			pVeModeSelect->mode = 1;
#if CONFIG_CHIP == OPTION_CHIP_1681
			pVeModeSelect->jpg_dec_en = 0;
#endif
			break;
		case VIDEO_CODEC_FORMAT_VP8:
			pVeModeSelect->mode = 1;
			break;
		case VIDEO_CODEC_FORMAT_AVS:
			pVeModeSelect->mode = 1;
			break;			
		case VIDEO_CODEC_FORMAT_WMV3:
			pVeModeSelect->mode = 2;
			break;
		case VIDEO_CODEC_FORMAT_RX:
			pVeModeSelect->mode = 3;
			break;
		case VIDEO_CODEC_FORMAT_H265:
			pVeModeSelect->mode = 4;
			break;
		default:
			pVeModeSelect->mode = 0; //* MPEG1/2/4 or JPEG decoder.
#if CONFIG_CHIP == OPTION_CHIP_1681
			pVeModeSelect->jpg_dec_en = 1;
#endif
			break;
	}
	pthread_mutex_unlock(&gVeRegisterMutex);		
}

void VeDisableDecoder()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	pVeModeSelect->mode = 7;
	pthread_mutex_unlock(&gVeRegisterMutex);	
}

void VeDecoderWidthMode(int nWidth)
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	
	if(nWidth >= 4096)
	{
		pVeModeSelect->pic_width_more_2048 = 1;
		pVeModeSelect->pic_width_is_4096 = 1;
	}
	else if(nWidth >= 2048)
	{
		pVeModeSelect->pic_width_more_2048 = 1;
		pVeModeSelect->pic_width_is_4096 = 0;
	}
	else
	{
		pVeModeSelect->pic_width_more_2048 = 0;
		pVeModeSelect->pic_width_is_4096 = 0;
	}
	pthread_mutex_unlock(&gVeRegisterMutex);
}

void VeResetDecoder()
{
#if (CONFIG_CHIP == OPTION_CHIP_1639 || CONFIG_CHIP == OPTION_CHIP_1681)
	volatile vetop_reg_reset_t* pVeReset;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeReset = (vetop_reg_reset_t*)(gVeEnvironmentInfo.address_macc + VE_RESET);
	pVeReset->decoder_reset = 1;
	pVeReset->decoder_reset = 0;
	pthread_mutex_unlock(&gVeRegisterMutex);
#else
	VeReset();	
#endif
}


void VeResetEncoder()
{
#if (CONFIG_CHIP == OPTION_CHIP_1639 || CONFIG_CHIP == OPTION_CHIP_1681)
	volatile vetop_reg_reset_t* pVeReset;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeReset = (vetop_reg_reset_t*)(gVeEnvironmentInfo.address_macc + VE_RESET);
	pVeReset->encoder_reset = 1;
	pVeReset->encoder_reset = 0;
	pthread_mutex_unlock(&gVeRegisterMutex);
#else
	VeReset();	
#endif
}

void VeInitEncoderPerformance(int nMode) //* 0: normal performance; 1. high performance 
{
#if CONFIG_CHIP == OPTION_CHIP_1639
	VeLock();
	VeEncoderLock();
	if(nMode == 0) 
	{
		if(gNomalEncRefCount == 0 && gPerfEncRefCount == 0)
		{
			ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 450); //MHZ
		}
		gNomalEncRefCount++;
	}
	else
	{
		if(gPerfEncRefCount == 0)
		{
			ioctl(gVeDriverFd, IOCTL_SET_VOL, 960); // mv
			ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 480); //MHZ
		}
		gPerfEncRefCount++;
	}
	VeEncoderUnLock();
	VeUnLock();
#else
	CEDARX_UNUSE(nMode);
#endif
}

void VeUninitEncoderPerformance(int nMode) //* 0: normal performance; 1. high performance 
{
#if CONFIG_CHIP == OPTION_CHIP_1639
	VeLock();
	VeEncoderLock();
	if(nMode == 0)
	{
		gNomalEncRefCount--;
		if(gNomalEncRefCount == 0)
		{
			if(gPerfEncRefCount == 0)
			{
				ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 600); //MHZ
			}
			else
			{
                //* do nothing 
			}
		}
	}
	else
	{
		gPerfEncRefCount--;
		if(gPerfEncRefCount == 0)
		{
			ioctl(gVeDriverFd, IOCTL_SET_VOL, 900);
			if(gNomalEncRefCount == 0)
			{
				ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 600); //MHZ
			}
			else
			{
				ioctl(gVeDriverFd, IOCTL_SET_VE_FREQ, 450); //MHZ
			}
		}
	}
	VeEncoderUnLock();
	VeUnLock();
#else
	CEDARX_UNUSE(nMode);
#endif
}

#if CONFIG_CHIP == OPTION_CHIP_1681
int VeWaitJpegDecodeInterrupt(void)
{
    int ret;
    
    ret = ioctl(gVeDriverFd, IOCTL_WAIT_JPEG_DEC, 1);
    if(ret <= 0)
    {
        logw("wait jepg decoder interrupt timeout.");
        return -1;  //* wait jepg decoder interrupt fail.
    }
    else
        return 0;
}

void VeEnableJpegDecoder()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);

	//pVeModeSelect->ddr_mode = 2; //MEMTYPE_DDR3_16BITS
	pVeModeSelect->jpg_dec_en = 1;
	
	pthread_mutex_unlock(&gVeRegisterMutex);
}


void VeDisableJpegDecoder()
{
	volatile vetop_reg_mode_sel_t* pVeModeSelect;
	pthread_mutex_lock(&gVeRegisterMutex);
	pVeModeSelect = (vetop_reg_mode_sel_t*)(gVeEnvironmentInfo.address_macc + VE_MODE_SELECT);
	pVeModeSelect->jpg_dec_en = 0;
	pthread_mutex_unlock(&gVeRegisterMutex);	
}

void VeResetJpegDecoder()
{
	volatile vetop_reg_jpg_reset_t *ve_reset;
	
	ve_reset = (vetop_reg_jpg_reset_t *)(gVeEnvironmentInfo.address_macc + JPG_VE_RESET);
	ve_reset->jpg_dec_reset = 1;
	ve_reset->jpg_dec_reset = 0;
}

int VeJpegDeLock(void)
{
    return pthread_mutex_lock(&gVeJpegDecoderMutex);
}


void VeJpegDeUnLock(void)
{
    pthread_mutex_unlock(&gVeJpegDecoderMutex);
}
#endif

//******************************************************************************************
//* for malloc from ve
#if (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
void *VeMalloc(int size)
{
    if(gVeDriverFd == -1)
    {
        loge("invalid fd.");
        return NULL;
    }
    pthread_mutex_lock(&gVeMemoryMutex);

    void *addr = NULL;
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    struct MemChunkS *c, *bestChunk = NULL;
    for(c = &firstMemchunk; c != NULL; c = c->next)
    {
        if(c->virtAddr == NULL && c->size >= size)
        {
            if(bestChunk == NULL || c->size < bestChunk->size)
            {
                bestChunk = c;
            }
            if(c->size == size)
            {
                break;
            }
        }
    }

    if(!bestChunk)
    {
        logw("no bestChunk");
        goto out;
    }

    int leftSize = bestChunk->size - size;

    addr = mmap(NULL, 
                size, 
                PROT_READ | PROT_WRITE, 
                MAP_SHARED, 
                gVeDriverFd,
                bestChunk->physAddr+PAGE_OFFSET);
    if(addr == MAP_FAILED)
    {
        loge("map failed.");
        addr = NULL;
        goto out;
    }
    bestChunk->virtAddr = addr;
    bestChunk->size = size;

    if(leftSize > 0)
    {
        c = malloc(sizeof(struct MemChunkS));
        c->physAddr = bestChunk->physAddr + size;
        c->size = leftSize;
        c->virtAddr = NULL;
        c->next = bestChunk->next;
        bestChunk->next = c;
    }
    
out:
    pthread_mutex_unlock(&gVeMemoryMutex);
    return addr;
}

void VeFree(void *ptr)
{
    if(gVeDriverFd == -1 || ptr == NULL)
    {
        loge("fd(%d), ptr(%p).", gVeDriverFd, ptr);
        return;
    }
    
    pthread_mutex_lock(&gVeMemoryMutex);

    struct MemChunkS *c;
    for(c = &firstMemchunk; c != NULL; c = c->next)
    {
        if(c->virtAddr == ptr)
        {
            munmap(ptr, c->size);
            c->virtAddr = NULL;
            break;
        }
    }

    for(c = &firstMemchunk; c != NULL; c = c->next)
    {
        if(c->virtAddr == NULL)
        {
            while(c->next != NULL && c->next->virtAddr == NULL)
            {
                struct MemChunkS *n = c->next;
                c->size += n->size;
                c->next = n->next;
                free(n);
            }
        }
    }
    
    pthread_mutex_unlock(&gVeMemoryMutex);
}

unsigned int VeVir2Phy(void *ptr)
{
    if(gVeDriverFd == -1)
    {
        loge("invalid fd.");
        return 0;
    }

    pthread_mutex_lock(&gVeMemoryMutex);

    unsigned int addr = 0;
    struct MemChunkS *c;
    for(c = &firstMemchunk; c != NULL; c = c->next)
    {
        if(c->virtAddr == NULL)
            continue;

        if(c->virtAddr == ptr)
        {
            addr = c->physAddr;
            break;
        }
        else if(ptr > c->virtAddr && ptr < (c->virtAddr + c->size))
        {
            addr = c->physAddr + (ptr - c->virtAddr);
            break;
        }
    }

    pthread_mutex_unlock(&gVeMemoryMutex);

    return addr;
}

unsigned int VePhy2Vir(void *ptr) //*
{
    unsigned int addrPhy = (unsigned int)ptr;
    
    if(gVeDriverFd == -1)
    {
        loge("invalid fd.");
        return 0;
    }

    pthread_mutex_lock(&gVeMemoryMutex);

    unsigned int addr = 0;
    struct MemChunkS *c;
    for(c = &firstMemchunk; c != NULL; c = c->next)
    {
        if(c->physAddr == 0)
            continue;

        if(c->physAddr == addrPhy)
        {
            addr = (unsigned int)c->virtAddr;
            break;
        }
        else if(addrPhy > c->physAddr && addrPhy < (c->physAddr + c->size))
        {
            addr = (unsigned int)c->virtAddr + (addrPhy - c->physAddr);
            break;
        }
    }

    pthread_mutex_unlock(&gVeMemoryMutex);

    return addr;
}

void VeFlushCache(void *startAddr, int size)
{
    if(gVeDriverFd == -1)
    {
        loge("invalid fd.");
        return ;
    }

    struct CedarvCacheRangeS range =
    {
        .start = (int)startAddr,
        .end = (int)(startAddr + size)
    };

    ioctl(gVeDriverFd, IOCTL_FLUSH_CACHE, (void*)(&range));
}

#endif
