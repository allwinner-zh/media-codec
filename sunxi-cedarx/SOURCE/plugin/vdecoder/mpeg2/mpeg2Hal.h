/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
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

#ifndef MPEG_HAL_H
#define MPEG_HAL_H


#ifdef __cplusplus
extern "C" {
#endif


#define	vdec_readuint32(offset)			SYS_ReadDWord(offset)
#define	vdec_writeuint32(offset,value)	SYS_WriteDWord(offset,value)
#define	vdec_writeuint8(offset,value)	SYS_WriteByte(offset,value)


#define  MACC_VE_VERSION 0xF0
// version 1.0

#define MPHR_REG         0x00
#define MVOPHR_REG       0x04
#define FSIZE_REG        0x08
#define PICSIZE_REG   	 0x0c
#define MBADDR_REG       0x10
#define VECTRL_REG       0x14
#define VETRIGGER_REG    0x18
#define VESTAT_REG       0x1c
#define TRBTRDFLD_REG    0x20
#define DMCOEFFRM_REG    0x24          
#define VLDBADDR_REG     0x28
#define VLDOFFSET_REG    0x2c
#define VLDLEN_REG       0x30
#define VBVENDADDR_REG   0x34

#define REYADDR_REG      0x48
#define RECADDR_REG       0x4c
#define FREFYADDR_REG    0x50
#define FREFCADDR_REG    0x54
#define BREFYADDR_REG    0x58
#define BREFCADDR_REG    0x5c

#define IQMINPUT_REG     0x80
#define ERRFLAG_REG      0xc4
#define CRTMBADDR_REG    0xc8

#define STCDdHWBITOFFSET_REG        0xf0


//* top level registers
typedef struct
{
    volatile  unsigned    ve_mode                 :4;
    volatile  unsigned    r0                      :28;
}regVEMode;

typedef struct
{
    volatile  unsigned    ve_reset                :1;
    volatile  unsigned    r0                      :31;
}regVEReset;

typedef struct  
{
    volatile  unsigned    ve_count_value          :31;
    volatile  unsigned    ve_count_en             :1;
}regVECounter;

typedef struct  
{
    volatile  unsigned    overtime_value          :31;
    volatile  unsigned    r0                      :1;
}regVETimeoutVal;


typedef struct
{
    volatile  unsigned    time_out_intr           :1;
    volatile  unsigned    r0                      :6;
    volatile  unsigned    mpg_int                 :1;
    volatile  unsigned    time_out_intr_en        :1;
    volatile  unsigned    r1                      :7;
    volatile  unsigned    mem_sync_idle           :1;
    volatile  unsigned    r2                      :13;
    volatile  unsigned    r3                      :1;
    volatile  unsigned    r4                      :1;
}regVETopStatus;


// mpeg control register
typedef struct
{
    volatile  unsigned fp_b_vec               : 1;    //lsb  0: motion vectors are coded in half-pel units (MPEG2 only);1:coded in full-pel units.
    volatile  unsigned fp_f_vec               : 1;    // |   0: motion vectors are coded in half-pel units (MPEG2 only);1:coded in full-pel units.
    volatile  unsigned alter_scan             : 1;    // |   0: Zig-Zag scan (MPEG1 only);1: Alternate scan.
    volatile  unsigned intra_vlc_format       : 1;    // |   0: select table B-14 as DCT coefficient VLC tables of intra MB (MPEG1 only);1: select table B-15
    volatile  unsigned q_scale_type           : 1;    // |   0: linear quantiser_sacle (MPEG1 only);1: non-linear quantiser_scale .
    volatile  unsigned con_motion_vec         : 1;    // |   0: no motion vectors are coded in intra macro-blocks (MPEG1 only);1: are coded.
    volatile  unsigned frame_pred_dct         : 1;    // |   0: other case;1: only frame prediction and frame dct(MPEG1 only).
    volatile  unsigned top_field_first        : 1;    // |   0: the bottom field is the field to be display;1: the top field is.
    volatile  unsigned pic_structure          : 2;    // |   00: reserved;01: Top Field;10: Bottom Field;11: Frame picture (MPEG1 only).
    volatile  unsigned intra_dc_precision     : 2;    // |   00: 8-bits precision (MPEG1 only);01: 9-bits precision;10: 10-bits ;11: 11-bots.
    volatile  unsigned f_code11               : 4;    // |   taking value 1 through 9, or 15.Backward vertical motion vector range code.
    volatile  unsigned f_code10               : 4;    // |   taking value 1 through 9, or 15.Backward horizontal.
    volatile  unsigned f_code01               : 4;    // |   taking value 1 through 9, or 15.Forward vertical .
    volatile  unsigned f_code00               : 4;    // |   taking value 1 through 9, or 15.Forward horizontal.
    volatile  unsigned pic_coding_type        : 3;    // |   decide I,p,B or D picture.
    volatile  unsigned r0                     : 1;    //msb  Reserved
}regMPHR;

typedef struct
{

    volatile  unsigned vop_fcode_b            : 3;    // taking values from 1 to 7.Backward motion vector range code
    volatile  unsigned vop_fcode_f            : 3;    // taking values from 1 to 7.Forward
    volatile  unsigned alter_v_scan           : 1;    // 0: other;1: indicates the use of alternate vertical scan fro interlaced VOPs
    volatile  unsigned top_field_first        : 1;    // 0: the bottom field is the field to be display;1: the top field.
    volatile  unsigned intra_dc_vlc_thr       : 3;    // about DC VLC ,AC VLC.
    volatile  unsigned r0                     : 1;    //
    volatile  unsigned r1                     : 1;    // 0: not, 1: yes
    volatile  unsigned en_adv_intra_pred      : 1;    // 0: disable, 1: enable advanced intra prediction
    volatile  unsigned en_modi_quant          : 1;    // 0: disable, 1: enable modified quantization
    volatile  unsigned r2                     : 1;    // 0: not, 1: yes
    volatile  unsigned r3                     : 1;
    volatile  unsigned vop_rounding_type      : 1;    // 0: the value of rounding_control is 0;1: is 1.
    volatile  unsigned vop_coding_type        : 2;    // I,P,B coded or sprite.
    volatile  unsigned swp_num                : 2;    //num of sprite_wrapping_points the range is 0~3
    volatile  unsigned resync_marker_dis      : 1;    // 0: other;1:  no resync_marker in coded VOP.
    volatile  unsigned quarter_sample         : 1;    // 0: indicate the half sample mode;1:that quarter sample mode shall be used for MC of the luminance component
    volatile  unsigned quant_type             : 1;    // 0: second inverse quantization method;1: first inverse  (MPEG)
    volatile  unsigned sprite_wrap_accuracy   : 2;    // 00: 1/2 pixel;01: 1/4 pixel;10: 1/8 pixel;11: 1/16 pixel.
    volatile  unsigned r4                     : 1;    // Reserved
    volatile  unsigned co_located_vop_type    : 2;    // It is used for B-vop.
    volatile  unsigned interlaced             : 1;    // 0: the VOP is of no-interlaced (or progressive) format;1: the VOP may contain interlaced video
    volatile  unsigned short_video_header     : 1;    // 0: other;1:an abbreviation header format is used for video content,forward compatibility with ITU-T Recommendation.
}regMVOPHR;

typedef struct                      //0x08
{
    volatile  unsigned pic_height_in_mbs      : 8;    // Picture height, in units of MBs
    volatile  unsigned pic_width_in_mbs       : 8;    // Picture width, in units of MBs
    volatile  unsigned r0                     : 16;    // Reserved
}regFSIZE;

typedef struct                      //0x0c
{
    volatile  unsigned pic_boundary_height    : 12;   //
    volatile  unsigned r0                     :  4;   //
    volatile  unsigned pic_boundary_width     : 12;
    volatile  unsigned r1                     :  4;
}regPICSIZE;

typedef struct                      //0x10
{
    volatile  unsigned mb_y                   : 8;    // Y coordinate of current decoding MB or start decoding MB
    volatile  unsigned mb_x                   : 8;    // X coordinate of current decoding MB or start decoding MB
    volatile  unsigned r0                     : 16;   // Reserved
}regMBADDR;

typedef struct
{
    volatile  unsigned r0                     : 3;    // Reserved
    volatile  unsigned ve_finish_int_en       : 1;    // 0: disable VE Finish Interrupt; 1: enable
    volatile  unsigned ve_error_int_en        : 1;    // 0: disable VE Error Interrupt ;1: enable
    volatile  unsigned vld_mem_req_int_en     : 1;    // 0: disable interrupt;1: enable
    volatile  unsigned r1                     : 1;    // 0: disable interrupt, 1: enable interrupt
    volatile  unsigned not_write_recons_flag  : 1;    // 0: write reconstruction picture, 1: not write
    volatile  unsigned r2                     : 1;    // enable scale/rotation pic output 0:disable 1: enable
    volatile  unsigned r3                     : 3;    // Reserved
    volatile  unsigned nc_flag_out_en         : 1;    // 0: disable output;1: enable
    volatile  unsigned outloop_dblk_en        : 1;    // 0: disable, 1: enable
    volatile  unsigned qp_ac_dc_out_en        : 1;    // 0: disable QP/AC/DC output;1: enable
    volatile  unsigned r4                     : 1;    // reserved
    volatile  unsigned histogram_output_en    : 1;    // 0: disable output ;1:enable
    volatile  unsigned sw_iq_is               : 1;    // 1: sw vld/iq/is decode; 0: HW iq/is decode
    volatile  unsigned r5                     : 1;    //
    volatile  unsigned mvcs_fld_hm            : 1;    // Field MV half pixel mode
    volatile  unsigned mvcs_fld_qm            : 2;    // Field MV quarter pixel mode
    volatile  unsigned mvcs_mv1_qm            : 2;    // 1 MV quarter pixel mode
    volatile  unsigned mvcs_mv4_qm            : 2;    // 4 MV quarter pixel mode
    volatile  unsigned r6                     : 1;
    volatile  unsigned swvld_flag             : 1;    // 0: hardware vld, 1: software vl
    volatile  unsigned r7                     : 3;    // Reserved
    volatile  unsigned mc_cache_enable        : 1;
}regVECTRL;

typedef struct                      //0x18
{
    volatile  unsigned ve_start_type          : 4;    // about HW VLD level and SW VLD level.
    volatile  unsigned stcd_type              : 2;
    volatile  unsigned r0                     : 1;    // Reserved
    volatile  unsigned r1                     : 1;
    volatile  unsigned num_mb_in_gob          : 15;   // Number of macro block is up to 8120 when "ve_start_type" is equal "100".
    volatile  unsigned r2                     : 1;    // Reserved
    volatile  unsigned dec_format             : 3;    // 000: reserved;001: mpeg1 format;010: mpeg2 ;011
    volatile  unsigned chrom_format           : 3;    // 00: 4:2:0 (default);01: 4:1:1;10: 4:2:2;11: Reserved
    volatile  unsigned r3                     : 1;
    volatile  unsigned mb_boundary            : 1;    // resync boundary
}regVETRIGGER;

typedef struct                      //0x1c
{
    volatile  unsigned ve_finish              : 1;    // The bit is set "1" by hardware when current decoding process is successful completed and send interrupt to host (CPU), the bit is set "1" by software to clear interrupt.
    volatile  unsigned ve_error               : 1;    // The bit is set "1" by hardware when error is encountered in decoding, and send interrupt to host (CPU), the bit is set "1" by software to clear interrupt.
    volatile  unsigned vld_mem_req            : 1;    // The bit is set "1" by hardware when no data for VLD to decode, and send interrupt to host(CPU), the bit is set "1" by software to clear interrupt.
    volatile  unsigned r0                     : 1;
    volatile  unsigned time_out_int           : 1;    
	volatile  unsigned r1                     : 3;
    volatile  unsigned vld_busy               : 1;    // 0: VLD free; 1: VLD busy
    volatile  unsigned dcac_busy              : 1;    // Not used
    volatile  unsigned iqis_busy              : 1;    // 0: IQIS free; 1: IQIS busy
    volatile  unsigned idct_busy              : 1;    // 0: IDCT free, 1: IDCT busy
    volatile  unsigned mc_busy                : 1;    // 0: MC free, 1: MC busy
    volatile  unsigned ve_busy                : 1;    // 0: VE free, 1: VE busy
    volatile  unsigned idct_in_buf_empty      : 1;    // 1: IDCT block in buffer is empty, ready to receive data;0:  is full, not ready to receive data
    volatile  unsigned iqis_in_buf_empty      : 1;    // 1: IQIS block in buffer is empty, ready to receive data;0:  is full, not ready to receive data
    volatile  unsigned r2                     : 1;    // 0
    volatile  unsigned dblk_busy              : 1;    //
	volatile  unsigned r3                     : 1;
	volatile  unsigned r4                     : 4;
    volatile  unsigned r5                     : 1;
    volatile  unsigned r6                     : 3;
	volatile  unsigned stcd_busy              : 1;
    volatile  unsigned r7                     : 4;   // Reserved
}regVESTAT;


typedef struct                      //0x28
{   
    volatile  unsigned vld_byte_start_addr   : 28;   // Current Bit Stream Read Start Address (byte address)
    volatile  unsigned vbv_buff_data_valid    : 1;    // SW set "1" to indicate the new data in VBV buffer is OK.
    volatile  unsigned vbv_buff_data_last     : 1;    // 1: Means HW needn't request data from memory if HW read the last valid data. HW will decode the part bytes data of a DW in internal buffer.0: Means HW can request memory to get the data. HW will decode the part bytes data of a DW in internal buffer until the rest bytes data are received.
    volatile  unsigned vbv_buff_data_first    : 1;    // 1: Means It's the first amount of data for a picture, VOP, packet, GOB;0: Means It's not the first amount of
    volatile  unsigned r1                     : 1;    // Reserved
}regVLDBADDR;


typedef struct                     //0x2c
{
    volatile  unsigned vld_bit_offset         : 29;   // Bit offset from VLD_Byte_Base_Address
    volatile  unsigned r0                     : 3;    // Reserved
}regVLDOFFSET;

typedef struct     //0x30
{
    volatile  unsigned vld_bit_len            : 28;   // Bit length form VLD bit start address
    volatile  unsigned r0                     : 4;    // Reserved
}regVLDLEN;

typedef struct     //0x34
{   
    volatile  unsigned vld_byte_end_addr      :32;    // Current Bit Stream Read End Address (byte address)
}regVBVENDADDR;


typedef struct   //0x48
{  
	volatile  unsigned re_ybuf_addr           : 32;
}regREYADDR;  
  
typedef struct   //0x4c
{
	volatile  unsigned re_cbuf_addr           : 32;
}regRECADDR; 

typedef struct   //0x50
{ 
	volatile  unsigned fref_ybuf_addr         : 32;
}regFREFYADDR;

typedef struct   //0x54
{
	volatile  unsigned fref_cbuf_addr         : 32;
}regFREFCADDR;

typedef struct   //0x58
{
	volatile  unsigned bref_ybuf_addr         : 32;
}regBREFYADDR;

typedef struct   //0x5c
{
	volatile  unsigned bref_cbuf_addr         : 32;
}regBREFCADDR;


typedef struct  //0xc4
{
    volatile  unsigned case0                  : 1;    // Error 0:VLD need data, but last flag is set to 1
    volatile  unsigned case1                  : 1;    // Error 1: non slice header is found such as "picture header is found"
    volatile  unsigned case2                  : 1;    // Error 2: slice location wrong (MBA >= MBAmax)
    volatile  unsigned case3                  : 1;    // Error 3: VLC or run level error
    volatile  unsigned case4                  : 1;    // Error 4: context error (e.g. an illegal value is found)
    volatile  unsigned case5                  : 1;    // Error 5: coefficient index >= 64
    volatile  unsigned r0                     : 26;   // reserved
}regERRFLAG;

typedef struct                      //0xc8
{
    volatile  unsigned crt_mb_num             : 16;   // It means how many MBs there are are correct for a picture, packet, GOB and VOP.
    volatile  unsigned r0                     : 16;   // Reserved

}regCRTMBADDR;

typedef struct  //0xf0
{
	volatile  unsigned stcd_hw_bitoffset       : 29;
	volatile  unsigned r0                      : 3;
}regSTCDHWBITOFFSET;


regMPHR         mphr_reg00;
regMVOPHR       mvophr_reg04;
regFSIZE        fsize_reg08;
regPICSIZE   	picsize_reg0c;
regMBADDR       mbaddr_reg10;
regVECTRL       vectrl_reg14;
regVETRIGGER    vetrigger_reg18;
regVESTAT       vestat_reg1c;
regVLDBADDR     vldbaddr_reg28;
regVLDOFFSET    vldoffset_reg2c;
regVLDLEN       vldlen_reg30;
regVBVENDADDR   vbvsize_reg34;

regREYADDR      reyaddr_reg48;
regRECADDR      recaddr_reg4c;
regFREFYADDR    frefyaddr_reg50;
regFREFCADDR    frefcaddr_reg54;
regBREFYADDR    brefyaddr_reg58;
regBREFCADDR    brefcaddr_reg5c;

regERRFLAG           errflag_regc4;
regCRTMBADDR         crtmb_regc8;
regSTCDHWBITOFFSET   stcdHwBitOffset_regf0;

#define SYS_WriteByte(uAddr, bVal) \
do{*(volatile uint8_t *)(uAddr) = (bVal);}while(0)
#define SYS_WriteWord(uAddr, wVal) \
do{*(volatile uint16_t *)(uAddr) = (wVal);}while(0)
#define SYS_WriteDWord(uAddr, uDwVal) \
do{*(volatile uint32_t *)(uAddr) = (uDwVal);}while(0)

#define SYS_ReadByte(uAddr) \
    bVal = (*(volatile uint8_t *)(uAddr));

#define SYS_ReadWord(uAddr) \
    wVal = (*(volatile uint16_t *)(uAddr));

#define SYS_ReadDWord(uAddr) \
    uDwVal = (*(volatile uint32_t *)(uAddr));


#ifdef __cplusplus
}
#endif

#endif


