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
#ifndef LIBVE_REGISTER_H
#define LIBVE_REGISTER_H

#include "adapter.h"
//#include "secureMemoryAdapter.h"
#include "log.h"

static const unsigned int REG_GROUP_OFFSET_ARR[] = {0, 0x100, 0x200};
static const char* REG_GROUP_NAME[] = {"Top Level", "Mpeg 1/2/4", "H264"};
    
typedef enum VE_REGISTER_GROUP
{
    REG_GROUP_VETOP             = 0,
    REG_GROUP_MPEG_DECODER      = 1,
    REG_GROUP_H264_DECODER      = 2,
    REG_GROUP_VC1_DECODER = 3,
}ve_register_group_e;


//* 0x08
typedef struct VETOP_REG_CODEC_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_codec_cntr_t;


//* 0x0c
typedef struct VETOP_REG_CODEC_OVERTIME
{
    volatile unsigned int overtime_value            :31;
    volatile unsigned int reserved0                 :1;
}vetop_reg_codec_overtime_t;


//* 0x10
typedef struct VETOP_REG_MMC_REQUEST_WNUM
{
    volatile unsigned int wnum                      :24;
    volatile unsigned int reserved0                 :8;
}vetop_reg_mmc_req_wnum_t;


//* 0x14
typedef struct VETOP_REG_CACHE_REQ_WNUM
{
    volatile unsigned int wnum                      :24;
    volatile unsigned int reserved0                 :8;
}vetop_reg_cache_req_wnum_t;

//* 0x1c
typedef struct VETOP_REG_STATUS
{
    volatile unsigned int timeout_intr              :1;
    
	//
    volatile unsigned int isp_intr                  :1;
    volatile unsigned int ave_intr                  :1;
    volatile unsigned int mpe_intr                  :1;
    volatile unsigned int reserved0                 :1;
    volatile unsigned int reserved1                 :1;
    volatile unsigned int avc_intr                  :1;
    volatile unsigned int mpg_intr                  :1;
    
    volatile unsigned int timeout_enable            :1;
    

    volatile unsigned int graphic_intr              :1;
    volatile unsigned int graphic_intr_enable       :1;
    volatile unsigned int reserved2                 :5;
    
    volatile unsigned int mem_sync_idle             :1;    //* this bit will be 1 when mem sync idle.
    
    volatile unsigned int reserved3                 :11;

    volatile unsigned int graphic_cmd_queue_busy    :1;
    volatile unsigned int graphic_busy              :1;
    
    //
    volatile unsigned int reserved4                 :1;
    
    volatile unsigned int esp_mode                  :1;
}vetop_reg_status_t;


//* 0x20
typedef struct VETOP_REG_DRAM_READ_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_dram_read_cntr_t;


//* 0x24
typedef struct VETOP_REG_DRAM_WRITE_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_dram_write_cntr_t;

//* 0x50
typedef struct VETOP_REG_DEBLK_INTRAPRED_BUF_CTRL 
{
    volatile unsigned int deblk_buf_ctrl             :2;     //* deblocking above buffer's control bits. 00: all data is stored in internal SRAM, 01: the data of left 1280 pixels of picture is stored in internal SRAM. 10: all data is stored in DRAM.
    volatile unsigned int intrapred_buf_ctrl         :2;     //* intra prediction above buffer's control bits.
    volatile unsigned int reserved0                  :28;
}vetop_reg_deblk_intrapred_buf_ctrl_t;


//* 0x54
typedef struct VETOP_REG_DEBLK_DRAM_BUF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_deblk_dram_buf_addr_t;


//* 0x58
typedef struct VETOP_REG_INTRAPRED_DRAM_BUF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_intrapred_dram_buf_addr_t;

//* 0x90 - 0xcc 16 registers.
typedef struct VETOP_REG_LUMA_HISTOGRAM_VALUE
{
    volatile unsigned int value                      :20;
    volatile unsigned int reserved0                  :12;
}vetop_reg_luma_hist_value_t;
//* 0xe0
typedef struct VETOP_REG_SRAM_OFFSET
{
    volatile unsigned int addr                       :12;
    volatile unsigned int reserved0                  :20;
}vetop_reg_sram_offset_t;


//* 0xe4 //* register not used in 1633.
typedef struct VETOP_REG_SRAM_DATA
{
    volatile unsigned int data;
}vetop_reg_sram_data_t;

