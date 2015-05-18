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
#include "mp4_decfuncs.h"

void mp4_set_quant_matrix(MP4_STATE * _mp4_state, int32_t intra_flag);


#define MP4_BASEADDR                    MPEG_REGS_BASE
#define vdec_readuint32(offset)         SYS_ReadDWord(MP4_BASEADDR+offset)
#define vdec_writeuint32(offset,value)  SYS_WriteDWord(MP4_BASEADDR+offset,value)
#define vdec_writeuint8(offset,value)   SYS_WriteByte(MP4_BASEADDR+offset,value)

#define X8_BASEADDR                    VC1_REGS_BASE
#define x8_readuint32(offset)         SYS_ReadDWord(X8_BASEADDR+offset)
#define x8_writeuint32(offset,value)  SYS_WriteDWord(X8_BASEADDR+offset,value)


void mp4_init_global_variable(void)
{
    SetRegValue(mp4mphr_reg00, 0);
	SetRegValue(mp4mvophr_reg04, 0);
	SetRegValue(mp4fsize_reg08, 0);
	SetRegValue(mp4picsize_reg0c, 0);
	SetRegValue(mp4mbaddr_reg10, 0);
	SetRegValue(mp4vectrl_reg14, 0);
	SetRegValue(mp4vetrigger_reg18, 0);
	SetRegValue(mp4vestat_reg1c, 0);
	SetRegValue(mp4trbtrdfld_reg20, 0);
	SetRegValue(mp4trbtrdfrm_reg24, 0);
	SetRegValue(mp4vldbaddr_reg28, 0);
	SetRegValue(mp4vldoffset_reg2c, 0);
	SetRegValue(mp4vldlen_reg30, 0);
	SetRegValue(mp4vbvsize_reg34, 0);
	SetRegValue(mp4mbhaddr_reg38, 0);
	SetRegValue(mp4vldoffset_reg38, 0);
	SetRegValue(mp4vldlen_reg3c, 0);
	SetRegValue(mp4dcacaddr_reg3c, 0);
	SetRegValue(mp4dblkaddr_reg40, 0);
	SetRegValue(mp4ncfaddr_reg44, 0);
	SetRegValue(mp4rec_yframaddr_reg48, 0);
	SetRegValue(mp4rec_cframaddr_reg4c, 0);
	SetRegValue(mp4for_yframaddr_reg50, 0);
	SetRegValue(mp4for_cframaddr_reg54, 0);
	SetRegValue(mp4back_yframaddr_reg58, 0);
	SetRegValue(mp4back_cframaddr_reg5c, 0);
	SetRegValue(mp4socx_reg60, 0);
	SetRegValue(mp4socy_reg64, 0);
	SetRegValue(mp4sol_reg68, 0);
	SetRegValue(mp4sdlx_reg6c, 0);
	SetRegValue(mp4sdly_reg70, 0);
	SetRegValue(mp4spriteshifter_reg74, 0);
	SetRegValue(mp4sdcx_reg78, 0);
	SetRegValue(mp4sdcy_reg7c, 0);
	SetRegValue(mp4iqminput_reg80, 0);
	SetRegValue(mp4qcinput_reg84, 0);
	SetRegValue(mp4iqidctinput_reg90, 0);
	SetRegValue(mp4mbh_reg94, 0);
	SetRegValue(mp4mv1_reg98, 0);
	SetRegValue(mp4mv2_reg9c, 0);
	SetRegValue(mp4mv3_rega0, 0);
	SetRegValue(mp4mv4_rega4, 0);
	SetRegValue(mp4mv5_rega8, 0);
	SetRegValue(mp4mv6_regac, 0);
	SetRegValue(mp4mv7_regb0, 0);
	SetRegValue(mp4mv8_regb4, 0);
	SetRegValue(mp4errflag_regc4, 0);
	SetRegValue(mp4crtmb_regc8, 0);
    mp4vectrl_reg14.ve_finish_int_en = 1;
    mp4vectrl_reg14.ve_error_int_en = 1;
}


void mp4_reset_ve_core(mp4_dec_ctx_t* mp4_context)
{
    ResetVeInternal(mp4_context->pVideoEngine);
}

void VERegWriteD(uint32_t offset, uint32_t val)
{
    vdec_writeuint32(offset, val);
}

// check MC status, return 1 if free, else 0
int8_t mp4_check_mc_free(void)
{
    volatile uint32_t dwVal;
    int32_t i;

    i=0;
    vdec_readuint32(VESTAT_REG);
    while(dwVal & 0x1000)
    {
        i++;
        if(i > 0x200000)
        {
            return 0;
        }
        vdec_readuint32(VESTAT_REG);
    }

    return 1;
}

int8_t mp4_check_VE_free(void)
{
    volatile uint32_t dwVal;
    int32_t i;
    i=0;
    vdec_readuint32(VESTAT_REG);
    while(dwVal & 0x2000)
    {
        i++;
        if(i > 0x200000)
        {
            return 0;
        }
        vdec_readuint32(VESTAT_REG);
    }

    return 1;

}

int8_t mp4_CheckVEBusy(void)
{
    volatile uint32_t dwVal;

    vdec_readuint32(VESTAT_REG);
    if(dwVal & 0x00003f00)//bit8 ~ bit13
        return 1;

    return 0;
}

int8_t mp4_check_finish_flag(void)
{
    volatile uint32_t dwVal;
    int32_t i;
    i=0;

    vdec_readuint32(VESTAT_REG);
    while(!(dwVal&1))
    {
        i++;
        if(i > 0x200000)
            return 0;
        vdec_readuint32(VESTAT_REG);
    }
    *(uint32_t *)(&mp4vestat_reg1c)= vdec_readuint32(VESTAT_REG);
    mp4vestat_reg1c.ve_finish = 1;
    vdec_writeuint32(VESTAT_REG,*(uint32_t *)(&mp4vestat_reg1c));

    return 1;
}

// set MPEG-4 VOP header information
void mp4_set_pic_size(MP4_STATE * s)
{
    volatile uint32_t *pdwTemp;

    // set MP4 picture size information (Picture Size in macroblock unit)
    pdwTemp = (uint32_t*)(&mp4fsize_reg08);
    //*pdwTemp = 0;
    mp4fsize_reg08.pic_width_in_mbs = (s->hdr.width + 15)/16;
    mp4fsize_reg08.pic_height_in_mbs = (s->hdr.height + 15)/16;
    mp4fsize_reg08.store_width_in_mbs = mp4fsize_reg08.pic_width_in_mbs;
    if(mp4fsize_reg08.store_width_in_mbs&1)
        mp4fsize_reg08.store_width_in_mbs ++;
    vdec_writeuint32(FSIZE_REG,*pdwTemp);

    pdwTemp = (uint32_t*)(&mp4picsize_reg0c);
    //*pdwTemp = 0;
    if ((s->userdata_codec_version < 500) && (s->userdata_codec_version != 0)
        && (s->userdata_codec_version != 311) && (s->userdata_codec_version != 263) )
    {
        mp4picsize_reg0c.pic_boundary_width = s->hdr.width;
        mp4picsize_reg0c.pic_boundary_height = s->hdr.height;
    }
    else
    {
        mp4picsize_reg0c.pic_boundary_width =  (s->hdr.width + 15)/16;
        mp4picsize_reg0c.pic_boundary_height = (s->hdr.height + 15)/16;
        mp4picsize_reg0c.pic_boundary_width *=16;
        mp4picsize_reg0c.pic_boundary_height *=16;
    }
    vdec_writeuint32(PICSIZE_REG,*pdwTemp);
}

