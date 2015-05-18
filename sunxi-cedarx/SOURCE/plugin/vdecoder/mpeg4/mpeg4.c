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


#define VALID   1
#define INVALID 0


int  Mp4DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo);
static void Mp4DecoderReset(DecoderInterface* pSelf);
static int  Mp4DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex);
static int  Mp4DecoderGetFbmNum(DecoderInterface* pSelf);
static Fbm* Mp4DecoderGetFbm(DecoderInterface* pSelf, int nIndex);
static int  Mp4DecoderDecode(DecoderInterface* pSelf, 
                            int               bEndOfStream, 
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay, 
                            int64_t           nCurrentTimeUs);
static int32_t mp4_parse_init_info(mp4_dec_ctx_t* p, uint8_t* data, uint32_t data_len);
static void mp4_flush_pictures(mp4_dec_ctx_t* mp4_context);

static void Destroy(DecoderInterface* pSelf);
static int32_t mp4_parse_init_info(mp4_dec_ctx_t* p, uint8_t* data, uint32_t data_len);
int32_t mp4_create_fbm(mp4_dec_ctx_t* p, MP4_STATE*s);

//*******************************************************************//
//*********************************************************************//
DecoderInterface* CreateMpeg4Decoder(VideoEngine* p)
{
	mp4_dec_ctx_t* pMp4Context;

    pMp4Context = (mp4_dec_ctx_t*) malloc(sizeof(mp4_dec_ctx_t));
    if(pMp4Context == NULL)
        return NULL;

    memset(pMp4Context, 0, sizeof(mp4_dec_ctx_t));
    
    pMp4Context->pVideoEngine        = p;
    pMp4Context->interface.Init      = Mp4DecoderInit;
    pMp4Context->interface.Reset     = Mp4DecoderReset;
    pMp4Context->interface.SetSbm    = Mp4DecoderSetSbm;
    pMp4Context->interface.GetFbmNum = Mp4DecoderGetFbmNum;
    pMp4Context->interface.GetFbm    = Mp4DecoderGetFbm;
    pMp4Context->interface.Decode    = Mp4DecoderDecode;
    pMp4Context->interface.Destroy   = Destroy;
    
    return &pMp4Context->interface;
}

//*******************************************************************//
//*********************************************************************//
 int  Mp4DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo)
{
    int32_t      ret;
	mp4_dec_ctx_t* p;
	MP4_STATE*	   s;
	
	p = NULL;
    p = (mp4_dec_ctx_t*)pSelf;
	s = &p->s;

    mp4_init_global_variable();
	
    memcpy(&p->vconfig, pConfig, sizeof(VConfig));
    
    p->videoStreamInfo.eCodecFormat    	   	 = pVideoInfo->eCodecFormat;
    p->videoStreamInfo.nWidth          		 = pVideoInfo->nWidth;
    p->videoStreamInfo.nHeight         	 	 = pVideoInfo->nHeight;
    p->videoStreamInfo.nFrameRate       	 = pVideoInfo->nFrameRate;
    p->videoStreamInfo.nAspectRatio     	 = pVideoInfo->nAspectRatio;
    p->videoStreamInfo.nCodecSpecificDataLen = pVideoInfo->nCodecSpecificDataLen;
    p->videoStreamInfo.pCodecSpecificData    = pVideoInfo->pCodecSpecificData;

    if(p->videoStreamInfo.nWidth ==0 || p->videoStreamInfo.nHeight==0)
    {
        loge("mpeg4_open, picture size zero.");
    	ret = VDECODE_RESULT_UNSUPPORTED;
    	goto _mpeg4_open_fail;
    }

	if(p->videoStreamInfo.nAspectRatio==0)
    {
		p->videoStreamInfo.nAspectRatio = 1000;
	}

	if(mp4_create_fbm(p, s)<0)
	{
		loge("allocate buffer for nc flag fail.\n");
        ret = VDECODE_RESULT_UNSUPPORTED;
        goto _mpeg4_open_fail;
	}
    mp4_set_buffer(s);
    mp4_set_pic_size(s);
    if(mp4_parse_init_info(p, (uint8_t*)pVideoInfo->pCodecSpecificData, (uint32_t)pVideoInfo->nCodecSpecificDataLen) != 0)
    {
    	loge("mpeg4_open, parse initialize information fail.");
        ret = VDECODE_RESULT_UNSUPPORTED;
        goto _mpeg4_open_fail;
    }
    
    return VDECODE_RESULT_OK;
    
_mpeg4_open_fail:
	Destroy((DecoderInterface*) p);
	return ret;
}

