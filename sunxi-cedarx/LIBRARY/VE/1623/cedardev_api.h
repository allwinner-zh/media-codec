#ifndef __CEDARDEV_API_H__
#define __CEDARDEV_API_H__

enum IOCTL_CMD {
	IOCTL_UNKOWN = 0x100,
	IOCTL_GET_ENV_INFO,
	IOCTL_WAIT_VE_DE,   //* cxc: change from IOCTL_WAIT_VE to IOCTL_WAIT_VE_DE to make code compliant.
	IOCTL_RESET_VE,
	IOCTL_ENABLE_VE,
	IOCTL_DISABLE_VE,
	IOCTL_SET_VE_FREQ,

	IOCTL_CONFIG_AVS2 = 0x200,
	IOCTL_GETVALUE_AVS2 ,
	IOCTL_PAUSE_AVS2 ,
	IOCTL_START_AVS2 ,
	IOCTL_RESET_AVS2 ,
	IOCTL_ADJUST_AVS2,
	IOCTL_ENGINE_REQ,
	IOCTL_ENGINE_REL,
	IOCTL_ENGINE_CHECK_DELAY,
    IOCTL_GET_IC_VER,
	IOCTL_ADJUST_AVS2_ABS,
	IOCTL_FLUSH_CACHE,
	IOCTL_SET_REFCOUNT,
	IOCTL_FLUSH_CACHE_ALL,
	IOCTL_TEST_VERSION,
	
	IOCTL_READ_REG = 0x300,
	IOCTL_WRITE_REG,
};

#define IOCTL_WAIT_VE_EN IOCTL_WAIT_VE_DE   //* decoder and encoder is together on 1623.

typedef struct CEDARV_ENV_INFOMATION{
	unsigned int phymem_start;
	int  phymem_total_size;
	unsigned int  address_macc;
}cedarv_env_info_t;

enum CEDARX_CACHE_OP {
	CEDARX_DCACHE_FLUSH = 0,
	CEDARX_DCACHE_CLEAN_FLUSH,
	CEDARX_DCACHE_FLUSH_ALL,
};

typedef struct cedarv_cache_range_{
	long start;
	long end;
}cedarv_cache_range;

struct cedarv_regop {
    unsigned int addr;
    unsigned int value;
};

#endif