void mp4_set_buffer(MP4_STATE * s)
{
    volatile uint32_t *pdwTemp;

    // NOT CODED flag base address
    pdwTemp = (uint32_t*)(&mp4ncfaddr_reg44);
    //*pdwTemp = 0;
    mp4ncfaddr_reg44.addr = (uint32_t)(AdapterMemGetPhysicAddress((void*)s->nc_flag_buf))>>10;
    vdec_writeuint32(NCFADDR_REG, *pdwTemp);

    // MB information base address
    pdwTemp = (uint32_t*)(&mp4mbhaddr_reg38);
    //*pdwTemp = 0;
    mp4mbhaddr_reg38.mbh_addr = (uint32_t)(AdapterMemGetPhysicAddress((void*)s->mbh_buf))>>10;
    vdec_writeuint32(MBHADDR_REG,*pdwTemp);

    // FDC/QAC base address
    pdwTemp = (uint32_t*)(&mp4dcacaddr_reg3c);
    //*pdwTemp = 0;
    mp4dcacaddr_reg3c.addr = (uint32_t)(AdapterMemGetPhysicAddress((void*)s->fdc_qac_buf))>>10;
    vdec_writeuint32(DCACADDR_REG,*pdwTemp);

    if(s->deblk_buf != 0)
    {
    	mp4_set_deblk_dram_buf(s);
    }
}

void mp4_get_pic_size(VideoPicture* picture, mp4_dec_ctx_t* ctx)
{
    picture->nFrameRate  = ctx->videoStreamInfo.nFrameRate;
    picture->bIsProgressive= 1;
}