//*******************************************************************//
//*********************************************************************//

static void Destroy(DecoderInterface* pSelf)
{
	mp4_dec_ctx_t* mp4_context;
	mp4_context = (mp4_dec_ctx_t*)pSelf;

    if(mp4_context != NULL)
    {
    	//mp4_flush_pictures(mp4_context);
        mp4_context->need_key_frame  = 1;
        mp4_context->first_IFrm_flag = 0;
        mp4_context->hasExtraData    = 0;
        if(mp4_context->stream && mp4_context->pSbm != NULL)
        {
            SbmFlushStream(mp4_context->pSbm, mp4_context->stream);
        	mp4_context->stream	= NULL;
        }
    	
    	if(mp4_context->s.mbh_buf != 0)
    	{
    		AdapterMemPfree((void*)mp4_context->s.mbh_buf);
    		mp4_context->s.mbh_buf = 0;
    	}
    	
    	if(mp4_context->s.fdc_qac_buf != 0)
    	{
    		AdapterMemPfree((void*)mp4_context->s.fdc_qac_buf);
    		mp4_context->s.fdc_qac_buf = 0;
    	}
    	
    	if(mp4_context->s.nc_flag_buf != 0)
    	{
    		AdapterMemPfree((void*)mp4_context->s.nc_flag_buf);
    		mp4_context->s.nc_flag_buf = 0;
    	}
    	
    	if(mp4_context->s.deblk_buf != 0)
    	{
    		AdapterMemPfree((void*)mp4_context->s.deblk_buf);
    		mp4_context->s.deblk_buf = 0;
    	}

    	if(mp4_context->s.codedmap != NULL)
    	{
    		free(mp4_context->s.codedmap);
    		mp4_context->s.codedmap = NULL;
    	}
    	
    	if(mp4_context->s.cbp_store != NULL)
    	{
    		free(mp4_context->s.cbp_store);
    		mp4_context->s.cbp_store = NULL;
    	}
    	
    	if(mp4_context->s.MV != NULL)
    	{
    		free(mp4_context->s.MV);
    		mp4_context->s.MV = NULL;
    	}
    	
    	if(mp4_context->s.MV_field != NULL)
    	{
    		free(mp4_context->s.MV_field);
    		mp4_context->s.MV_field = NULL;
    	}
    	
    	if(mp4_context->s.MVBack != NULL)
    	{
    		free(mp4_context->s.MVBack);
    		mp4_context->s.MVBack = NULL;
    	}
    	
    	if(mp4_context->s.fieldpredictedmap != NULL)
    	{
    		free(mp4_context->s.fieldpredictedmap);
    		mp4_context->s.fieldpredictedmap = NULL;
    	}
    	
    	if(mp4_context->s.fieldrefmap != NULL)
    	{
    		free(mp4_context->s.fieldrefmap);
    		mp4_context->s.fieldrefmap = NULL;
    	}
    	
    	if(mp4_context->s.data_offset != NULL)
    	{
    		free(mp4_context->s.data_offset);
    		mp4_context->s.data_offset = NULL;
    	}

        //* release frame buffer manager
        if(mp4_context->pFbm)
        {
            FbmDestroy(mp4_context->pFbm);
            mp4_context->pFbm = NULL;
        }
        free(mp4_context);
        mp4_context = NULL;
    }
    
    return;
}

//*******************************************************************//
//*********************************************************************//

static void Mp4DecoderReset(DecoderInterface* pSelf)
{
    mp4_dec_ctx_t* pMp4Context;
    
    pMp4Context = (mp4_dec_ctx_t*)pSelf;
   
    mp4_flush_pictures(pMp4Context);
    mp4_reset_ve_core(pMp4Context);
    mp4_set_buffer(&pMp4Context->s);
    mp4_set_pic_size(&pMp4Context->s);
    pMp4Context->nDecFrmNum = 0;
    pMp4Context->need_key_frame = 1;
    return;
}
//*******************************************************************//
//*********************************************************************//

