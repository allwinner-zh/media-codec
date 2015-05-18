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

#include "h264_dec.h"
#include "h264_hal.h"

extern uint32_t H264GetUeGolomb(H264DecCtx* h264DecCtx);
extern int32_t H264GetSeGolomb(H264DecCtx* h264DecCtx);
extern uint32_t H264GetBits(H264DecCtx* h264DecCtx, uint8_t len);
extern uint32_t H264ShowBits(H264DecCtx *h264DecCtx, uint8_t len);
extern int32_t H264DecodeRefPicListReordering(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern int32_t H264FillDefaultRefList(H264Context* hCtx, uint8_t nDecStreamIndex);
extern int32_t H264DecodeRefPicMarking(H264DecCtx* h264DecCtx, H264Context* hCtx);

extern void H264ConfigureSliceRegister(H264DecCtx *h264DecCtx, H264Context* hCtx, uint8_t decStreamIndex);
extern void H264CongigureWeightTableRegisters(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern void H264exchangeValues(void* param1,void* param2);
extern void H264DisableStartcodeDetect(H264DecCtx *h264DecCtx);
extern void H264EnableStartcodeDetect(H264DecCtx *h264DecCtx);
extern uint32_t H264GetbitOffset(H264DecCtx* h264DecCtx, uint8_t offsetMode);
extern void H264FlushDelayedPictures(H264Context* hCtx);



static const uint8_t default_scaling4[2][16]=
{
    {
        6,13,20,28,
        13,20,28,32,
        20,28,32,37,
        28,32,37,42
   },
	  {
        10,14,20,24,
        14,20,24,27,
        20,24,27,30,
        24,27,30,34
    }
	};

static const uint8_t default_scaling8[2][64]=
{
    {
        6,10,13,16,18,23,25,27,
        10,11,16,18,23,25,27,29,
        13,16,18,23,25,27,29,31,
        16,18,23,25,27,29,31,33,
        18,23,25,27,29,31,33,36,
        23,25,27,29,31,33,36,38,
        25,27,29,31,33,36,38,40,
        27,29,31,33,36,38,40,42
    },
    {
        9,13,15,17,19,21,22,24,
        13,13,17,19,21,22,24,25,
        15,17,19,21,22,24,25,27,
        17,19,21,22,24,25,27,28,
        19,21,22,24,25,27,28,30,
        21,22,24,25,27,28,30,32,
        22,24,25,27,28,30,32,33,
        24,25,27,28,30,32,33,35
    }
	};

static const uint8_t zigzag_scan[16]=
{
    0+0*4, 1+0*4, 0+1*4, 0+2*4,
    1+1*4, 2+0*4, 3+0*4, 2+1*4,
    1+2*4, 0+3*4, 1+3*4, 2+2*4,
    3+1*4, 3+2*4, 2+3*4, 3+3*4,
};

static const uint8_t zigzag_scan8x8[64]=
{
    0+0*8, 1+0*8, 0+1*8, 0+2*8,
    1+1*8, 2+0*8, 3+0*8, 2+1*8,
    1+2*8, 0+3*8, 0+4*8, 1+3*8,
    2+2*8, 3+1*8, 4+0*8, 5+0*8,
    4+1*8, 3+2*8, 2+3*8, 1+4*8,
    0+5*8, 0+6*8, 1+5*8, 2+4*8,
    3+3*8, 4+2*8, 5+1*8, 6+0*8,
    7+0*8, 6+1*8, 5+2*8, 4+3*8,
    3+4*8, 2+5*8, 1+6*8, 0+7*8,
    1+7*8, 2+6*8, 3+5*8, 4+4*8,
    5+3*8, 6+2*8, 7+1*8, 7+2*8,
    6+3*8, 5+4*8, 4+5*8, 3+6*8,
    2+7*8, 3+7*8, 4+6*8, 5+5*8,
    6+4*8, 7+3*8, 7+4*8, 6+5*8,
    5+6*8, 4+7*8, 5+7*8, 6+6*8,
    7+5*8, 7+6*8, 6+7*8, 7+7*8,
};

//**************************************************************************************//
//**************************************************************************************//
/**
 * Returns and optionally allocates SPS / PPS structures in the supplied array 'vec'
 */
static void *H264AllocParameterSet(void **vec, const uint32_t id, const uint32_t maxCount, const uint32_t size)
               
{
    if(id >= maxCount)
    {  
        return NULL;
    }

    if(vec[id] == NULL)
    {
        vec[id] = malloc(size);
        memset(vec[id], 0, size);
    }
    return vec[id];
}

static void H264DecodeScalingList(H264DecCtx* h264DecCtx, uint8_t *factors, int32_t size,
                                const uint8_t *jvt_list, const uint8_t *fallback_list)
{
    int32_t i = 0;
    int32_t last = 8;
    int32_t next = 8;
    	
    const uint8_t *scan = (size==16)? zigzag_scan : zigzag_scan8x8;
    if(!H264GetBits(h264DecCtx, 1)) /* matrix not written, we use the predicted one */
    {
        memcpy((void*)factors, (void*)fallback_list, (uint32_t)(size*sizeof(uint8_t)));
    }
    else
    {
        for(i=0;i<size;i++)
        {
            if(next)
            {	
                next = (last + H264GetSeGolomb(h264DecCtx)) & 0xff;
            }
            if(!i && !next)
            { /* matrix not written, we use the preset one */
                memcpy((void*)factors, (void*)jvt_list, (uint32_t)(size*sizeof(uint8_t)));
                break;
            }
            last = factors[scan[i]] = next ? next : last;
        }
    }
}


static void H264DecodeScalingMatrices(H264DecCtx* h264DecCtx, H264SpsInfo *pSps, H264PpsInfo *pPps, int isSps,
								                                	uint8_t (*scaling_matrix4)[16], uint8_t (*scaling_matrix8)[64])
{   
    int32_t fallbackSps = !isSps && pSps->bScalingMatrixPresent;
    const uint8_t *fallback[4] = {
                            fallbackSps? pSps->nScalingMatrix4[0] : default_scaling4[0],
                            fallbackSps? pSps->nScalingMatrix4[3] : default_scaling4[1],
                            fallbackSps? pSps->nScalingMatrix8[0] : default_scaling8[0],
                            fallbackSps? pSps->nScalingMatrix8[1] : default_scaling8[1]
                            };

    if(H264GetBits(h264DecCtx, 1))
    {   
        if(pPps!= NULL)
        {
            pPps->bScalingMatrixPresent = 1;
        }
        pSps->bScalingMatrixPresent |= isSps;
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[0],16,default_scaling4[0],fallback[0]); // Intra, Y
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[1],16,default_scaling4[0],scaling_matrix4[0]); // Intra, Cr
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[2],16,default_scaling4[0],scaling_matrix4[1]); // Intra, Cb
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[3],16,default_scaling4[1],fallback[1]); // Inter, Y
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[4],16,default_scaling4[1],scaling_matrix4[3]); // Inter, Cr
        H264DecodeScalingList(h264DecCtx,scaling_matrix4[5],16,default_scaling4[1],scaling_matrix4[4]); // Inter, Cb
     
        if(isSps || pPps->bTransform8x8Mode)
        {
            H264DecodeScalingList(h264DecCtx,scaling_matrix8[0],64,default_scaling8[0],fallback[2]);  // Intra, Y
            H264DecodeScalingList(h264DecCtx,scaling_matrix8[1],64,default_scaling8[1],fallback[3]);  // Inter, Y
        }
    } 
    else if(fallbackSps)
    {
        memcpy((void*)scaling_matrix4, (void*)pSps->nScalingMatrix4, 6*16*sizeof(uint8_t));
        memcpy((void*)scaling_matrix8, (void*)pSps->nScalingMatrix8, 2*64*sizeof(uint8_t));
    }
}
//*********************************************************************************************//
//*********************************************************************************************//

