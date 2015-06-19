/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd. 
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 * 
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from 
 * Softwinner Products. 
 *
 * The entire notice above must be reproduced on all copies 
 * and should not be removed. 
 */
 
#ifndef __H264REGISTER_H
#define __H264REGISTER_H


#define H264ENC_PICINFO_REG			        0x00
#define H264ENC_PARA0_REG			        0x04
#define H264ENC_PARA1_REG			        0x08
#define H264ENC_PARA2_REG			        0x0c
#define H264ENC_MEPARA_REG			        0x10
#define H264ENC_INT_ENABLE_REG		        0x14
#define H264ENC_STARTTRIG_REG		        0x18
#define H264ENC_STATUS_REG			        0x1c
#define H264ENC_PUTBITSDATA_REG		        0x20
#define H264ENC_OVERTIMEMB_REG              0x24
#define H264ENC_CYCLICINTRAREFRESH_REG      0x28

#define H264ENC_RC_INIT_REG                 0x2c
#define H264ENC_RC_MAD_TH0_REG              0x30
#define H264ENC_RC_MAD_TH1_REG              0x34
#define H264ENC_RC_MAD_TH2_REG              0x38
#define H264ENC_RC_MAD_TH3_REG              0x3c
#define H264ENC_RC_MAD_SUM_REG              0x40
#define H264ENC_TEMPORAL_FILTER_PAR         0x44
#define H264ENC_DYNAMIC_ME_PAR0             0x48
#define H264ENC_DYNAMIC_ME_PAR1             0x4c

//..RESEVERD
#define H264ENC_MAD_REG	                    0x50
#define H264ENC_TXTBITS_REG                 0x54
#define H264ENC_HDRBITS_REG                 0x58
#define H264ENC_MEINFO_REG                  0x5C
#define H264ENC_MVBUFADDR_REG               0x60
//..RESEVERD
#define H264ENC_STMSTARTADDR_REG            0x80
#define H264ENC_STMENDADDR_REG              0x84
#define H264ENC_STMOST_REG                  0x88
#define H264ENC_STMVSIZE_REG                0x8c
#define H264ENC_STMLEN_REG                  0x90
//..RESEVERD
#define H264ENC_REFADDRY_REG                0xa0
#define H264ENC_REFADDRC_REG		        0xa4
#define H264ENC_REF1ADDRY_REG		        0xa8
#define H264ENC_REF1ADDRC_REG		        0xac
#define H264ENC_RECADDRY_REG		        0xb0
#define H264ENC_RECADDRC_REG		        0xb4
#define H264ENC_SUBPIXADDRLAST_REG	        0xb8
#define H264ENC_SUBPIXADDRNEW_REG           0xbc
#define H264ENC_MBINFO_REG			        0xc0
#define H264ENC_DBLKADDR_REG		        0xc4
#define H264ENC_TFCNT_ADDR_REG              0xc8

#define H264ENC_ROI_QP_OFFSET_REG           0xcc
#define H264ENC_ROI_0_AREA_REG		        0xd0
#define H264ENC_ROI_1_AREA_REG		        0xd4
#define H264ENC_ROI_2_AREA_REG		        0xd8
#define H264ENC_ROI_3_AREA_REG		        0xdc
//..RESEVERD                                
#define SRAM_PORT_RW_OFFSET			        0xe0
#define SRAM_PORT_RW_DATA                   0xe4

#define MACC_AVC_ENC_QUR_SLICEEND		(1<<0)
#define MACC_AVC_ENC_QUR_BSSTALL		(1<<1)
#define MACC_AVC_ENC_QUR_MBOVTIME		(1<<2)
#define MACC_AVC_ENC_QUR_OVTIME			(1<<3)

#define MACC_AVC_ENC_SLICEEND			 0// (1<<0)
#define MACC_AVC_ENC_BSSTALL			 1// (1<<1)
#define MACC_AVC_ENC_MBOVTIME			 2// (1<<2)
#define MACC_AVC_ENC_OVTIME				 3// (1<<3)

#define MB_OVERTIME_IE				(1<<2)
#define BS_STALL_IE					(1<<1)
#define VE_FINISH_IE				(1<<0)

#define PUTBITS_SATUS				(1<<9)
#define SYNC_IDLE					(1<<8)

#define OVERTIME_I					(1<<3)
#define MB_OVERTIME_I				(1<<2)
#define BS_STALL_I					(1<<1)
#define VE_FINISH_I					(1<<0)

#define PUT_BITS					1
#define EMPTY_BITS_BUFFER			2
#define PUT_VLCSE					4
#define PUT_VLCUE					5
#define VE_START					8
#define VE_CONTINUE					9

typedef struct
{
	unsigned picHeighthInMbs		: 16;	//lsb
	unsigned picWidthInMbs			: 16;	//msb
}regPicInfo;

