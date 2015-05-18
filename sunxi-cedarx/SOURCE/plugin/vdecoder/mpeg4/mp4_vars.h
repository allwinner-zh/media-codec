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
#ifndef MP4_VARS_H
#define MP4_VARS_H

#include "mpeg4_config.h"
#include "mp4_register.h"

#ifdef __cplusplus
extern "C" {
#endif

	#define mmax(a, b)      ((a) > (b) ? (a) : (b))
	#define mmin(a, b)      ((a) < (b) ? (a) : (b))
	#define mnint(a)        ((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))
	#define sign(a)         ((a) < 0 ? -1 : 1)
	#define abs(a)          ((a)>0 ? (a) : -(a))
	#define sign(a)         ((a) < 0 ? -1 : 1)
	#define mnint(a)        ((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))
	#define _div_div(a, b)  (a>0) ? (a+(b>>1))/b : (a-(b>>1))/b
		
	// b frames
	#define MODB_1          0
	#define MODB_00         1
	#define MODB_01         2
	
	
	#define MB_TYPE_1       0 // direct mode
	#define MB_TYPE_01      1 // bi-directional mode
	#define MB_TYPE_001     2 // backward mode
	#define MB_TYPE_0001    3 // forward mode
	#define MB_TYPE_00001   4 //intra mode
	
	#define DIRECT_MODE     MB_TYPE_1
	#define BIDIR_MODE      MB_TYPE_01
	#define BACKWARD_MODE   MB_TYPE_001
	#define FORWARD_MODE    MB_TYPE_0001
	#define INTRA_MODE      MB_TYPE_00001
	
	#define NOT_VALID       (-2)
	#define NOT_CODED       (-1)
	#define INTER           0
	#define INTER_Q         1
	#define INTER4V         2
	#define INTRA           3
	#define INTRA_Q         4
	#define STUFFING        7
	#define B_MBLOCK        8    // used as a shortcut (overload) in mp4_recon_qpel
	#define B_MBLOCK_DEST   9    // in this case, the destination is an array (mb)
	#define B_BLOCK_DEST_4V 10
	
	#define RESYNC_MARKER   1
	
	#define RECTANGULAR             0
	#define BINARY                  1
	#define BINARY_SHAPE_ONLY       2
	#define GRAY_SCALE              3
	
	#define NOTUSE_SPRITE           0
	#define STATIC_SPRITE           1
	#define GMC_SPRITE              2
	#define RESERVED_SPRITE         3
	
	#define LEFT 2
	#define TOP 1
	
	typedef enum
	{
	    MP4_I_VOP=0,
	    MP4_P_VOP,
	    MP4_B_VOP,
	    MP4_S_VOP
	}mp4_picture_type;
	
	#define VO_START_CODE           0x8
	#define VO_START_CODE_MIN       0x100
	#define VO_START_CODE_MAX       0x11f
	
	#define VOL_START_CODE          0x12
	#define VOL_START_CODE_MIN      0x120
	#define VOL_START_CODE_MAX      0x12f
	
	#define VOS_START_CODE          0x1b0
	#define USR_START_CODE          0x1b2
	#define GOP_START_CODE          0x1b3
	#define VSO_START_CODE          0x1b5
	#define VOP_START_CODE          0x1b6
	#define STF_START_CODE          0x1c3   // stuffing_start_code
	#define SHV_START_CODE          0x020
	#define SHV_END_MARKER          0x03f
	
	typedef enum
	{
	    FORBIDDEN   = 0,
	    SQCIF       = 1,
	    QCIF        = 2,
	    CIF         = 3,
	    fCIF        = 4,
	    ssCIF       = 5,
	    CUSTOM      = 6,
	    EPTYPE      = 7
	}FRAMESIZE;

	typedef enum {
	    CODEC_ID_RV10,
	    CODEC_ID_RV20,
	    CODEC_ID_RV30,
	    CODEC_ID_RV40
	}RVCodecID;

	/* Frame type enum */
	typedef enum
	{
	    RVFrameTypePartial,
	    RVFrameTypeWhole,
	    RVFrameTypeLastPartial,
	    RVFrameTypeMultiple
	} RVFrameType;

	typedef struct
	{
	    int32_t val;
	    int32_t len;
	}tab_type;

	typedef struct
	{
    	int32_t   last;
    	int32_t   run;
    	int32_t   level;
	}event_t;

	typedef struct
	{
    	int32_t x;
    	int32_t y;
	}MotionVector;

	typedef MotionVector (*MVtype)[6];

	typedef struct
	{
    	int32_t   iLeft;
    	int32_t   iAbove;
	}T_Edge;

	typedef struct
	{
    	uint32_t length;
    	uint32_t count;
    	uint8_t*	startptr;
    	uint8_t*	buf_start_ptr;
    	uint8_t*	buf_end_ptr;
    	uint8_t*	rdptr;

    	uint32_t bitcnt;
    	uint32_t bit_a;
    	uint32_t bit_b;
	}MP4_STREAM;

	#define DEC_MBC        120
	#define DEC_MBR        80

	typedef struct
	{
    	uint8_t corners[4];
    	uint8_t top[DEC_MBC];
    	uint8_t bottom[DEC_MBC];
    	uint8_t left[DEC_MBR];
    	uint8_t right[DEC_MBR];
	}mp4_edge_info;

	struct _MP4_STATE_;

	typedef struct _mp4_header
	{
    	// video packet record
    	uint32_t   intra_quant_matrix[64];
    	uint32_t   nonintra_quant_matrix[64];
    	// vol
    	int32_t   ident;
    	int32_t   random_accessible_vol;
    	int32_t   type_indication;
    	int32_t   is_object_layer_identifier;
    	int32_t   visual_object_layer_verid;
    	int32_t   visual_object_layer_priority;
    	int32_t   aspect_ratio_info;
    	int32_t   par_width;
    	int32_t   par_height;
    	int32_t   vol_control_parameters;
    	int32_t   chroma_format;
    	int32_t   low_delay;
    	int32_t   vbv_parameters;
    	int32_t   first_half_bit_rate;
    	int32_t   latter_half_bit_rate;
    	int32_t   first_half_vbv_buffer_size;
    	int32_t   latter_half_vbv_buffer_size;
    	int32_t   first_half_vbv_occupancy;
    	int32_t   latter_half_vbv_occupancy;
    	int32_t   shape;
    	int32_t   video_object_layer_shape_extension;
    	int32_t   time_increment_resolution;
    	int32_t   test_timeinc;
    	int32_t   fixed_vop_rate;
    	int32_t   fixed_vop_time_increment;
    	uint16_t   width;    //come from parser, so may be not mb aligned
    	uint16_t   height;
    	int32_t   interlaced;
    	int32_t   obmc_disable;
    	int32_t   sprite_usage;
    	int32_t   sprite_width;
    	int32_t   sprite_height;
    	int32_t   sprite_left_coordinate;
    	int32_t   sprite_top_coordinate;

    	int32_t   sadct_disable;
    	int32_t   not_8_bit;
    	int32_t   quant_precision;
    	int32_t   bits_per_pixel;
    	int32_t   quant_type;
    	int32_t   load_intra_quant_matrix;
    	int32_t   load_nonintra_quant_matrix;
    	int32_t   quarter_pixel;
    	int32_t   complexity_estimation_disable;
    	int32_t   resync_marker_disable;
    	int32_t   data_partitioning;
    	int32_t   reversible_vlc;
    	int32_t   intra_acdc_pred_disable;
    	int32_t   scalability;
    	int32_t   quant_scale;
    	// complexity estimation
    	int32_t   estimation_method;
    	int32_t   shape_complexity_estimation_disable;
    	int32_t   opaque;
    	int32_t   transparent;
    	int32_t   intra_cae;
    	int32_t   inter_cae;
    	int32_t   no_update;
    	int32_t   upsampling;
    	int32_t   texture_complexity_estimation_set_1_disable;
    	int32_t   intra_blocks;
    	int32_t   inter_blocks;
    	int32_t   inter4v_blocks;
    	int32_t   not_coded_blocks;
    	int32_t   texture_complexity_estimation_set_2_disable;
    	int32_t   dct_coefs;
    	int32_t   dct_lines;
    	int32_t   vlc_symbols;
    	int32_t   vlc_bits;
    	int32_t   motion_compensation_complexity_disable;
    	int32_t   apm;
    	int32_t   npm;
    	int32_t   interpolate_mc_q;
    	int32_t   forw_back_mc_q;
    	int32_t   halfpel2;
    	int32_t   halfpel4;
    	int32_t   version2_complexity_estimation_disable;
    	int32_t   sadct;
    	int32_t   quarterpel;
    	int32_t   newpred_enable;
    	int32_t   request_upstream_message_type;
    	int32_t   newpred_segment_type;
    	int32_t   reduced_resolution_vop_enable;
    	int32_t   dcecs_opaque;
    	int32_t   dcecs_transparent;
    	int32_t   dcecs_intra_cae;
    	int32_t   dcecs_inter_cae;
    	int32_t   dcecs_no_update;
    	int32_t   dcecs_upsampling;
    	int32_t   dcecs_intra_blocks;
    	int32_t   dcecs_not_coded_blocks;
    	int32_t   dcecs_dct_coefs;
    	int32_t   dcecs_dct_lines;
    	int32_t   dcecs_vlc_symbols;
    	int32_t   dcecs_vlc_bits;
    	int32_t   dcecs_sadct;
    	int32_t   dcecs_inter_blocks;
    	int32_t   dcecs_inter4v_blocks;
    	int32_t   dcecs_apm;
    	int32_t   dcecs_npm;
    	int32_t   dcecs_forw_back_mc_q;
    	int32_t   dcecs_halfpel2;
    	int32_t   dcecs_halfpel4;
    	int32_t   dcecs_quarterpel;
    	int32_t   dcecs_interpolate_mc_q;

    	// svh
    	int32_t   short_video_header;
    	int32_t   temporal_reference;
    	int32_t   split_screen_indicator;
    	int32_t   document_camera_indicator;
    	int32_t   full_picture_freeze_release;
    	int32_t   source_format;
    	int32_t   picture_coding_type;
    	int32_t   four_reserved_zero_bits;
    	int32_t   vop_quant;
    	
    	// gob
    	int32_t   gob_number;
    	int32_t   gob_frame_id;
    	int32_t   mb_boundary;
    	
    	// gop
    	int32_t   time_code;
    	int32_t   closed_gov;
    	int32_t   broken_link;
    	
    	// vop
    	int32_t   preceding_vop_coding_type;
    	int32_t   old_prediction_type;
    	int32_t   last_coded_prediction_type;
    	int32_t   prediction_type;
    	int32_t   old_time_base;
    	int32_t   time_base;
    	int32_t   time_inc;
    	int32_t   vop_coded;
    	int32_t   rounding_type;
    	int32_t   hor_spat_ref;
    	int32_t   ver_spat_ref;
    	int32_t   change_CR_disable;
    	int32_t   constant_alpha;
    	int32_t   constant_alpha_value;
    	int32_t   intra_dc_vlc_thr;
    	int32_t   use_intra_dc_vlc;
    	int32_t   quantizer;
    	int32_t   fcode_for;
    	int32_t   fcode_back;
    	int32_t   shape_coding_type;
    	int32_t   trb; // temporal diff. between next and previous reference VOP
    	int32_t   trd; // temporal diff. between current B-VOP and previous reference VOP
    	int32_t   trbi;
    	int32_t   trdi;
    	int32_t   display_time_next;
    	int32_t   display_time_prev;
    	int32_t   tframe; // see paragraph 7.7.2.2
    	
    	// video packet
    	int32_t   header_extension_code;
    	
    	// macroblock
    	int32_t   not_coded;
    	int32_t   derived_mb_type;
    	int32_t   cbpy;
    	int32_t   cbpc;
    	int32_t   cbpb;
    	int32_t   cbp;
    	int32_t   ac_pred_flag;
    	int32_t   b_mb_not_coded;
    	int32_t   modb;
    	int32_t   mb_type;
    	int32_t   dquant;
    	int32_t   dbquant;
    	int32_t   mp4_direct_mv[8][2];    // storing direct motion vectors
    	int8_t    macroblok_bounday;
    	
    	// extra/derived
    	int32_t   mba_size;
    	int16_t   mb_xsize;
    	int16_t   mb_ysize;
    	int32_t   picnum;
    	int32_t   packetnum;  // if ~0 indicates the presece of at least 1 video packet
    	int32_t   gobnum;     // needed for short header decoding
    	int32_t   mba;
    	int32_t   mb_xpos;
    	int32_t   mb_ypos;
    	int32_t   num_mb_in_gob;      // shv
    	int32_t   num_gobs_in_vop;    // shv
    	
    	int32_t   mb_in_vop_length;
    	int32_t   resync_length;
    	int32_t   intrablock_rescaled; // indicates, when the quantizer changes inside a VOP, that
    	
		int32_t   iEffectiveWarpingPoints;
    	int32_t   no_of_sprite_warping_points;
    	int32_t   warping_points[4][2];
    	int32_t   sprite_warping_accuracy;
    	int32_t   sprite_brightness_change;
    	int32_t   sprite_brightness_change_factor;
    	int32_t   low_latency_sprite_enable;
    	int32_t   gmc_lum_mv_x, gmc_lum_mv_y;
    	int32_t   gmc_chrom_mv_x, gmc_chrom_mv_y;

    	// 3.11 specific values
    	int16_t   (*dc_chrom_table)       (struct _MP4_STATE_*,MP4_STREAM*);
    	int16_t   (*dc_lum_table)         (struct _MP4_STATE_*,MP4_STREAM*);
    	event_t (*ac_inter_table)       (struct _MP4_STATE_*,MP4_STREAM*);
    	event_t (*ac_intra_chrom_table) (struct _MP4_STATE_*,MP4_STREAM*);
    	event_t (*ac_intra_lum_table)   (struct _MP4_STATE_*,MP4_STREAM*);
    	void    (*mv_table)             (MP4_STREAM*, int32_t*, int32_t*);
    	int16_t   (*get_cbp)              (MP4_STREAM*);
    	int32_t   has_skips;
    	int32_t   vol_mode; /* see comment in mp4_header_311.c */
    	int32_t   switch_rounding;
    	    // the ac rescaling have been applied to avoid to repeat it
    	
    	//interlace
    	int32_t   top_field_first;
    	int32_t   dct_type;
    	int32_t   field_prediction;
    	int32_t   forward_top_field_reference;
    	int32_t   forward_bottom_field_reference;
    	int32_t   backward_top_field_reference;
    	int32_t   backward_bottom_field_reference;
    	int32_t   alternate_vertical_scan_flag;
    	
    	//h263
    	int32_t   h263_aic;
    	int32_t   h263_aic_bak;
    	int32_t   h263_aic_dir;
    	int32_t   bSliceStructured;
    	int32_t   iModifiedQantization;
    	int32_t   UMV;
    	int32_t   UUI;
    	int32_t   RPR;
    	int32_t   RRU;
    	int32_t   h263_ap;
    	int32_t   deblockingflag;
    	int32_t   flag_keyframe;  // indicates that the current frame is a keyframe
    	int32_t   flag_disposable;
    	int32_t   RPS;
    	int32_t   ISD;
    	int32_t   AIV;
    	int32_t   PB;
    	T_Edge  GobEdge;
    	int32_t   OSVQUANT;
    	int32_t   iRatio0,iRatio1;
    	int32_t   quant_prev;
    	int32_t   EntropyQP;
	}mp4_header;

	typedef struct
	{
	    int32_t   X0, Y0;
	    int32_t   XX, YX, XY, YY;
	    int32_t   rounder1, rounder2;
	    int32_t   shifter;
	} AFFINE_TRANSFORM;

	typedef event_t     (VldProc)(MP4_STREAM * ld);
	typedef VldProc     *VldProcPtr;

	typedef struct
	{
	    int16_t eBlkType;       /** Macroblock type */
	    int16_t DC;             /** Quantized DC for prediction */
	    int16_t ACTop[7];       /** Quantized AC top row for prediction */
	    int16_t ACLeft[7];      /** Quantized AC left column for prediction */
	} mp4_sBlk;

	typedef struct
	{
		mp4_sBlk        sBlk[6];    /** Block level information */
	}mp4_sMB;

	typedef struct _MP4_STATE_
		{
	    MP4_STREAM  		bits;
	    int32_t       			valid_data_offset;
	    int32_t       			packet_length;
	                    	
	    mp4_header  		hdr;
	    uint32_t       			frame_ctr;
	    uint32_t       			not_coded_frm_cnt;  	// not coded frame count before current frame
	    uint32_t       			chunk_extra_frm_cnt;  	// not coded frame count before current frame
	    uint32_t       			newfrmrate;         	// new frame rate

	    uint32_t         		last_dc[3];				//MSMPEGV1V2
	                    	
	    int32_t*				codedmap;          		// codedmap[DEC_MBR][DEC_MBC]
	    int16_t*				cbp_store;         		// cbp_store[DEC_MBR+1][DEC_MBC+1]
	    MotionVector 		(*MV)[6];          		// MV[DEC_MBR+1][DEC_MBC+2][6][2]
	    MotionVector 		(*MVBack)[6];      		// MV[DEC_MBR+1][DEC_MBC+2][6][2]
	                    	
	    int32_t*				fieldpredictedmap;
	    int32_t     			fieldpredictedmap_stride;
	    uint8_t*					fieldrefmap;
	    int32_t     			fieldrefmap_stride;
	    MotionVector 		(*MV_field)[6];
	                    	
	    int32_t   				modemap_stride;
	    int32_t   				codedmap_stride;
	    int32_t   				cbp_store_stride;
	    int32_t   				quant_store_stride;
	    int32_t   				MV_stride;
	                    	
	    // b-vop        	
	    MotionVector 		MV_pfor[2];    // top and bottom field
	    MotionVector 		MV_pback[2];
	    int32_t   				prefixed; // avoid delay caused by B-VOPs using prefixed information, no_delay_frame_flag is on
	    int32_t   				history_prefixed;   // there was a prefixed I-VOP, b_vop_grouping is on
	    int32_t   				preceding_vop_coding_type;
	                    	
	    int32_t     			flag_keyframe;      // indicates that the current frame is a keyframe
	    int32_t     			flag_disposable;
	    int32_t     			deblockingflag;
	                    	
	    mp4_edge_info 		edge_info;
	                    	
	    int16_t   				width;              //self-converted, so it should be mb aligned
	    int16_t   				height;
	
	    // user data info
	    int32_t   				PacketFormat;
	    int32_t   				userdata_codec_version;
	    int32_t   				userdata_build_number;
	    int32_t   				isH263;
	    int16_t   				packet_num;
	    int32_t   				FormatPlus;
	    uint32_t   				ulSPOExtra;
	    uint32_t   				ulStreamVersion;
	    uint32_t   				ulMajorStreamVersion;
	    uint32_t   				ulMinorStreamVersion;
	    uint32_t   				ulNumResampledImageSizes;
	    uint32_t   				ulEncodeSize;
	    uint32_t   				ulLargestPels;
	    uint32_t   				ulLargestLines;
	    int32_t   				Num_RPR_Sizes;
	    int32_t   				multi_frame;
	    int32_t*				data_offset;
	    int32_t   				fid;
	    //double 				m_picture_clock_frequency;
	    int32_t   				m_uTRWrap;
	    int32_t   				m_pctszSize;
	    int32_t   				multi_frame_data;
	                    	
	    int32_t   				stride;
	
	    AFFINE_TRANSFORM 	at_lum;
	    AFFINE_TRANSFORM 	at_chrom;
	
	    uint32_t 				mbh_buf;
	    uint32_t 				fdc_qac_buf;
	    uint32_t 				nc_flag_buf;
	    uint32_t					deblk_buf;
	
	    int32_t   				sw_vld;
		int32_t 				enLumaHistogram;
	    VldProcPtr 			vld_inter_fun;
	    VldProcPtr 			vld_intra_fun;

	    int32_t 				mb_width;
	    int32_t					mb_height;
	   
		/* hardware support */
	}MP4_STATE;

#ifdef __cplusplus
}
#endif

#endif