static inline void H264DecodeHrdParameters(H264DecCtx* h264DecCtx, H264SpsInfo* pSpsInf)
{
    int32_t cpbCount = 0;
    int32_t i = 0;
    	
    cpbCount = H264GetUeGolomb(h264DecCtx) + 1;
    H264GetBits(h264DecCtx, 4);         /* bit_rate_scale */
    H264GetBits(h264DecCtx, 4);         /* cpb_size_scale */
        
    for(i=0; i<cpbCount; i++)
    {
        H264GetUeGolomb(h264DecCtx); /* bit_rate_value_minus1 */
        H264GetUeGolomb(h264DecCtx); /* cpb_size_value_minus1 */
        H264GetBits(h264DecCtx, 1);     /* cbr_flag */
    }
    H264GetBits(h264DecCtx, 5);      /* initial_cpb_removal_delay_length_minus1 */
    pSpsInf->nCpbRemovalDelayLength = H264GetBits(h264DecCtx, 5)+1;      /* cpb_removal_delay_length_minus1 */
    pSpsInf->nDpbOutputDelayLength  = H264GetBits(h264DecCtx, 5)+1;     /* dpb_output_delay_length_minus1 */
    H264GetBits(h264DecCtx, 5);     /* time_offset_length */
}
int32_t H264DecVuiParameters(H264DecCtx* h264DecCtx, H264Context* hCtx, H264SpsInfo* pSpsInf)
{   
    uint32_t vuiSarWidth = 0;
    uint32_t vuiSarHeight = 0;
    uint32_t timeScale = 0;
    uint32_t numUnitsInTick = 0;
    uint8_t vuiAspectRatioIdc =0;
    uint32_t nNumReorderFrames = 0;
     
    
    if(H264GetBits(h264DecCtx, 1) == 1)   // vui aspect ration info present flag
    {
        vuiAspectRatioIdc = H264GetBits(h264DecCtx, 8);

        if(vuiAspectRatioIdc == 255)
        {
            vuiSarWidth = H264GetBits(h264DecCtx, 16);
            vuiSarHeight = H264GetBits(h264DecCtx, 16);
            if(vuiSarHeight != 0)
            {
                pSpsInf->nAspectRatio = vuiSarWidth*1000/vuiSarHeight;
            }
        }
    }

    if(H264GetBits(h264DecCtx, 1))   // overscan info present flag
    {
        H264GetBits(h264DecCtx, 1);   // overscan appropriate flag
    }

    if(H264GetBits(h264DecCtx, 1))    // video signal type present flag
    {
        H264GetBits(h264DecCtx, 3);   // video format
        H264GetBits(h264DecCtx, 1);   // video full range flag
        if(H264GetBits(h264DecCtx, 1))      /* colour_description_present_flag */
        {
        	H264GetBits(h264DecCtx, 8); /* colour_primaries */
            H264GetBits(h264DecCtx, 8); /* transfer_characteristics */
            H264GetBits(h264DecCtx, 8); /* matrix_coefficients */
        }
    }

    if(H264GetBits(h264DecCtx, 1))   /* chroma_location_info_present_flag */
    {     
        H264GetUeGolomb(h264DecCtx);  /* chroma_sample_location_type_top_field */
        H264GetUeGolomb(h264DecCtx);  /* chroma_sample_location_type_bottom_field */
    }

    if(H264GetBits(h264DecCtx, 1))    // time info present flag
    {
        numUnitsInTick = H264GetBits(h264DecCtx, 32);  //num_units_in_tick
    	//numUnitsInTick = H264GetBitsLong(h264DecCtx, 32);

        timeScale = H264GetBits(h264DecCtx, 32);  //time_scale
        hCtx->vbvInfo.nFrameRate = timeScale/(numUnitsInTick*2);
        
        if(hCtx->vbvInfo.nFrameRate != 0)
        {
            hCtx->vbvInfo.nPicDuration = (1000*1000);
            hCtx->vbvInfo.nPicDuration /= hCtx->vbvInfo.nFrameRate;
        }
        H264GetBits(h264DecCtx, 1);   //>fixed_frame_rate_flag
    }


    pSpsInf->bNalHrdParametersPresentFlag = H264GetBits(h264DecCtx, 1);
    if(pSpsInf->bNalHrdParametersPresentFlag)
    {
        H264DecodeHrdParameters(h264DecCtx, pSpsInf);
    }
    pSpsInf->bVclHrdParametersPresentFlag = H264GetBits(h264DecCtx, 1);
    if(pSpsInf->bVclHrdParametersPresentFlag)
    {
        H264DecodeHrdParameters(h264DecCtx, pSpsInf);
    }
    if(pSpsInf->bNalHrdParametersPresentFlag || pSpsInf->bVclHrdParametersPresentFlag)
    {
    	pSpsInf->bLowDelayFlag = H264GetBits(h264DecCtx, 1);     /* low_delay_hrd_flag */
    }

    pSpsInf->bPicStructPresentFlag = H264GetBits(h264DecCtx, 1);         /* pic_struct_present_flag */

    pSpsInf->bBitstreamRestrictionFlag = H264GetBits(h264DecCtx, 1);
    if(pSpsInf->bBitstreamRestrictionFlag)
    {
        H264GetBits(h264DecCtx, 1);     /* motion_vectors_over_pic_boundaries_flag */
        H264GetUeGolomb(h264DecCtx); /* max_bytes_per_pic_denom */
        H264GetUeGolomb(h264DecCtx); /* max_bits_per_mb_denom */
        H264GetUeGolomb(h264DecCtx); /* log2_max_mv_length_horizontal */
        H264GetUeGolomb(h264DecCtx); /* log2_max_mv_length_vertical */
		
        nNumReorderFrames = H264GetUeGolomb(h264DecCtx);
        H264GetUeGolomb(h264DecCtx); /*max_dec_frame_buffering*/
        
        if(nNumReorderFrames > 16 /*max_dec_frame_buffering || max_dec_frame_buffering > 16*/)
        {
            return VRESULT_ERR_FAIL;
        }
        pSpsInf->nNumReorderFrames = nNumReorderFrames;
    }
    return 0;
}

//*********************************************************************************************************//
//*********************************************************************************************************//

int32_t H264DecodeSps(H264DecCtx* h264DecCtx, H264Context* hCtx)
{   
    uint32_t i = 0;
    uint8_t crop = 0;
    uint8_t nProfileIdc = 0;
    uint8_t vuiParamPresenFlag = 0;
    uint8_t frmMbsOnlyFlag = 0;
    uint16_t levelIdc = 0;
    uint32_t nSpsId = 0;
    uint32_t tmp = 0;
    uint32_t nMbWidth = 0;
    uint32_t nMbHeight = 0;
    uint32_t nCropLeft = 0;
    uint32_t nCropRight = 0;
    uint32_t nCropTop = 0;
    uint32_t nCropBottom = 0;
    uint32_t nCodedFrameRatio = 0;
    H264SpsInfo* pSpsInf = NULL;
    H264Dec* h264Dec = NULL;
    H264SpsInfo pStaticSpsInf; 
    H264SpsInfo* pNewSpsInf = NULL;

    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    
    /**********************
     *profileIdc  = 44: CAVLC 4:4:4 INTRA
     *profileIdc  = 66; baseLine;
     *profileIdc  = 77; main;
     *profileIdc  = 88; extented;
     *profileIdc  = 100; high
     *profileIdc  = 110; high 10 or high 10 intra
     *profileIdc  = 122; high 4:2:2 predictive or high 4:2:2 intra
     *profileIdc  = 244; high 4:4:4 or hight 4:4:4 intra
     *profileIdc  = 118;  multiview high
     *profileIdc  = 128; stereo high
     **********************/

    nProfileIdc = H264GetBits(h264DecCtx, 8);
    levelIdc = H264GetBits(h264DecCtx, 16);
    nSpsId = H264GetUeGolomb(h264DecCtx);
#if 0
    pSpsInf = (H264SpsInfo*)H264AllocParameterSet(hCtx, (void**)hCtx->pSpsBuffers, nSpsId, MAX_SPS_COUNT, sizeof(H264SpsInfo));
    if(pSpsInf == NULL)
    {   
        return VDECODE_RESULT_UNSUPPORTED;
    }
    hCtx->pRecordSps = pSpsInf;
#else
    pSpsInf = &pStaticSpsInf;
    memset(pSpsInf, 0, sizeof(H264SpsInfo));
#endif
    
    pSpsInf->nProfileIdc = nProfileIdc;
    pSpsInf->bScalingMatrixPresent = 0;
    
    if(nProfileIdc >= 100) //high profile
    {
        tmp = H264GetUeGolomb(h264DecCtx); 
        if(tmp == 3)      // chroma format idc
        {
            H264GetBits(h264DecCtx, 1);   //residual color transform flag
        }
        H264GetUeGolomb(h264DecCtx);      // bit depth luma minuint8_t
        H264GetUeGolomb(h264DecCtx);      // bit depth chroma minuint8_t
        H264GetBits(h264DecCtx, 1);        // transform bypass
        H264DecodeScalingMatrices(h264DecCtx, pSpsInf, NULL, 1, pSpsInf->nScalingMatrix4, pSpsInf->nScalingMatrix8);
        hCtx->bScalingMatrixPresent = 1;
    }

    pSpsInf->nLog2MaxFrmNum = H264GetUeGolomb(h264DecCtx) + 4;
    pSpsInf->nPocType = H264GetUeGolomb(h264DecCtx);
    
    if(pSpsInf->nPocType == 0)
    {
        pSpsInf->nLog2MaxPocLsb = H264GetUeGolomb(h264DecCtx)+4;
    }
    else if(pSpsInf->nPocType == 1)
    {
        pSpsInf->bDeltaPicOrderAlwaysZeroFlag = H264GetBits(h264DecCtx,1);
        pSpsInf->nOffsetForNonRefPic = H264GetSeGolomb(h264DecCtx);
        pSpsInf->nOffsetForTopToBottomField = H264GetSeGolomb(h264DecCtx);
        pSpsInf->nPocCycleLength = H264GetUeGolomb(h264DecCtx);
        for(i=0; i<pSpsInf->nPocCycleLength;i++)
        {
            pSpsInf->nOffsetForRefFrm[i] = H264GetSeGolomb(h264DecCtx);
        }
    }
    else if(pSpsInf->nPocType != 2)
    {   
    	return VRESULT_ERR_FAIL;
    }
    pSpsInf->nRefFrmCount = H264GetUeGolomb(h264DecCtx);

    H264GetBits(h264DecCtx, 1);
    
    nMbWidth = H264GetUeGolomb(h264DecCtx)+1;
    nMbHeight = H264GetUeGolomb(h264DecCtx)+1;
    
    if(hCtx->nMbWidth!=0 && hCtx->nMbHeight!=0)
    {
    	if(nMbWidth != hCtx->nMbWidth  || nMbHeight != hCtx->nMbHeight)
    	{
    		SbmReturnStream(hCtx->vbvInfo.vbv, hCtx->vbvInfo.pVbvStreamData);
    		if(h264Dec->pHContext != NULL)
    		{
    			H264FlushDelayedPictures(h264Dec->pHContext);
    		}
    		return VDECODE_RESULT_RESOLUTION_CHANGE;
    	}
    }
	
    frmMbsOnlyFlag = H264GetBits(h264DecCtx, 1);
    pSpsInf->bFrameMbsOnlyFlag = frmMbsOnlyFlag;
    
    pSpsInf->nMbWidth = nMbWidth;
    pSpsInf->nMbHeight = nMbHeight;
    
      
    pSpsInf->nFrmMbWidth = nMbWidth*16;
    pSpsInf->nFrmMbHeight = nMbHeight*(2-frmMbsOnlyFlag)*16;
    
    hCtx->nMbWidth = nMbWidth;
    hCtx->nMbHeight = nMbHeight;
    hCtx->nFrmMbWidth = pSpsInf->nFrmMbWidth;
    hCtx->nFrmMbHeight = pSpsInf->nFrmMbHeight;
    hCtx->bFrameMbsOnlyFlag = pSpsInf->bFrameMbsOnlyFlag;
    hCtx->nRefFrmCount = pSpsInf->nRefFrmCount;

    pSpsInf->bMbAff = 0;
    if(frmMbsOnlyFlag == 0)
    {
        pSpsInf->bMbAff = H264GetBits(h264DecCtx, 1);
    }
    pSpsInf->bDirect8x8InferenceFlag = H264GetBits(h264DecCtx, 1);
    
    nCropLeft  = 0;
    nCropRight = 0;
    nCropTop = 0;
    nCropBottom = 0;
    
    crop = H264GetBits(h264DecCtx,1);
    if(crop == 1)
    {
        H264GetUeGolomb(h264DecCtx);
        nCropRight = H264GetUeGolomb(h264DecCtx);
        H264GetUeGolomb(h264DecCtx);
        nCropBottom = H264GetUeGolomb(h264DecCtx);
        nCropRight *= 2;
        nCropBottom  *= 2*(2-frmMbsOnlyFlag);
    }
	
    pSpsInf->nFrmRealWidth = pSpsInf->nFrmMbWidth - nCropLeft - nCropRight;
    pSpsInf->nFrmRealHeight = pSpsInf->nFrmMbHeight - nCropTop - nCropBottom;
    nCodedFrameRatio = pSpsInf->nFrmRealWidth*1000/pSpsInf->nFrmRealHeight;

    if(nCodedFrameRatio <= 1460)
    {
    	pSpsInf->bCodedFrameRatio = H264_CODED_FRAME_RATIO_4_3;
    }
    else  if(nCodedFrameRatio <= 1660)
    {
    	pSpsInf->bCodedFrameRatio = H264_CODED_FRAME_RATIO_14_9;
    }
    else if(nCodedFrameRatio <= 1900)
    {
    	pSpsInf->bCodedFrameRatio = H264_CODED_FRAME_RATIO_16_9;
    }
    else
    {
    	pSpsInf->bCodedFrameRatio = H264_CODED_FRAME_RATIO_OTHER;
    }
    pSpsInf->nAspectRatio = 1000;
    vuiParamPresenFlag = H264GetBits(h264DecCtx, 1); // vui parameters present flag


    if(vuiParamPresenFlag == 1)
    {   
        H264DecVuiParameters(h264DecCtx,hCtx, pSpsInf);
    }
#if 0
    memcpy(&hCtx->pSps, pSpsInf, sizeof(H264SpsInfo));
    hCtx->nSpsId = nSpsId;
#endif

    if(hCtx->pSpsBuffers[nSpsId]==NULL)
    {
    	hCtx->nSpsBufferIndex[hCtx->nSpsBufferNum++] = nSpsId;
    }
    pNewSpsInf = (H264SpsInfo*)H264AllocParameterSet((void**)hCtx->pSpsBuffers, nSpsId, MAX_SPS_COUNT, sizeof(H264SpsInfo));
    if(pNewSpsInf == NULL)
    {
    	hCtx->nSpsBufferNum -= 1;
    	return VDECODE_RESULT_UNSUPPORTED;
    }
    hCtx->nNewSpsId = nSpsId;
    memcpy(pNewSpsInf, pSpsInf,  sizeof(H264SpsInfo));

    //hCtx->pRecordSps = pSpsInf;
    return VDECODE_RESULT_OK;	
}

