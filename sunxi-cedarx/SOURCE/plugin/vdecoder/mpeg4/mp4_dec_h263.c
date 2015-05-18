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

int32_t mp4_get_gobhdr_h263(mp4_dec_ctx_t* ctx)
{
	MP4_STATE*  mp4_state;
    MP4_STREAM*	ld;
    uint32_t       	start_bit_pos;
    uint32_t       	end_bit_pos;
    uint32_t       	bit_length;
    uint32_t       	i;
    
    mp4_state = &ctx->s;

    ld = &mp4_state->bits;
    if (mp4_state->hdr.gob_number != 0)
  	    mp4_state->hdr.macroblok_bounday = 0;
	else
        mp4_state->hdr.macroblok_bounday = 1;
        
    if (mp4_state->hdr.gob_number != 0)
    {
        //* read gob header;
        if (mp4_nextbits(ld, 17) == RESYNC_MARKER)
        {
            if (mp4_getbits(ld, 17) != RESYNC_MARKER)
                return -1; // resync marker not found!

            mp4_state->hdr.gob_number        = mp4_getbits(ld, 5);
            mp4_state->hdr.gob_frame_id      = mp4_getbits(ld, 2);
            mp4_state->hdr.vop_quant         = mp4_getbits(ld, 5);
            mp4_state->hdr.quantizer         = mp4_state->hdr.vop_quant;
            mp4_state->hdr.quant_scale       = mp4_state->hdr.vop_quant;
            mp4_state->hdr.use_intra_dc_vlc  = mp4_get_use_intra_dc_vlc(mp4_state->hdr.quantizer, mp4_state->hdr.intra_dc_vlc_thr);
            mp4_state->hdr.macroblok_bounday = 1;
        }

    }

    mp4_set_pic_size(mp4_state);
    start_bit_pos = mp4_bitpos(ld);

    if(mp4_state->hdr.prediction_type == MP4_B_VOP)
    {
        i= (uint32_t)mp4_state->hdr.mba;
        for ( ;i < (uint32_t)mp4_state->hdr.mba_size; i++)
        {
            if(mp4_macroblock_h263_bvop(mp4_state)<0)
                break;

            mp4_state->hdr.mba++;
        }
    }
    else
    {
        mp4_set_packet_info(mp4_state);

        start_bit_pos   = mp4_bitpos(ld);
        bit_length      = (ld->length<<3) - start_bit_pos;
        start_bit_pos   += (ld->startptr - ld->buf_start_ptr)<<3;

        mp4_set_vbv_info(
                         mp4_state->hdr.num_mb_in_gob,
                         mp4_state->hdr.macroblok_bounday,
                         start_bit_pos,
                         bit_length,
                         ctx->nStreamBufferSize);

        //video engine busy to ready isr
        //WaitVeReady();
        if(AdapterVeWaitInterrupt() == 0)
        {
        	mp4_clear_status_reg();
        }
        else
        {
    		mp4_reset_ve_core(ctx);
        	mp4_set_buffer(mp4_state);
        	mp4_set_pic_size(mp4_state);
            return -1;
        }

        //after finish, get mba_addr, and bit offset
        mp4_state->hdr.mba      += mp4_state->hdr.num_mb_in_gob;
        mp4_state->hdr.mb_xpos  =  mp4_state->hdr.mba % mp4_state->hdr.mb_xsize;
        mp4_state->hdr.mb_ypos  =  mp4_state->hdr.mba / mp4_state->hdr.mb_xsize;

        //reset bits info
        end_bit_pos = mp4_get_bitoffset();
        mp4_reset_sw_bits_status(ld,end_bit_pos);
    }

    if ((mp4_nextbits(ld, 17) != RESYNC_MARKER) &&
        (mp4_nextbits_bytealigned(ld, 17, 1, NULL) == RESYNC_MARKER))
    {
        mp4_bytealign(ld);
    }
    mp4_state->hdr.gob_number++;

    return 0;
}


void init_platform_vld(MP4_STATE * mp4_state)
{
    if(mp4_state->hdr.h263_aic  && mp4_state->hdr.prediction_type == MP4_I_VOP)
        mp4_state->vld_intra_fun = mp4_vld_intra_aic_dct;
        
    if(mp4_state->hdr.iModifiedQantization)
        mp4_state->vld_inter_fun = mp4_vld_inter_mq_dct;
    else
        mp4_state->vld_inter_fun = mp4_vld_inter_dct;
}