typedef struct
{
	unsigned picType				: 2;	//lsb	0:frame 1:Top field 2:Bottom Field
	unsigned refPicType				: 2;    // |	0:frame 1:Top field 2:Bottom Field
	unsigned sliceType				: 3;	// | 	0:I-Slice 1:P-Slice 2:P_Skiped_Slice 3:B_Slice
	unsigned r0						: 1;    // |	Reserved
	unsigned entropyType			: 1;	// | 	0:CAVLC 1:CABAC
	unsigned ctxInitMethod			: 1;    // |	0:fixed 
	unsigned fixModeNum				: 2;	// | 	0, 1, 2
	unsigned deBlockIdc				: 2;    // |	0:enable 1:disable 2:disable filter in sliceboundary
	unsigned r1						: 1;	// |	Reserved
	unsigned idr_pic_id   			: 1;  	// use ifor h264 encoder slice header
	unsigned alphaOffsetDiv2		: 4;	// |	0~6:0~6 9~15:-6~-1
	unsigned betaOffsetDiv2			: 4;	// |	0~6:0~6 9~15:-6~-1
	unsigned frame_num				: 4;	// 1 
	unsigned ref_pic_reorder_flag	: 1;	// |	1:do refer ref pic reordering   
	unsigned h264_slice_enable		: 1;	// |    0:h264 frame mode 1: for slice mode
	unsigned stuff_zero_enable		: 1;	// |    0 disable insert 0 after 0xff
	unsigned eptbDisable			: 1;	//msb 	0: insert EPTB
}regPara0;

#if 0
typedef struct
{
	unsigned dcLuma					: 11;	//lsb	Jpeg Emcoder Luma Default DC
	unsigned r0						: 5;    // |	Reserved
	unsigned dcChroma				: 11;	// | 	Jpeg Encoder Chroma Default DC
	unsigned r1						: 3;    // |	Reserved
	unsigned stuffZeroEnable		: 1;	// |	enable stuffing zero after 0xff
	unsigned r2						: 1;	//msb 	Reserved
}regJpegPara;

#else

typedef struct
{
	unsigned dcLuma					: 11;	//lsb	Jpeg Emcoder Luma Default DC
	unsigned r0						: 5;    // |	Reserved
	unsigned dcChroma				: 11;	// | 	Jpeg Encoder Chroma Default DC
	unsigned r1						: 3;    // |	Reserved
	unsigned stuffZeroEnable		: 1;	// |	enable stuffing zero after 0xff
	unsigned EPTB_disable			: 1;	//msb 	Reserved
}regJpegPara;

#endif

//0x08
typedef struct
{
	unsigned class_thres			: 8;    //lsb	classfy threshold
	unsigned run_length_th			: 4;	// | 	run length threshold of the optimize
	unsigned r0						: 18;    // |	Reserved
	unsigned coef_down_disenable	: 1;	// |	0:dct coef down sample; 1:disable coef down sample
	unsigned run_len_opt_disenable	: 1;	// |	0:optimize run length size; 1:disable optimize run length size
}regJpegPara1;


typedef struct
{
	unsigned fixedQP				: 6;    //lsb	0~51 
	unsigned rcMode					: 2;	// | 	0:Fixed
	unsigned fixedIntraQp			: 6;    // |	0~51 fixed QP of intra MB in P Slice
	unsigned r0						: 2;	// |	Reserved
	unsigned qpcOffset0				: 3;	// |	first chrome QP offset
	unsigned r1						: 13;	//msb	Reserved
}regPara1_version0;

typedef struct
{
	unsigned fixedQP				: 6;    //lsb	0~51 
	unsigned rcMode					: 2;	// | 	0:Fixed
	unsigned dynamicMeEn			: 1;    // |	0:disable	1:enable(only width > 12mbs valid)
	unsigned tile32OutputEn			: 1;    // |	0:output compress frame buffer	1:output tile32 rec frame buffer
	unsigned lenStride				: 4;    // |	mb_width div 48
	unsigned r0						: 2;	// |	Reserved
	unsigned qpcOffset0				: 3;	// |	first chrome QP offset

	//for 1681
	unsigned temFilterHisOutDisable	: 1;	// |	0:enable	1:disable
	unsigned r1						: 4;	// |	Reserved
	unsigned ClipMVPar				: 2;	// |	0:Coarse delta mv clip(-2,2)according to left intefer nv, 1:(-4,4), 2:(-6,6), 3:(-8,8)
	unsigned ClipMVEn				: 1;	// |	0:enable	1:disable
	unsigned r2						: 1;	// |	Reserved
	unsigned ModeOptimizePar		: 3;	// |	0:intra cost + (intra cost>>4), 1:intra cost + (intra cost>>3), 2:intra cost + (intra cost>>2), 3:intra cost + ((intra cost*3)>>3)
	unsigned ModeOptimizeEn			: 1;	// |    0:disable	1:enable
}regPara1_version1;