//***************************************************************************************************//
//***************************************************************************************************//

static int32_t H264DecodeRbspTraiLing(uint8_t *src)
{
    int32_t v = 0;
    int32_t r = 0;

    v = *src;
    for(r=1; r<9; r++)
    {
        if(v&1)
        {
        	return r;
        }
        v>>=1;
    }
    return 0;
}

int32_t H264GetVLCSymbol(uint8_t* pBuf,int32_t nBitOffset, int32_t nBytecount)
{
	int32_t inf;
	long byteoffset=0;       // byte from start of buffer
	int32_t  bitoffset=0;       // bit from start of byte
	int32_t  bitcounter=0;
	int32_t  len = 0;
    uint8_t*  curByte = NULL;
    int32_t  ctrBit = 0;


	byteoffset = (nBitOffset>>3);       // byte from start of buffer
	bitoffset  = (7-(nBitOffset&0x07)); // bit from start of byte
	bitcounter = 1;

	curByte = pBuf+byteoffset;
	ctrBit    = ((*curByte)>>(bitoffset)) & 0x01;  // control bit for current bit posision

	while(ctrBit == 0)
	{                 // find leading 1 bit
		len++;
		bitcounter++;
		bitoffset--;
		bitoffset &= 0x07;
		curByte  += (bitoffset == 7);
		byteoffset+= (bitoffset == 7);
		ctrBit    = ((*curByte) >> (bitoffset)) & 0x01;
	}

	if(byteoffset + ((len + 7) >> 3) > nBytecount)
	{
		return -1;
	}

	// make infoword
	inf = 0;                          // shortest possible code is 1, then info is always 0

	while (len--)
	{
		bitoffset --;
		bitoffset &= 0x07;
		curByte  += (bitoffset == 7);
		bitcounter++;
		inf <<= 1;
		inf |= ((*curByte) >> (bitoffset)) & 0x01;
	}
	return bitcounter;           // return absolute offset in bit from start of frame
}