int16_t mp4_set_vop_info(mp4_dec_ctx_t* ctx)
{
    MP4_STATE* 	mp4_state;
    uint32_t* 		pdwTemp;
    uint32_t startAddr = 0;

    mp4_state = &ctx->s;

    // set MPEG-4 VOP header information
    pdwTemp = (uint32_t*)(&mp4mvophr_reg04);
    *pdwTemp = 0;
    mp4mvophr_reg04.short_video_header = mp4_state->hdr.short_video_header;
    mp4mvophr_reg04.interlaced 		= mp4_state->hdr.interlaced;
    
    if(mp4_state->hdr.prediction_type == MP4_B_VOP)
        mp4mvophr_reg04.co_located_vop_type = mp4_state->hdr.old_prediction_type;
        
    mp4mvophr_reg04.sprite_wrap_accuracy 	= mp4_state->hdr.sprite_warping_accuracy;
    mp4mvophr_reg04.no_wrapping_points 	= mp4_state->hdr.iEffectiveWarpingPoints;
    mp4mvophr_reg04.quant_type 			= mp4_state->hdr.quant_type;
    mp4mvophr_reg04.quarter_sample 		= mp4_state->hdr.quarter_pixel;
    mp4mvophr_reg04.resync_marker_dis 		= mp4_state->hdr.resync_marker_disable;
    mp4mvophr_reg04.vop_coding_type 		= mp4_state->hdr.prediction_type;
    mp4mvophr_reg04.vop_rounding_type 		= mp4_state->hdr.rounding_type;
    
    if(mp4_state->isH263 || mp4_state->hdr.short_video_header)
        mp4mvophr_reg04.use_h263_escape = 1;
        
    if(mp4_state->isH263)
    {
        mp4mvophr_reg04.is_h263_pmv = 1;
        mp4mvophr_reg04.is_h263_umv = 1;
    }
    
    if(mp4_state->isH263==1)
    {
        mp4mvophr_reg04.en_modi_quant 		= mp4_state->hdr.iModifiedQantization;
        mp4mvophr_reg04.en_adv_intra_pred 	= mp4_state->hdr.h263_aic;
    }
   
    mp4mvophr_reg04.intra_dc_vlc_thr 	= mp4_state->hdr.intra_dc_vlc_thr;
    mp4mvophr_reg04.top_field_first 	= mp4_state->hdr.top_field_first;
    mp4mvophr_reg04.alter_v_scan 		= mp4_state->hdr.alternate_vertical_scan_flag;
    mp4mvophr_reg04.vop_fcode_f 		= mp4_state->hdr.fcode_for;
    mp4mvophr_reg04.vop_fcode_b 		= mp4_state->hdr.fcode_back;
	
    // set MP4 picture size information (Picture Size in pixel unit)
    pdwTemp = (uint32_t*)(&mp4picsize_reg0c);
    *pdwTemp = 0;
    
    if ((mp4_state->userdata_codec_version < 500) && (mp4_state->userdata_codec_version != 0)
        && (mp4_state->userdata_codec_version != 311) && (mp4_state->userdata_codec_version != 263) )
    {
        mp4picsize_reg0c.pic_boundary_width = mp4_state->hdr.width;
        mp4picsize_reg0c.pic_boundary_height = mp4_state->hdr.height;
    }
    else
    {
        mp4picsize_reg0c.pic_boundary_width =  (mp4_state->hdr.width + 15)/16;
        mp4picsize_reg0c.pic_boundary_height = (mp4_state->hdr.height + 15)/16;
        mp4picsize_reg0c.pic_boundary_width *=16;
        mp4picsize_reg0c.pic_boundary_height *=16;
    }

    // set MB address information (Macro Block Address Register)
    pdwTemp = (uint32_t*)(&mp4mbaddr_reg10);
    *pdwTemp = 0;

    // set video engine control information
    pdwTemp = (uint32_t*)(&mp4vectrl_reg14);                // issue??
    *pdwTemp &= 0x78;
        
    mp4vectrl_reg14.qp_ac_dc_out_en = 1;
    
    if(mp4_state->sw_vld)
        mp4vectrl_reg14.swvld_flag = 1;
    else    // hardware vld
        mp4vectrl_reg14.swvld_flag = 0;

    if(mp4_state->hdr.prediction_type == MP4_P_VOP && !mp4_state->sw_vld)
    {
        mp4vectrl_reg14.nc_flag_out_en = 1;
    }
    if(mp4_state->hdr.prediction_type == MP4_S_VOP)
    {
        mp4mv5_rega8.mv0 = mp4_state->hdr.gmc_lum_mv_x & 0x7fff;
        mp4mv5_rega8.mv1 = mp4_state->hdr.gmc_lum_mv_y & 0x7fff;
        mp4mv6_regac.mv0 = mp4_state->hdr.gmc_chrom_mv_x & 0x7fff;
        mp4mv6_regac.mv1 = mp4_state->hdr.gmc_chrom_mv_y & 0x7fff;
    }
    mp4vectrl_reg14.mvcs_fld_hm = 1;
    if(mp4_state->hdr.quarter_pixel)
    {
        switch(mp4_state->hdr.prediction_type)
        {
            case MP4_P_VOP:
            case MP4_S_VOP:
            {
            	if(ctx->videoStreamInfo.eCodecFormat== VIDEO_CODEC_FORMAT_XVID)
            	{
                    mp4vectrl_reg14.mvcs_mv1_qm = 0;
                    mp4vectrl_reg14.mvcs_mv1_qm = 0;
                    mp4vectrl_reg14.mvcs_fld_qm = 1;
            	}
                else
                {
                	if((mp4_state->userdata_codec_version == 0) || (mp4_state->userdata_codec_version >= 503))
                        mp4vectrl_reg14.mvcs_mv1_qm = 2;        // EightRound
                    else
                        mp4vectrl_reg14.mvcs_mv1_qm = 1;        // Two step's Div2Round
                        
                    mp4vectrl_reg14.mvcs_mv4_qm = 1;        // Right shifted by 1 before summation and 1/16 rounding
                    mp4vectrl_reg14.mvcs_fld_qm = 2;            // Firstly Div2Round, then Div2Round again, finally y component right shift by 1
                }
                break;
            }
            case MP4_B_VOP:
            {
            	if(ctx->videoStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_XVID)
            	{
            		mp4vectrl_reg14.mvcs_mv1_qm = 0;
            		mp4vectrl_reg14.mvcs_mv4_qm = 0;
            	}
            	else
            	{
            		mp4vectrl_reg14.mvcs_mv1_qm = 1;        // Two step's Div2Round
            		mp4vectrl_reg14.mvcs_mv4_qm = 2;        // Summation and 1/16 rounding, and  Div2Round
            	}
                mp4vectrl_reg14.mvcs_fld_qm = 1;        // Firstly divided by 2, then Div2Round, final y component right shift by 1
                break;
            }
        }
    }
	
    mp4vectrl_reg14.outloop_dblk_en 		= 0;
    mp4vectrl_reg14.Histogram_output_en 	= mp4_state->enLumaHistogram;
    mp4vectrl_reg14.mc_cache_en 			= 1;
    mp4vectrl_reg14.not_write_recons_flag 	= 0;

    // set video engine start trigger information
    pdwTemp = (uint32_t*)(&mp4vetrigger_reg18);
    *pdwTemp = 0;
    mp4vetrigger_reg18.dec_format   = 4;                    // MPEG-4 format
    mp4vetrigger_reg18.chrom_format = 0;                // 4:2:0 format
    // set VLD VBV start address information
    pdwTemp  = (uint32_t*)(&mp4vldbaddr_reg28);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4vldoffset_reg2c);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4vldlen_reg30);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4vbvsize_reg34);
    *pdwTemp = 0;
    startAddr = (uint32_t)AdapterMemGetPhysicAddress(ctx->pStreamBufferBase);
    mp4vldbaddr_reg28.vld_byte_start_addr = ((startAddr>>28)&0x7) | (startAddr&0x0ffffff0);
    mp4vbvsize_reg34.vld_byte_end_addr = (uint32_t)AdapterMemGetPhysicAddress(ctx->pStreamBufferBase + ctx->nStreamBufferSize - 1);
    // clear TRB and TRD registers
    pdwTemp  = (uint32_t*)(&mp4trbtrdfld_reg20);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4trbtrdfrm_reg24);
    *pdwTemp = 0;
    // clear SPRITE registers
    pdwTemp  = (uint32_t*)(&mp4socx_reg60);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4socy_reg64);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4sol_reg68);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4sdlx_reg6c);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4sdly_reg70);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4spriteshifter_reg74);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4sdcx_reg78);
    *pdwTemp = 0;
    pdwTemp  = (uint32_t*)(&mp4sdcy_reg7c);
    *pdwTemp = 0;

    // set MB Quantiser Scale Input register
    pdwTemp = (uint32_t*)(&mp4qcinput_reg84);
    *pdwTemp = 0;
    mp4qcinput_reg84.quant_scale = mp4_state->hdr.quantizer;

    if(mp4_state->hdr.prediction_type == MP4_B_VOP)
    {
        mp4trbtrdfrm_reg24.trd = mp4_state->hdr.trd;
        mp4trbtrdfrm_reg24.trb = mp4_state->hdr.trb;

        if(mp4_state->hdr.interlaced)
        {
            mp4trbtrdfld_reg20.trb = mp4_state->hdr.trbi;
            mp4trbtrdfld_reg20.trd = mp4_state->hdr.trdi;
        }
    }

    if(mp4_state->hdr.prediction_type == MP4_S_VOP)
    {
        if(mp4_state->at_lum.XX>32767 || mp4_state->at_lum.XX<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_lum.XY>32767 || mp4_state->at_lum.XY<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_lum.YX>32767 || mp4_state->at_lum.YX<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_lum.YY>32767 || mp4_state->at_lum.YY<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_chrom.XX>32767 || mp4_state->at_chrom.XX<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_chrom.XY>32767 || mp4_state->at_chrom.XY<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_chrom.YX>32767 || mp4_state->at_chrom.YX<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;
        if(mp4_state->at_chrom.YY>32767 || mp4_state->at_chrom.YY<-32768)
            mp4spriteshifter_reg74.sprite_delta_data_mode = 1;

        mp4sol_reg68.sprite_offset_x = mp4_state->at_lum.X0;
        mp4sol_reg68.sprite_offset_y = mp4_state->at_lum.Y0;
        mp4spriteshifter_reg74.sprite_luma_shifter = mp4_state->at_lum.shifter;

        mp4socx_reg60.sprite_offset_x = mp4_state->at_chrom.X0;
        mp4socy_reg64.sprite_offset_y = mp4_state->at_chrom.Y0;
        mp4spriteshifter_reg74.sprite_chroma_shifter = mp4_state->at_chrom.shifter;

        if(!mp4spriteshifter_reg74.sprite_delta_data_mode)
        {
            mp4sdlx_reg6c.sprite_delta_xx = mp4_state->at_lum.XX;
            mp4sdlx_reg6c.sprite_delta_xy = mp4_state->at_lum.XY;
            mp4sdly_reg70.sprite_delta_yx = mp4_state->at_lum.YX;
            mp4sdly_reg70.sprite_delta_yy = mp4_state->at_lum.YY;
            mp4sdcx_reg78.sprite_delta_xx = mp4_state->at_chrom.XX;
            mp4sdcx_reg78.sprite_delta_xy = mp4_state->at_chrom.XY;
            mp4sdcy_reg7c.sprite_delta_yx = mp4_state->at_chrom.YX;
            mp4sdcy_reg7c.sprite_delta_yy = mp4_state->at_chrom.YY;
        }
        else
        {
            mp4sdlx_reg6c.sprite_delta_xx = mp4_state->at_lum.XX>>1;
            mp4sdlx_reg6c.sprite_delta_xy = mp4_state->at_lum.XY>>1;
            mp4sdly_reg70.sprite_delta_yx = mp4_state->at_lum.YX>>1;
            mp4sdly_reg70.sprite_delta_yy = mp4_state->at_lum.YY>>1;
            mp4sdcx_reg78.sprite_delta_xx = mp4_state->at_chrom.XX>>1;
            mp4sdcx_reg78.sprite_delta_xy = mp4_state->at_chrom.XY>>1;
            mp4sdcy_reg7c.sprite_delta_yx = mp4_state->at_chrom.YX>>1;
            mp4sdcy_reg7c.sprite_delta_yy = mp4_state->at_chrom.YY>>1;
            mp4spriteshifter_reg74.sprite_delta_luma_xx_b0 = mp4_state->at_lum.XX & 1;
            mp4spriteshifter_reg74.sprite_delta_luma_xy_b0 = mp4_state->at_lum.XY & 1;
            mp4spriteshifter_reg74.sprite_delta_luma_yx_b0 = mp4_state->at_lum.YX & 1;
            mp4spriteshifter_reg74.sprite_delta_luma_yy_b0 = mp4_state->at_lum.YY & 1;
            mp4spriteshifter_reg74.sprite_delta_chroma_xx_b0 = mp4_state->at_chrom.XX & 1;
            mp4spriteshifter_reg74.sprite_delta_chroma_xy_b0 = mp4_state->at_chrom.XY & 1;
            mp4spriteshifter_reg74.sprite_delta_chroma_yx_b0 = mp4_state->at_chrom.YX & 1;
            mp4spriteshifter_reg74.sprite_delta_chroma_yy_b0 = mp4_state->at_chrom.YY & 1;
        }
    }
    
    mp4_set_buffer(mp4_state);

    // set frame buffer information
    pdwTemp  = (uint32_t*)(&mp4rec_yframaddr_reg48);
    *pdwTemp = 0;
    if(ctx->m_cur)
    {
    	//mp4rec_yframaddr_reg48.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_cur->pData0);
    	mp4rec_yframaddr_reg48.addr = ctx->m_cur->phyYBufAddr;
    }
    	
    pdwTemp  = (uint32_t*)(&mp4rec_cframaddr_reg4c);
    *pdwTemp = 0;
    if(ctx->m_cur)
    {
    	//mp4rec_cframaddr_reg4c.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_cur->pData1);
    	mp4rec_cframaddr_reg4c.addr = ctx->m_cur->phyCBufAddr;
    }
    	
    pdwTemp  = (uint32_t*)(&mp4for_yframaddr_reg50);
    *pdwTemp = 0;
    


    if(ctx->m_for)
    {
    	//mp4for_yframaddr_reg50.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_for->pData0);
    	mp4for_yframaddr_reg50.addr = ctx->m_for->phyYBufAddr;
    }

    pdwTemp  = (uint32_t*)(&mp4for_cframaddr_reg54);
    *pdwTemp = 0;
    if(ctx->m_for)
    {
    	//mp4for_cframaddr_reg54.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_for->pData1);
    	mp4for_cframaddr_reg54.addr = ctx->m_for->phyCBufAddr;
    }

    pdwTemp = (uint32_t*)(&mp4back_yframaddr_reg58);
    *pdwTemp = 0;
    if(ctx->m_bac)
    {
    	//mp4back_yframaddr_reg58.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_bac->pData0);
    	mp4back_yframaddr_reg58.addr = ctx->m_bac->phyYBufAddr;
    }

    pdwTemp = (uint32_t*)(&mp4back_cframaddr_reg5c);
    *pdwTemp = 0;
    if(ctx->m_bac)
    {
    	//mp4back_cframaddr_reg5c.addr = (uint32_t)AdapterMemGetPhysicAddress((void*)ctx->m_bac->pData1);
    	mp4back_cframaddr_reg5c.addr = ctx->m_bac->phyCBufAddr;
    }
    // set iquant weight matrix
    if(mp4_state->hdr.quant_type)
    {
        //intra weight matrix
        mp4_set_quant_matrix(mp4_state, 1);
        // non-intra weight matrix
        mp4_set_quant_matrix(mp4_state, 0);
    }

    // write register to hardware
    pdwTemp = (uint32_t*)(&mp4mvophr_reg04);
    vdec_writeuint32(MVOPHR_REG,*pdwTemp);
	//* as android
	{
		uint32_t tmp = 0;
		tmp = mp4fsize_reg08.pic_height_in_mbs;
		tmp |= mp4fsize_reg08.pic_width_in_mbs<<8;
		tmp |= mp4fsize_reg08.store_width_in_mbs<<16;
    	vdec_writeuint32(FSIZE_REG,tmp);
	}
    pdwTemp = (uint32_t*)(&mp4mbaddr_reg10);
    vdec_writeuint32(MBADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4vectrl_reg14);
    vdec_writeuint32(VECTRL_REG,*pdwTemp);
    mp4vetrigger_reg18.ve_start_type = 0;
    pdwTemp = (uint32_t*)(&mp4vetrigger_reg18);
    vdec_writeuint32(VETRIGGER_REG,*pdwTemp);
    // picture boundary size in pixel unit
    pdwTemp = (uint32_t*)(&mp4picsize_reg0c);
    vdec_writeuint32(PICSIZE_REG,*pdwTemp);
    // MB quantiser scale
    pdwTemp = (uint32_t*)(&mp4qcinput_reg84);
    vdec_writeuint32(QCINPUT_REG,*pdwTemp);
    // select decode and display buffers
    pdwTemp = (uint32_t*)(&mp4rec_yframaddr_reg48);
    vdec_writeuint32(RECYADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4rec_cframaddr_reg4c);
    vdec_writeuint32(RECCADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4for_yframaddr_reg50);
    vdec_writeuint32(FORYADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4for_cframaddr_reg54);
    vdec_writeuint32(FORCADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4back_yframaddr_reg58);
    vdec_writeuint32(BACKYADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4back_cframaddr_reg5c);
    vdec_writeuint32(BACKCADDR_REG,*pdwTemp);

    if(mp4_state->hdr.prediction_type == MP4_S_VOP)
    {
        // sprite offset, sprite delta and sprite shifter
        pdwTemp = (uint32_t*)(&mp4sol_reg68);
        vdec_writeuint32(SOL_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4sdlx_reg6c);
        vdec_writeuint32(SDLX_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4sdly_reg70);
        vdec_writeuint32(SDLY_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4spriteshifter_reg74);
        vdec_writeuint32(SSR_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4socx_reg60);
        vdec_writeuint32(SOCX_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4socy_reg64);
        vdec_writeuint32(SOCY_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4sdcx_reg78);
        vdec_writeuint32(SDCX_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4sdcy_reg7c);
        vdec_writeuint32(SDCY_REG,*pdwTemp);

        // luma and chrom motion vectors
        pdwTemp = (uint32_t*)(&mp4mv5_rega8);
        vdec_writeuint32(MV5_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4mv6_regac);
        vdec_writeuint32(MV6_REG,*pdwTemp);
    }
    if(mp4_state->hdr.prediction_type == MP4_B_VOP)
    {
        // TRB and TRD
        pdwTemp = (uint32_t*)(&mp4trbtrdfrm_reg24);
        vdec_writeuint32(TRBTRDFRM_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4trbtrdfld_reg20);
        vdec_writeuint32(TRBTRDFLD_REG,*pdwTemp);
    }

    return 0;
}