typedef struct
{
	unsigned mvScale				: 8;    //lsb	mv scale use for B frame
	unsigned slice_height           : 8;    //slice height in mb
	unsigned pic_order_cnt_lsb      : 8;    //pic order counter
	unsigned log2_maxpicorder_lsb   : 3;    //pic order cnd 0~4
	unsigned nal_ref_idc            : 2;         //used in slice header encoder
	unsigned bDirectDisable			: 1;	// |	1:Disable
	unsigned bFrameWbEn				: 1;	// |	1:Enable
	unsigned fRefIsISlice			: 1;	//msb	1:Forward Reference frame is I_Slice 1
}regPara2;

typedef struct
{
	unsigned intraPredDisable		: 1;	//lsb	1:Disable
	unsigned fmeSearchLevel			: 2;    // |	invalid bit in aw1633
	unsigned qsmtssDisable			: 1;	// | 	1:Disable 3²½ËÑË÷
	unsigned qsmtssRefineDisable	: 1;    // |	1:for less band width
	unsigned imeTimePrio			: 1;	// | 	0:time priority 1:band width priority
	unsigned coarseCostCtrl			: 2;    // |
	unsigned meDisable				: 1;	// |    invalid bit in aw1633
	unsigned skipmdoptDisable		: 1;    // |    0:enable the skip mb  1: disable the skip mb 
	unsigned r0						: 1;	// | 	invalid bit in aw1633
	unsigned qpix_16x16_off			: 1;	// |	1:enable qpix 16x16 off
	unsigned quantThresholdDisable	: 1;	// |	1:Disable
	unsigned intraPlanDisable		: 1;	// |	1:Disable
	unsigned intra4x4Disable		: 1;	// |	1:Disable
	unsigned splitMbDisable			: 1;	// |	1:Disable
	unsigned wbMVInfoDisable		: 1;	// |	1:Disable
	unsigned roiEnable				: 1;	// |	1:Enable
	unsigned deblk_to_dram          : 1;    // |    0:for deblock line buffer write to dram when width more zhan 2048, 1 for write to dram
	unsigned tpix_to_dram           : 1;    // |    0:for tpix write to dram when width more than 2048 1 for write to dram
	unsigned qpix_smart_off			: 1;	// |    1:enable qpix_smart off
	unsigned qpix_split_off			: 1;	// |    1:enable qpix_split_off
	unsigned r2						: 10;	//msb 	Reserved
}regMePara;

typedef struct
{
	unsigned pic_var               : 8; // the picture's var which is calculate by last frame
	unsigned maxcoef               : 7; // the threshold for coef
	unsigned r0                    : 1; // Reserved
	unsigned picvar_maxcoef        : 14;// = picvar*maxcoef used for coef's calc
	unsigned fixFilterEn           : 1; //3D filter use the fix picvar which is from isp
	unsigned temp_filter_enable    : 1; //0 disbale filter, 1 enable filter
}regTemporalPara;

typedef struct
{
	unsigned startType				: 4;	//lsb	
	unsigned r0						: 4;    // |	Reserved
	unsigned nBits					: 6;	// |	The bits number to put
	unsigned r1						: 2;    // |	Reserved
	unsigned encodeMode				: 2;	// | 	0:h264 1:jpeg 2:vp8
	unsigned r2						: 14;	//msb 	Reserved
}regStartTrig;

typedef struct intra_refresh_reg
{
	unsigned	col_end_mb       : 8;   //for column end mb
	unsigned    r0               : 8;   //for reserved
	unsigned	col_start_mb	 : 8;   //for column start mb
	unsigned	r1				 : 7;	//for reserved
	unsigned	refresh_enable	 : 1;   //for enable the intra refresh
}intra_refresh_reg;

typedef struct dynamic_me_par0{
	unsigned dynamic_me_th0			:10;	//for move step=1 threshold
	unsigned r0						:6;		//reserved
	unsigned dynamic_me_th1			:10;	//for move step=2 threshold
	unsigned r1						:6;		//reserved
}dynamic_me_par0;

typedef struct dynamic_me_par1{
	unsigned dynamic_me_th2			:10;	//for move step=3 threshold
	unsigned r0						:6;		//reserved
	unsigned dynamic_me_th3			:10;	//for move step=4 threshold
	unsigned r1						:6;		//reserved
}dynamic_me_par1;


typedef struct roi_area{
	unsigned roi_x_start			:8;
	unsigned roi_x_end				:8;
	unsigned roi_y_start			:8;
	unsigned roi_y_end				:8;
}roi_area;


typedef struct roi_qp_offset{
	unsigned qp_offset_0			:6;
	unsigned r0						:2;
	unsigned qp_offset_1			:6;
	unsigned r1						:2;
	unsigned qp_offset_2			:6;
	unsigned r2						:2;
	unsigned qp_offset_3			:6;
	unsigned r3						:2;
}roi_qp_offset;


#define readVeReg(n)		(*((volatile unsigned int *)(n)))          /* word input */
#define writeVeReg(n,c)		(*((volatile unsigned int *)(n)) = (unsigned int)(c))    /* word output */

#endif //__H264REGISTER_H