int32_t H264DecodePps(H264DecCtx* h264DecCtx, H264Context* hCtx, int32_t sliceDataLen)
{   
    uint32_t nPpsId = 0;
    uint32_t nSpsId = 0;
    uint32_t tmp = 0; 
    H264PpsInfo* pPpsInf = NULL;
    H264Dec* h264Dec =  NULL; 
    uint32_t nextCode = 0;
    int32_t nByteCount = 0;
    uint8_t  dataBuffer[1024];
    uint8_t* pBuf = NULL;
    uint8_t* pBuffer = NULL;
    int32_t nBitOffset = 0;
    int32_t nUsedBits = 0;
    int32_t i = 0;
    int32_t nTriLingBits = 0;
    H264PpsInfo pStaticPpsInf; 
    H264PpsInfo* pNewPpsInf = NULL;

    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

    nByteCount = sliceDataLen/8;
	
	if((nByteCount+4) > 1024)
	{
		pBuffer = (uint8_t*)malloc(nByteCount+4);
		if(pBuffer == NULL)
		{
			logd("malloc buffer for pBuffer error\n");
		}
	}
	else
	{
		pBuffer = dataBuffer;
	}

    if(hCtx->vbvInfo.pReadPtr+nByteCount+4 <= hCtx->vbvInfo.pVbvBufEnd)
    {
    	//(*MemRead)(pBuffer,hCtx->vbvInfo.pReadPtr, nByteCount+4);
    	(*MemRead)(hCtx->vbvInfo.pReadPtr,pBuffer, nByteCount+4);
    }
    else
    {

        tmp = hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pReadPtr+1;
        //(*MemRead)(pBuffer,hCtx->vbvInfo.pReadPtr, tmp);
        //(*MemRead)(pBuffer+tmp,hCtx->vbvInfo.pVbvBuf,  nByteCount+4-tmp);
        (*MemRead)(hCtx->vbvInfo.pReadPtr,pBuffer, tmp);
        (*MemRead)(hCtx->vbvInfo.pVbvBuf,pBuffer+tmp,  nByteCount+4-tmp);
    }

    if((hCtx->bDecExtraDataFlag==1) || (hCtx->bIsAvc==0))
    {
    	for(i=0; i<=nByteCount; i++)
    	{
    		if(pBuffer[i]==0x00 && pBuffer[i+1]==0x00 && pBuffer[i+2]==0x00 && pBuffer[i+3]==0x01)
    		{
    			//logd("find the startcode\n");
    			nByteCount = i;
    			break;
    		}
    	}
    }

    nTriLingBits = H264DecodeRbspTraiLing(pBuffer+nByteCount-1);

    pBuf = pBuffer+1;
    nByteCount -= 1;

    nPpsId = H264GetUeGolomb(h264DecCtx);
    pPpsInf = &pStaticPpsInf;
    memset(pPpsInf, 0, sizeof(H264PpsInfo));
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    tmp = H264GetUeGolomb(h264DecCtx);

    if((tmp >= MAX_SPS_COUNT) || (hCtx->pSpsBuffers[tmp]==NULL))
    {
    	logd("the sequence spsId is not equal to the spsId\n");
    	if(pBuffer != dataBuffer)
    	{
    		free(pBuffer);
    		pBuffer = NULL;
    	}
    	//hCtx->bNeedFindPPS = 1;
        //hCtx->bNeedFindSPS = 1;
        return VRESULT_ERR_FAIL;
    }
	nSpsId = tmp;
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    pPpsInf->nSpsId = tmp;
    pPpsInf->bCabac = H264GetBits(h264DecCtx, 1);   // entropy coding mode flag
    pPpsInf->pPicOrderPresent = H264GetBits(h264DecCtx, 1);

    nBitOffset += 2;

    pPpsInf->nSliceGroupCount = H264GetUeGolomb(h264DecCtx)+1;

    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    if(pPpsInf->nSliceGroupCount > 1)
    {
    	if(pBuffer != dataBuffer)
    	{
    		free(pBuffer);
    	    pBuffer = NULL;
    	}
    	//hCtx->bNeedFindPPS = 1;
    	return VRESULT_ERR_FAIL;
    }
    pPpsInf->nRefCount[0] = H264GetUeGolomb(h264DecCtx)+1;   // num_ref_idx_l0_active_minus1 +1
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    pPpsInf->nRefCount[1] = H264GetUeGolomb(h264DecCtx)+1;   // num_ref_idx_l1_active_minus1 +1
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    if(pPpsInf->nRefCount[0]>32 || pPpsInf->nRefCount[1]>32)
    {
    	if(pBuffer != dataBuffer)
    	{
    		free(pBuffer);
    	    pBuffer = NULL;
    	}
    	//hCtx->bNeedFindPPS = 1;
        return VRESULT_ERR_FAIL;
    }
	
    //hCtx->nPpsId = nPpsId;

    pPpsInf->bWeightedPred = H264GetBits(h264DecCtx, 1);
    pPpsInf->nWeightedBIpredIdc = H264GetBits(h264DecCtx, 2);
    nBitOffset += 3;

    pPpsInf->nInitQp = H264GetSeGolomb(h264DecCtx)+26;
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    H264GetSeGolomb(h264DecCtx);
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    pPpsInf->nChromaQpIndexOffset[0] = H264GetSeGolomb(h264DecCtx);
    nUsedBits = H264GetVLCSymbol(pBuf, nBitOffset, nByteCount);
    nBitOffset += nUsedBits;

    pPpsInf->bDeblockingFilterParamPresent = H264GetBits(h264DecCtx, 1);
    pPpsInf->bConstrainedIntraPred = H264GetBits(h264DecCtx, 1);
    pPpsInf->pRedundatPicCntPresent =  H264GetBits(h264DecCtx, 1);
    nBitOffset += 3;
    	
    pPpsInf->nChromaQpIndexOffset[1] = pPpsInf->nChromaQpIndexOffset[0];
	
    pPpsInf->bTransform8x8Mode = 0;
    memset(pPpsInf->nScalingMatrix4, 16, 6*16*sizeof(uint8_t));
    memset(pPpsInf->nScalingMatrix8, 16, 2*64*sizeof(uint8_t));

    if(hCtx->pPpsBuffers[nPpsId]==NULL)
    {
    	hCtx->nPpsBufferIndex[hCtx->nPpsBufferNum++] = nPpsId;
    }
    pNewPpsInf = (H264PpsInfo*)H264AllocParameterSet((void**)hCtx->pPpsBuffers, nPpsId, MAX_PPS_COUNT, sizeof(H264PpsInfo));
    if(pNewPpsInf == NULL)
    {
    	hCtx->nPpsBufferNum--;
    	if(pBuffer != dataBuffer)
    	{
    		free(pBuffer);
    		pBuffer = NULL;
    	}
        return VDECODE_RESULT_UNSUPPORTED;
    }

    if(nBitOffset+nTriLingBits >= nByteCount*8)
    {
    	memcpy(pNewPpsInf, pPpsInf, sizeof(H264PpsInfo));
    	if(pBuffer != dataBuffer)
    	{
    		free(pBuffer);
    	    pBuffer = NULL;
    	}
    	return VDECODE_RESULT_OK;
    }

    nextCode = H264ShowBits(h264DecCtx, 18);
    if(nextCode != 0x20000)
    {
        pPpsInf->bTransform8x8Mode = H264GetBits(h264DecCtx, 1);

        H264DecodeScalingMatrices(h264DecCtx, hCtx->pSpsBuffers[nSpsId], pPpsInf, 0, pPpsInf->nScalingMatrix4, pPpsInf->nScalingMatrix8);
        hCtx->bScalingMatrixPresent = 1;
        pPpsInf->nChromaQpIndexOffset[1] = H264GetSeGolomb(h264DecCtx);
    }
    
    memcpy(pNewPpsInf, pPpsInf, sizeof(H264PpsInfo));

    if(pBuffer != dataBuffer)
    {
    	free(pBuffer);
        pBuffer = NULL;
    }
    return VDECODE_RESULT_OK;
}

void H264ProcessActiveFormat(H264DecCtx* h264DecCtx, H264Context* hCtx, H264SpsInfo* pCurSps, int32_t nSliceDataLen)
{
	H264Dec* h264Dec = NULL;
	uint32_t remainDataLen = 0;
    uint8_t  buffer[4096];
    uint8_t* ptr = NULL;
    int32_t i = 0;
    uint32_t nextCode = 0;
    uint8_t acticveFormatFlag = 0;
    uint8_t activeFormat = 0;

	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

    if((pCurSps->bCodedFrameRatio != H264_CODED_FRAME_RATIO_4_3)
    		&& (pCurSps->bCodedFrameRatio!= H264_CODED_FRAME_RATIO_16_9))
    {
    	return;
    }

    if(nSliceDataLen > 4096)
    {
        loge(" the buffer[4096] is not enought, requstSize = %d",nSliceDataLen);
        abort();
    }
	if(hCtx->vbvInfo.pReadPtr+nSliceDataLen > hCtx->vbvInfo.pVbvBufEnd)
	{
		remainDataLen = hCtx->vbvInfo.pVbvBufEnd - hCtx->vbvInfo.pReadPtr+1;
		//(*MemRead)(buffer, hCtx->vbvInfo.pReadPtr, remainDataLen);
	    //(*MemRead)(buffer+remainDataLen, hCtx->vbvInfo.pVbvBuf, nSliceDataLen-remainDataLen);
        (*MemRead)(hCtx->vbvInfo.pReadPtr, buffer, remainDataLen);
	    (*MemRead)(hCtx->vbvInfo.pVbvBuf, buffer+remainDataLen, nSliceDataLen-remainDataLen);
		ptr = buffer;
	}
	else
	{
        (*MemRead)(hCtx->vbvInfo.pReadPtr, buffer, nSliceDataLen);
		ptr = buffer;
	}

	nextCode = 0xffffffff;
	for(i=0; i<nSliceDataLen; i++)
	{
		nextCode <<= 8;
		nextCode |= ptr[i];
		if(nextCode == 0x44544731)
		{
			break;
		}
	}

	if(i >= nSliceDataLen)
	{
		return;
	}

	acticveFormatFlag = (ptr[i+1]>>6)& 0x01;
	if(acticveFormatFlag == 1)
	{
		activeFormat = ptr[i+2]& 0x0f;
	}

	hCtx->nLeftOffset = 0;
	hCtx->nRightOffset = 0;
	hCtx->nTopOffset = 0;
	hCtx->nBottomOffset = 0;

	switch(activeFormat)
	{
		case 2:            //box 16:9 (top)
            if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_4_3)
            {
            	hCtx->nBottomOffset = pCurSps->nFrmRealHeight-(pCurSps->nFrmRealWidth*9/16);
            }
			break;
		case 3:            //box 14:9 (top)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_4_3)
			{
				hCtx->nBottomOffset = pCurSps->nFrmRealHeight-(pCurSps->nFrmRealWidth*9/14);
			}
			else if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_16_9)
			{
				hCtx->nLeftOffset = pCurSps->nFrmRealWidth-(pCurSps->nFrmRealHeight*14/9);
				hCtx->nLeftOffset /= 2;
				hCtx->nRightOffset = hCtx->nLeftOffset;
			}
			break;
		case 4:            //box > 16:9 (center)
			hCtx->nTopOffset =  pCurSps->nFrmRealHeight-(pCurSps->nFrmRealWidth*10/19);
			hCtx->nTopOffset /= 2;
			hCtx->nBottomOffset = hCtx->nTopOffset;
			break;
		case 9:            //4:3 (center)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_16_9)
			{
				hCtx->nLeftOffset = pCurSps->nFrmRealWidth-(pCurSps->nFrmRealHeight*4/3);
				hCtx->nLeftOffset /= 2;
				hCtx->nRightOffset = hCtx->nLeftOffset;
			}
			break;
		case 10:            //16:9 (center)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_4_3)
			{
				hCtx->nTopOffset = pCurSps->nFrmRealHeight -(pCurSps->nFrmRealWidth*9/16);
				hCtx->nTopOffset /= 2;
				hCtx->nBottomOffset = hCtx->nTopOffset;
			}
			break;
		case 11:            //14:9 (center)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_4_3)
			{
				hCtx->nTopOffset = pCurSps->nFrmRealHeight -(pCurSps->nFrmRealWidth*9/14);
				hCtx->nTopOffset /= 2;
				hCtx->nBottomOffset = hCtx->nTopOffset;
			}
			else if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_16_9)
			{
				hCtx->nLeftOffset = pCurSps->nFrmRealWidth -(pCurSps->nFrmRealHeight*14/9);
				hCtx->nLeftOffset /= 2;
				hCtx->nRightOffset = hCtx->nLeftOffset;
			}
			break;
		case 13:            //4:3 (with shoot and protect 14:9 center)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_16_9)
			{
				hCtx->nLeftOffset = pCurSps->nFrmRealWidth -(pCurSps->nFrmRealHeight*4/3);
				hCtx->nLeftOffset /= 2;
				hCtx->nRightOffset = hCtx->nLeftOffset;
			}
			break;
		case 14:            //16:9 (with shoot and protect 14:9 center)
		case 15:            //16:9 (with shoot and protect 4:3 center)
			if(pCurSps->bCodedFrameRatio == H264_CODED_FRAME_RATIO_4_3)
			{
				hCtx->nTopOffset = pCurSps->nFrmRealHeight -(pCurSps->nFrmRealWidth*9/16);
				hCtx->nTopOffset /= 2;
				hCtx->nBottomOffset = hCtx->nTopOffset;
			}
			break;
		default:
			break;
	}
	return;
}