// set quantisation matrix
void mp4_set_quant_matrix(MP4_STATE * _mp4_state, int32_t intra_flag)
{
    int32_t i;
    uint32_t value;
    MP4_STATE * mp4_state = _mp4_state;

    for(i=0; i<64; i++)
    {
        value = i<<8;
        if(intra_flag)
        {
            value |= mp4_state->hdr.intra_quant_matrix[zig_zag_scan[i]];
            value |= 0x4000;
        }
        else
            value |= mp4_state->hdr.nonintra_quant_matrix[zig_zag_scan[i]];
        vdec_writeuint32(IQMINPUT_REG,value);
    }
}

static void mp4_trigger(uint32_t trigger_type)
{
    vdec_writeuint32(VETRIGGER_REG,trigger_type);
}

void mp4_set_vbv_info(int32_t mb_num_in_gob, int32_t boundary, int32_t start_bit_pos, int32_t bit_length, uint32_t vbv_buf_size)
{
    uint32_t* pdwTemp;

    // clear some register before doing one new packet, GOB and VOP
    vdec_writeuint32(VESTAT_REG,0xffffffff);
    vdec_writeuint32(CRTMB_REG,0x0);
    vdec_writeuint32(ERRFLAG_REG,0x0);

    mp4vetrigger_reg18.ve_start_type = GOB_LEVEL_START;            // GOB level VLD
    mp4vetrigger_reg18.num_mb_in_gob = mb_num_in_gob;
    mp4vetrigger_reg18.mb_boundary = boundary;

    mp4vldbaddr_reg28.vbv_buff_data_first = 1;
    mp4vldbaddr_reg28.vbv_buff_data_last = 1;
    mp4vldbaddr_reg28.vbv_buff_data_valid = 1;

    if(start_bit_pos >= (int32_t)vbv_buf_size * 8)
    {
        start_bit_pos -= (int32_t)vbv_buf_size * 8;
    }
    mp4vldoffset_reg2c.vld_bit_offset = start_bit_pos;

    mp4vldlen_reg30.vld_bit_len = ((bit_length+31)>>5)<<5;

    pdwTemp = (uint32_t*)(&mp4vldoffset_reg2c);
    vdec_writeuint32(VLDOFFSET_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4vldlen_reg30);
    vdec_writeuint32(VLDLEN_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4vbvsize_reg34);
    vdec_writeuint32(VBVENDADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4vldbaddr_reg28);
    vdec_writeuint32(VLDBADDR_REG,*pdwTemp);
    pdwTemp = (uint32_t*)(&mp4vetrigger_reg18);

    mp4_trigger(*pdwTemp);
}

