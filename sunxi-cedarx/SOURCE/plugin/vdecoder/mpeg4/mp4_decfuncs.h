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
#ifndef _MP4_DECFUNCS_H_
#define _MP4_DECFUNCS_H_


#include "mpeg4.h"


#ifdef __cplusplus
extern "C" {
#endif


extern uint32_t msk[33];
extern uint32_t intra_quant_matrix[64];
extern uint32_t nonintra_quant_matrix[64];

extern int32_t mp4_getCBPY(MP4_STREAM * _ld, int32_t intraFlag);
extern int32_t mp4_setMV_263B(MP4_STREAM * _ld, MP4_STATE * _mp4_state, int32_t mb_xpos, int32_t mb_ypos, int32_t mode);
extern void mp4_set_mb_info(MP4_STATE * _mp4_state);
extern int32_t mp4_blockIntra(MP4_STREAM *ld, MP4_STATE *mp4_state, int32_t block_num, int32_t coded);
extern int32_t mp4_blockInter(MP4_STREAM *ld, MP4_STATE *mp4_state);
extern int mp4_check_iqis_in_empty(void);
extern void VERegWriteD(uint32_t offset, uint32_t val);

extern int32_t mp4_getMB_h263B_TYPE(MP4_STREAM * ld, int8_t *stuffing);
extern void mp4_init_global_variable(void);
extern int32_t mp4_decode_frame_h263(mp4_dec_ctx_t* ctx,   int32_t bDecodeKeyFrameOnly, int32_t skip_bframe, int64_t cur_time);
extern int32_t mp4_decode_frame_xvid(mp4_dec_ctx_t* ctx, int32_t bDecodeKeyFrameOnly, int32_t skip_bframe, int64_t cur_time);
extern int16_t mp4_get_use_intra_dc_vlc(int32_t quantizer, int32_t intra_dc_vlc_thr);;
extern int32_t mp4_macroblock_h263_bvop(MP4_STATE *mp4_state);
extern void mp4_reset_sw_bits_status(MP4_STREAM *ld, uint32_t end_bit_pos);
extern event_t mp4_vld_intra_aic_dct(MP4_STREAM * _ld) ;
extern event_t mp4_vld_inter_mq_dct(MP4_STREAM * _ld) ;
extern event_t mp4_vld_inter_dct(MP4_STREAM * _ld) ;
extern void mp4_config_interrupt_register(void);
extern  int8_t   mp4_CheckVEBusy(void);
extern  void mp4_reset_ve_core(mp4_dec_ctx_t* mp4_context);
extern  void mp4_set_pic_size(MP4_STATE * s);
extern  void mp4_set_buffer(MP4_STATE * s);
extern  int16_t mp4_set_vop_info(mp4_dec_ctx_t* ctx);
extern  void mp4_set_quant_matrix(MP4_STATE * _mp4_state, int32_t int32_tra_flag);
extern  void mp4_set_vbv_info(int32_t mb_num_in_gob, int32_t boundary, int32_t start_bit_pos, int32_t bit_length, uint32_t vbv_buf_size);
extern  void mp4_set_packet_info(MP4_STATE * _mp4_state);
extern  int8_t   mp4_check_finish_flag(void);
extern  uint32_t  mp4_get_bitoffset(void);
extern  void mp4_set_mb_info(MP4_STATE * _mp4_state);
extern  int8_t   mp4_check_mc_free(void);
extern  int32_t  mp4_check_idct_in_empty(void);
extern  int32_t  mp4_check_iqis_in_empty(void);
extern  void VERegWriteD(uint32_t offset,uint32_t val);
extern  uint32_t  mp4_get_mba(void);
extern  void mp4_get_pic_size(VideoPicture* picture, mp4_dec_ctx_t* ctx);
extern int32_t  log2ceil(int32_t arg);
            
extern int16_t  mp4_getvoshdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getvsohdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getvolhdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getshvhdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_get_h263_pic_hdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getgophdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getvophdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  mp4_getpackethdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern void mp4_getusrhdr(MP4_STREAM * ld, MP4_STATE * mp4_state);

extern int32_t  mp4_nextbits(MP4_STREAM * _ld, int32_t nbits);
extern int16_t  mp4_bytealign(MP4_STREAM * _ld);
extern int16_t  mp4_bytealigned(MP4_STREAM * _ld, int32_t nbits);
extern void mp4_next_start_code(MP4_STREAM * _ld);
extern void mp4_next_resync_marker(MP4_STREAM * ld);
extern int32_t  mp4_nextbits_bytealigned(MP4_STREAM * _ld, int32_t nbit, int32_t int16_t_video_header, int32_t *skip_cnt);
extern int16_t  mp4_nextbits_resync_marker(MP4_STREAM * ld, MP4_STATE * mp4_state);
extern int16_t  mp4_get_resync_marker(MP4_STREAM * _ld, MP4_STATE * _mp4_state);
extern int16_t  check_sync_marker(MP4_STREAM* ld);
extern int8_t mp4_check_VE_free(void);

extern void mp4_initbits (MP4_STREAM * _ld, uint8_t *stream, int32_t length,uint8_t *bufstartptr,uint8_t *bufendptr);
extern void mp4_ve_mode_select(uint8_t mode);
extern int16_t mp4_set_vop_info(mp4_dec_ctx_t* ctx);

extern void mp4_initbits (MP4_STREAM * _ld, uint8_t *stream, int32_t length, uint8_t* startptr, uint8_t* endptr);
extern void mp4_flushbits (MP4_STREAM * ld, int32_t n);
extern uint32_t  mp4_showbits (MP4_STREAM * ld, int32_t n);
extern int32_t  mp4_showbits1 (MP4_STREAM * ld);
extern uint32_t  mp4_bitpos(MP4_STREAM * ld);
extern uint32_t  mp4_getbits(MP4_STREAM * ld, int32_t n);
extern uint32_t  mp4_getbits1(MP4_STREAM * ld);
extern int16_t  mp4_bytealign(MP4_STREAM * _ld);
extern int32_t  mp4_nextbits(MP4_STREAM * _ld, int32_t nbit);
extern int16_t  mp4_bytealigned(MP4_STREAM * _ld, int32_t nbit);
extern uint32_t  mp4_getLeftData(MP4_STREAM * _ld);
extern void mp4_setld_offset(MP4_STREAM * _ld);
extern void mp4_clear_status_reg(void);
extern int32_t mp4_update_framerate(mp4_dec_ctx_t* ctx, uint32_t uFrmRate);
extern int32_t mp4_getgobhdr(mp4_dec_ctx_t* ctx, int32_t gob_index);
extern void mp4_set_deblk_dram_buf(MP4_STATE* s);

extern tab_type tableB16_1[];
extern tab_type tableB16_2[];
extern tab_type tableB16_3[];

extern tab_type tableB17_1[];
extern tab_type tableB17_2[];
extern tab_type tableB17_3[];

extern	tab_type tableI2_1[];
extern	tab_type tableI2_2[];
extern	tab_type tableI2_3[];

int32_t vldTableB19(int last, int run);
int32_t vldTableB20(int last, int run);
int32_t vldTableB21(int last, int level);
int32_t vldTableB22(int last, int level);

tab_type *vldTableB16(MP4_STREAM * _ld, int32_t code);
tab_type *vldTableB17(MP4_STREAM * _ld, int32_t code);
tab_type *vldTableI2(MP4_STREAM * _ld, int32_t code);

extern int32_t mp4_getMVdata(MP4_STREAM * _ld);

extern	uint32_t zig_zag_scan[64];
extern  uint32_t alternate_horizontal_scan[64];
extern  uint32_t alternate_vertical_scan[64];
extern  uint32_t MBA_NumMBs[6];
extern  uint32_t MBA_FieldWidth[6];
extern	tab_type CBPYtab[48];
extern  int32_t gNewTAB_DQUANT_MQ[32][2];
extern  int32_t DQtab[4];
extern	tab_type MVtab0[14];
extern	tab_type MVtab1[96];
extern	tab_type MVtab2[124];

extern 
#ifdef __cplusplus
}
#endif

#endif // _MP4_DECFUNCS_H_