int32_t H264DecodeSei(H264DecCtx* h264DecCtx, H264Context* hCtx,int32_t nSliceDataLen)
{
    int32_t nDataBitIndex = 0;
    int32_t num = 0;
    int32_t type = 0;
    int32_t code = 0;
    int32_t size = 0;
    int32_t remainBits = 0;
    int32_t i = 0;
    int32_t n = 0;
    int32_t seiPicStruct = 0;
    H264SpsInfo* pCurSps = NULL;

    pCurSps = hCtx->pSpsBuffers[hCtx->nNewSpsId];

	H264ProcessActiveFormat(h264DecCtx, hCtx, pCurSps, nSliceDataLen>>3);

	nSliceDataLen -= 8;

	while(nDataBitIndex+16 < nSliceDataLen)
	{
		size = 0;
		type = 0;

	    do
	    {
	    	if((nDataBitIndex+8) > nSliceDataLen)
	    	{
	    		return VDECODE_RESULT_OK;
	    	}
	    	code = H264GetBits(h264DecCtx, 8);
	    	nDataBitIndex += 8;
	    	type += code;
	    }while(code == 255);

	    size=0;
	    do
	    {
	    	if((nDataBitIndex+8) > nSliceDataLen)
	    	{
	    		return VDECODE_RESULT_OK;
	    	}
	    	code = H264GetBits(h264DecCtx, 8);
	    	nDataBitIndex += 8;
	    	size += code;
	    }while(code== 255);


	    if(type == 1)
	    {
	        if(pCurSps->bNalHrdParametersPresentFlag || pCurSps->bVclHrdParametersPresentFlag)
	        {
	        	 if((nDataBitIndex+(int32_t)pCurSps->nCpbRemovalDelayLength)>nSliceDataLen)
	        	 {
	        		  return VDECODE_RESULT_OK;
	        	 }
                 H264GetBits(h264DecCtx, pCurSps->nCpbRemovalDelayLength);
                 nDataBitIndex += pCurSps->nCpbRemovalDelayLength;
                 if((nDataBitIndex+(int32_t)pCurSps->nDpbOutputDelayLength)>nSliceDataLen)
                 {
                	  return VDECODE_RESULT_OK;
                 }
                 H264GetBits(h264DecCtx, pCurSps->nDpbOutputDelayLength);
                 nDataBitIndex += pCurSps->nDpbOutputDelayLength;
	        }

	    	if(pCurSps->bPicStructPresentFlag)
	    	{
	    		if((nDataBitIndex+4)>nSliceDataLen)
	    		{
	    			  return VDECODE_RESULT_OK;
	    		}
	    		seiPicStruct = H264GetBits(h264DecCtx, 4);
	    		nDataBitIndex += 4;
	    		hCtx->bProgressice = 1;
	    		if(seiPicStruct>=1 && seiPicStruct<=6)
	    		{
	    			hCtx->bProgressice = 0;
	    		}
	    	}
	    	return VDECODE_RESULT_OK;
	    }
	    else
	    {
	    	if((nDataBitIndex+8*size)>nSliceDataLen)
	    	{
	    		 return VDECODE_RESULT_OK;
	    	}
	    	num = 8*size/32;
	    	remainBits = 8*size - num*32;

	    	for(i=0; i<num; i++)
	    	{
	    		H264GetBits(h264DecCtx, 32);
	    	}
	    	H264GetBits(h264DecCtx, remainBits);
	    	nDataBitIndex += 8*size;
	    }
	    //align_get_bits(&s->gb);
	    n = (-nDataBitIndex) & 7;
        if(n)
        {
           	if((nDataBitIndex+n)>nSliceDataLen)
           	{
           		return VDECODE_RESULT_OK;
           	}
        	num = n/24;
        	remainBits = n- num*24;

        	for(i=0; i<num; i++)
        	{
        		H264GetBits(h264DecCtx, 24);
        	}
        	H264GetBits(h264DecCtx, remainBits);
        	nDataBitIndex += n;
        }
	}
    return VDECODE_RESULT_OK;
}


void H264InitPoc(H264Context* hCtx)
{   
    uint32_t i = 0;
    int32_t nPoc = 0;
    int32_t nPocCycleCnt = 0;
    int32_t expectedPoc = 0;
    int32_t maxFrmNum = 0;
    int32_t maxPocLsb = 0;
    int32_t nFieldPoc[2] = {0};
    int32_t absFrmNum = 0;
    uint32_t nFrmNumInnPocCycle = 0;
    int32_t expectedDeltaPerPocCycle = 0;
    
    #define MIN(a,b) ((a) > (b) ? (b) : (a))
	
    maxFrmNum = 1 << hCtx->pCurSps->nLog2MaxFrmNum;
    
    //if(hCtx->nalUnitType == NAL_IDR_SLICE)
    if(hCtx->bIdrFrmFlag == 1)
    {
        hCtx->nFrmNumOffset = 0;
    }
    else
    {
        if(hCtx->nFrmNum < hCtx->nPrevFrmNum)
        {
            hCtx->nFrmNumOffset = hCtx->nPrevFrmNumOffset + maxFrmNum;
        }
        else
        {
            hCtx->nFrmNumOffset = hCtx->nPrevFrmNumOffset;
        }
    }
    
    if(hCtx->pCurSps->nPocType == 0)
    {
        maxPocLsb = 1 << hCtx->pCurSps->nLog2MaxPocLsb;
        //if(hCtx->nalUnitType == NAL_IDR_SLICE)
        if(hCtx->bIdrFrmFlag == 1)
        {
            hCtx->nPrevPocMsb = 0;
            hCtx->nPrevPocLsb = 0;
        }
        if(hCtx->nPocLsb<hCtx->nPrevPocLsb && hCtx->nPrevPocLsb-hCtx->nPocLsb>=maxPocLsb/2)
        {
            hCtx->nPocMsb = hCtx->nPrevPocMsb + maxPocLsb;
	    }
        else if(hCtx->nPocLsb > hCtx->nPrevPocLsb && hCtx->nPrevPocLsb-hCtx->nPocLsb <-maxPocLsb/2)
        {
            hCtx->nPocMsb = hCtx->nPrevPocMsb - maxPocLsb;
        }
        else
        {
            hCtx->nPocMsb = hCtx->nPrevPocMsb;
        }
        nFieldPoc[0] = nFieldPoc[1] = hCtx->nPocMsb+hCtx->nPocLsb;
        if(hCtx->nPicStructure == PICT_FRAME)
        {
            nFieldPoc[1] += hCtx->nDeltaPocBottom;
        }
    }
    else if(hCtx->nPocType == 1)
    {   
        absFrmNum = 0;
        if(hCtx->pCurSps->nPocCycleLength != 0)
        {
            absFrmNum = hCtx->nFrmNumOffset + hCtx->nFrmNum;
        }
        if(hCtx->nNalRefIdc==0 && absFrmNum > 0)
        {
            absFrmNum--;
        }
        expectedDeltaPerPocCycle = 0;
        for(i=0; i<hCtx->pCurSps->nPocCycleLength; i++)
        {
            expectedDeltaPerPocCycle += hCtx->pCurSps->nOffsetForRefFrm[i];
        }
        expectedPoc = 0;
        if(absFrmNum > 0)
        {
            nPocCycleCnt = (absFrmNum-1)/hCtx->pCurSps->nPocCycleLength;
            nFrmNumInnPocCycle = (absFrmNum-1)%hCtx->pCurSps->nPocCycleLength;  
            expectedPoc = nPocCycleCnt * expectedDeltaPerPocCycle;
            for(i=0; i<=nFrmNumInnPocCycle; i++)
            {
                expectedPoc = expectedPoc + hCtx->pCurSps->nOffsetForRefFrm[i];
            }
        }
        if(hCtx->nNalRefIdc == 0)
        {
            expectedPoc = expectedPoc + hCtx->pCurSps->nOffsetForNonRefPic;
        }
		
        nFieldPoc[0] = expectedPoc + hCtx->nDeltaPoc[0];
        nFieldPoc[1] = nFieldPoc[0]+hCtx->pCurSps->nOffsetForTopToBottomField;
        if(hCtx->nPicStructure == PICT_FRAME)
        {
            nFieldPoc[1] += hCtx->nDeltaPoc[1];
        }
    }
    else 
    {
        //if(hCtx->nalUnitType == NAL_IDR_SLICE)
    	if(hCtx->bIdrFrmFlag == 1)
        {
            nPoc = 0;
        }
        else
        {
            if(hCtx->nNalRefIdc)
            {
                nPoc = 2*(hCtx->nFrmNumOffset+hCtx->nFrmNum);
            }
            else
            {
                nPoc = 2*(hCtx->nFrmNumOffset+hCtx->nFrmNum) -1;
            }
        }
        nFieldPoc[0] = nPoc;
        nFieldPoc[1] = nPoc;
    }

    if(hCtx->nPicStructure != PICT_BOTTOM_FIELD)
    {
        if(nFieldPoc[0]%2==1)
        {
        	hCtx->nPicPocDeltaNum = 1;
        }

       // hCtx->frmBufInf.pCurPicturePtr->nFrmNum = hCtx->nCurPicNum;
        hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[0] = nFieldPoc[0];
        hCtx->frmBufInf.pCurPicturePtr->nPoc = nFieldPoc[0];
        hCtx->frmBufInf.pCurPicturePtr->nFieldPicStructure[0] = hCtx->nPicStructure;
        hCtx->frmBufInf.pCurPicturePtr->nFieldNalRefIdc[0] = hCtx->nNalRefIdc;
        hCtx->frmBufInf.pCurPicturePtr->bMbAffFrame = hCtx->bMbAffFrame;
    }
	
    if(hCtx->nPicStructure != PICT_TOP_FIELD)
    {
    	//hCtx->frmBufInf.pCurPicturePtr->nFrmNum = hCtx->nCurPicNum;
        hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[1] = nFieldPoc[1];
        hCtx->frmBufInf.pCurPicturePtr->nPoc = nFieldPoc[1];
        hCtx->frmBufInf.pCurPicturePtr->nFieldPicStructure[1] = hCtx->nPicStructure;
        hCtx->frmBufInf.pCurPicturePtr->nFieldNalRefIdc[1] = hCtx->nNalRefIdc;
    }
    if(hCtx->nPicStructure == PICT_FRAME || hCtx->bFstField==0)
    {
        hCtx->frmBufInf.pCurPicturePtr->nPoc = MIN(hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[0], hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[1]);
    }
}


