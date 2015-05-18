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
#ifndef VE_H
#define VE_H

#ifdef __cplusplus
extern "C" {
#endif

enum DRAMTYPE
{
    DDRTYPE_DDR1_16BITS = 0,
    DDRTYPE_DDR1_32BITS = 1,
    DDRTYPE_DDR2_16BITS = 2,
    DDRTYPE_DDR2_32BITS = 3,
    DDRTYPE_DDR3_16BITS = 4,
    DDRTYPE_DDR3_32BITS = 5,
    DDRTYPE_DDR3_64BITS = 6,
    
    DDRTYPE_MIN = DDRTYPE_DDR1_16BITS,
    DDRTYPE_MAX = DDRTYPE_DDR3_64BITS,
};

enum VeRegionE
{
    VE_REGION_INVALID,
    VE_REGION_0, /* Mpeg1/2/4, mjpeg */
    VE_REGION_1, /* H264,VP8 */
    VE_REGION_2, /* VC-1 */
    VE_REGION_3  /* H265 */
};

typedef struct VETOP_REG_MODE_SELECT
{
	volatile unsigned int mode						:4; //* 0: mpeg/jpeg, 1:h264
	volatile unsigned int reserved0					:1;
	volatile unsigned int jpg_dec_en 				:1; //
	volatile unsigned int enc_isp_enable 			:1; //
	volatile unsigned int enc_enable 				:1; //* H264 encoder enable.
	volatile unsigned int read_counter_sel			:1;
	volatile unsigned int write_counter_sel			:1;
	
	volatile unsigned int decclkgen					:1;
	volatile unsigned int encclkgen					:1;
	volatile unsigned int reserved1					:1;
	volatile unsigned int rabvline_spu_dis			:1;
	volatile unsigned int deblk_spu_dis				:1;
	volatile unsigned int mc_spu_dis 				:1;
	volatile unsigned int ddr_mode					:2; //* 00: 16-DDR1, 01: 32-DDR1 or DDR2, 10: 32-DDR2 or 16-DDR3, 11: 32-DDR3
	
	volatile unsigned int reserved2					:1;
	volatile unsigned int mbcntsel					:1;
	volatile unsigned int rec_wr_mode				:1;
	volatile unsigned int pic_width_more_2048		:1;

	volatile unsigned int reserved3			        :1;
	volatile unsigned int reserved4					:9;
}vetop_reg_mode_sel_t;


//* 0x04
typedef struct VETOP_REG_RESET
{
    volatile unsigned int reset                     :1;
    volatile unsigned int reserved0                 :3;
    volatile unsigned int mem_sync_mask             :1;

    volatile unsigned int wdram_clr                 :1;
    volatile unsigned int reserved1                 :2;
    volatile unsigned int write_dram_finish         :1;
    volatile unsigned int ve_sync_idle              :1;     //* this bit can be used to check the status of sync module before rest.
    volatile unsigned int reserved2                 :6;
    volatile unsigned int decoder_reset             :1;     //* 1: reset assert, 0: reset de-assert.
    volatile unsigned int dec_req_mask_enable       :1;     //* 1: mask, 0: pass.
   // volatile unsigned int reserved3	                :6;
    volatile unsigned int  dec_vebk_reset		    :1;		//* 1: reset assert, 0: reset de-assert. used in decoder.
    volatile unsigned int  reserved3				:5;

    volatile unsigned int encoder_reset             :1;     //* 1. reset assert, 0: reset de-assert.
    volatile unsigned int enc_req_mask_enable       :1;     //* 1: mask, 0: pass.
    volatile unsigned int reserved4                 :6;
}vetop_reg_reset_t;


int   VeInitialize(void);

void  VeRelease(void);

int   VeLock(void);

void  VeUnLock(void);

int VeEncoderLock(void);

void VeEncoderUnLock(void);

void VeSetDramType();

void  VeReset(void);

int   VeWaitInterrupt(void);

int VeWaitEncoderInterrupt(void);

void* VeGetRegisterBaseAddress(void);

unsigned int VeGetIcVersion();

int   VeGetDramType(void);

int   VeSetSpeed(int nSpeedMHz);

void VeEnableEncoder();

void VeDisableEncoder();

void VeEnableDecoder(enum VeRegionE region);

void VeDisableDecoder();

void VeDecoderWidthMode(int nWidth);

void VeResetDecoder();

void VeResetEncoder();

void VeInitEncoderPerformance(int nMode);

void VeUninitEncoderPerformance(int nMode);

#if (CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_VE)
void *VeMalloc(int size);

void VeFree(void *ptr);

unsigned int VeVir2Phy(void *ptr);

unsigned int VePhy2Vir(void *ptr); //*

void VeFlushCache(void *startAddr, int size);

#endif

#ifdef __cplusplus
}
#endif


#endif