// set packet information when resync_mark is found
void mp4_set_packet_info(MP4_STATE * _mp4_state)
{
    MP4_STATE *mp4_state = _mp4_state;
    uint32_t  *pdwTemp;
    int32_t mb_xpos;
    int32_t mb_ypos;
    
    if(mp4_state->hdr.macroblok_bounday==0)
    	return;


    mp4_state->hdr.mb_xpos = mp4_state->hdr.mba % mp4_state->hdr.mb_xsize;
    mp4_state->hdr.mb_ypos = mp4_state->hdr.mba / mp4_state->hdr.mb_xsize;
    mb_xpos = mp4_state->hdr.mb_xpos;
    mb_ypos = mp4_state->hdr.mb_ypos;
    mp4qcinput_reg84.quant_scale = mp4_state->hdr.quantizer;
    mp4mvophr_reg04.intra_dc_vlc_thr = mp4_state->hdr.intra_dc_vlc_thr;
    mp4mbaddr_reg10.mb_x = mb_xpos;
    mp4mbaddr_reg10.mb_y = mb_ypos;
    mp4mvophr_reg04.vop_coding_type = mp4_state->hdr.picture_coding_type;
    mp4mvophr_reg04.en_adv_intra_pred = mp4_state->hdr.h263_aic;
    mp4mvophr_reg04.vop_rounding_type = mp4_state->hdr.rounding_type;
    // TBD for S-VOP
    //...
    if(mp4_CheckVEBusy()==0)
    {
        pdwTemp = (uint32_t*)(&mp4mvophr_reg04);
        vdec_writeuint32(MVOPHR_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4qcinput_reg84);
        vdec_writeuint32(QCINPUT_REG,*pdwTemp);
        pdwTemp = (uint32_t*)(&mp4mbaddr_reg10);
        vdec_writeuint32(MBADDR_REG,*pdwTemp);
    }
}

uint32_t mp4_get_mba()
{
    volatile uint32_t dwVal=0;
    vdec_readuint32(MBADDR_REG);
    return dwVal;
}

uint32_t mp4_get_bitoffset()
{
    volatile uint32_t dwVal=0;

    vdec_readuint32(VLDOFFSET_REG);

    return dwVal;

}


void mp4_set_ivop_mbinfo(MP4_STATE * _mp4_state)
{
    MP4_STATE * mp4_state = _mp4_state;

    if((mp4_state->hdr.derived_mb_type == INTRA) ||
        (mp4_state->hdr.derived_mb_type == INTRA_Q))
    {
        mp4mbh_reg94.mb_intra = 1;
        mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
        mp4mbh_reg94.ac_pred_flag = mp4_state->hdr.ac_pred_flag;
        mp4mbh_reg94.cbp = mp4_state->hdr.cbp;            // issue ???
    }
}
void mp4_set_pvop_mbinfo(MP4_STATE * _mp4_state)
{
    MP4_STATE * mp4_state = _mp4_state;
    int32_t mb_xpos = mp4_state->hdr.mba%mp4_state->hdr.mb_xsize;
    int32_t mb_ypos = mp4_state->hdr.mba/mp4_state->hdr.mb_xsize;

    if(!mp4_state->hdr.not_coded)
    {
        if((mp4_state->hdr.derived_mb_type == INTRA) ||
             (mp4_state->hdr.derived_mb_type == INTRA_Q))
        {//3, 4
            mp4mbh_reg94.mb_intra = 1;
            mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
            mp4mbh_reg94.ac_pred_flag = mp4_state->hdr.ac_pred_flag;
            mp4mbh_reg94.cbp = mp4_state->hdr.cbp;        // issue ???
        }
        else if((mp4_state->hdr.derived_mb_type == INTER) ||
            (mp4_state->hdr.derived_mb_type == INTER_Q))
        {//0, 1
            mp4mbh_reg94.mb_forw = 1;
            // set MVs and prediction type
            // set first forward MV
            mp4mv1_reg98.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].x) & 0x1fff);
            mp4mv1_reg98.mv1 =    (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].y) & 0x1fff);
            if((mp4_state->hdr.interlaced==1)&&(mp4_state->hdr.field_prediction == 1))
            {
                mp4mbh_reg94.mc_field = 1;        // field prediction
                mp4mv3_rega0.mv0 = (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].x) & 0x1fff);
                mp4mv3_rega0.mv1 =    (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].y) & 0x1fff);
                // forward field reference
                mp4mv1_reg98.mv_fs = mp4_state->hdr.forward_top_field_reference;
                mp4mv3_rega0.mv_fs = mp4_state->hdr.forward_bottom_field_reference;
            }
            else
                mp4mbh_reg94.mc_frame = 1;
            mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
            if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
            else
                mp4mbh_reg94.dct_type = 0;
        }
        else if(mp4_state->hdr.derived_mb_type == INTER4V)
        {//2
            mp4mbh_reg94.mb_forw = 1;
            mp4mbh_reg94.mc_8x8 = 1;
            mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
            if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
            else
                mp4mbh_reg94.dct_type = 0;
            // set 4 MVs
            // MV1 (Y0 block MV)
            mp4mv1_reg98.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].x) & 0x1fff);
            mp4mv1_reg98.mv1 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0].y) & 0x1fff);
            // MV2 (Y1 block MV)
            mp4mv2_reg9c.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][1].x) & 0x1fff);
            mp4mv2_reg9c.mv1 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][1].y) & 0x1fff);
            // MV3 (Y2 block MV)
            mp4mv3_rega0.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][2].x) & 0x1fff);
            mp4mv3_rega0.mv1 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][2].y) & 0x1fff);
            // MV4 (Y3 block MV)
            mp4mv4_rega4.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][3].x) & 0x1fff);
            mp4mv4_rega4.mv1 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][3].y) & 0x1fff);
        }
        else if(mp4_state->hdr.derived_mb_type == 5)    // H.263 inter4v+q
        {
            // TBD
        }
    }
    else
    {    // not_coded MB in P-VOP
        mp4mbh_reg94.mb_forw = 1;
        mp4mbh_reg94.mc_frame = 1;
        mp4mbh_reg94.cbp = 0;
        mp4mbh_reg94.dct_type = 0;
        // MVs should be zero for not_coded MB
        mp4mv1_reg98.mv0 = 0;
        mp4mv1_reg98.mv1 =    0;
    }
}