int32_t H264GetValidBufferIndex(H264Context* hCtx, int32_t* bufIndex)
{
	int32_t i = 0;

	if(hCtx->frmBufInf.nMaxValidFrmBufNum < 18)
	{
		for(i=0; i<hCtx->frmBufInf.nMaxValidFrmBufNum; i++)
		{
			if(hCtx->frmBufInf.picture[i].pVPicture==NULL)
			{
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nDecodeBufIndex = i;
				hCtx->frmBufInf.picture[i].nDispBufIndex = i;

				break;
			}
		}

		if(i == hCtx->frmBufInf.nMaxValidFrmBufNum)
		{
			return VDECODE_RESULT_NO_FRAME_BUFFER;
		}
	}
	else if(hCtx->nNalRefIdc == 0)
	{
		for(i=hCtx->frmBufInf.nMaxValidFrmBufNum-1; i>=hCtx->nRefFrmCount+2; i--)
		{
			if(hCtx->frmBufInf.picture[i].pVPicture==NULL)
			{
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nDecodeBufIndex = 0;
				hCtx->frmBufInf.picture[i].nDispBufIndex = i;
				break;
			}
		}
		if(i < hCtx->nRefFrmCount+2)
		{
			return VDECODE_RESULT_NO_FRAME_BUFFER;
		}
	}
	else
	{
		for(i=1; i<18; i++)
		{
			if(hCtx->frmBufInf.picture[i].pVPicture==NULL)
			{
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nReference = 0;
				hCtx->frmBufInf.picture[i].nDecodeBufIndex = i;
				hCtx->frmBufInf.picture[i].nDispBufIndex = i;
				break;
			}
		}
		if(i==18)
		{
			return VDECODE_RESULT_NO_FRAME_BUFFER;
		}
	}

	*bufIndex = i;
	return VDECODE_RESULT_OK;
}



int32_t H264FrameStart(H264DecCtx* h264DecCtx, H264Context* hCtx)
{   
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0; 
    int32_t bufIndex = 0xff;
    H264PicInfo* curPicPtr  = NULL;
    uint32_t minDecFrmOrder = 0xffffffff;
    uint32_t validBufIndex[32];
	H264DecCtx* p264DecCtx = NULL;

	p264DecCtx = h264DecCtx;
	
    if(hCtx->nCurFrmNum >= 0xffffffff)
    {
    	for(i=0; i<hCtx->frmBufInf.nMaxValidFrmBufNum; i++)
    	{
    		if(hCtx->frmBufInf.picture[i].pVPicture!=NULL)
    		{
               if(hCtx->frmBufInf.picture[i].nDecFrameOrder < minDecFrmOrder)
               {
            	   minDecFrmOrder = hCtx->frmBufInf.picture[i].nDecFrameOrder;
               }
               validBufIndex[k] = i;
               k++;
    		}
    	}
    	//logv("hCtx->nCurFrmNum=%d, minDecFrmOrder=%d\n", hCtx->nCurFrmNum, minDecFrmOrder);
    	hCtx->nCurFrmNum -= minDecFrmOrder;
    	for(i=0; i<k; i++)
    	{
    		j = validBufIndex[i];
    		hCtx->frmBufInf.picture[j].nDecFrameOrder -= minDecFrmOrder;
    		//logv("index=%d, nDecFrameOrder=%d\n", j, hCtx->frmBufInf.picture[j].nDecFrameOrder);
    	}
    }

    if(H264GetValidBufferIndex(hCtx, &i) == VDECODE_RESULT_NO_FRAME_BUFFER)
    {
    	return VDECODE_RESULT_NO_FRAME_BUFFER;
    }

    hCtx->frmBufInf.picture[i].pVPicture = FbmRequestBuffer(hCtx->pFbm);
    if(hCtx->frmBufInf.picture[i].pVPicture == NULL)
    {
    	return VDECODE_RESULT_NO_FRAME_BUFFER;
    }

    hCtx->frmBufInf.picture[i].bHasDispedFlag = 0;

    if(hCtx->frmBufInf.picture[i].nDecodeBufIndex != hCtx->frmBufInf.picture[i].nDispBufIndex)
    {
    	bufIndex = hCtx->frmBufInf.picture[i].nDecodeBufIndex;
    	hCtx->frmBufInf.picture[bufIndex].pVPicture = hCtx->frmBufInf.picture[i].pVPicture;
    }

    hCtx->frmBufInf.pCurPicturePtr = &(hCtx->frmBufInf.picture[i]);
    
    curPicPtr  = hCtx->frmBufInf.pCurPicturePtr;

    curPicPtr->bDecErrorFlag = 0;
    curPicPtr->nPicStructure = hCtx->nPicStructure;
    curPicPtr->nPictType = hCtx->nSliceType;

    curPicPtr->bKeyFrame = (curPicPtr->nPictType==H264_I_TYPE);
    curPicPtr->nReference = 0;
    if(hCtx->nNalRefIdc != 0)
    {
        curPicPtr->nReference = hCtx->nPicStructure;
    }

    if(hCtx->frmBufInf.pCurPicturePtr->nPictType != H264_B_TYPE)
    {
    	hCtx->frmBufInf.pLastPicturePtr = hCtx->frmBufInf.pNextPicturePtr;
    	if(hCtx->nNalRefIdc != 0)
        {
    		hCtx->frmBufInf.pNextPicturePtr = hCtx->frmBufInf.pCurPicturePtr;;
        }
    }
    return VDECODE_RESULT_OK;
}



void H264ExchangePts(int64_t* pts1, int64_t*pts2)
{
    int64_t temp = 0;
    
    temp = *pts1;
    *pts1 = *pts2;
    *pts2 = temp;
}

void H264CalculatePicturePts( H264Context* hCtx)
{
    int32_t idrFrmIndex = 0;
    int32_t i = 0;
    int32_t j = 0;
    int32_t nPoc= 0;
    int64_t *ptsPtr = 0;
    int64_t* pPicPts = 0;
    int32_t flag1 = 0;
    int32_t flag2 = 0;
    int64_t nPicDuration = 0;
    int32_t diffPoc = 0;
    VideoPicture* pVPicture = NULL;
    

    pVPicture = hCtx->frmBufInf.pCurPicturePtr->pVPicture;

    pVPicture->nPts = hCtx->vbvInfo.nValidDataPts;
    hCtx->vbvInfo.nValidDataPts = H264VDEC_ERROR_PTS_VALUE;
    if(hCtx->bOnlyDecKeyFrame==1)
    {
    	return;
    }
    if(hCtx->nPicPocDeltaNum == 0 && hCtx->frmBufInf.pLastPicturePtr!=NULL)
    {
        hCtx->nPicPocDeltaNum = 2;
    }

    if(hCtx->vbvInfo.nPicDuration != 0)
    {
    	hCtx->nEstimatePicDuration = hCtx->vbvInfo.nPicDuration;
    }
    else if(hCtx->nEstimatePicDuration == 0)
    {
    	hCtx->nEstimatePicDuration = 40000;
    }

    if(pVPicture->nPts == H264VDEC_ERROR_PTS_VALUE)
    {
    	if((hCtx->frmBufInf.pCurPicturePtr->nPoc == 0) || (hCtx->nPicPocDeltaNum==0))
    	{
    		pVPicture->nPts = hCtx->vbvInfo.nNextPicPts;
    		hCtx->vbvInfo.nPrePicPts = pVPicture->nPts;
    		hCtx->vbvInfo.nPrePicPoc = hCtx->frmBufInf.pCurPicturePtr->nPoc;

    	}
    	else
    	{
    		pVPicture->nPts = hCtx->vbvInfo.nPrePicPts;
    		nPicDuration = hCtx->nEstimatePicDuration/hCtx->nPicPocDeltaNum;
    		pVPicture->nPts += (hCtx->frmBufInf.pCurPicturePtr->nPoc-hCtx->vbvInfo.nPrePicPoc)*nPicDuration;

    	}
    }
    else
    {
    	if(hCtx->vbvInfo.nPrePicPoc!=0 && hCtx->vbvInfo.nPrePicPts!=0 && hCtx->frmBufInf.pCurPicturePtr->nPoc!=0)
    	{
	  	    diffPoc = (hCtx->frmBufInf.pCurPicturePtr->nPoc - hCtx->vbvInfo.nPrePicPoc);
			if(diffPoc != 0)
    		{
				hCtx->nEstimatePicDuration = (pVPicture->nPts - hCtx->vbvInfo.nPrePicPts);
    			hCtx->nEstimatePicDuration /= diffPoc;
    			hCtx->nEstimatePicDuration *= hCtx->nPicPocDeltaNum;
			}
    	}
    	hCtx->vbvInfo.nPrePicPts = pVPicture->nPts;
    	hCtx->vbvInfo.nPrePicPoc = hCtx->frmBufInf.pCurPicturePtr->nPoc;
    }
    if(hCtx->vbvInfo.nNextPicPts < (pVPicture->nPts+hCtx->nEstimatePicDuration))
    {
    	hCtx->vbvInfo.nNextPicPts = pVPicture->nPts+hCtx->nEstimatePicDuration;
    }

    if(hCtx->frmBufInf.pCurPicturePtr->nPoc==0)
    {
        return;
    }
		
    for(i=hCtx->nDelayedPicNum-1; i>=0; i--)
    {
    	if(hCtx->frmBufInf.pDelayedPic[i]->nPoc==0)
    	{
    		idrFrmIndex = i;
    		break;
    	}
    }
    
    i = idrFrmIndex-1;
    ptsPtr = &pVPicture->nPts;
    nPoc = hCtx->frmBufInf.pCurPicturePtr->nPoc;
    
    while(1)
    {   
        for(j=i+1; ; j++)
        {   
        	if(hCtx->frmBufInf.pDelayedPic[j]== NULL)
        	{
        		break;
        	}

        	if(hCtx->frmBufInf.pDelayedPic[j]->pVPicture == NULL)
        	{
        		break;
        	}
            flag1 = nPoc < hCtx->frmBufInf.pDelayedPic[j]->nPoc;
            pPicPts = &hCtx->frmBufInf.pDelayedPic[j]->pVPicture->nPts;
            flag2 = (*ptsPtr) < *pPicPts;
            if((flag1+flag2)==1)
            {   
            	if(nPoc!=hCtx->frmBufInf.pDelayedPic[j]->nPoc)
            	{
            		H264ExchangePts(ptsPtr, pPicPts);
            	}
            }
        }
        i++;
        if(hCtx->frmBufInf.pDelayedPic[i] == NULL)
        {
            break;
        }

        ptsPtr = &hCtx->frmBufInf.pDelayedPic[i]->pVPicture->nPts;
        nPoc = hCtx->frmBufInf.pDelayedPic[i]->nPoc;
    }
}