static int  Mp4DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex)
{
    mp4_dec_ctx_t* mp4_context;
	mp4_context = (mp4_dec_ctx_t*)pSelf;
    
    if(nIndex != 0)
    {
        loge("multiple video stream unsupported.");
        return VDECODE_RESULT_UNSUPPORTED;
    }
    
    mp4_context->pSbm              = pSbm;
    mp4_context->pStreamBufferBase = (uint8_t*)SbmBufferAddress(pSbm);
    mp4_context->nStreamBufferSize = SbmBufferSize(pSbm);
    
    return VDECODE_RESULT_OK;
}
//*******************************************************************//
//*********************************************************************//


void mp4_reg_init(mp4_dec_ctx_t *p)
{
	MP4_STATE*	s;
	s  = &p->s;

    ResetVeInternal(p->pVideoEngine);
    mp4_init_global_variable();
	mp4_set_buffer(s);//on
	mp4_set_pic_size(s);//on
}
//*******************************************************************//
//*********************************************************************//

static Fbm* Mp4DecoderGetFbm(DecoderInterface* pSelf, int nIndex)
{
    mp4_dec_ctx_t* mp4_context;
    
    mp4_context = (mp4_dec_ctx_t*)pSelf;
    
    if(nIndex != 0)
        return NULL;
    
    return mp4_context->pFbm;
}
//*******************************************************************//
//*********************************************************************//

static int32_t mp4_parse_init_info(mp4_dec_ctx_t* p, uint8_t* data, uint32_t data_len)
{
    MP4_STATE *s;

    s = &p->s;

    if(0 == p->videoStreamInfo.nFrameDuration)
    {
        if(p->videoStreamInfo.nFrameRate)
        {
        	p->videoStreamInfo.nFrameDuration = 1000*1000*1000 / p->videoStreamInfo.nFrameRate;
        }
        else
        {
        	p->videoStreamInfo.nFrameDuration = 1000*1000*1000 / 25000;
        }
    }
    
    if((p->videoStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_XVID) && 
            data_len != 0)
    {
		MP4_STREAM* ld;
		uint8_t*			extra_data;
		uint32_t 		size;
		int32_t 		parser_fps;
		int32_t			ret;
		
		ld 		   = &s->bits;
		extra_data = data;
		size       = data_len;
		
		mp4_initbits(ld, extra_data, size, extra_data, extra_data+size-1);
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
			loge("Mpeg4 decode frame failed, get vol header failed(%d)!\n", ret);
			return ret;
		}
		
		parser_fps = p->videoStreamInfo.nFrameRate/1000;
		
		if(s->newfrmrate  && 
		   parser_fps!=15 && 
		   parser_fps!=23 && 
		   parser_fps!=24 && 
		   parser_fps!=25 && 
		   
		   parser_fps!=29 && 
		   parser_fps!=30 &&
		   parser_fps!=59 && 
		   parser_fps!=60)
		{
			mp4_update_framerate(p, s->newfrmrate);
			s->newfrmrate = 0;
		}
    }

    return 0;
}

//*******************************************************************//
//*********************************************************************//

static void mp4_flush_pictures( mp4_dec_ctx_t* mp4_context)
{
    if (mp4_context == NULL)
    	return;

    if (mp4_context->m_for)
    {
        FbmReturnBuffer(mp4_context->pFbm,mp4_context->m_for, 1);
        mp4_context->m_for = NULL;
    }
    if (mp4_context->m_cur)
    {
        FbmReturnBuffer(mp4_context->pFbm,mp4_context->m_cur, 1 );
        mp4_context->m_cur = NULL;
    }
    if (mp4_context->m_bac)
    {
        FbmReturnBuffer(mp4_context->pFbm,mp4_context->m_bac, 1 );
        mp4_context->m_bac = NULL;
    }

}
//*******************************************************************//
//*********************************************************************//

static int  Mp4DecoderGetFbmNum(DecoderInterface* pSelf)
{
    mp4_dec_ctx_t* mp4_context;
    
    mp4_context = (mp4_dec_ctx_t*)pSelf;
    
    if(mp4_context->pFbm != NULL)
    {
    	return 1;
    }
    else
    {
    	return 0;
    }
}