void mp4_set_bvop_mbinfo(MP4_STATE * _mp4_state)
{
    MP4_STATE * mp4_state = _mp4_state;
    int32_t mb_xpos = mp4_state->hdr.mba%mp4_state->hdr.mb_xsize;
    int32_t mb_ypos = mp4_state->hdr.mba/mp4_state->hdr.mb_xsize;

    mp4_state->hdr.b_mb_not_coded = 0;
    if ((mp4_state->codedmap[mb_ypos * mp4_state->codedmap_stride + mb_xpos] != 1) || // co_located_not_coded
        (mp4_state->hdr.old_prediction_type == MP4_S_VOP) || mp4_state->isH263)
    {
        if(mp4_state->hdr.modb != MODB_1){
            switch(mp4_state->hdr.mb_type){
            case MB_TYPE_1:            // direct
                mp4mbh_reg94.mb_forw = 1;
                mp4mbh_reg94.mb_back = 1;
                mp4mbh_reg94.mb_direct = 1;                    // m3357 hardware debug flag
                mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
                if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                    mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
                else
                    mp4mbh_reg94.dct_type = 0;
                // in DIRECT mode, prediction mode ( interlaced/frame or progressive/field ) is determined
                // by prediction mode of co-located macroblock.
                if((mp4_state->hdr.interlaced)&&
                    (mp4_state->fieldpredictedmap[(mb_ypos+1) * mp4_state->fieldpredictedmap_stride + mb_xpos + 1] == 1))
                {
                    mp4mbh_reg94.mc_field = 1;
                    // forward top field reference
                    mp4mv1_reg98.mv_fs = mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride];
                    // forward bottom field reference
                    mp4mv3_rega0.mv_fs = mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride + 1];
                    // backward top field reference
                    mp4mv2_reg9c.mv_fs = 0;
                    // backward bottom field reference
                    mp4mv4_rega4.mv_fs = 1;
                    // get direct MB's motion vectors
                    mp4mv1_reg98.mv0 = mp4_state->hdr.mp4_direct_mv[0][0];
                    mp4mv1_reg98.mv1 = mp4_state->hdr.mp4_direct_mv[0][1];
                    mp4mv2_reg9c.mv0 = mp4_state->hdr.mp4_direct_mv[1][0];
                    mp4mv2_reg9c.mv1 = mp4_state->hdr.mp4_direct_mv[1][1];
                    mp4mv3_rega0.mv0 = mp4_state->hdr.mp4_direct_mv[2][0];
                    mp4mv3_rega0.mv1 = mp4_state->hdr.mp4_direct_mv[2][1];
                    mp4mv4_rega4.mv0 = mp4_state->hdr.mp4_direct_mv[3][0];
                    mp4mv4_rega4.mv1 = mp4_state->hdr.mp4_direct_mv[3][1];
                }
                else{
                    mp4mbh_reg94.mc_8x8 = 1;
                    // Y0, Y1, Y2, Y3 blocks' forward motion vectors
                    mp4mv1_reg98.mv0 = mp4_state->hdr.mp4_direct_mv[0][0];
                    mp4mv1_reg98.mv1 = mp4_state->hdr.mp4_direct_mv[0][1];
                    mp4mv2_reg9c.mv0 = mp4_state->hdr.mp4_direct_mv[1][0];
                    mp4mv2_reg9c.mv1 = mp4_state->hdr.mp4_direct_mv[1][1];
                    mp4mv3_rega0.mv0 = mp4_state->hdr.mp4_direct_mv[2][0];
                    mp4mv3_rega0.mv1 = mp4_state->hdr.mp4_direct_mv[2][1];
                    mp4mv4_rega4.mv0 = mp4_state->hdr.mp4_direct_mv[3][0];
                    mp4mv4_rega4.mv1 = mp4_state->hdr.mp4_direct_mv[3][1];
                    // Y0, Y1, Y2, Y3 blocks' backward motion vectors
                    mp4mv5_rega8.mv0 = mp4_state->hdr.mp4_direct_mv[4][0];
                    mp4mv5_rega8.mv1 = mp4_state->hdr.mp4_direct_mv[4][1];
                    mp4mv6_regac.mv0 = mp4_state->hdr.mp4_direct_mv[5][0];
                    mp4mv6_regac.mv1 = mp4_state->hdr.mp4_direct_mv[5][1];
                    mp4mv7_regb0.mv0 = mp4_state->hdr.mp4_direct_mv[6][0];
                    mp4mv7_regb0.mv1 = mp4_state->hdr.mp4_direct_mv[6][1];
                    mp4mv8_regb4.mv0 = mp4_state->hdr.mp4_direct_mv[7][0];
                    mp4mv8_regb4.mv1 = mp4_state->hdr.mp4_direct_mv[7][1];
                }
                break;
            case MB_TYPE_01:        // interpolate mc+q
                mp4mbh_reg94.mb_forw = 1;
                mp4mbh_reg94.mb_back = 1;
                mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
                if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                    mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
                else
                    mp4mbh_reg94.dct_type = 0;
                // set first forward MV
                mp4mv1_reg98.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x) & 0x1fff);
                mp4mv1_reg98.mv1 =    (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y) & 0x1fff);
                // set first backward MB
                mp4mv2_reg9c.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x) & 0x1fff);
                mp4mv2_reg9c.mv1 =    (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y) & 0x1fff);

                if((mp4_state->hdr.interlaced==1)&&(mp4_state->hdr.field_prediction == 1))
                {    // second forward MV
                    mp4mv3_rega0.mv0 = (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x) & 0x1fff);
                    mp4mv3_rega0.mv1 =    (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y) & 0x1fff);
                    // forward field reference
                    mp4mv1_reg98.mv_fs = mp4_state->hdr.forward_top_field_reference;
                    mp4mv3_rega0.mv_fs = mp4_state->hdr.forward_bottom_field_reference;
                    // second backward MV
                    mp4mv4_rega4.mv0 = (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x) & 0x1fff);
                    mp4mv4_rega4.mv1 =    (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y) & 0x1fff);
                    //backward field reference
                    mp4mv2_reg9c.mv_fs = mp4_state->hdr.backward_top_field_reference;
                    mp4mv4_rega4.mv_fs = mp4_state->hdr.backward_bottom_field_reference;
                    mp4mbh_reg94.mc_field = 1;        // field prediction
                }
                else
                    mp4mbh_reg94.mc_frame = 1;
                break;
            case MB_TYPE_001:        // backward mc+q
                mp4mbh_reg94.mb_back = 1;
                mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
                if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                    mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
                else
                    mp4mbh_reg94.dct_type = 0;
                // set first backward MB
                mp4mv2_reg9c.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x) & 0x1fff);
                mp4mv2_reg9c.mv1 =    (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y) & 0x1fff);
                if((mp4_state->hdr.interlaced==1)&&(mp4_state->hdr.field_prediction == 1))
                {    // second backward MV
                    mp4mv4_rega4.mv0 = (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x) & 0x1fff);
                    mp4mv4_rega4.mv1 =    (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y) & 0x1fff);
                    //backward field reference
                    mp4mv2_reg9c.mv_fs = mp4_state->hdr.backward_top_field_reference;
                    mp4mv4_rega4.mv_fs = mp4_state->hdr.backward_bottom_field_reference;
                    mp4mbh_reg94.mc_field = 1;        // field prediction
                }
                else
                    mp4mbh_reg94.mc_frame = 1;
                break;
            case MB_TYPE_0001:        // forward mc+q
                mp4mbh_reg94.mb_forw = 1;
                mp4mbh_reg94.cbp = mp4_state->hdr.cbp;
                if((mp4_state->hdr.interlaced==1)&&(mp4mbh_reg94.cbp!= 0))
                    mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
                else
                    mp4mbh_reg94.dct_type = 0;
                // set first forward MV
                mp4mv1_reg98.mv0 = (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x) & 0x1fff);
                mp4mv1_reg98.mv1 =    (uint32_t)((mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y) & 0x1fff);
                if((mp4_state->hdr.interlaced==1)&&(mp4_state->hdr.field_prediction == 1))
                {    // second forward MV
                    mp4mv3_rega0.mv0 = (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x) & 0x1fff);
                    mp4mv3_rega0.mv1 =    (uint32_t)((mp4_state->MV_field[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y) & 0x1fff);
                    // forward field reference
                    mp4mv1_reg98.mv_fs = mp4_state->hdr.forward_top_field_reference;
                    mp4mv3_rega0.mv_fs = mp4_state->hdr.forward_bottom_field_reference;
                    mp4mbh_reg94.mc_field = 1;        // field prediction
                }
                else
                    mp4mbh_reg94.mc_frame = 1;
                break;
            case MB_TYPE_00001:        // intra
                mp4mbh_reg94.mb_intra = 1;
                mp4mbh_reg94.dct_type = mp4_state->hdr.dct_type;
                mp4mbh_reg94.ac_pred_flag = mp4_state->hdr.ac_pred_flag;
                mp4mbh_reg94.cbp = mp4_state->hdr.cbp;            // issue ???
                break;
            default:
                //printf("error mb_type in B-VOP!\n");
                break;
            }
        }
        else
        {    // direct mode when modb is 1
            mp4mbh_reg94.mb_forw = 1;
            mp4mbh_reg94.mb_back = 1;
            mp4mbh_reg94.mb_direct = 1;            // m3357 hardware debug flag
            mp4mbh_reg94.cbp = 0;
            mp4mbh_reg94.dct_type = 0;
            if((mp4_state->hdr.interlaced) &&
                (mp4_state->fieldpredictedmap[(mb_ypos+1) * mp4_state->fieldpredictedmap_stride + mb_xpos + 1] == 1))
            {
                mp4mbh_reg94.mc_field = 1;
                // forward top field reference
                mp4mv1_reg98.mv_fs = mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride];
                // forward bottom field reference
                mp4mv3_rega0.mv_fs = mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride + 1];
                // backward top field reference
                mp4mv2_reg9c.mv_fs = 0;
                // backward bottom field reference
                mp4mv4_rega4.mv_fs = 1;
                // get direct MB's motion vectors
                mp4mv1_reg98.mv0 = mp4_state->hdr.mp4_direct_mv[0][0];
                mp4mv1_reg98.mv1 = mp4_state->hdr.mp4_direct_mv[0][1];
                mp4mv2_reg9c.mv0 = mp4_state->hdr.mp4_direct_mv[1][0];
                mp4mv2_reg9c.mv1 = mp4_state->hdr.mp4_direct_mv[1][1];
                mp4mv3_rega0.mv0 = mp4_state->hdr.mp4_direct_mv[2][0];
                mp4mv3_rega0.mv1 = mp4_state->hdr.mp4_direct_mv[2][1];
                mp4mv4_rega4.mv0 = mp4_state->hdr.mp4_direct_mv[3][0];
                mp4mv4_rega4.mv1 = mp4_state->hdr.mp4_direct_mv[3][1];
            }
            else{
                mp4mbh_reg94.mc_8x8 = 1;
                // Y0, Y1, Y2, Y3 blocks' forward motion vectors
                mp4mv1_reg98.mv0 = mp4_state->hdr.mp4_direct_mv[0][0];
                mp4mv1_reg98.mv1 = mp4_state->hdr.mp4_direct_mv[0][1];
                mp4mv2_reg9c.mv0 = mp4_state->hdr.mp4_direct_mv[1][0];
                mp4mv2_reg9c.mv1 = mp4_state->hdr.mp4_direct_mv[1][1];
                mp4mv3_rega0.mv0 = mp4_state->hdr.mp4_direct_mv[2][0];
                mp4mv3_rega0.mv1 = mp4_state->hdr.mp4_direct_mv[2][1];
                mp4mv4_rega4.mv0 = mp4_state->hdr.mp4_direct_mv[3][0];
                mp4mv4_rega4.mv1 = mp4_state->hdr.mp4_direct_mv[3][1];
                // Y0, Y1, Y2, Y3 blocks' backward motion vectors
                mp4mv5_rega8.mv0 = mp4_state->hdr.mp4_direct_mv[4][0];
                mp4mv5_rega8.mv1 = mp4_state->hdr.mp4_direct_mv[4][1];
                mp4mv6_regac.mv0 = mp4_state->hdr.mp4_direct_mv[5][0];
                mp4mv6_regac.mv1 = mp4_state->hdr.mp4_direct_mv[5][1];
                mp4mv7_regb0.mv0 = mp4_state->hdr.mp4_direct_mv[6][0];
                mp4mv7_regb0.mv1 = mp4_state->hdr.mp4_direct_mv[6][1];
                mp4mv8_regb4.mv0 = mp4_state->hdr.mp4_direct_mv[7][0];
                mp4mv8_regb4.mv1 = mp4_state->hdr.mp4_direct_mv[7][1];
            }
        }
    }
    else    // not_coded MB in B-VOP
    {
        mp4mbh_reg94.mb_skip = 1;            // m3357 hardware debug flag
        mp4mbh_reg94.mb_forw = 1;
        mp4mbh_reg94.cbp = 0;
        mp4mbh_reg94.mc_frame = 1;
        mp4mbh_reg94.dct_type = 0;
        // set first forward MV
        mp4mv1_reg98.mv0 = 0;
        mp4mv1_reg98.mv1 =    0;
        // set one flag about not_coded MB
        mp4_state->hdr.b_mb_not_coded = 1;
    }
}

