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
#ifndef _MP4_REGISTER_H
#define _MP4_REGISTER_H

#include "mpeg4_config.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		unsigned mode		:  4;
		unsigned r0			:  4;
		unsigned isrdreq	:  1;
		unsigned iswrreq	:  1;
		unsigned r1			:  22;
	}mp4regVECORE_MODESEL;
	
	typedef struct
	{
		unsigned timeout_int	: 1;
		unsigned r0				: 7;
		unsigned timeout_int_en	: 1;
		unsigned r1				: 7;
		unsigned mem_sync_idel	: 1;
		unsigned r2				:14;
		unsigned esp_mode		: 1;
	}mp4regVECORE_STATUS;
	
	extern mp4regVECORE_MODESEL mp4vecore_modesel_reg00;
	extern mp4regVECORE_STATUS mp4vecore_status_reg1c;
	
	// Video Engine start type
	#define PICTURE_LEVEL_VLD_START			0xf
	#define GOB_LEVEL_START					0xd
	#define PIC_ROTATE_START				0xc
	#define MB_MC_START						0x7
	#define BLOCK_IQ_START					0x6
	#define BLOCK_IDCT_START				0x5
	
	// version 1.0
	#define MPHR_REG				0x00
	#define MVOPHR_REG				0x04
	#define FSIZE_REG               0x08
	#define PICSIZE_REG				0x0c
	#define MBADDR_REG				0x10
	#define VECTRL_REG              0x14
	#define VETRIGGER_REG			0x18
	#define VESTAT_REG				0x1c
	#define TRBTRDFLD_REG			0x20
	#define TRBTRDFRM_REG			0x24
	#define VLDBADDR_REG			0x28
	#define VLDOFFSET_REG			0x2c
	#define VLDLEN_REG				0x30
	#define VBVENDADDR_REG			0x34
	#define MBHADDR_REG				0x38
	#define DCACADDR_REG			0x3c
	#define DBLKUPADDR_REG			0x40
	#define NCFADDR_REG				0x44
	#define RECYADDR_REG			0x48
	#define RECCADDR_REG			0x4c
	#define FORYADDR_REG			0x50
	#define FORCADDR_REG			0x54
	#define BACKYADDR_REG			0x58
	#define BACKCADDR_REG			0x5c
	#define SOCX_REG				0x60
	#define SOCY_REG				0x64
	#define SOL_REG					0x68
	#define SDLX_REG				0x6c
	#define SDLY_REG				0x70
	#define SSR_REG					0x74
	#define SDCX_REG				0x78
	#define SDCY_REG				0x7c
	#define IQMINPUT_REG			0x80
	#define QCINPUT_REG				0x84
	#define IQ_IDCTINPUT_REG		0x90
	#define MBH_REG					0x94
	#define MV1_REG					0x98
	#define MV2_REG					0x9c
	#define MV3_REG					0xa0
	#define MV4_REG					0xa4
	#define MV5_REG					0xa8
	#define MV6_REG					0xac
	#define MV7_REG					0xb0
	#define MV8_REG					0xb4
	#define ERRFLAG_REG				0xc4
	#define CRTMB_REG				0xc8
	
	// Video Engine start type
	#define MP4_PICTURE_START					0xf
	#define MP4_GOB__START						0xd
	#define MP4_MB_MC_START						0x7
	#define MP4__BLOCK_IQ_START					0x6
	#define MP4_BLOCK_IDCT_START				0x5
	#define MP4_PICTURE_DEBLOCK_START			0x1
	
	#define SKIP_FLAGS_SRAM_ADDR			0x10000
	#define AC_MODEL_SRAM_ADDR				0x20000
	
	typedef struct
	{
		unsigned fp_b_vec			: 1;	//lsb  0: motion vectors are coded in half-pel units (MPEG2 only);1:coded in full-pel units.
		unsigned fp_f_vec			: 1;	// |   0: motion vectors are coded in half-pel units (MPEG2 only);1:coded in full-pel units.
		unsigned alter_scan			: 1;	// |   0: Zig-Zag scan (MPEG1 only);1: Alternate scan.
		unsigned intra_vlc_format	: 1;	// |   0: select table B-14 as DCT coefficient VLC tables of intra MB (MPEG1 only);1: select table B-15
		unsigned q_scale_type		: 1;	// |   0: linear quantiser_sacle (MPEG1 only);1: non-linear quantiser_scale .
		unsigned con_motion_vec		: 1;	// |   0: no motion vectors are coded in intra macro-blocks (MPEG1 only);1: are coded.
		unsigned frame_pred_dct		: 1;	// |   0: other case;1: only frame prediction and frame dct(MPEG1 only).
		unsigned top_field_first	: 1;	// |   0: the bottom field is the field to be display;1: the top field is.
		unsigned pic_structure		: 2;	// |   00: reserved;01: Top Field;10: Bottom Field;11: Frame picture (MPEG1 only).
		unsigned intra_dc_precision	: 2;	// |   00: 8-bits precision (MPEG1 only);01: 9-bits precision;10: 10-bits ;11: 11-bots.
		unsigned f_code11			: 4;	// |   taking value 1 through 9, or 15.Backward vertical motion vector range code.
		unsigned f_code10			: 4;	// |   taking value 1 through 9, or 15.Backward horizontal.
		unsigned f_code01			: 4;	// |   taking value 1 through 9, or 15.Forward vertical .
		unsigned f_code00			: 4;	// |   taking value 1 through 9, or 15.Forward horizontal.
		unsigned pic_coding_type	: 3;	// |   decide I,p,B or D picture.
		unsigned r0					: 1;	//msb  Reserved
	}mp4regMPHR;
	
	typedef struct
	{
	
		unsigned vop_fcode_b			: 3;    //  taking values from 1 to 7.Backward motion vector range code
		unsigned vop_fcode_f			: 3;    //  taking values from 1 to 7.Forward
		unsigned alter_v_scan			: 1;    //  0: other;1: indicates the use of alternate vertical scan fro interlaced VOPs
		unsigned top_field_first		: 1;    //  0: the bottom field is the field to be display;1: the top field.
		unsigned intra_dc_vlc_thr		: 3;    //  about DC VLC ,AC VLC.
		unsigned r0            	        : 1;	//  
		unsigned is_h263_umv			: 1;	//  0: not, 1: yes
		unsigned en_adv_intra_pred		: 1;	//  0: disable, 1: enable advanced intra prediction defined in h.263 Annex I
		unsigned en_modi_quant			: 1;	//  0: disable, 1: enable modified quantization defined in H.263 Annex T.
		unsigned is_h263_pmv			: 1;    //  0: not, 1: yes
		unsigned use_h263_escape		: 1;	//  0: not, 1: yes
		unsigned vop_rounding_type		: 1;    //  0: the value of rounding_control is 0 (h263 only);1: is 1.
		unsigned vop_coding_type		: 2;    //  I,P,B coded or sprite.
		unsigned no_wrapping_points		: 2;    //
		unsigned resync_marker_dis		: 1;    //  0: other;1:  no resync_marker in coded VOP (h263 only).
		unsigned quarter_sample			: 1;    //  0: indicate the half sample mode;1:that quarter sample mode shall be used for MC of the luminance component
		unsigned quant_type				: 1;    //  0: second inverse quantization method (H263 only);1: first inverse  (MPEG)
		unsigned sprite_wrap_accuracy	: 2;    //  00: 1/2 pixel;01: 1/4 pixel;10: 1/8 pixel;11: 1/16 pixel.
		unsigned r1             		: 1;    //  0 others, 
		unsigned co_located_vop_type	: 2;    //  It is used for B-vop.
		unsigned interlaced				: 1;    //  0: the VOP is of no-interlaced (or progressive) format;1: the VOP may contain interlaced video
		unsigned short_video_header		: 1;    //  0: other;1:an abbreviation header format is used for video content,forward compatibility with ITU-T Recommendation H.263.
	}mp4regMVOPHR;
	
	typedef struct
	{
		unsigned pic_height_in_mbs		: 8;    //   Picture height, in units of MBs
		unsigned pic_width_in_mbs		: 8;    //   Picture width, in units of MBs
		unsigned store_width_in_mbs		: 8;    //
		unsigned r2						: 8;    //   Reserved
	}mp4regFSIZE;
	
	typedef struct
	{
		unsigned pic_boundary_height	: 12;		//
		unsigned r0						:  4;		//
		unsigned pic_boundary_width		: 12;
		unsigned r1						:  4;
	}mp4regPICSIZE;
	
	typedef struct
	{
		unsigned mb_y                   : 8;    //   Y coordinate of current decoding MB or start decoding MB
		unsigned mb_x                   : 8;    //   X coordinate of current decoding MB or start decoding MB
		unsigned r1                     : 16;   //   Reserved
	}mp4regMBADDR;
	
	typedef struct
	{
		unsigned r0						: 3;    //   Reserved
		unsigned ve_finish_int_en		: 1;    //	0: disable VE Finish Interrupt; 1: enable
		unsigned ve_error_int_en		: 1;    //	0: disable VE Error Interrupt ;1: enable
		unsigned vld_mem_req_int_en		: 1;    //	0: disable interrupt;1: enable
		unsigned r1             		: 1;	//	0: disable interrupt;1: enable
		unsigned not_write_recons_flag	: 1;	//	0: write resonstruction picture, 1: not write
		unsigned r2               		: 1;	//  0: not write rotated picture, 1: write
		unsigned r3						: 3;    //  Reserved
		unsigned nc_flag_out_en			: 1;	//	0: disable output;1: enable
		unsigned outloop_dblk_en		: 1;	//	0: disable out-loop deblocking, 1: enable out-loop deblocking
		unsigned qp_ac_dc_out_en		: 1;	//	0: disable QP/AC/DC output;1: enable
		unsigned r4						: 1;
		unsigned Histogram_output_en	: 1;	//	0: disable histogram output;1:enable
		unsigned by_pass_iqis			: 1;	//  1: enable software iq is, 0: disable
		unsigned r5						: 1;	//
		unsigned mvcs_fld_hm			: 1;	//  Field MV half pixel mode
		unsigned mvcs_fld_qm			: 2;	//  Field MV quarter pixel mode
		unsigned mvcs_mv1_qm			: 2;	//  1 MV quarter pixel mode
		unsigned mvcs_mv4_qm			: 2;	//  4 MV quarter pixel mode
		unsigned r6                  	: 1;	//
		unsigned swvld_flag				: 1;	//  0: hardware vld, 1: software vld
		unsigned sw_chrom_mv_sel		: 1;	//  0: use hw chrom calculation resul; 1: choose sw calculating chrominace MVs
		unsigned r7						: 1;	//  Reserved
		unsigned fdc_qac_in_dram		: 1;	//  0--in internal sram, 1--in external sdram
		unsigned mc_cache_en			: 1;	//	0:disable mc cache, 1: enable mc cache
	}mp4regVECTRL;
	
	typedef struct
	{
		unsigned ve_start_type			: 4;	//	about HW VLD level and SW VLD level.
		unsigned stcd_type				: 2;
		unsigned r0						: 1;	//	Reserved
		unsigned isGetBit				: 1;
		unsigned num_mb_in_gob			: 15;	//	Number of macro block is up to 8120 when "ve_start_type" is equal "100".
		unsigned r1						: 1;	//	Reserved
		unsigned dec_format				: 3;	//
		unsigned chrom_format			: 3;	//	00: 4:2:0 (default);01: 4:1:1;10: 4:2:2;11: Reserved
		unsigned r2				        : 1;	//  
		unsigned mb_boundary			: 1;	//	resync boundary
	}mp4regVETRIGGER;
	
	#define IQ_IN_BUF_EMPTY		1<<15
	#define IDCT_IN_BUF_EMPTY	1<<14
	#define VE_BUSY	1<<13
	#define MC_BUSY	1<<12
	#define IDCT_BUSY	1<<11
	#define IQIS_BUSY	1<<10
	#define DCAC_BUSY	1<<9
	#define VLD_BUSY	1<<8
	#define VLD_MEM_REQ	1<<2
	#define VE_ERROR	1<<1
	#define VE_FINISH	1
	
	typedef struct
	{
		unsigned ve_finish					: 1;	//	The bit is set "1" by hardware when current decoding process is successful completed and send interrupt to host (CPU), the bit is set "1" by software to clear interrupt.
		unsigned ve_error					: 1;	//	The bit is set "1" by hardware when error is encountered in decoding, and send interrupt to host (CPU), the bit is set "1" by software to clear interrupt.
		unsigned vld_mem_req				: 1;	//	The bit is set "1" by hardware when no data for VLD to decode, and send interrupt to host(CPU), the bit is set "1" by software to clear interrupt.
		unsigned r0       			        : 1;	//	The bit is set "1" by hardware when finishing rotation operation, and send interrupt to host(CPU), the bit is set "1" by software to clear interrupt.
		unsigned timeout_int				: 1;
		unsigned r1							: 3;	//	Not used
		unsigned vld_busy					: 1;	//	0: VLD free; 1: VLD busy
		unsigned dcac_busy					: 1;	//	Not used
		unsigned iqis_busy					: 1;	//	0: IQIS free; 1: IQIS busy
		unsigned idct_busy					: 1;	//	0: IDCT free, 1: IDCT busy
		unsigned mc_busy					: 1;	//	0: MC free, 1: MC busy
		unsigned ve_busy					: 1;	//	0: VE free, 1: VE busy
		unsigned idct_in_buf_empty	        : 1;	//	1: IDCT block in buffer is empty, ready to receive data;0:  is full, not ready to receive data
		unsigned iqis_in_buf_empty          : 1;    //	1: IQIS block in buffer is empty, ready to receive data;0:  is full, not ready to receive data
		unsigned r2			                : 1;	//	
		unsigned dblk_busy					: 1;	//	0: MAF free, 1: MAF busy
		unsigned r3			            	: 1;	//18
		unsigned r4             			: 1;	//19
		unsigned r5             			: 1;	//20
		unsigned r6           				: 1;	//21
		unsigned r7							: 2;	//22~23	Reserved
		unsigned sync_idle					: 1;	//24
		unsigned r8							: 7;	//27~31
	}mp4regVESTAT;
	
	typedef struct
	{
		unsigned trd						: 8;
		unsigned trb						: 8;
		unsigned r0							: 16;	//	Reserved
	}mp4regTRBTRDFLD;
	
	typedef struct
	{
		unsigned trd 					:16;		// trd for frame based prediction
		unsigned trb					:16;		// trb for frame based prediction
	}mp4regTRBTRDFRM;
	
	typedef struct
	{
		unsigned vld_byte_start_addr	: 28;		//	Current Bit Stream Read Start Address (byte address)
		unsigned vbv_buff_data_valid	: 1;		//  SW set "1" to indicate the new data in VBV buffer is OK.
		unsigned vbv_buff_data_last		: 1;		//	1: Means HW needn't request data from memory if HW read the last valid data. HW will decode the part bytes data of a DW in internal buffer.0: Means HW can request memory to get the data. HW will decode the part bytes data of a DW in internal buffer until the rest bytes data are received.
		unsigned vbv_buff_data_first	: 1;        //  1: Means It's the first amount of data for a picture, VOP, packet, GOB;0: Means It's not the first amount of
		unsigned r1						: 1;		//  Reserved
	}mp4regVLDBADDR;
	
	typedef struct
	{
		unsigned vld_bit_offset			: 29;       //  Bit offset from VLD_Byte_Base_Address
		unsigned r0	                    : 3;        //  Reserved
	}mp4regVLDOFFSET;
	
	typedef struct
	{
		unsigned vld_bit_len	        : 28;		//	Bit length form VLD bit start address
		unsigned r0						: 4;		//	Reserved
	}mp4regVLDLEN;
	
	typedef struct
	{
		unsigned vld_byte_end_addr		:31;        //  Current Bit Stream Read End Address (byte address)
		unsigned r0						: 1;		//  Reserved
	}mp4regVBVENDADDR;
	
	typedef struct
	{
		unsigned r0						: 10;		// lowest 10 bits
		unsigned mbh_addr				: 21;		// In unit of 1024 bytes (1K)
		unsigned r1						: 1;		// Reserved
	}mp4regMBHADDR;
	
	typedef struct
	{
		unsigned r0						: 10;		// lowest 10 bits
		unsigned addr					: 21;		// highest 18 bits
		unsigned r1						: 1;		//	Reserved
	}mp4regDecBufADDR;
	
	typedef struct
	{
		unsigned addr				    : 31;		// highest 18 bits
		unsigned r0					    : 1;		// reserved
	}mp4regFRMADDR;
	
	typedef struct
	{
		signed sprite_offset_y			: 16;		//	Vertical offset of luminance part in sprite_wrapping_accuracy unit
		signed sprite_offset_x			: 16;		//	Horizontal offset of luminance part in sprite_wrapping_accuacy unit
	}mp4regSO;
	
	typedef struct
	{
		signed sprite_offset_x			: 32;		//	Horizontal offset of luminance part in sprite_wrapping_accuacy unit
	}mp4regSOCX;
	
	typedef struct
	{
		signed sprite_offset_y			: 32;		//	Vertical offset of luminance part in sprite_wrapping_accuracy unit
	}mp4regSOCY;
	
	typedef struct
	{
		signed sprite_delta_xy			: 16;		//	The value of the y-difference between the x-coordinate of a node and the previous node in sprite_wrapping_accuracy unit
		signed sprite_delta_xx			: 16;		//	The value of the x-difference between the x-coordinate of a node and the previous node in sprite_wrapping_accuracy unit
	}mp4regSDX;
	
	typedef struct
	{
		signed sprite_delta_yy			: 16;		//	The value of the y-difference between the y-coordinate of a node and the previous node in sprite_wrapping_accuracy unit
		signed sprite_delta_yx			: 16;		//	The value of the x-difference between the y-coordinate of a node and the previous node in sprite_wrapping_accuracy unit
	}mp4regSDY;
	
	typedef struct
	{
		unsigned sprite_luma_shifter	:5;
		unsigned r0						:3;
		unsigned sprite_chroma_shifter	:5;
		unsigned r1						:3;
		unsigned sprite_delta_luma_xx_b0 :1;
		unsigned sprite_delta_luma_xy_b0 :1;
		unsigned sprite_delta_luma_yx_b0 :1;
		unsigned sprite_delta_luma_yy_b0 :1;
		unsigned sprite_delta_chroma_xx_b0 :1;
		unsigned sprite_delta_chroma_xy_b0 :1;
		unsigned sprite_delta_chroma_yx_b0 :1;
		unsigned sprite_delta_chroma_yy_b0 :1;
		unsigned r2						:7;
		unsigned sprite_delta_data_mode :1;
	}mp4regSSR;
	
	typedef struct
	{
		unsigned quant_weight_value		: 8;		//	8-bit signed integer
		unsigned quant_weight_index		: 6;		//	Default zigzag scanning order
		unsigned intra_matrix_flag		: 1;		//	1: Intra quantiser weight matrix flag;0: Non-intra
		unsigned r0						: 17;		//	Reserved
	}mp4regIQMINPUT;
	
	typedef struct
	{
		unsigned quant_scale			: 6;		//	Quantiser scale in a slice (MPEG2), a packet (MPEG4) a GOB (H263), a macro block 
		unsigned r0						: 2;		//	Reserved 
		unsigned ac_coeff_scale			: 7;		//	Quantiser scale in a slice (MPEG2), a packet (MPEG4) a GOB (H263), a macro block 
		unsigned r1						:17;		//	Reserved 
	}mp4regQCINPUT;
	
	typedef struct
	{
		signed qfsf						:12;		//	12-bit IQ level coefficient, 12-bit IDCT coefficient value
		unsigned qfsf_index				: 6;		//	6-bit IQ run coefficient, 6-bit IDCT coefficient index
		unsigned m4_eob_flag			: 1;		//	1: It's end of block for Mpeg4; 0: It's not end of block for Mpeg4
		unsigned r0						: 13;		//  reserved
	}mp4regIQIDCTINPUT;
	
	typedef struct
	{
		unsigned cbp					: 6;		//	Coded block pattern [5:0].
		unsigned ac_pred_flag			: 1;		//	0: no use AC prediction in MPEG4 decoding; 1: use
		unsigned dct_type				: 1;		//	0: frame based DCT; 1: field based DCT
		unsigned mb_back				: 1;		//	0: no use backward motion vectors; 1: use
		unsigned mb_forw				: 1;		//	0: no use forward motion vectors;  1: use
		unsigned mc_gmc					: 1;		//	0: no use global motion compensation; 1: use
		unsigned mc_8x8					: 1;		//	0: no use 8x8 MC (four motion vectors); 1: use
		unsigned mc_dp					: 1;		//	0: no use dual prime motion compensation; 1: use
		unsigned mc_16x8				: 1;		//	0: no use 16x8 motion compensation in field picture; 1: use
		unsigned mc_field				: 1;		//	0: no use field based motion compensation; 1: use
		unsigned mc_frame				: 1;		//	0: no use frame based motion compensation; 1: use
		unsigned mb_intra				: 1;		//	0: non intra macro block; 1: Intra macro block
		unsigned mb_skip				: 1;		//	0: not skip macro block; 1: skip macro block
		unsigned mb_direct				: 1;		//	0: not use direct mode in B-VOP. 1: use
		unsigned mb_copy				: 1;		//	1: This MB is a copy MB when error occurs; 0: This MB is not copy MB.
		unsigned r0						: 12;		//  Reserved
	}mp4regMBH;
	
	typedef struct
	{
		signed mv1						: 13;		//	First motion vector in MB, forward, vertical
		unsigned r0						: 3;		//  Reserved
		signed mv0						: 13;		//	First motion vector in MB, forward, horizontal
		unsigned mv_fs					: 1;		//	0: the top reference field is used to form the prediction; 1: the bottom reference field is used
		unsigned r1						: 2;		//	Reserved
	}mp4regMV1234;
	
	typedef struct
	{
		signed mv1						: 15;		//	Direct mode, derived MV, forward, vertical
		unsigned r0						: 1;		//  Reserved
		signed mv0						: 15;		//	Direct mode, derived MV, forward, horizontal
		unsigned r1						: 1;		//  Reserved
	
	}mp4regMV5678;
	

	
	typedef struct
	{
		unsigned crt_mb_num				: 11;		//  It means how many MBs there are are correct for a picture, packet, GOB and VOP.
		unsigned r0						: 21;		//  Reserved
	}mp4regCRTMBADDR;
	
	typedef struct
	{
		unsigned case0					: 1;		//	Error 0:VLD need data, but last flag is set to 1
		unsigned case1					: 1;		//	Error 1: non slice header is found such as "picture header is found"
		unsigned case2					: 1;		//	Error 2: slice location wrong (MBA >= MBAmax)
		unsigned case3					: 1;		//	Error 3: VLC or run level error
		unsigned case4					: 1;		//	Error 4: context error (e.g. an illegal value is found)
		unsigned case5					: 1;		//	Error 5: coefficient index >= 64
		unsigned r0						: 26;		//	reserved
	}mp4regERRFLAG;
	
	extern mp4regMPHR mp4mphr_reg00;
	extern mp4regMVOPHR mp4mvophr_reg04;
	extern mp4regFSIZE mp4fsize_reg08;
	extern mp4regPICSIZE	mp4picsize_reg0c;
	extern mp4regMBADDR mp4mbaddr_reg10;
	extern mp4regVECTRL mp4vectrl_reg14;
	extern mp4regVETRIGGER mp4vetrigger_reg18;
	extern mp4regVESTAT mp4vestat_reg1c;
	extern mp4regTRBTRDFLD mp4trbtrdfld_reg20;
	extern mp4regTRBTRDFRM mp4trbtrdfrm_reg24;
	extern mp4regVLDBADDR mp4vldbaddr_reg28;
	extern mp4regVLDOFFSET mp4vldoffset_reg2c;
	extern mp4regVLDLEN mp4vldlen_reg30;
	extern mp4regVBVENDADDR mp4vbvsize_reg34;
	extern mp4regMBHADDR	mp4mbhaddr_reg38;
	extern mp4regVLDOFFSET mp4vldoffset_reg38;
	extern mp4regVLDLEN mp4vldlen_reg3c;
	extern mp4regDecBufADDR mp4dcacaddr_reg3c;
	extern mp4regDecBufADDR mp4dblkaddr_reg40;
	extern mp4regDecBufADDR mp4ncfaddr_reg44;
	extern mp4regFRMADDR mp4rec_yframaddr_reg48,mp4rec_cframaddr_reg4c;
	extern mp4regFRMADDR mp4for_yframaddr_reg50,mp4for_cframaddr_reg54;
	extern mp4regFRMADDR mp4back_yframaddr_reg58,mp4back_cframaddr_reg5c;
	extern mp4regSOCX mp4socx_reg60;
	extern mp4regSOCY mp4socy_reg64;
	extern mp4regSO mp4sol_reg68;
	extern mp4regSDX mp4sdlx_reg6c;
	extern mp4regSDY mp4sdly_reg70;
	extern mp4regSSR mp4spriteshifter_reg74;
	extern mp4regSDX mp4sdcx_reg78;
	extern mp4regSDY mp4sdcy_reg7c;
	extern mp4regIQMINPUT mp4iqminput_reg80;
	extern mp4regQCINPUT mp4qcinput_reg84;
	extern mp4regIQIDCTINPUT mp4iqidctinput_reg90;
	extern mp4regMBH mp4mbh_reg94;
	extern mp4regMV1234 mp4mv1_reg98,mp4mv2_reg9c,mp4mv3_rega0,mp4mv4_rega4;
	extern mp4regMV5678 mp4mv5_rega8,mp4mv6_regac,mp4mv7_regb0,mp4mv8_regb4;
	extern mp4regERRFLAG mp4errflag_regc4;
	extern mp4regCRTMBADDR mp4crtmb_regc8;
	
	
	#define SYS_WriteByte(uAddr, bVal) \
	do{*(volatile uint8_t *)(uAddr) = (bVal);}while(0)
	#define SYS_WriteWord(uAddr, wVal) \
	do{*(volatile uint16_t *)(uAddr) = (wVal);}while(0)
	#define SYS_WriteDWord(uAddr, dwVal) \
	do{*(volatile uint32_t *)(uAddr) = (dwVal);}while(0)
	
	#define SYS_ReadByte(uAddr) \
		bVal = (*(volatile uint8_t *)(uAddr));
	
	#define SYS_ReadWord(uAddr) \
		wVal = (*(volatile uint16_t *)(uAddr));
	
	#define SYS_ReadDWord(uAddr) \
		dwVal = (*(volatile uint32_t *)(uAddr));
	
#ifdef __cplusplus
}
#endif

#endif	//_REGISTER_H

