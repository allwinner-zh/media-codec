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

/* 
*********************************************************************************************************
* Description: decoding one mpeg4 xvid frame
*
* Arguments  : pDev
*
* Returns    : 		PIC_DEC_ERR_FRMNOTDEC
*				0: one frame is decoded
*				-1: decoding error
*********************************************************************************************************
*/
int32_t mp4_set_stream_xvid(mp4_dec_ctx_t* ctx, VideoStreamDataInfo* stream)
{
    int32_t 		ret;
	MP4_STATE*	s;
	uint8_t*			pv;
	uint32_t 		len;
	MP4_STREAM*	ld;
	uint8_t*			vbv_base;
	uint8_t* 		vbv_end;
	uint32_t			vbv_size;

	s  = &ctx->s;
	ld = &s->bits;
	ret = 0;
	
	vbv_base = ctx->pStreamBufferBase;
	vbv_size = ctx->nStreamBufferSize;
	vbv_end  = vbv_base + vbv_size - 1;

    pv = (uint8_t*)stream->pData;

    mp4_initbits(ld, pv, 12, vbv_base, vbv_end);
    len = mp4_getbits(ld,8) | (mp4_getbits(ld,8)<<8) | (mp4_getbits(ld,8)<<16) | (mp4_getbits(ld,8)<<24);

	if((len+4) ==(uint32_t)ctx->data_len)
	{
    	if((pv+4) > vbv_end)
        	pv = vbv_base + (pv+3-vbv_end);    //loop back
    	else
        	pv = pv+4;
        
        ctx->stream_data_format = 0;
    }
    else
    {
    	pv = (uint8_t*)stream->pData;
    	len = ctx->data_len;
    	
        ctx->stream_data_format = 1;
    }

    mp4_initbits(ld, pv, len, vbv_base, vbv_end);

    s->hdr.gob_number = 0;

    if ((len <= 2 && *pv == 0x7f) || (0 == len))
    {
        // [prefix] stuffing chunk, indicated that the encoder is buffering
        logw("Mpeg4 decode frame failed, stuffing chunk! len=%d\n", len);

        s->not_coded_frm_cnt++;
        return -1;
    }

    s->prefixed = 0; // will avoid the delay caused by B frames

    if(1)
    {
    	int32_t parser_fps;
        mp4_getvoshdr(ld, s);
        
        if (mp4_getvsohdr(ld, s))
        {
            if (mp4_bytealign(ld) == 8) // mp4_next_start_code()
            {
                s->prefixed = 1;
            }

            
		    while ((parser_fps= mp4_showbits(ld, 24)) != 0x000001)
				mp4_getbits(ld, 8);
            while (mp4_showbits(ld, 32) == USR_START_CODE)
                mp4_getusrhdr(ld, s);
        }

        ret = mp4_getvolhdr(ld, s);
        if(ret < 0)
        {
            logw("Mpeg4 decode frame failed, get vol header failed(%d)!\n", ret);
            return -1;
        }
        parser_fps = ctx->videoStreamInfo.nFrameRate/1000;

        if(s->newfrmrate && parser_fps!=15 && parser_fps!=23 && parser_fps!=24 && parser_fps!=25 && parser_fps!=29 && parser_fps!=30
        	 && parser_fps!=59 && parser_fps!=60)
        {
            mp4_update_framerate(ctx, s->newfrmrate);
            s->newfrmrate = 0;
        }
    }


    mp4_getshvhdr(ld, s);

    if((s->hdr.data_partitioning == 1) || (s->hdr.reversible_vlc == 1))
    {
        //Not support
        logw("Mpeg4 decode frame failed, data_partitioning(%d), reversible_vlc(%d)!\n", s->hdr.data_partitioning, s->hdr.reversible_vlc);
        return -1;
    }

    if ((s->preceding_vop_coding_type == MP4_B_VOP ||
        s->preceding_vop_coding_type == MP4_S_VOP ||
        s->hdr.shape != RECTANGULAR) &&
        mp4_showbits(ld, 32) == STF_START_CODE)
    {
        mp4_getbits(ld, 32); // stuffing_start_code
        while (mp4_showbits(ld, 24) != 0x01)
            mp4_getbits(ld, 8);
    }
    mp4_getgophdr(ld, s);

    if (!s->hdr.short_video_header)
    {
        int16_t err = mp4_getvophdr(ld, s);
        if(err < 0)
        {
            logw("Mpeg4 decode frame failed, get vop header failed(%d)!\n", err);
            return -1;
        }
    }
    
    ctx->data_len = len;
  
    // [prefix]
    if (s->hdr.prediction_type == MP4_I_VOP)
    {
        s->history_prefixed = s->prefixed ? 1 : 0;
    }

    if(!s->hdr.vop_coded)
    {
        if(s->chunk_extra_frm_cnt)
            s->chunk_extra_frm_cnt--;
        else
            s->not_coded_frm_cnt++;
        
        return -1;
    }
    
    return 0;
}