//*******************************************************************//
//*********************************************************************//
static int  Mp4DecoderDecode(DecoderInterface* pSelf, 
                            int               bEndOfStream, 
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay, 
                            int64_t           nCurrentTimeUs)

{
	mp4_dec_ctx_t* mp4_context;
	int ret = 0;

	mp4_context = (mp4_dec_ctx_t*)pSelf;
	
	mp4_reg_init(mp4_context);
	mp4_config_interrupt_register();
	if(mp4_context->videoStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_XVID)
	{   
            
		ret = mp4_decode_frame_xvid(mp4_context,bDecodeKeyFrameOnly,  bSkipBFrameIfDelay, nCurrentTimeUs);
	}
	else
	{
		ret =  mp4_decode_frame_h263(mp4_context, bDecodeKeyFrameOnly, bSkipBFrameIfDelay, nCurrentTimeUs);
	}

    if((bEndOfStream==1) && (ret==VDECODE_RESULT_NO_BITSTREAM))
	{
    	mp4_flush_pictures(mp4_context);
	}
    return ret;
}



//*******************************************************************************//
//*******************************************************************************//

 int mp4_malloc_buffer(MP4_STATE* s)
{

    uint32_t size_codedmap;
	uint32_t size_cbp_store;
	uint32_t size_mv;
    uint32_t size_fieldpredictedmap;
	uint32_t size_fieldrefmap;
	
	size_codedmap          = (s->hdr.mb_ysize+1) * (s->hdr.mb_xsize+1) * sizeof(int32_t);
	size_cbp_store         = (s->hdr.mb_ysize+1) * (s->hdr.mb_xsize+1) * sizeof(int16_t);
	size_mv                = 6 * (s->hdr.mb_ysize+1) * (s->hdr.mb_xsize+2) * sizeof(MotionVector);
	size_fieldpredictedmap = (s->hdr.mb_ysize+1) * (s->hdr.mb_xsize+1) * sizeof(int32_t);
	size_fieldrefmap	   = 2 * s->hdr.mb_ysize  * s->hdr.mb_xsize * sizeof(uint8_t);
	s->codedmap = (int32_t*)malloc(size_codedmap);
    if(s->codedmap == NULL)
    {
		return -1;
    }
    memset(s->codedmap, (uint8_t)-1, size_codedmap);
    
    s->cbp_store = (int16_t*)malloc(size_cbp_store);
    if(s->cbp_store == NULL)
    {
    	return -1;
    }
    memset(s->cbp_store, 0, size_cbp_store);
    
    s->MV = (MVtype)malloc(size_mv);
    if(s->MV == NULL)
    {
		return -1;
    }
    memset(s->MV, 0, size_mv);
    
    s->MV_field = (MVtype)malloc(size_mv);
    if(s->MV_field == NULL)
    {
		return -1;
    }
    memset(s->MV_field, 0, size_mv);
    
    s->MVBack = (MVtype)malloc(size_mv);
    if(s->MVBack == NULL)
    {
		return -1;
    }
    memset(s->MVBack, 0, size_mv);
    
    s->fieldpredictedmap = (int32_t*)malloc(size_fieldpredictedmap);
    if(s->fieldpredictedmap == NULL)
    {
		return -1;
    }
    memset(s->fieldpredictedmap, 0, size_fieldpredictedmap);
    
    s->fieldrefmap = (uint8_t*)malloc(size_fieldrefmap);
    if(s->fieldrefmap == NULL)
    {
		return -1;
    }
    memset(s->fieldrefmap, 0, size_fieldrefmap);
	return 0;
}