int32_t mp4_set_stream_h263(mp4_dec_ctx_t* ctx, VideoStreamDataInfo* stream)
{
	MP4_STATE*	s;
	uint8_t*			pv;
	uint32_t 		len;
	MP4_STREAM*	ld;
    int32_t 		retVal;
	uint8_t*			vbv_base;
	uint8_t* 		vbv_end;
	uint32_t			vbv_size;

	s  = &ctx->s;
	ld = &s->bits;
	
	vbv_base = ctx->pStreamBufferBase;
	vbv_size = ctx->nStreamBufferSize;
	vbv_end  = vbv_base + vbv_size - 1;

    pv = (uint8_t*)stream->pData;

    len = *pv++;
    if (pv > vbv_end)
        pv -= vbv_size;
    len |= (*pv++)<<8;
    if (pv > vbv_end)
        pv -= vbv_size;
    len |= (*pv++)<<16;
    if (pv > vbv_end)
        pv -= vbv_size;
    len |= (*pv++)<<24;
    if (pv > vbv_end)
        pv -= vbv_size;

	if((len+4) != (uint32_t)stream->nLength)
	{
		pv = (uint8_t*)stream->pData;
		len =(uint32_t)stream->nLength;
	}
	
    s->valid_data_offset = pv - vbv_base;
    s->packet_length = len;
    mp4_initbits(ld, pv, len, vbv_base, vbv_end);

    s->isH263 = 2;    //* 263 stream;

    retVal = mp4_get_h263_pic_hdr(ld,s);
	if(retVal != 0)
	{
	    if(retVal == 1) //* frame type not support, skip it;
			loge("mp4_set_stream_h263, frame type not support.");
		return -1;
	}

	init_platform_vld(s);

	if (s->hdr.picture_coding_type == 0 || s->hdr.picture_coding_type == 1)
	{
		s->hdr.prediction_type      = MP4_I_VOP;
		s->hdr.picture_coding_type  = MP4_I_VOP;
		s->flag_keyframe            = 1;
		s->flag_disposable          = 0;
		s->sw_vld                   = 0;
	}
	else if (s->hdr.picture_coding_type == 2)
	{
		s->hdr.picture_coding_type  = MP4_P_VOP;
		s->hdr.prediction_type      = MP4_P_VOP;
		s->flag_keyframe            = 0;
		s->flag_disposable          = 0;
		s->sw_vld                   = 0;
		if(s->hdr.h263_aic)
			s->hdr.h263_aic = 0;
	}
	else if (s->hdr.picture_coding_type == 3)
	{
		s->hdr.picture_coding_type  = MP4_B_VOP;
		s->hdr.prediction_type      = MP4_B_VOP;
		s->flag_keyframe            = 0;
		s->flag_disposable          = 1;
		s->sw_vld                   = 1;
		if(s->hdr.h263_aic)
			s->hdr.h263_aic = 0;
	}
	
	ctx->data_len = len;
	
	return 0;
}

int32_t mp4_decode_stream_h263(mp4_dec_ctx_t* ctx)
{
	MP4_STATE*	s;
    uint32_t   		i;
    int32_t         err;

	s  = &ctx->s;
	
	mp4_set_vop_info(ctx);

	s->hdr.gob_number   = 0;
	s->hdr.mba          = 0;
	s->hdr.mb_xpos      = 0;
	s->hdr.mb_ypos      = 0;

    err = 0;
	memset(&(s->edge_info), 0, sizeof(s->edge_info));
	for (i = 0; s->hdr.mba<s->hdr.mba_size; i++)
	{
		if(s->hdr.num_gobs_in_vop == 1)
			s->hdr.GobEdge.iAbove = s->hdr.mb_xsize;

		if(mp4_get_gobhdr_h263(ctx)<0)
		{
            err = -1;
            break;
		}
	}

	s->frame_ctr ++;
	s->multi_frame_data = 0;
	s->multi_frame 		= 0;
	
	if(s->hdr.prediction_type != MP4_B_VOP)
		ctx->bframe_num = 0;
	else
		ctx->bframe_num++;
	
	return err;
}