void mp4_set_mb_info(MP4_STATE * _mp4_state)
{
    int mb_x, mb_y;
    MP4_STATE * mp4_state = _mp4_state;
    volatile uint32_t *pdwTemp;
	
	mp4_check_VE_free();

    // calaculat MB coordinate
    mb_x = mp4_state->hdr.mba%mp4_state->hdr.mb_xsize;
    mb_y = mp4_state->hdr.mba/mp4_state->hdr.mb_xsize;
    mp4mbaddr_reg10.mb_x = mb_x;
    mp4mbaddr_reg10.mb_y = mb_y;

    // set MB header information
    pdwTemp = (uint32_t *)(&mp4mbh_reg94);
    *pdwTemp = 0;
    // clear all MVs
    pdwTemp = (uint32_t*)(&mp4mv1_reg98);
    *pdwTemp = 0;
    pdwTemp = (uint32_t*)(&mp4mv2_reg9c);
    *pdwTemp = 0;
    pdwTemp = (uint32_t*)(&mp4mv3_rega0);
    *pdwTemp = 0;
    pdwTemp = (uint32_t*)(&mp4mv4_rega4);
    *pdwTemp = 0;
    // MV5, MV6, MV7 and MV8 are used in Direct-MB in MPEG4
    if(mp4_state->hdr.prediction_type != MP4_S_VOP)
    {
        pdwTemp = (uint32_t*)(&mp4mv5_rega8);
        *pdwTemp = 0;
        pdwTemp = (uint32_t*)(&mp4mv6_regac);
        *pdwTemp = 0;
    }
    pdwTemp = (uint32_t*)(&mp4mv7_regb0);
    *pdwTemp = 0;
    pdwTemp = (uint32_t*)(&mp4mv8_regb4);
    *pdwTemp = 0;

    switch(mp4_state->hdr.prediction_type)
    {
    case MP4_I_VOP:
        mp4_set_ivop_mbinfo(mp4_state);
        break;
    case MP4_P_VOP:
        mp4_set_pvop_mbinfo(mp4_state);
        break;
    case MP4_B_VOP:
        mp4_set_bvop_mbinfo(mp4_state);
        break;
    case MP4_S_VOP:
        //not support sw vld, can add in future
        break;
    }

    if(mp4mbh_reg94.mb_intra  && !mp4_state->hdr.h263_aic)
    {    // see 6.2.1 14496-2
        if(mp4_state->hdr.short_video_header ==1)
            mp4mbh_reg94.cbp = 0x3f;
        else if(mp4_state->hdr.use_intra_dc_vlc == 1)
            mp4mbh_reg94.cbp = 0x3f;
        if(mp4_state->userdata_codec_version == 311)
            mp4mbh_reg94.cbp = 0x3f;
    }

    mp4qcinput_reg84.quant_scale = mp4_state->hdr.quantizer;

    if(!((mp4_state->hdr.derived_mb_type == STUFFING)&&(mp4_state->hdr.prediction_type != MP4_B_VOP)))
    {
        if(mp4_check_mc_free())
        {    // Video Engine free
            // MB address
            pdwTemp = (uint32_t*)(&mp4mbaddr_reg10);
            vdec_writeuint32(MBADDR_REG,*pdwTemp);
            // MB information
            pdwTemp = (uint32_t*)(&mp4mbh_reg94);
            vdec_writeuint32(MBH_REG,*pdwTemp);

            // 8 MVs
            if(mp4_state->hdr.prediction_type == MP4_P_VOP || mp4_state->hdr.prediction_type == MP4_S_VOP)
            {
                if(!mp4mbh_reg94.mb_intra)
                {
                    pdwTemp = (uint32_t*)(&mp4mv1_reg98);
                    vdec_writeuint32(MV1_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv2_reg9c);
                    vdec_writeuint32(MV2_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv3_rega0);
                    vdec_writeuint32(MV3_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv4_rega4);
                    vdec_writeuint32(MV4_REG,*pdwTemp);
                }
            }
            else if(mp4_state->hdr.prediction_type == MP4_B_VOP)
            {
                pdwTemp = (uint32_t*)(&mp4mv1_reg98);
                vdec_writeuint32(MV1_REG,*pdwTemp);
                pdwTemp = (uint32_t*)(&mp4mv2_reg9c);
                vdec_writeuint32(MV2_REG,*pdwTemp);
                pdwTemp = (uint32_t*)(&mp4mv3_rega0);
                vdec_writeuint32(MV3_REG,*pdwTemp);
                pdwTemp = (uint32_t*)(&mp4mv4_rega4);
                vdec_writeuint32(MV4_REG,*pdwTemp);
                if(mp4mbh_reg94.mb_direct)
                {
                    pdwTemp = (uint32_t*)(&mp4mv5_rega8);
                    vdec_writeuint32(MV5_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv6_regac);
                    vdec_writeuint32(MV6_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv7_regb0);
                    vdec_writeuint32(MV7_REG,*pdwTemp);
                    pdwTemp = (uint32_t*)(&mp4mv8_regb4);
                    vdec_writeuint32(MV8_REG,*pdwTemp);
                }
            }
            // quantizer scale code
            pdwTemp = (uint32_t*)(&mp4qcinput_reg84);
            vdec_writeuint32(QCINPUT_REG,*pdwTemp);
            // set boundary, dvix flag, chroma_format and mpeg format information
            mp4vetrigger_reg18.ve_start_type = 0x0;
            if(mp4_state->hdr.macroblok_bounday){
                mp4vetrigger_reg18.mb_boundary = 1;
                mp4_state->hdr.macroblok_bounday = 0;
            }
            else
                mp4vetrigger_reg18.mb_boundary = 0;
            pdwTemp = (uint32_t*)(&mp4vetrigger_reg18);
            vdec_writeuint32(VETRIGGER_REG,*pdwTemp);
            // start MC
            vdec_writeuint8(VETRIGGER_REG,0x7);
        }

        // start IQIS if it isn't not-coded macroblock
        if(    ((mp4_state->hdr.not_coded == 1)&&((mp4_state->hdr.prediction_type == MP4_P_VOP)||(mp4_state->hdr.prediction_type == MP4_S_VOP)))||
            ((mp4_state->hdr.prediction_type == MP4_B_VOP)&&mp4_state->hdr.b_mb_not_coded)||
            ((mp4_state->hdr.modb ==MODB_1)&&(mp4_state->hdr.prediction_type == MP4_B_VOP)))
        {
            // do nothing for NOT_CODED MB
        }
        else
        {
            // do nothing for NOT_CODED MB
        }

    }
}

// check IDCT input buffer status, reture 1 if empty, else 0
int mp4_check_idct_in_empty(void)
{
    volatile uint32_t dwVal;
    int i;

    i=0;
    vdec_readuint32(VESTAT_REG);
    while(!(dwVal & 0x4000))
    {
        i++;
        if(i > 0x200000)
        {
            return 0;
        }
        vdec_readuint32(VESTAT_REG);
    }

    return 1;
}

// check IQIS input buffer status, return 1 if empty, else 0
int mp4_check_iqis_in_empty(void)
{
    volatile uint32_t dwVal;
    int i;

    i=0;
    vdec_readuint32(VESTAT_REG);
    while(!(dwVal&0x8000))
    {
        i++;
        if(i > 0x200000)
        {
            return 0;
        }
        vdec_readuint32(VESTAT_REG);
    }

    return 1;
}