//* 0xf8 //* register not used in 1633.
typedef struct VETOP_REG_DEBUG_CONTROL
{
    volatile unsigned int dbg_ctrl      :15;
    volatile unsigned int reserved0     :17;
}vetop_reg_dbg_ctrl_t;


//* 0xfc //* register not used in 1633.
typedef struct VETOP_REG_DEBUG_OUTPUT
{
    volatile unsigned int dbg_output    :32;
}vetop_reg_dbg_output_t;


//* define VE top level register list.
typedef struct VETOP_REGISTER_LIST
{
    volatile vetop_reg_mode_sel_t           _00_mode_sel;
    vetop_reg_reset_t                       _04_reset;
    vetop_reg_codec_cntr_t                  _08_codec_cntr;
    vetop_reg_codec_overtime_t              _0c_overtime;
    vetop_reg_mmc_req_wnum_t                _10_mmc_req;
    vetop_reg_cache_req_wnum_t              _14_cache_req;
    unsigned int                            _18_reserved0;
    vetop_reg_status_t                      _1c_status;
    vetop_reg_dram_read_cntr_t              _20_dram_read_cntr;
    vetop_reg_dram_write_cntr_t             _24_dram_write_cntr;

    unsigned int                            _28_reserved;
    unsigned int                            _2c_reserved1;
    
    //*
    unsigned int                           _30_reserved;
    unsigned int                           _34_reserved;
    unsigned int                           _38_reserved;
    unsigned int                           _3c_reserved;
    unsigned int                           _40_reserved;
    unsigned int                           _44_reserved;
    unsigned int                           _48_reserved;
    unsigned int                           _4c_reserved;
    
    //* the following 12 registers are for 1619.
    vetop_reg_deblk_intrapred_buf_ctrl_t    _50_deblk_intrapred_buf_ctrl;
    vetop_reg_deblk_dram_buf_addr_t         _54_deblk_dram_buf;
    vetop_reg_intrapred_dram_buf_addr_t     _58_intrapred_dram_buf;
    unsigned int                            _5c_reserved;
    unsigned int                            _60_reserved;
    unsigned int                            _64_reserved;
    unsigned int                            _68_reserved;
    unsigned int                            _6c_reserved;
    unsigned int                            _70_reserved;
    unsigned int                            _74_reserved;
    unsigned int                            _78_reserved;
    unsigned int                            _7c_reserved;
    
    unsigned int                            _80_luma_hist_threshold[4]; //
    vetop_reg_luma_hist_value_t             _90_luma_hist_value[12];    //

	unsigned int                             _c0_reserved;
	unsigned int                             _c4_reserved;
	unsigned int                             _c8_reserved;
	unsigned int                             _cc_reserved;
	unsigned int                             _d0_reserved;
	unsigned int                             _d4_reserved;
	unsigned int                             _d8_reserved;
	unsigned int                             _dc_reserved[3];
    unsigned int				             _e8_reserved;
    unsigned int 				             _ec_reserved;

	unsigned int                            _f0_reserved[2];
    vetop_reg_dbg_ctrl_t                    _f8_dbg_ctrl;
    vetop_reg_dbg_output_t                  _fc_dbg_output;
}vetop_reglist_t;




static __inline void* ve_get_reglist(ve_register_group_e reg_group_id)
{
    void*           reglist;
    unsigned int    tmp;
    
    tmp = (unsigned int)AdapterVeGetBaseAddress();
    
    tmp += REG_GROUP_OFFSET_ARR[reg_group_id];
    
    reglist = (void*)tmp;
    
    return reglist;
}

static __inline void ve_print_regs(ve_register_group_e reg_group_id, unsigned int offset, unsigned int count)
{
	unsigned int  i;
    unsigned int  tmp;
    unsigned int* base;
    unsigned int nOffset = 0;

    nOffset = offset;

    tmp = (unsigned int)AdapterVeGetBaseAddress();

    base = (unsigned int*)(tmp + REG_GROUP_OFFSET_ARR[reg_group_id]);

    logv("print register of %s, start offset = %d, count = %d", REG_GROUP_NAME[reg_group_id], offset, count);
    for(i=0; i<count; i++)
    {
    	logv("    reg[%2.2x] = %8.8x", offset + 4*i, base[(offset>>2) + i]);
    }

    logv("\n");
}

extern unsigned int gVeVersion;
static __inline unsigned int get_ve_version_id(void)
{
    return gVeVersion;
}

#define SetRegValue(REG, VALUE)     (*((uint32_t*)&REG) = (VALUE))
#define GetRegValue(REG)            (*((uint32_t*)&REG))

#endif