int32_t H264DecodeSliceHeader(H264DecCtx* h264DecCtx, H264Context* hCtx)
{   
    uint8_t  lastPicStructure = 0;
    uint8_t lastFieldFlag = 0;
    int32_t tmp  = 0;
    uint32_t nPpsId = 0;
    uint32_t bFstMbInSlice = 0;
    uint32_t nSliceType    = 0;
    int32_t defaultRefListDone = 0;
    int32_t nLastFrmNum = 0;
    uint8_t  nSliceTypeMap[5]= {H264_P_TYPE, H264_B_TYPE, H264_I_TYPE, H264_SP_TYPE, H264_SI_TYPE};
    H264Dec* h264Dec = NULL;
  
    
    h264Dec = (H264Dec*) h264DecCtx->pH264Dec;

    hCtx->bCanResetHw  = 0;
    bFstMbInSlice = H264GetUeGolomb(h264DecCtx);

    if(bFstMbInSlice == 0) // the fist slice of frame or field
    {
        if(hCtx->nDecFrameStatus == H264_START_DEC_FRAME)
        {
            if((hCtx->frmBufInf.pCurPicturePtr!= NULL) && ((hCtx->bFstField==0)||(hCtx->nPicStructure==PICT_FRAME)))
            {
                hCtx->nNalRefIdc = hCtx->frmBufInf.pCurPicturePtr->nNalRefIdc;
                hCtx->nSliceType = hCtx->frmBufInf.pCurPicturePtr->nPictType;
                hCtx->nFrmNum   = hCtx->frmBufInf.pCurPicturePtr->nFrmNum;
                hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
                hCtx->nDecFrameStatus = H264_END_DEC_FRAME;
                logi("here1: the first slice of the frame is not 0\n");
                return VRESULT_DEC_FRAME_ERROR;
            }
        }
        hCtx->nCurSliceNum = 0;
    }
    else if(hCtx->nCurSliceNum == 0)
    {
    	logi("here2: the last frame is not complete\n");
    	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
        {
    		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
        }
    	return VRESULT_ERR_FAIL;
    }

    hCtx->bFstMbInSlice = bFstMbInSlice;
    nSliceType = H264GetUeGolomb(h264DecCtx);
    
    if(nSliceType > 9)
    {
        //log("slice type is too large\n");
      	logi("the slice type is invalid\n");
      	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
      	{
      		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
      	}
        return VRESULT_ERR_FAIL;
    }
    nSliceType = (nSliceType > 4)? nSliceType-5 : nSliceType;   //0: P slice; 1: B slice; 2: I slice; 3: SP slice; 4: SI slice
    nSliceType =  nSliceTypeMap[nSliceType];
    
    if(hCtx->bNeedFindIFrm == 1)
    {
        if((h264Dec->nDecStreamIndex==0) && (nSliceType!=H264_I_TYPE))
        {
          	logi("need find the I frame to start decode\n");
            return VRESULT_ERR_FAIL;
        }
        else if((h264Dec->nDecStreamIndex==1)&&(nSliceType!=H264_P_TYPE))
        {
        	logi("need find the I frame to start decode\n");
            return VRESULT_ERR_FAIL;
        }
        hCtx->bNeedFindIFrm = 0;
    }

    if((nSliceType == H264_I_TYPE)||(hCtx->nCurSliceNum!=0 && nSliceType==hCtx->nLastSliceType))
    {
        defaultRefListDone = 1;
    }
	
    if(nSliceType == H264_B_TYPE && hCtx->frmBufInf.pLastPicturePtr==NULL)
    {
      	logi("need drop the B frame\n");
        return VRESULT_ERR_FAIL;
    }

    hCtx->nSliceType = nSliceType;
    nPpsId = H264GetUeGolomb(h264DecCtx);

    if(nPpsId >= 256)
    {
    	logi("the ppsId is %d, larger than 256\n", nPpsId);
     	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
       	{
     		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
        }
        return VRESULT_ERR_FAIL;
    }

    if(hCtx->pPpsBuffers[nPpsId] == NULL)
    {
    	//logd("the slice ppsid is not equal to the ppsid, nPpsId=%d,  hCtx->nPpsId=%d\n", nPpsId, hCtx->nPpsId);
    	logi("nPpsId=%d, hCtx->pPpsBuffers[nPpsId]=NULL\n", nPpsId);
     	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
      	{
     		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
       	}
    	return VRESULT_ERR_FAIL;
    }

    hCtx->pCurPps = hCtx->pPpsBuffers[nPpsId];
    hCtx->pCurSps = hCtx->pSpsBuffers[hCtx->pCurPps->nSpsId];

    nLastFrmNum = hCtx->nFrmNum;

    hCtx->nFrmNum = H264GetBits(h264DecCtx, hCtx->pCurSps->nLog2MaxFrmNum);
    
    lastPicStructure = hCtx->nPicStructure;
    hCtx->bMbAffFrame = 0;
	
    if(hCtx->pCurSps->bFrameMbsOnlyFlag == 1)
    {
        hCtx->nPicStructure = PICT_FRAME;
    }
    else
    {   
        if(H264GetBits(h264DecCtx, 1) == 1)
        {
            hCtx->nPicStructure = PICT_TOP_FIELD+H264GetBits(h264DecCtx, 1) ;
        }
        else
        {
            hCtx->nPicStructure = PICT_FRAME;
            hCtx->bMbAffFrame = hCtx->pCurSps->bMbAff;
        }
    }
    if(bFstMbInSlice != 0)
    {
    	if(hCtx->nPicStructure != lastPicStructure)
    	{
    		hCtx->nPicStructure = lastPicStructure;
    	}
    }
    else if(hCtx->bFstField ==1)
    {
    	if((hCtx->nPicStructure==PICT_FRAME) || (hCtx->nPicStructure==lastPicStructure))
    	{
    		hCtx->bFstField = 0;
    		hCtx->nNalRefIdc = hCtx->frmBufInf.pCurPicturePtr->nNalRefIdc;
    		hCtx->nSliceType = hCtx->frmBufInf.pCurPicturePtr->nPictType;
    		hCtx->nFrmNum   = hCtx->frmBufInf.pCurPicturePtr->nFrmNum;
    	 	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
    	   	{
    	 		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
    	    }
    		hCtx->nDecFrameStatus = H264_END_DEC_FRAME;
    		logi("the last frame is not complete\n");
    		return VRESULT_DEC_FRAME_ERROR;
    	}
    }

    lastFieldFlag = hCtx->bFstField;
    if(hCtx->nCurSliceNum == 0)
    {   
    	hCtx->nLastFrmNum = nLastFrmNum;

    	if(hCtx->nPicStructure == PICT_FRAME)
    	{
    		hCtx->bFstField = 0;
    	}
    	else if(hCtx->bFstField == 1)
        {
            if((hCtx->nPicStructure==PICT_FRAME) || (hCtx->nPicStructure==lastPicStructure))
            {   
                // Previous field is unmatched. 
                // Don't display it,but let i remain for nReference if marked as such.
            	logv("first field and the bottom field is not matched\n");
            	if((hCtx->frmBufInf.pCurPicturePtr!= NULL) &&(hCtx->frmBufInf.pCurPicturePtr->pVPicture!=NULL))
            	{
            		hCtx->nNalRefIdc = hCtx->frmBufInf.pCurPicturePtr->nNalRefIdc;
            		hCtx->nSliceType = hCtx->frmBufInf.pCurPicturePtr->nPictType;
            		hCtx->nFrmNum   = hCtx->frmBufInf.pCurPicturePtr->nFrmNum;
            	 	if(hCtx->frmBufInf.pCurPicturePtr != NULL)
            	   	{
            	 		hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = 1;
            	   	}
            		hCtx->nDecFrameStatus = H264_END_DEC_FRAME;
            	    return VRESULT_DEC_FRAME_ERROR;
            	}
            }
            else
            {
                hCtx->bFstField = 0;
                if(hCtx->nLastFrmNum != hCtx->nFrmNum)
                {
                	logd("the top field and the bottom field is not matched\n");
                	if(hCtx->frmBufInf.pCurPicturePtr->pVPicture != NULL)
                	{
                    	FbmReturnBuffer(hCtx->pFbm, hCtx->frmBufInf.pCurPicturePtr->pVPicture, 0);
                    	hCtx->frmBufInf.pCurPicturePtr->pVPicture = NULL;
                		hCtx->frmBufInf.picture[hCtx->frmBufInf.pCurPicturePtr->nDispBufIndex].pVPicture = NULL;
                		hCtx->frmBufInf.picture[hCtx->frmBufInf.pCurPicturePtr->nDecodeBufIndex].pVPicture = NULL;
                	}
                	hCtx->frmBufInf.pCurPicturePtr->nReference = 0;
                	hCtx->bFstField = 1;
                }
            }
        }
        else
        {
            hCtx->bFstField = (hCtx->nPicStructure!=PICT_FRAME);
        }
		  
        if(bFstMbInSlice==0 && ((hCtx->nPicStructure==PICT_FRAME)||(hCtx->bFstField==1)))
        {   
            if(H264FrameStart(h264DecCtx, hCtx) == VDECODE_RESULT_NO_FRAME_BUFFER)   //  request frame buffer and decode the frame
            {   
                //fbm_print_status(hCtx->pFbm);
            	logv("request buffer failed\n");
                hCtx->nPicStructure = lastPicStructure;
                hCtx->bFstField = lastFieldFlag;
                return VDECODE_RESULT_NO_FRAME_BUFFER;
            }
        }
    }

    if(hCtx->frmBufInf.pCurPicturePtr == NULL)
    {
    	return VRESULT_ERR_FAIL;
    }

    hCtx->frmBufInf.pCurPicturePtr->nFrmNum = hCtx->nFrmNum;
    hCtx->frmBufInf.pCurPicturePtr->nNalRefIdc = hCtx->nNalRefIdc;
    hCtx->frmBufInf.pCurPicturePtr->nPictType = hCtx->nSliceType;

    hCtx->nMbX = bFstMbInSlice % hCtx->pCurSps->nMbWidth;
    hCtx->nMbY = (bFstMbInSlice/hCtx->pCurSps->nMbWidth)<<(hCtx->bMbAffFrame);
	    
	    
    if(hCtx->nPicStructure == PICT_FRAME)
    {
        hCtx->nCurPicNum = hCtx->nFrmNum;
        hCtx->nMaxPicNum = 1<<hCtx->pCurSps->nLog2MaxFrmNum;

    }
    else
    {
        hCtx->nCurPicNum = 2*hCtx->nFrmNum + 1;
        hCtx->nMaxPicNum = 1<<(hCtx->pCurSps->nLog2MaxFrmNum + 1);
    }

    if(hCtx->bIdrFrmFlag == 1)
    {
        tmp = H264GetUeGolomb(h264DecCtx);  // idr_pic_id
    }

    if(hCtx->pCurSps->nPocType==0)
    {  
        hCtx->nPocLsb = H264GetBits(h264DecCtx, hCtx->pCurSps->nLog2MaxPocLsb);
        
        if(hCtx->pCurPps->pPicOrderPresent==1 && hCtx->nPicStructure==PICT_FRAME)
        {
            hCtx->nDeltaPocBottom = H264GetSeGolomb(h264DecCtx);
        }
    }
    if(hCtx->pCurSps->nPocType==1 && !hCtx->pCurSps->bDeltaPicOrderAlwaysZeroFlag)
    {
        hCtx->nDeltaPoc[0] = H264GetSeGolomb(h264DecCtx);
        if(hCtx->pCurPps->pPicOrderPresent==1 && hCtx->nPicStructure==PICT_FRAME)
        {
            hCtx->nDeltaPoc[1] = H264GetSeGolomb(h264DecCtx);
        }
    }
    
    if(bFstMbInSlice==0)
    {
    	H264InitPoc(hCtx);
    }

    if(hCtx->pCurPps->pRedundatPicCntPresent)
    {   
        hCtx->nRedundantPicCount = H264GetUeGolomb(h264DecCtx);
    }

    // set defaults, might be overriden a few line later
    hCtx->nRefCount[0] = hCtx->pCurPps->nRefCount[0];
    hCtx->nRefCount[1] = hCtx->pCurPps->nRefCount[1];

    hCtx->nListCount = 0;
    hCtx->bDirectSpatialMvPred = 0;
    hCtx->bNumRefIdxActiveOverrideFlag = 0;
    
    if(hCtx->nSliceType==H264_P_TYPE || hCtx->nSliceType==H264_SP_TYPE || hCtx->nSliceType==H264_B_TYPE)
    {
        if(hCtx->nSliceType == H264_B_TYPE)
        {
            hCtx->bDirectSpatialMvPred = H264GetBits(h264DecCtx, 1);
        }
        hCtx->bNumRefIdxActiveOverrideFlag = H264GetBits(h264DecCtx, 1);
        if(hCtx->bNumRefIdxActiveOverrideFlag == 1)
        {
            hCtx->nRefCount[0] = H264GetUeGolomb(h264DecCtx)+1;
			
            if(hCtx->nSliceType == H264_B_TYPE)
            {
                hCtx->nRefCount[1] = H264GetUeGolomb(h264DecCtx)+1;
            }
            if(hCtx->nRefCount[0]>32 || hCtx->nRefCount[1]>32)
            {
            	//log("nReference overflow\n");
            	hCtx->nRefCount[0] = 1;
            	hCtx->nRefCount[1] = 1;
            	logv("refcount is more than 32\n");
            }
        }
        hCtx->nListCount = (hCtx->nSliceType == H264_B_TYPE)? 2 : 1;
    }


    if(!defaultRefListDone)
    {
        H264FillDefaultRefList(hCtx, h264Dec->nDecStreamIndex);
    }

    if(H264DecodeRefPicListReordering(h264DecCtx, hCtx) < 0)
    {
        logv("reference pic list reordering error\n");
    }


    if(bFstMbInSlice==0 && ((hCtx->nPicStructure==PICT_FRAME)||(hCtx->bFstField==1)))
    {
        H264CalculatePicturePts(hCtx);
    }
    hCtx->vbvInfo.nValidDataPts = H264VDEC_ERROR_PTS_VALUE;

    if((hCtx->pCurPps->bWeightedPred &&(hCtx->nSliceType==H264_P_TYPE || hCtx->nSliceType==H264_SP_TYPE))
    		||(hCtx->pCurPps->nWeightedBIpredIdc>0 && hCtx->nSliceType==H264_B_TYPE))
    {
        H264CongigureWeightTableRegisters(h264DecCtx, hCtx);
    }

    if(hCtx->nNalRefIdc)
    {
        H264DecodeRefPicMarking(h264DecCtx, hCtx);
    }

    if(hCtx->nSliceType!=H264_I_TYPE && hCtx->nSliceType!=H264_SI_TYPE && hCtx->pCurPps->bCabac)
    {
        tmp = H264GetUeGolomb(h264DecCtx);

        if(tmp > 2)
        {
            logv("nCabacInitIdc is %d, larger than 2\n", tmp);
        }
        hCtx->nCabacInitIdc = tmp;
    }

    hCtx->nLastQscaleDiff = 0;
	
    tmp = hCtx->pCurPps->nInitQp + H264GetSeGolomb(h264DecCtx);

    if(tmp > 51)
    {
        logv("nQscale is %d, larger than 51\n", tmp);

    }
    hCtx->nQscale = tmp;
    
    hCtx->nChromaQp[0] = 0;
    hCtx->nChromaQp[1] = 0;
    
    if(hCtx->nSliceType == H264_SP_TYPE)
    {
        H264GetBits(h264DecCtx, 1);   // sp for switch flag
    }
	
    if(hCtx->nSliceType == H264_SP_TYPE || hCtx->nSliceType == H264_SI_TYPE)
    {
        H264GetSeGolomb(h264DecCtx);
    }
	
    hCtx->bDisableDeblockingFilter = 0;
    hCtx->nSliceAlphaC0offset = 0;
    hCtx->nSliceBetaoffset = 0;

    if(hCtx->pCurPps->bDeblockingFilterParamPresent)
    {
        hCtx->bDisableDeblockingFilter  = H264GetUeGolomb(h264DecCtx);

        if(hCtx->bDisableDeblockingFilter != 1)
        {
            hCtx->nSliceAlphaC0offset = H264GetSeGolomb(h264DecCtx)&0x0f;
            hCtx->nSliceBetaoffset = H264GetSeGolomb(h264DecCtx)&0x0f;
        }
    }
    hCtx->nLastSliceType = nSliceType;
    hCtx->nCurSliceNum += 1;
    return VDECODE_RESULT_OK;
}   