int32_t mp4_decode_stream_xvid(mp4_dec_ctx_t* ctx, VideoStreamDataInfo* stream)
{
	MP4_STATE*	s;
	uint32_t 		len;
	MP4_STREAM*	ld;
    uint32_t   		start_bit_pos;
    uint32_t			bit_length;
	uint8_t*			vbv_base;
	uint8_t* 		vbv_end;
	uint32_t			vbv_size;

	s  = &ctx->s;
	ld = &s->bits;
	len = ctx->data_len;
	
	vbv_base = ctx->pStreamBufferBase;
	vbv_size = ctx->nStreamBufferSize;
	vbv_end  = vbv_base + vbv_size - 1;

    mp4_set_vop_info(ctx);

    s->hdr.packetnum = 0;
    s->hdr.mba = 0;
    ctx->hasExtraData = 0;
    
    if (!s->hdr.short_video_header)
    {
        if (!s->hdr.data_partitioning)
        {
            uint32_t hw_off;
            uint32_t next_mba;
            uint8_t* mptr;
            
Mp4NextPacketDecode:
            start_bit_pos = mp4_bitpos(ld);
            bit_length = (ld->length<<3) - start_bit_pos;
            start_bit_pos += (ld->startptr - ld->buf_start_ptr)<<3;
            if(start_bit_pos >= vbv_size<<3)
            {
                start_bit_pos -= vbv_size<<3;
            }

            if(s->hdr.resync_marker_disable == 1)
                mp4_set_vbv_info(s->hdr.mba_size, 1, start_bit_pos, bit_length, vbv_size);
            else
            {

                uint32_t uTemp, uHeadValue=0;
                int32_t l;
                uint8_t  sync_buf[6];
                
                next_mba = s->hdr.mba_size;
                mptr = (unsigned char *)ld->rdptr - 4 - ((32-ld->bitcnt)>>3);
                if(mptr < ld->buf_start_ptr)
                	mptr += vbv_size;
                for(l=0;l<4;l++)
                {
                	uHeadValue <<= 8;
                	uHeadValue |= mptr[0];
                    //sync_buf[l] = mptr[0];
                	  mptr ++;
                	  if(mptr > ld->buf_end_ptr)
                		    mptr = ld->buf_start_ptr;
                }
                l = mp4_getLeftData(ld)-4;
                while (l>=1)
                {
                  uTemp = uHeadValue;
                  uTemp >>= (32-s->hdr.resync_length);
                  if(uTemp == 1)
                  {
                      sync_buf[4] = mptr[0];
                      mptr ++;
                      if(mptr > ld->buf_end_ptr)
                          mptr = ld->buf_start_ptr;
                      sync_buf[5] = mptr[0];
                      mptr ++;
                      if(mptr > ld->buf_end_ptr)
                          mptr = ld->buf_start_ptr;

                      #if 0
                      next_mba = uHeadValue<<16;
                      uHeadValue |= (sync_buf[4]<<8) | sync_buf[5];
                      #else
                      uHeadValue <<= 16;
                      uHeadValue |= (sync_buf[4]<<8) | sync_buf[5];
                      next_mba = uHeadValue;
                      #endif
                      next_mba <<= s->hdr.resync_length-16;
                      next_mba >>= 32-s->hdr.mb_in_vop_length;
                      break;
                   }
                   //sync_buf[1] = sync_buf[2];
                   //sync_buf[2] = sync_buf[3];
                   //sync_buf[3] = mptr[0];
                   uHeadValue <<= 8;
                   uHeadValue |= mptr[0];
                   mptr ++;
                   if(mptr > ld->buf_end_ptr)
                       mptr = ld->buf_start_ptr;
                   l--;
                }
                mp4_set_vbv_info(next_mba-s->hdr.mba,1,start_bit_pos,bit_length,vbv_size);
            }

            //wait video engine decode one frame finish

        	if(AdapterVeWaitInterrupt() == 0)
        	{
        		mp4_clear_status_reg();
        	}
        	else
        	{
    			mp4_reset_ve_core(ctx);
        		mp4_set_buffer(s);
        		mp4_set_pic_size(s);
        	}

            //after one picture is decoded update the ld status.
            hw_off = mp4_get_bitoffset();
            if(hw_off&7)
                hw_off = (hw_off+7)&0xfffffff8;
            hw_off >>= 3;
            if((uint32_t)(ld->startptr - ld->buf_start_ptr) > hw_off)
            {
                hw_off += vbv_size;
            }
            ld->count = hw_off - (ld->startptr - ld->buf_start_ptr);

            while (hw_off>=vbv_size)
            {
                hw_off -= vbv_size;
            }
            ld->rdptr = ld->buf_start_ptr + hw_off;
            if((ld->length-(ld->count- 4 - ((32-ld->bitcnt)>>3)))>4)
            {
            	ctx->hasExtraData = 1;
				mp4_setld_offset(ld);
                //check whether packet mode
                if(s->hdr.resync_marker_disable == 0)
                {
                    uint32_t cur_mba;
                    //read back mba
                    cur_mba = mp4_get_mba();
                    s->hdr.mba = (cur_mba&0x7f)*s->hdr.mb_xsize + ((cur_mba>>8)&0x7f);
                    //check resyncmarker
                    if((mp4_nextbits_resync_marker(ld,s)==1) && s->hdr.mba<s->hdr.mba_size && (ld->count- 4 - ((32-ld->bitcnt)>>3))<ld->length)
                    {
            			s->hdr.macroblok_bounday = 1;
                        mp4_getpackethdr(ld,s);
                        mp4_set_packet_info(s);
                        goto Mp4NextPacketDecode;
                    }
                    else
                    {

                    }
                }
			}

            // [prefix]
			if((s->userdata_codec_version >= 500) && s->history_prefixed  && ctx->hasExtraData)
			{
			    mp4_bytealign(ld);
                while (8*ld->length - mp4_bitpos(ld) >= 32)
                {
                    if (mp4_showbits(ld, 32) == VOP_START_CODE)
                    {
                        int32_t   i = 0, tmpChunkLen;
                        uint8_t    *tmpChkHdr;

                        start_bit_pos = mp4_bitpos(ld);
                        start_bit_pos = (start_bit_pos+7)&0xfffffff8;
                        start_bit_pos >>= 3;
                        stream->pData += start_bit_pos;
                        if(stream->pData > (char*)vbv_end)
                        	stream->pData -= vbv_size;
                        
                        tmpChunkLen = len - start_bit_pos;
                        if(ctx->stream_data_format == 0)
                        {
                        	tmpChkHdr = (uint8_t*)stream->pData;
                        	for(i=0; i<4; i++)
                        	{
                        	    if(tmpChkHdr > vbv_end)
                        	    {
                        	        tmpChkHdr -= vbv_size;
                        	    }
                        	
                        	    *tmpChkHdr++ = (tmpChunkLen >> (8*i)) & 0xff;
                        	}
                        	
                        	ctx->data_len = tmpChunkLen + 4;
                        }
                        else
                        {
                        	ctx->data_len = tmpChunkLen;
                        }

                        s->chunk_extra_frm_cnt ++;

                        return 0;
                    }
            	    else 
            	    	mp4_getbits(ld, 8);
                }
            }

            //after one picture is decoded, update the vbv buffer status
            ctx->hasExtraData = 0;
            s->frame_ctr ++;
            return 0;
        }
        else
        {    //Not support
            logw("Mpeg4 decode frame failed, data_partitioning!\n");
            return -1;
        }
    }
    // short video header vop decoding
    else
    {
        int32_t i;
        for (i = 0; s->hdr.mba<s->hdr.mba_size; i++)
        {
            if(mp4_getgobhdr(ctx,i)<0)
            {
                logw("Mpeg4 decode frame failed, get vob header failed!\n");
                return -1;
            }
        }

        //after one picture is decoded, updat the vbv buffer status
        s->frame_ctr ++;
    }

    return 0;
}