int32_t mp4_create_fbm(mp4_dec_ctx_t* p, MP4_STATE*s)
{
	uint32_t			   x;
    uint32_t            y;
    uint32_t            num_frames;
    uint32_t 		   size_mbh;
    uint32_t			   size_fdc_qac;
    uint32_t			   size_nc_flag;
    uint32_t			   size_deblk_buf;
    int32_t            nRefPixelFormat;
    int32_t            nDispPixelFormat;
    int32_t            nProgressiveFlag = 1;
    FbmCreateInfo mFbmCreateInfo;
	
	nRefPixelFormat = PIXEL_FORMAT_YUV_MB32_420;
	nDispPixelFormat = PIXEL_FORMAT_YUV_MB32_420;
	//allocate frame buffers and other buffers
    s->width 							 = p->videoStreamInfo.nWidth;
    s->height 							 = p->videoStreamInfo.nHeight;
    s->hdr.mb_xsize 					 = (s->width + 15 )>>4;
    s->hdr.mb_ysize 					 = (s->height + 15 )>>4;
    s->hdr.width 						 = s->hdr.mb_xsize<<4;
    s->hdr.height 						 = s->hdr.mb_ysize<<4;
    s->hdr.mba_size 					 = s->hdr.mb_xsize * s->hdr.mb_ysize;
    s->hdr.mb_in_vop_length 			 = log2ceil(s->hdr.mba_size);
	s->hdr.complexity_estimation_disable = 1;
	s->hdr.quant_precision 				 = 5;
	s->hdr.resync_marker_disable 		 = 1;

    x = s->hdr.mb_xsize;
    if(x&1)
        x++;
    y = s->hdr.mb_ysize;
    if(y&1)
        y++;
        
    size_mbh     = 128*y*16;
    size_fdc_qac = x*64;
    size_nc_flag = y*16;
    size_deblk_buf = (s->width + 31) & ~31;
    size_deblk_buf *= 8;

    if(s->width > 2048)
    {
    	size_mbh <<= 1;
        size_nc_flag <<= 1;
        size_deblk_buf <<= 1;
    }

    
    if(p->videoStreamInfo.eCodecFormat == VIDEO_CODEC_FORMAT_H263)
    {   
        s->isH263 = 1;
		if(mp4_malloc_buffer(s)<0)
		{
			return -1;
		}
    }
    
	num_frames = 10;

    p->pFbm  = NULL;
 
    memset(&mFbmCreateInfo, 0, sizeof(FbmCreateInfo));
    mFbmCreateInfo.nFrameNum          = num_frames;
    mFbmCreateInfo.nWidth             = s->width;
    mFbmCreateInfo.nHeight            = s->height;
    mFbmCreateInfo.ePixelFormat       = nRefPixelFormat;
    mFbmCreateInfo.bProgressiveFlag   = nProgressiveFlag;

    p->pFbm = FbmCreate(&mFbmCreateInfo);

    if(p->pFbm == NULL)
    {
        return -1;
    }
    
    //* allocate other buffers for decoding.
    s->mbh_buf = (uint32_t)AdapterMemPalloc(size_mbh);
    if(s->mbh_buf == 0)
    {
    	return -1;
    }

    //* allocate fdc/qac buffer
    s->fdc_qac_buf = (uint32_t)AdapterMemPalloc(x*64);
    if(s->fdc_qac_buf == 0)
    {
    	return -1;
    }

    //* allocate nc_flag buffer
    s->nc_flag_buf = (uint32_t)AdapterMemPalloc(size_nc_flag);
    if(s->nc_flag_buf == 0)
    {
		return -1;
    }
	s->frame_ctr = 0;
    s->not_coded_frm_cnt = 0;
    s->chunk_extra_frm_cnt = 0;

    //initial mp4_state vars
    s->codedmap_stride 			= s->hdr.mb_xsize +1;
    s->modemap_stride 			= s->hdr.mb_xsize +2;
    s->cbp_store_stride 		= s->hdr.mb_xsize +1;
    s->quant_store_stride 		= s->hdr.mb_xsize +1;
    s->MV_stride 				= s->hdr.mb_xsize +2;
    s->fieldpredictedmap_stride = s->hdr.mb_xsize +1;
    s->fieldrefmap_stride 		= 2 * s->hdr.mb_xsize;
	
    p->need_key_frame = 1;
	s->sw_vld    = 0;
	return 0;
}

void CedarPluginVDInit(void)
{
    int ret;
    ret = VDecoderRegister(VIDEO_CODEC_FORMAT_XVID, "xvid", CreateMpeg4Decoder);
    if (0 == ret)
    {
        logi("register xvid decoder success!");
    }
    else
    {
        loge("register xvid decoder failure!!!");
    }
    
    ret = VDecoderRegister(VIDEO_CODEC_FORMAT_H263, "h263", CreateMpeg4Decoder);
    if (0 == ret)
    {
        logi("register h263 decoder success!");
    }
    else
    {
        loge("register h263 decoder failure!!!");
    }

    return ;
}