int32_t mp4_decode_frame_h263(mp4_dec_ctx_t* ctx, int32_t bDecodeKeyFrameOnly, int32_t skip_bframe, int64_t cur_time)
{
	uint32_t              aspect_ratio;
	VideoStreamDataInfo*  stream;
    int32_t       		 retVal;
	MP4_STATE*		 s;
    VideoPicture*		 cur;
	int64_t pts = 0;
    
    stream = NULL;
    cur = NULL;
    
    if(ctx != NULL && ctx->pSbm != NULL)
    {
    	s = &ctx->s;
		if(FbmEmptyBufferNum(ctx->pFbm) == 0)
		{
			return VDECODE_RESULT_NO_FRAME_BUFFER;
		}
        
_h263_get_stream_again:
    	stream = SbmRequestStream(ctx->pSbm);
    	if(stream == NULL)
    	{
    		logw("mp4_decode_frame_rxg2, VBV underflow.");
        	return VDECODE_RESULT_NO_BITSTREAM;
    	}

    	if(stream->pData == NULL || stream->nLength == 0 || stream->bValid == 0)
    	{
    		logw("mp4_decode_frame_rxg2, no stream data.");
        	SbmFlushStream(ctx->pSbm, stream);
        	goto _h263_get_stream_again;
    	}

    	cur = FbmRequestBuffer(ctx->pFbm);
    	if(cur == NULL)
    	{
    		logw("mp4_decode_frame_rxg2, request frame buffer fail.");
    	    SbmReturnStream(ctx->pSbm, stream);
    		return VDECODE_RESULT_NO_FRAME_BUFFER;
    	}


    	retVal = mp4_set_stream_h263(ctx, stream);

    	
    	if (retVal != 0)
    	{
    	    SbmFlushStream(ctx->pSbm, stream);
    	    FbmReturnBuffer(ctx->pFbm, cur, INVALID);
    	    goto _h263_get_stream_again;
    	}
    	

   		if(s->hdr.prediction_type != MP4_I_VOP &&  (bDecodeKeyFrameOnly||ctx->need_key_frame))
   		{
   		    SbmFlushStream(ctx->pSbm, stream);
   		    FbmReturnBuffer(ctx->pFbm, cur, INVALID);
   		    goto _h263_get_stream_again;
   		}

    	if (s->hdr.prediction_type == MP4_B_VOP)
    	{
    	    uint32_t ref_cnt;
    	
    	    //* check whether B frame has two reference frames.
    	    ref_cnt = (ctx->m_for!=NULL) + (ctx->m_bac!=NULL) + (ctx->m_cur!=NULL);
    	    if (ref_cnt < 2)
    	    {
    	    	logw("B frame has not enough reference.");
    	        SbmFlushStream(ctx->pSbm, stream);
    	        FbmReturnBuffer(ctx->pFbm, cur, INVALID);
    	        goto _h263_get_stream_again;
    	    }
    	
    	    //* set B frame pts.
    	    if(stream->nPts == (int64_t)-1)
    			stream->nPts = ctx->m_for->nPts + (ctx->bframe_num + 1) * ctx->videoStreamInfo.nFrameDuration;
    	
    	    //* check whether to skip b frame.
    	    if (skip_bframe)
    	    {
    	        if (stream->nPts < (int64_t)cur_time)
    	        {
    	        	logw("skip B frame stream.");
    	            SbmFlushStream(ctx->pSbm, stream);
    	            FbmReturnBuffer(ctx->pFbm, cur, INVALID);
    	    		ctx->bframe_num++;
    	            goto _h263_get_stream_again;
    	        }
    	    }
    	}
    	
        if (s->hdr.prediction_type == MP4_I_VOP || s->hdr.prediction_type == MP4_P_VOP || s->hdr.prediction_type == MP4_S_VOP)
        {
            if (ctx->m_bac)
            {
                FbmReturnBuffer( ctx->pFbm, ctx->m_for, VALID);
                ctx->m_for = ctx->m_bac;
                ctx->m_bac = NULL;
                FbmShareBuffer(ctx->pFbm, ctx->m_for);
            }
            else
            {
                if(ctx->m_for != NULL)
                {
                    FbmReturnBuffer( ctx->pFbm, ctx->m_for, VALID);
                    ctx->m_for = ctx->m_cur;
                    FbmShareBuffer(ctx->pFbm, ctx->m_for);
                }
                else
                {
                    ctx->m_for = ctx->m_cur;
                }
            }
        }
        else
        {
            if (ctx->m_bac == NULL)
                ctx->m_bac = ctx->m_cur;
        }
        
        ctx->m_cur = cur;

    	mp4_decode_stream_h263(ctx);

        mp4_get_pic_size(ctx->m_cur, ctx);
        ctx->m_cur->nPts = stream->nPts;
        if(s->hdr.prediction_type == MP4_B_VOP)
        {
        	if(ctx->m_cur->nPts > ctx->m_bac->nPts)
        	{
        		pts = ctx->m_bac->nPts;
        		ctx->m_bac->nPts = ctx->m_cur->nPts;
        		ctx->m_cur->nPts = pts;
        	}
        	else if(ctx->m_cur->nPts == ctx->m_bac->nPts)
        	{
        		ctx->m_bac->nPts += ctx->videoStreamInfo.nFrameDuration;
        	}
        }
        ctx->m_cur->nPcr = stream->nPcr;
        if(ctx->m_cur->nWidth == 0 || ctx->videoStreamInfo.nHeight == 0)
        	aspect_ratio = 1000;
        else
        {
        	aspect_ratio = ctx->videoStreamInfo.nWidth * 1000 / ctx->m_cur->nWidth;
        	aspect_ratio = aspect_ratio * ctx->m_cur->nHeight / ctx->videoStreamInfo.nHeight;
        }
        ctx->m_cur->nAspectRatio = aspect_ratio;
        
        if (s->hdr.prediction_type == MP4_B_VOP)
        {
            FbmReturnBuffer(ctx->pFbm, ctx->m_cur, VALID);
            ctx->m_cur = NULL;
        }
        else if(ctx->m_for == NULL)
        {
             FbmShareBuffer(ctx->pFbm, ctx->m_cur);
        }

    	SbmFlushStream(ctx->pSbm, stream);
    
    	ctx->need_key_frame = 0;
    
   	 	if(s->hdr.prediction_type == MP4_I_VOP)
      	  	return VDECODE_RESULT_KEYFRAME_DECODED;
   	 	else
        	return VDECODE_RESULT_FRAME_DECODED;
	}
    return VDECODE_RESULT_UNSUPPORTED;
}