void mp4_cal_picture_pts(mp4_dec_ctx_t* ctx, VideoStreamDataInfo*  stream)
{
	MP4_STATE*		 s;
	int64_t pts = 0;

	s = &ctx->s;

	if(s->hdr.prediction_type == MP4_P_VOP || s->hdr.prediction_type == MP4_S_VOP)
   	{
		if(stream->nPts==-1)
			ctx->m_cur->nPts = ctx->m_for->nPts + (s->not_coded_frm_cnt + 1) * ctx->videoStreamInfo.nFrameDuration;
		else
			ctx->m_cur->nPts = stream->nPts;
   	}
	else if(s->hdr.prediction_type == MP4_B_VOP)
	{
		if(stream->nPts==-1)
			ctx->m_bac->nPts += (s->not_coded_frm_cnt + 1) * ctx->videoStreamInfo.nFrameDuration;

		if(stream->nPts==-1)
			ctx->m_cur->nPts  = ctx->m_bac->nPts - ctx->videoStreamInfo.nFrameDuration;
		else
			ctx->m_cur->nPts = stream->nPts;
	}
	else
		ctx->m_cur->nPts = stream->nPts;

	if (s->hdr.prediction_type == MP4_B_VOP)
	{
		if(ctx->m_cur->nPts > ctx->m_bac->nPts)
       	{
			pts = ctx->m_bac->nPts;
           	ctx->m_bac->nPts = ctx->m_cur->nPts;
          	ctx->m_cur->nPts = pts;
       	}
		else if(ctx->m_cur->nPts == ctx->m_bac->nPts)
		{
			 ctx->m_bac->nPts = ctx->m_cur->nPts+ctx->videoStreamInfo.nFrameDuration;
		}
	}
}


int32_t mp4_decode_frame_xvid(mp4_dec_ctx_t* ctx, int32_t bDecodeKeyFrameOnly, int32_t skip_bframe, int64_t cur_time)
{
	uint32_t              aspect_ratio;
	VideoStreamDataInfo*  stream;
	MP4_STATE*		 s;
    int32_t       		 retVal;
    VideoPicture*		 cur;
	int64_t  pts;

    if(ctx != NULL && ctx->pSbm != NULL)
    {
    	s = &ctx->s;
    	cur = FbmRequestBuffer(ctx->pFbm);
    	if(cur==NULL)
    	{
        	//loge("mp4_decode_frame_xvid, request frame buffer fail.");
        	return VDECODE_RESULT_NO_FRAME_BUFFER;
    	}
    
_normal_get_stream_again:
		if(ctx->stream != NULL)
		{
			stream = ctx->stream;
		}
		else
		{
			stream = SbmRequestStream(ctx->pSbm);
			ctx->stream = stream;
			if(stream != NULL)
				ctx->data_len = stream->nLength;
		}
		
		if(stream == NULL)
		{
	    	//loge("mp4_decode_frame_xvid, VBV underflow.");
	    	FbmReturnBuffer(ctx->pFbm, cur, INVALID);
	    	return VDECODE_RESULT_NO_BITSTREAM;
		}

		if(stream->pData == NULL|| stream->bValid == 0)
		{
	    	loge("mp4_decode_frame_xvid, no stream data.");
	    	SbmFlushStream(ctx->pSbm, stream);
	    	ctx->stream = NULL;
	    	goto _normal_get_stream_again;
		}

		if(stream->nLength == 0)
		{
			ctx->s.not_coded_frm_cnt++;
	    	SbmFlushStream(ctx->pSbm, stream);
	    	ctx->stream = NULL;
	    	goto _normal_get_stream_again;
		}

		retVal = mp4_set_stream_xvid(ctx, stream);
		
		if (retVal != 0)
		{
	    	//loge("mp4_decode_frame_xvid, decode slice header fail.");
	    	SbmFlushStream(ctx->pSbm, stream);
	    	ctx->stream = NULL;
	    	goto _normal_get_stream_again;
		}

		if(s->hdr.prediction_type != MP4_I_VOP && (bDecodeKeyFrameOnly==1 || ctx->need_key_frame))
		{
	    	//loge("mp4_decode_frame_xvid, decode keyframe only but this is not a keyframe.");
	    	SbmFlushStream(ctx->pSbm, stream);
	    	ctx->stream = NULL;
	    	goto _normal_get_stream_again;
		}
	    
		if (s->hdr.prediction_type == MP4_B_VOP)
		{
		    uint32_t ref_cnt;

		    //* check whether B frame has two reference frames.
		    ref_cnt = (ctx->m_for!=NULL) + (ctx->m_bac!=NULL) + (ctx->m_cur!=NULL);
		    if (ref_cnt < 2)
		    {
		        loge("B frame has not enough reference.");
		        SbmFlushStream(ctx->pSbm, stream);
		        ctx->stream = NULL;
		    	s->not_coded_frm_cnt++;
		        goto _normal_get_stream_again;
		    }
		
		    //* set B frame pts.
		    if(stream->nPts == (int64_t)-1)
		    {
				pts = ctx->m_for->nPts + (s->not_coded_frm_cnt + 1) * ctx->videoStreamInfo.nFrameDuration;
			}
			else
			{
				pts = stream->nPts;
			}
		
		    //* check whether to skip b frame.
		    if(skip_bframe)
		    {
		        if ((int64_t)pts < cur_time)
		        {
		            loge("skip B frame stream.");
		            SbmFlushStream(ctx->pSbm, stream);
		    		ctx->stream = NULL;
		    		s->not_coded_frm_cnt++;
		            goto _normal_get_stream_again;
		        }
		    }
		}
	    
	    if (s->hdr.prediction_type == MP4_I_VOP ||
	        (s->hdr.prediction_type == MP4_P_VOP) ||
	        s->hdr.prediction_type == MP4_S_VOP)
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

	    //* do decoding stream.
		mp4_decode_stream_xvid(ctx, stream);

	    //* set picture information
	    mp4_get_pic_size(ctx->m_cur, ctx);

	 	mp4_cal_picture_pts(ctx,stream);

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
	    
		s->not_coded_frm_cnt = 0;

		if(ctx->hasExtraData)
		{
			ctx->stream = stream;
			ctx->stream->nPts = -1;
		}
		else
		{
	    	SbmFlushStream(ctx->pSbm, stream);
	    	ctx->stream = NULL;
	    }
	    
	    ctx->need_key_frame = 0;
	    
	    if (s->hdr.prediction_type == MP4_I_VOP)
	        return VDECODE_RESULT_KEYFRAME_DECODED;
	    else
        	return VDECODE_RESULT_FRAME_DECODED;
		
        }
	    return VDECODE_RESULT_UNSUPPORTED;
}
