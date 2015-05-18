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
#include "mpeg2Dec.h"

extern void Mpeg2DecoderReset(DecoderInterface* pSelf);
#define VALID  1
#define INVALID 0


/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2InitDecode(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    int16_t i = 0;
    pMpeg2Dec->frmBufInfo.pCurFrm        = NULL;
    pMpeg2Dec->frmBufInfo.pLastRefFrm    = NULL; 
    pMpeg2Dec->frmBufInfo.pForRefFrm     = NULL;
    pMpeg2Dec->frmBufInfo.pBacRefFrm     = NULL;

    pMpeg2Dec->sbmInfo.pSbm = 0;
    Mpeg2GetVersion(pMpeg2Dec);
    
    for(i=0; i<MPEG2VDEC_MAX_STREAM_NUM; i++)
    {
        pMpeg2Dec->sbmInfo.vbvStreamData[i] = NULL;
    }
    pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx = -1;
    pMpeg2Dec->sbmInfo.nCurUpdateStrmIdx  = -1;
    pMpeg2Dec->sbmInfo.nStrmLen = 0;
        
    pMpeg2Dec->sbmInfo.pSbmBuf 	 	   = NULL;
    pMpeg2Dec->sbmInfo.pReadPtr 	   = NULL;
    pMpeg2Dec->sbmInfo.pSbmBufEnd 	   = NULL;
    pMpeg2Dec->sbmInfo.nSbmBufSize 	   = 0;
    pMpeg2Dec->sbmInfo.nSbmDataSize    = 0;
    pMpeg2Dec->sbmInfo.nSbmDataPts     = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->sbmInfo.nValidDataPts   = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->sbmInfo.bGetFstDataFlag = 0;
    
    memset(&pMpeg2Dec->picInfo, 0, sizeof(Mpeg2PicInfo));
    pMpeg2Dec->nDecStep = MP2VDEC_PARSE_HEADER;
    pMpeg2Dec->bDecPicOkFlag  = 0;
    pMpeg2Dec->bSearchStdFlag = 0;
    pMpeg2Dec->bResetHwFlag   = 0;

    pMpeg2Dec->bDecEndFlag 	  = 0;
    pMpeg2Dec->bFstSetVbvFlag = 0;
    pMpeg2Dec->picInfo.bMallocFrmBufFlag = 0;
    pMpeg2Dec->picInfo.eLastPicType 	 = MP2VDEC_B_PIC;
    pMpeg2Dec->picInfo.nCurPicPts 		 = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->picInfo.nNextPicPts 		 = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->sbmInfo.nValidDataPts 	 = MP2VDEC_ERROR_PTS_VALUE; 
    pMpeg2Dec->sbmInfo.nLastDataPts      = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->uRegisterBaseAddr 		 = (uint32_t)ve_get_reglist(REG_GROUP_MPEG_DECODER);
    pMpeg2Dec->bNormalChipFlag = 1;
    pMpeg2Dec->bSearchFstStartCodeFlag = 1;
}
/****************************************************************************************************/
/*****************************************************************************************************/
void Mpeg2FlushPictures(Mpeg2DecodeInfo* pMpeg2Dec, Fbm* pMpeg2Fbm, uint8_t bFlushPictureFlag)
{
	VideoPicture* fbmBuf[8]={NULL};
	int32_t i = 0;
	int32_t j = 0;
	int32_t frmBufIndex = 0;

	if(pMpeg2Fbm == NULL)
	{
		return;
	}

	if(pMpeg2Dec->picInfo.nFrmNum > 1)
	{
		if((pMpeg2Dec->frmBufInfo.pBacRefFrm!=NULL)&&(pMpeg2Dec->frmBufInfo.pBacRefFrm->nWidth!=0))
		{
			fbmBuf[0] = pMpeg2Dec->frmBufInfo.pBacRefFrm;
			pMpeg2Dec->frmBufInfo.pBacRefFrm = NULL;
		}
		else if((pMpeg2Dec->frmBufInfo.pForRefFrm!=NULL)&&(pMpeg2Dec->frmBufInfo.pForRefFrm->nWidth!=0))
		{
			fbmBuf[0] = pMpeg2Dec->frmBufInfo.pForRefFrm;
			pMpeg2Dec->frmBufInfo.pForRefFrm = NULL;
		}
	}

	frmBufIndex = 1;

	if(pMpeg2Dec->frmBufInfo.pBacRefFrm != NULL)
	{
		fbmBuf[frmBufIndex++] = pMpeg2Dec->frmBufInfo.pBacRefFrm;
		pMpeg2Dec->frmBufInfo.pBacRefFrm = NULL;
	}
	if(pMpeg2Dec->frmBufInfo.pForRefFrm != NULL)
	{
		fbmBuf[frmBufIndex++] = pMpeg2Dec->frmBufInfo.pForRefFrm;
		pMpeg2Dec->frmBufInfo.pForRefFrm = NULL;
	}
	if(pMpeg2Dec->frmBufInfo.pCurFrm != NULL)
	{
		fbmBuf[frmBufIndex++] = pMpeg2Dec->frmBufInfo.pCurFrm;
		pMpeg2Dec->frmBufInfo.pCurFrm = NULL;
	}
	if(pMpeg2Dec->frmBufInfo.pLastRefFrm != NULL)
	{
		fbmBuf[frmBufIndex++] = pMpeg2Dec->frmBufInfo.pLastRefFrm;
		pMpeg2Dec->frmBufInfo.pLastRefFrm = NULL;
	}

	for(i=0; i<(frmBufIndex-1); i++)
	{
		for(j=i+1; j<frmBufIndex; j++)
		{
			if(fbmBuf[i] == fbmBuf[j])
			{
				fbmBuf[j] = NULL;
			}
		}
	}

	if(fbmBuf[0]!= NULL)
	{
		FbmReturnBuffer(pMpeg2Fbm, fbmBuf[0], bFlushPictureFlag);
	}
	for(i=1; i<frmBufIndex; i++)
	{
		if(fbmBuf[i] != NULL)
		{
			FbmReturnBuffer(pMpeg2Fbm, fbmBuf[i], 0);
		}
	}
 }

/***********************************************************************************************/
/***********************************************************************************************/
void Mpeg2DecodeSetSbmBuf(uint8_t* pSbmBase, uint32_t nSbmSize, Mpeg2DecodeInfo* pMpeg2Dec)
{   
    int16_t i = 0;
    pMpeg2Dec->sbmInfo.pSbmBuf 		= pSbmBase;
    pMpeg2Dec->sbmInfo.nSbmBufSize  = nSbmSize;
    pMpeg2Dec->sbmInfo.pSbmBufEnd   = pMpeg2Dec->sbmInfo.pSbmBuf + nSbmSize -1;
    pMpeg2Dec->sbmInfo.nSbmDataSize = 0;
    pMpeg2Dec->sbmInfo.pReadPtr 	= pSbmBase;

    for(i=0; i<MPEG2VDEC_MAX_STREAM_NUM; i++)
    {   
        if(pMpeg2Dec->sbmInfo.vbvStreamData[i]!=NULL)
        {
            SbmFlushStream(pMpeg2Dec->sbmInfo.pSbm,pMpeg2Dec->sbmInfo.vbvStreamData[i]);
            pMpeg2Dec->sbmInfo.vbvStreamData[i]->nLength= 0;
        }
        pMpeg2Dec->sbmInfo.vbvStreamData[i] = NULL;
    }
    pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx = -1;
    pMpeg2Dec->sbmInfo.nCurUpdateStrmIdx  = -1;
    pMpeg2Dec->sbmInfo.nStrmLen = 0;
}

/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2SbmUpdateReadPointer(Mpeg2DecodeInfo* pMpeg2Dec, uint32_t nUpdateDataSize)
{
    if(pMpeg2Dec->sbmInfo.nSbmDataSize < nUpdateDataSize)
    {  
        loge("the data size of pSbm buffer is smaller than the update data size.\n");
        loge("pMpeg2Dec->sbmInfo.nSbmDataSize=%d\n",pMpeg2Dec->sbmInfo.nSbmDataSize);
        loge("nUpdateDataSize=%d\n",nUpdateDataSize);
    }
    else
    {
        if((pMpeg2Dec->sbmInfo.pReadPtr+nUpdateDataSize) <= pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
            pMpeg2Dec->sbmInfo.pReadPtr += nUpdateDataSize;
            pMpeg2Dec->sbmInfo.nSbmDataSize -= nUpdateDataSize;
        }
        else
        {
            pMpeg2Dec->sbmInfo.pReadPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pReadPtr+nUpdateDataSize-pMpeg2Dec->sbmInfo.nSbmBufSize);
            pMpeg2Dec->sbmInfo.nSbmDataSize -= nUpdateDataSize;
        }
    }
}

/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2SetDefaultForMpeg1(Mpeg2DecodeInfo* pMpeg2Dec)
{
    //picture_coding_extension
    pMpeg2Dec->picInfo.uAlternateScan 		 = 0;
    pMpeg2Dec->picInfo.uIntraVlcFormat 		 = 0;
    pMpeg2Dec->picInfo.uQScaleType  		 = 0;
    pMpeg2Dec->picInfo.uConcealMotionVectors = 0;
    pMpeg2Dec->picInfo.uFrmPredFrmDet 		 = 1;
    pMpeg2Dec->picInfo.eCurPicStructure 	 = MP2VDEC_FRAME;
    pMpeg2Dec->picInfo.nIntraDcPresion  	 = 0;
    pMpeg2Dec->picInfo.bTopFieldFstFlag 	 = 1;
    pMpeg2Dec->picInfo.bRepeatFstFieldFlag   = 0;
    pMpeg2Dec->picInfo.uChromaFormat 		 = 0;
    pMpeg2Dec->picInfo.uProgressiveFlag 	 = 1;
    pMpeg2Dec->picInfo.uProgressiveSequence  = 1;
}

/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2PictureHeaderAction(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint8_t  eLastPicType = 0;
    uint8_t  eCurPicType  = 0;
    
    if(pMpeg2Dec->picInfo.bGetSeqFlag==1)
    {
        // has decoded one pic, or the current pic is I pic 
        if((pMpeg2Dec->picInfo.bGetForFrmFlag==1) ||(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_I_PIC))
        {
            if(pMpeg2Dec->picInfo.eCurPicStructure == MP2VDEC_FRAME)
            {
                pMpeg2Dec->picInfo.bFstFieldFlag = 1;
            }
            else
            {
                eLastPicType = (pMpeg2Dec->picInfo.eLastPicType==MP2VDEC_B_PIC)? MP2VDEC_B_PIC : MP2VDEC_P_PIC;
                eCurPicType  = (pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_B_PIC)? MP2VDEC_B_PIC : MP2VDEC_P_PIC;

                if((pMpeg2Dec->picInfo.eLastPicStructure==MP2VDEC_FRAME) || (eLastPicType!=eCurPicType))
                {  
                    
                    pMpeg2Dec->picInfo.bFstFieldFlag = 1;
                }
                else
                {
                    pMpeg2Dec->picInfo.bFstFieldFlag = 1-pMpeg2Dec->picInfo.bFstFieldFlag;
          
                }
                if(pMpeg2Dec->picInfo.bFstFieldFlag==1)
                {
                    pMpeg2Dec->picInfo.bTopFieldFstFlag = (pMpeg2Dec->picInfo.eCurPicStructure==MP2VDEC_TOP_FIELD)? 1: 0;
                }
                else
                {
                    pMpeg2Dec->picInfo.bTopFieldFstFlag = (pMpeg2Dec->picInfo.eCurPicStructure==MP2VDEC_BOT_FIELD)? 1: 0;
                }
            }

            pMpeg2Dec->picInfo.eLastPicStructure = pMpeg2Dec->picInfo.eCurPicStructure;
            pMpeg2Dec->picInfo.eLastPicType      = pMpeg2Dec->picInfo.eCurPicType;
        }
    }

    if(pMpeg2Dec->picInfo.bGetBacFrmFlag == 1)
    {
         pMpeg2Dec->picInfo.bDropFrmFlag = 0;
    }
    else 
    {
        if(pMpeg2Dec->picInfo.bGetSeqFlag == 0)
        {
            pMpeg2Dec->picInfo.bDropFrmFlag = 1;
        }
        else if(pMpeg2Dec->picInfo.bFstFieldFlag == 1)
        {
            if((pMpeg2Dec->picInfo.eCurPicType != MP2VDEC_I_PIC) && (pMpeg2Dec->picInfo.bGetForFrmFlag==0))
            {
                pMpeg2Dec->picInfo.bDropFrmFlag = 1;
            }
            else if(pMpeg2Dec->picInfo.bGetForFrmFlag==1)
            {
                if((pMpeg2Dec->picInfo.eCurPicType!= MP2VDEC_I_PIC) && (pMpeg2Dec->picInfo.eCurPicType!= MP2VDEC_P_PIC))
                {
                    pMpeg2Dec->picInfo.bDropFrmFlag  = 1;
                }
            }
        }
    }
}

/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2ComputeAspectRatio(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t uAspectRatioInfo)
{   
    if(pMpeg2Dec->picInfo.nPicWidth == 0)
    {
        pMpeg2Dec->picInfo.nAspectRatio = 1000;
    }
    else
    {
        switch(uAspectRatioInfo)
        {
            case 2:
            case 8:
            {
                //pMpeg2Dec->picInfo.nAspectRatio = (pMpeg2Dec->picInfo.nPicHeight*4*1000)/(3*pMpeg2Dec->picInfo.nPicWidth);
                pMpeg2Dec->picInfo.nAspectRatio = (4*1000)/(3);
                break;
            }
            case 3:
            {
                //pMpeg2Dec->picInfo.nAspectRatio = (pMpeg2Dec->picInfo.nPicHeight*16*1000)/(9*pMpeg2Dec->picInfo.nPicWidth);
                pMpeg2Dec->picInfo.nAspectRatio = (16*1000)/(9);
                break;
            }
            case 4:
            {
                pMpeg2Dec->picInfo.nAspectRatio = 3*1000/2;
                break;
            }
            default:
            {
               pMpeg2Dec->picInfo.nAspectRatio = 1000;
               break;
            }
        }
    }
    return;
}

/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ComputeScaleRatio(uint32_t nOrgSize, uint32_t nDstSize)
{   
    uint8_t uScaleRatio = 0;
    
    if(nDstSize == 0)
    {
        uScaleRatio = 0;
    }
    else if(nOrgSize > nDstSize)
    {
        uScaleRatio = nOrgSize / nDstSize;
        switch(uScaleRatio)
        {
            case 0:
            case 1:
            case 2:
            {
                uScaleRatio = 1;
                break;
            }
            case 3:
            case 4:
            {
                uScaleRatio = 2;
                break;
            }
            default:
            {
                uScaleRatio = 2; //* 1/8 scale down is not support.
                break;
            }
        }
    }

    return uScaleRatio;
}

/****************************************************************************************************/
/****************************************************************************************************/

int8_t Mpeg2RequestFrameMemory(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec, uint32_t nPicWidth, uint32_t nPicHeight)
{   
	uint8_t  nFrameNum 		= 0;
	uint32_t nRefPixelFormat = 0;
	int32_t nProgressiveFlag = 0;
	FbmCreateInfo mFbmCreateInfo;

	nRefPixelFormat = PIXEL_FORMAT_YUV_MB32_420;

    pMpeg2Dec->picInfo.nPicWidth  = nPicWidth;
    pMpeg2Dec->picInfo.nPicHeight = nPicHeight;
    
    pMpeg2Ctx->pFbm = NULL;

    nFrameNum = 9;

    nProgressiveFlag = pMpeg2Dec->picInfo.uProgressiveFlag;

    memset(&mFbmCreateInfo, 0, sizeof(FbmCreateInfo));
    mFbmCreateInfo.nFrameNum          = nFrameNum;
    mFbmCreateInfo.nWidth             = nPicWidth;
    mFbmCreateInfo.nHeight            = nPicHeight;
    mFbmCreateInfo.ePixelFormat       = nRefPixelFormat;
    mFbmCreateInfo.nBufferType        = BUF_TYPE_REFERENCE_DISP;
    mFbmCreateInfo.bProgressiveFlag   = nProgressiveFlag;
    pMpeg2Ctx->pFbm = FbmCreate(&mFbmCreateInfo);
    pMpeg2Dec->picInfo.nDispWidth = nPicWidth;
    pMpeg2Dec->picInfo.nDispHeight = nPicHeight;
    if(pMpeg2Ctx->pFbm == NULL)
    {
    	return VDECODE_RESULT_UNSUPPORTED;
    }
    return VDECODE_RESULT_OK;
}


/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ParseSequenceInfo(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec, uint8_t* pDataBuf, uint8_t uParseMode)
{   
    uint8_t  i =0;
    uint8_t  uAspectRatioInfo = 0;
    uint8_t  uNextByte 		 = 0;
    uint8_t  uPrebit   		 = 0;
    uint16_t nOffset 		 = 0;
    uint32_t nPicWidth 		 = 0;
    uint32_t nPicHeight 		 = 0;
    int32_t nCodedFrameRatio = 0;
    
    uint32_t aMpgFrmRate[16] = {00000, 23976, 24000, 25000,
                          29970, 30000, 50000, 59940,
                          60000, 00000, 00000, 00000,
                          00000, 00000, 00000, 00000};
    
    uint8_t aMpeg2DefaultIntraQuantizerMatrix[64] =
       {
           8,  16, 19, 22, 26, 27, 29, 34,
           16, 16, 22, 24, 27, 29, 34, 37,
           19, 22, 26, 27, 29, 34, 34, 38,
           22, 22, 26, 27, 29, 34, 37, 40,
           22, 26, 27, 29, 32, 35, 40, 48,
           26, 27, 29, 32, 35, 40, 48, 58,
           26, 27, 29, 34, 38, 46, 56, 69,
           27, 29, 35, 38, 46, 56, 69, 83
       };
       
       
       uint8_t aMpeg2Normal2zigzag[64] =
       {
           0,  1,  8,  16, 9,  2,  3,  10,
           17, 24, 32, 25, 18, 11, 4,  5,
           12, 19, 26, 33, 40, 48, 41, 34,
           27, 20, 13, 6,  7,  14, 21, 28,
           35, 42, 49, 56, 57, 50, 43, 36,
           29, 22, 15, 23, 30, 37, 44, 51,
           58, 59, 52, 45, 38, 31, 39, 46,
           53, 60, 61, 54, 47, 55, 62, 63
       };
    
    pMpeg2Dec->picInfo.bGetSeqFlag = 1;
    nPicWidth  = (pDataBuf[4]<<4) | (pDataBuf[5]>>4);
    nPicHeight = ((pDataBuf[5]&0x0f)<<8) | (pDataBuf[6]);

    if((pMpeg2Dec->picInfo.nPicWidth==0) || (pMpeg2Dec->picInfo.nPicHeight==0))
    {   
        pMpeg2Dec->picInfo.nMbXNum = (nPicWidth+15)>>4;
        pMpeg2Dec->picInfo.nMbYNum = (nPicHeight+15)>>4;
        pMpeg2Dec->picInfo.nMbWidth = (pMpeg2Dec->picInfo.nMbXNum<<4);
        pMpeg2Dec->picInfo.nMbHeight = (pMpeg2Dec->picInfo.nMbYNum<<4);

        if(pMpeg2Dec->picInfo.bMallocFrmBufFlag == 0)
        {
            if(Mpeg2RequestFrameMemory(pMpeg2Ctx, pMpeg2Dec, nPicWidth, nPicHeight) != VDECODE_RESULT_OK)
            {   
                return VDECODE_RESULT_UNSUPPORTED;
            }
            pMpeg2Dec->picInfo.bMallocFrmBufFlag = 1;
        }
    }
    else if((nPicWidth!=pMpeg2Dec->picInfo.nPicWidth)||(nPicHeight!=pMpeg2Dec->picInfo.nPicHeight))
    {
         loge("change the size.need extra process.\n");
         if((pMpeg2Dec->picInfo.nPicWidth>nPicWidth) &&(pMpeg2Dec->picInfo.nPicHeight>nPicHeight))
         {   
             pMpeg2Dec->picInfo.nPicWidth = nPicWidth;
             pMpeg2Dec->picInfo.nPicHeight = nPicHeight;
             pMpeg2Dec->picInfo.nMbXNum = (pMpeg2Dec->picInfo.nPicWidth+15)>>4;
             pMpeg2Dec->picInfo.nMbYNum = (pMpeg2Dec->picInfo.nPicHeight+15)>>4;
             pMpeg2Dec->picInfo.nMbWidth = (pMpeg2Dec->picInfo.nMbXNum<<4);
             pMpeg2Dec->picInfo.nMbHeight = (pMpeg2Dec->picInfo.nMbYNum<<4);
         }
    }

    if(pMpeg2Dec->picInfo.nFrmRate == 0)
    {
        pMpeg2Dec->picInfo.nFrmRate = aMpgFrmRate[pDataBuf[7]&0x0f];
        if(pMpeg2Dec->picInfo.nFrmRate == 0)
        {
            pMpeg2Dec->picInfo.nFrmRate = aMpgFrmRate[3];
        }
        pMpeg2Dec->picInfo.nPicDuration = 1000;
        pMpeg2Dec->picInfo.nPicDuration *= (1000*1000);
        pMpeg2Dec->picInfo.nPicDuration /= pMpeg2Dec->picInfo.nFrmRate; 
    }

    uAspectRatioInfo = (pDataBuf[7]>>4) & 0x0f;
    if(uAspectRatioInfo == 0)
    {
        pMpeg2Dec->picInfo.nAspectRatio = 1000;
    }
    else
    {
        Mpeg2ComputeAspectRatio(pMpeg2Dec, uAspectRatioInfo);
    }

    pMpeg2Dec->picInfo.bLoadIntraQuartMatrixFlag = (pDataBuf[11]>>1) & 0x01;
    
    nOffset = 12;
    uNextByte = pDataBuf[11];

    if(pMpeg2Dec->picInfo.bLoadIntraQuartMatrixFlag == 1)
    {
        for(i=0; i<64; i++)
        {
            uPrebit   = uNextByte;
            uNextByte = pDataBuf[nOffset++];
            pMpeg2Dec->picInfo.aIntraQuartMatrix[i] = (uPrebit<<7)| (uNextByte>>1);
        }
    }
    else
    {
        for(i=0; i<64; i++)
        {
            pMpeg2Dec->picInfo.aIntraQuartMatrix[i] = aMpeg2DefaultIntraQuantizerMatrix[aMpeg2Normal2zigzag[i]];
        }
    } 
    pMpeg2Dec->picInfo.bLoadNonIntraQuartMatrixFlag = uNextByte&0x01;
        
    if(pMpeg2Dec->picInfo.bLoadNonIntraQuartMatrixFlag == 1)
    {
        for(i=0; i<64; i++)
        {
            uNextByte = pDataBuf[nOffset++];
            pMpeg2Dec->picInfo.aNonIntraQuartMatrix[i] = uNextByte;
        }
    }
    else
    {
        for(i=0; i<64; i++)
        {
            pMpeg2Dec->picInfo.aNonIntraQuartMatrix[i] = 16;
        }
    }
    if(uParseMode == MP2VDEC_PARSE_SEQ_MODE1)
    {   
        Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nOffset);
    }

    nCodedFrameRatio = pMpeg2Dec->picInfo.nPicWidth*1000/pMpeg2Dec->picInfo.nPicHeight;

    if(nCodedFrameRatio <= 1460)
    {
    	pMpeg2Dec->picInfo.bCodedFrameRatio = MPEG_CODED_FRAME_RATIO_4_3;
    }
    else  if(nCodedFrameRatio <= 1660)
    {
    	pMpeg2Dec->picInfo.bCodedFrameRatio = MPEG_CODED_FRAME_RATIO_14_9;
    }
    else if(nCodedFrameRatio <= 1900)
    {
    	pMpeg2Dec->picInfo.bCodedFrameRatio = MPEG_CODED_FRAME_RATIO_16_9;
    }
    else
    {
    	pMpeg2Dec->picInfo.bCodedFrameRatio = MPEG_CODED_FRAME_RATIO_OTHER;
    }
    return VDECODE_RESULT_OK;
}

/***************************************************************************************************/
/***************************************************************************************************/
int8_t Mpeg2ParsePictureInfo(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t*pDataBuf)
{
    uint32_t uNextdword = 0;
    uint32_t uNextByte  = 0;
    uint32_t uPreByte   = 0;
    uint32_t uCurByte   = 0;

    pMpeg2Dec->picInfo.bGetPicFlag = 1;
    uNextdword  = (pDataBuf[4]<<24);
    uNextdword |= (pDataBuf[5]<<16);
    uNextdword |= (pDataBuf[6]<<8);
    uNextdword |= (pDataBuf[7]<<0);

    pMpeg2Dec->picInfo.eCurPicType = (uNextdword >> 19) & 0x07;
    uNextByte = (uNextdword & 0x07);
    uPreByte  = uNextByte;

    if((pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_P_PIC)||(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_B_PIC))
    {
        uNextByte = pDataBuf[8];
        uCurByte  = (uPreByte<<5)|(uNextByte>>3);
        pMpeg2Dec->picInfo.uFullPerForVector = (uCurByte>>7) & 0x01;
        pMpeg2Dec->picInfo.uForFCode = (uCurByte>>4) & 0x07;
        if(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_B_PIC)
        {
            pMpeg2Dec->picInfo.uFullPerBacVector = (uCurByte>>3)&0x01;
            pMpeg2Dec->picInfo.uBacFCode = uCurByte & 0x07;
        }
    }

    if(pMpeg2Dec->picInfo.bIsMpeg1Flag == 0)
    {
        pMpeg2Dec->picInfo.uFullPerForVector = 0;
        pMpeg2Dec->picInfo.uForFCode = 7;
        pMpeg2Dec->picInfo.uFullPerBacVector = 0;
        pMpeg2Dec->picInfo.uBacFCode = 7;
    }
    Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 8);
    return VDECODE_RESULT_OK;
}

/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ParsePictureExtensionInfo(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t*pDataBuf)
{   
    uint8_t  nOffset 	  = 0;
    uint8_t  bCompDispFlag = 0;
    uint8_t  uNextByte 	  = 0;
    uint32_t uNextWord 	  = 0;

    uNextWord  = (pDataBuf[4]<<24);
    uNextWord |= (pDataBuf[5]<<16);
    uNextWord |= (pDataBuf[6]<<8);
    uNextWord |= (pDataBuf[7]<<0);

    pMpeg2Dec->picInfo.uFCode00 = (uNextWord>>24) & 0x0f;
    pMpeg2Dec->picInfo.uFCode01 = (uNextWord>>20) & 0x0f;
    pMpeg2Dec->picInfo.uFCode10 = (uNextWord>>16) & 0x0f;
    pMpeg2Dec->picInfo.uFCode11 = (uNextWord>>12) & 0x0f;
    pMpeg2Dec->picInfo.uIntraDcPrecision     = (uNextWord>>10)&0x03;//bit2
    pMpeg2Dec->picInfo.eCurPicStructure      = (uNextWord>>8)&0x03;    //bit2 //bit8,9
    pMpeg2Dec->picInfo.bTopFieldFstFlag      = (uNextWord>>7)&0x01;//bit1
    pMpeg2Dec->picInfo.uFrmPredFrmDet        = (uNextWord>>6)&0x01;//bit1
    pMpeg2Dec->picInfo.uConcealMotionVectors = (uNextWord>>5)&0x01;//bit1
    pMpeg2Dec->picInfo.uQScaleType           = (uNextWord>>4)&0x01;//bit1
    pMpeg2Dec->picInfo.uIntraVlcFormat       = (uNextWord>>3)&0x01;//bit1
    pMpeg2Dec->picInfo.uAlternateScan        = (uNextWord>>2)&0x01;//bit1
    pMpeg2Dec->picInfo.bRepeatFstFieldFlag   = ((uNextWord>>1)&0x01)?1:0;//bit1
    pMpeg2Dec->picInfo.uChroma420Type        = (uNextWord)&0x01;//bit1

    uNextByte = pDataBuf[8];
    pMpeg2Dec->picInfo.uProgressiveFlag = ((uNextByte>>7)&0x01)?1:0;//bit1    
    pMpeg2Dec->picInfo.uProgressiveFlag = pMpeg2Dec->picInfo.uProgressiveFlag&&pMpeg2Dec->picInfo.uProgressiveSequence;
    bCompDispFlag = (uNextByte>>6) & 0x01;
    nOffset = (bCompDispFlag==1)? 7 : 5;
    Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nOffset+4);
    return VDECODE_RESULT_OK;
}

/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2ParseQuantMatrixExtension(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t*pDataBuf)
{   
    uint8_t i 		 = 0;
    uint8_t uPreByte  = 0;
    uint8_t uNextByte = 0;
    uint8_t nOffset   = 5;
    uNextByte = pDataBuf[4];
    pMpeg2Dec->picInfo.bLoadIntraQuartMatrixFlag = (uNextByte>>3)&0x01;
    uNextByte &= 0x07;

    if(pMpeg2Dec->picInfo.bLoadIntraQuartMatrixFlag == 1)
    {
        for(i=0; i<64; i++)
        {
            uPreByte = uNextByte;
            uNextByte = pDataBuf[nOffset++];
            pMpeg2Dec->picInfo.aIntraQuartMatrix[i] = (uPreByte<<5) | (uNextByte>>3);
        }
    }
    
    pMpeg2Dec->picInfo.bLoadNonIntraQuartMatrixFlag = (uNextByte>>2)&0x01;

    if(pMpeg2Dec->picInfo.bLoadNonIntraQuartMatrixFlag == 1)
    {
        for(i=0; i<64; i++)
        {
            uPreByte = uNextByte;
            uNextByte = pDataBuf[nOffset++];
            pMpeg2Dec->picInfo.aNonIntraQuartMatrix[i] = (uPreByte<<6) | (uNextByte>>2);
        }
    }
    Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nOffset);
    return;
}
    
  
/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ParseExtensionInfo(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t*pDataBuf)
{
    uint8_t uExtensionId = 0;
    uint8_t uChromFormat = 0;
    uExtensionId = (pDataBuf[4]>>4);
    switch(uExtensionId)
    {
        case MP2VDEC_SEQ_EXT_ID:              // usefull
        {  
            if(pMpeg2Dec->picInfo.eLastStartCode == MP2VDEC_SEQ_START_CODE)
            {   
                uChromFormat = (pDataBuf[5]>>1) & 0x03;
                if(uChromFormat != 1)
                    return VDECODE_RESULT_UNSUPPORTED;
                pMpeg2Dec->picInfo.uProgressiveSequence = (pDataBuf[5]& 0x0f)>>3;

                pMpeg2Dec->picInfo.bIsMpeg1Flag = 0;
                pMpeg2Dec->picInfo.bGetSeqFlag = 1;
                pMpeg2Dec->picInfo.eLastPicStructure = MP2VDEC_FRAME;
            }
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            break;
        }
        case MP2VDEC_PIC_EXT_ID:             // usefull
        {   
            if(pMpeg2Dec->picInfo.eLastStartCode == MP2VDEC_PIC_START_CODE)
            {
                 Mpeg2ParsePictureExtensionInfo(pMpeg2Dec, pDataBuf);
                 pMpeg2Dec->picInfo.bIsMpeg1Flag = 0;
                 //Mpeg2PictureHeaderAction(pMpeg2Dec);
            
            }
            else
            {
                Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            }
            break;
        }
        case MP2VDEC_PIC_QUANT_MATRIX_EXT_ID:  // usefull
        {   
            Mpeg2ParseQuantMatrixExtension(pMpeg2Dec, pDataBuf);
            break;
        }
        default:
        {   
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            break;
        }
    }

    return VDECODE_RESULT_OK;
}

/************************************************************************************************/
/************************************************************************************************/

void Mpeg2ComputePicPts(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint8_t  bFlag1 = 0;
    uint8_t  bFlag2 = 0;
    uint8_t  bFlag3 = 0;
    uint64_t nPicDuration = 0;

    if(pMpeg2Dec->picInfo.bFstFieldFlag == 0)
    {
        return;
    }
    
    if(pMpeg2Dec->picInfo.bRepeatFstFieldFlag == 1)
    {
        nPicDuration = pMpeg2Dec->picInfo.nPicDuration*3/2;
    }
    else
    {
        nPicDuration = pMpeg2Dec->picInfo.nPicDuration;
    } 
    
    bFlag1 = (pMpeg2Dec->sbmInfo.nSbmDataPts!= MP2VDEC_ERROR_PTS_VALUE);
    bFlag2 = ((pMpeg2Dec->sbmInfo.nSbmDataPts!=pMpeg2Dec->picInfo.nCurPicPts)&&(pMpeg2Dec->sbmInfo.nSbmDataPts!=0));
    bFlag3 = ((pMpeg2Dec->sbmInfo.nSbmDataPts==0)&&(pMpeg2Dec->picInfo.nFrmNum==0));
    
    pMpeg2Dec->picInfo.bHasPtsFlag = (bFlag1 &&(bFlag2||bFlag3));

    if(pMpeg2Dec->picInfo.bOnlyDispKeyFrmFlag == 1)   // only disp I frame 
    {
        if(pMpeg2Dec->sbmInfo.nSbmDataPts != MP2VDEC_ERROR_PTS_VALUE)
        {
            pMpeg2Dec->picInfo.nCurPicPts = pMpeg2Dec->sbmInfo.nSbmDataPts;
        }
        else if(pMpeg2Dec->picInfo.nNextPicPts != MP2VDEC_ERROR_PTS_VALUE)
        {
            pMpeg2Dec->picInfo.nCurPicPts = pMpeg2Dec->picInfo.nNextPicPts;
            pMpeg2Dec->picInfo.bHasPtsFlag = 1;
        }
        else
        {
            pMpeg2Dec->picInfo.nCurPicPts = MP2VDEC_ERROR_PTS_VALUE;
        }
        return;
    }
    
    if(pMpeg2Dec->picInfo.eCurPicType == MP2VDEC_B_PIC)
    {   
        if(pMpeg2Dec->picInfo.bHasPtsFlag == 1)
        {
            pMpeg2Dec->picInfo.nCurPicPts = pMpeg2Dec->sbmInfo.nSbmDataPts;
            pMpeg2Dec->picInfo.nNextPicPts = pMpeg2Dec->picInfo.nCurPicPts + nPicDuration;
            pMpeg2Dec->sbmInfo.nSbmDataPts = MP2VDEC_ERROR_PTS_VALUE;
        }
        else
        {
            pMpeg2Dec->picInfo.nCurPicPts = pMpeg2Dec->picInfo.nNextPicPts;
            pMpeg2Dec->picInfo.nNextPicPts = pMpeg2Dec->picInfo.nNextPicPts + nPicDuration; 
        }
    }
    else
    {   
        if(pMpeg2Dec->picInfo.bRepeatFstFieldFlag == 1)
        {
            nPicDuration = pMpeg2Dec->picInfo.nPicDuration*3/2;
        }
        else
        {
            nPicDuration = pMpeg2Dec->picInfo.nPicDuration;
        } 

        if(pMpeg2Dec->frmBufInfo.pBacRefFrm != NULL)
        {
            if(pMpeg2Dec->frmBufInfo.pBacRefFrm->nPts==MP2VDEC_ERROR_PTS_VALUE)
            {
                pMpeg2Dec->frmBufInfo.pBacRefFrm->nPts = pMpeg2Dec->picInfo.nNextPicPts;
            }
            nPicDuration = pMpeg2Dec->picInfo.nPicDuration;
            if(pMpeg2Dec->frmBufInfo.pBacRefFrm->bRepeatTopField==1)
            {
                nPicDuration = pMpeg2Dec->picInfo.nPicDuration*3/2;
            }
            pMpeg2Dec->picInfo.nNextPicPts = pMpeg2Dec->frmBufInfo.pBacRefFrm->nPts + nPicDuration;
        }
        else
        {
            if(pMpeg2Dec->picInfo.bHasPtsFlag==1)
            {
                pMpeg2Dec->picInfo.nNextPicPts = pMpeg2Dec->sbmInfo.nSbmDataPts + nPicDuration;
            }
            else
            {
                pMpeg2Dec->picInfo.nNextPicPts += nPicDuration;
            }
        }
        if(pMpeg2Dec->picInfo.bHasPtsFlag==1)
        {
            pMpeg2Dec->picInfo.nCurPicPts = pMpeg2Dec->sbmInfo.nSbmDataPts;
            pMpeg2Dec->sbmInfo.nSbmDataPts = MP2VDEC_ERROR_PTS_VALUE;
        }
    }
    return;
}


void Mpeg2ProcessActiveFormat(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t* ptr)
{
    int32_t i = 0;
    uint32_t nextCode = 0;
    uint8_t acticveFormatFlag = 0;
    uint8_t activeFormat = 0;

	nextCode = 0xffffffff;
	for(i=0; i<64; i++)
	{
		nextCode <<= 8;
		nextCode |= ptr[i];
		if(nextCode == 0x44544731)
		{
			break;
		}
		if(nextCode == 0x000001)
		{
			return;
		}
	}

	if(i >= 64)
	{
		return;
	}

	acticveFormatFlag = (ptr[i+1]>>6)& 0x01;
	if(acticveFormatFlag == 1)
	{
		activeFormat = ptr[i+2]& 0x0f;
	}

	pMpeg2Dec->picInfo.nLeftOffset = 0;
	pMpeg2Dec->picInfo.nRightOffset = 0;
	pMpeg2Dec->picInfo.nTopOffset = 0;
	pMpeg2Dec->picInfo.nBottomOffset = 0;

	switch(activeFormat)
	{
		case 2:            //box 16:9 (top)
            if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_4_3)
            {
            	pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nPicHeight -(pMpeg2Dec->picInfo.nPicWidth*9/16);
            }
			break;
		case 3:            //box 14:9 (top)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_4_3)
			{
				pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nPicHeight-(pMpeg2Dec->picInfo.nPicWidth*9/14);
			}
			else if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_16_9)
			{
				pMpeg2Dec->picInfo.nLeftOffset = pMpeg2Dec->picInfo.nPicWidth-(pMpeg2Dec->picInfo.nPicHeight*14/9);
				pMpeg2Dec->picInfo.nLeftOffset /= 2;
				pMpeg2Dec->picInfo.nRightOffset = pMpeg2Dec->picInfo.nLeftOffset;
			}
			break;
		case 4:            //box > 16:9 (center)
			pMpeg2Dec->picInfo.nTopOffset =  pMpeg2Dec->picInfo.nPicHeight-(pMpeg2Dec->picInfo.nPicWidth*10/19);
			pMpeg2Dec->picInfo.nTopOffset /= 2;
			pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nTopOffset;
			break;
		case 9:            //4:3 (center)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_16_9)
			{
				pMpeg2Dec->picInfo.nLeftOffset = pMpeg2Dec->picInfo.nPicWidth-(pMpeg2Dec->picInfo.nPicHeight*4/3);
				pMpeg2Dec->picInfo.nLeftOffset /= 2;
				pMpeg2Dec->picInfo.nRightOffset = pMpeg2Dec->picInfo.nLeftOffset;
			}
			break;
		case 10:            //16:9 (center)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_4_3)
			{
				pMpeg2Dec->picInfo.nTopOffset = pMpeg2Dec->picInfo.nPicHeight -(pMpeg2Dec->picInfo.nPicWidth*9/16);
				pMpeg2Dec->picInfo.nTopOffset /= 2;
				pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nTopOffset;
			}
			break;
		case 11:            //14:9 (center)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_4_3)
			{
				pMpeg2Dec->picInfo.nTopOffset = pMpeg2Dec->picInfo.nPicHeight -(pMpeg2Dec->picInfo.nPicWidth*9/14);
				pMpeg2Dec->picInfo.nTopOffset /= 2;
				pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nTopOffset;
			}
			else if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_16_9)
			{
				pMpeg2Dec->picInfo.nLeftOffset = pMpeg2Dec->picInfo.nPicWidth -(pMpeg2Dec->picInfo.nPicHeight*14/9);
				pMpeg2Dec->picInfo.nLeftOffset /= 2;
				pMpeg2Dec->picInfo.nRightOffset = pMpeg2Dec->picInfo.nLeftOffset;
			}
			break;
		case 13:            //4:3 (with shoot and protect 14:9 center)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_16_9)
			{
				pMpeg2Dec->picInfo.nLeftOffset = pMpeg2Dec->picInfo.nPicWidth -(pMpeg2Dec->picInfo.nPicHeight*4/3);
				pMpeg2Dec->picInfo.nLeftOffset /= 2;
				pMpeg2Dec->picInfo.nRightOffset = pMpeg2Dec->picInfo.nLeftOffset;
			}
			break;
		case 14:            //16:9 (with shoot and protect 14:9 center)
		case 15:            //16:9 (with shoot and protect 4:3 center)
			if(pMpeg2Dec->picInfo.bCodedFrameRatio == MPEG_CODED_FRAME_RATIO_4_3)
			{
				pMpeg2Dec->picInfo.nTopOffset = pMpeg2Dec->picInfo.nPicHeight -(pMpeg2Dec->picInfo.nPicWidth*9/16);
				pMpeg2Dec->picInfo.nTopOffset /= 2;
				pMpeg2Dec->picInfo.nBottomOffset = pMpeg2Dec->picInfo.nTopOffset;
			}
			break;
		default:
			break;
	}
	return;
}

/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ProcessStartCode(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec, uint8_t*pDataBuf)
{
    int8_t eRet = VDECODE_RESULT_OK;
    switch(pMpeg2Dec->picInfo.eCurStartCode)
    {
        case MP2VDEC_SEQ_START_CODE:
        {   
            // mpeg2_record_sequence_data();       // if need record the sequence data
           eRet = Mpeg2ParseSequenceInfo(pMpeg2Ctx, pMpeg2Dec, pDataBuf, MP2VDEC_PARSE_SEQ_MODE1);
           if(eRet != VDECODE_RESULT_OK)
           {
              return eRet;
           }
           
           break;
        }
        case MP2VDEC_GOP_START_CODE:
        {   
            pMpeg2Dec->picInfo.bGetGopFlag = 1;
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            break;
        }
        case MP2VDEC_PIC_START_CODE:
        {   
            Mpeg2ParsePictureInfo(pMpeg2Dec, pDataBuf);
            break;
        }
        case MP2VDEC_USER_START_CODE:
        {   
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            Mpeg2ProcessActiveFormat(pMpeg2Dec, pDataBuf+4);
            break;
        }
        case MP2VDEC_EXT_START_CODE:
        {   
            if(Mpeg2ParseExtensionInfo(pMpeg2Dec, pDataBuf) != VDECODE_RESULT_OK)
            {
                return VDECODE_RESULT_UNSUPPORTED;
            }
            break;
        }
        case MP2VDEC_SLICE_START_CODE:
        {   
            Mpeg2PictureHeaderAction(pMpeg2Dec);
            Mpeg2ComputePicPts(pMpeg2Dec);
            break;
        }
        default:
        {
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
        }
    }

    return VDECODE_RESULT_OK;
}
    

/****************************************************************************************************/
/****************************************************************************************************/

int8_t Mpeg2SearchStartcode(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t* pStartPtr, uint32_t nSearchSize, uint32_t* pStartCode, uint8_t bUpdataFlag)
{   
    uint32_t i = 0;
    uint32_t uNextWord = 0;
    uint8_t  bFlag1, bFlag2;
    uint8_t  bFindStartCodeFlag = 0;


    for(i=0; i<nSearchSize; i++)
    {   
        if(pStartPtr[i]== 0x01)
        {   
            if((i>=2) &&(i<(nSearchSize-1))&&(pStartPtr[i-2]==0x00) &&(pStartPtr[i-1]==0x00))
            {   
                uNextWord = 0x00000100+(pStartPtr[i+1]);
                bFlag1 = (uNextWord==MP2VDEC_SEQ_START_CODE)||(uNextWord==MP2VDEC_GOP_START_CODE)||(uNextWord==MP2VDEC_PIC_START_CODE);
                bFlag2 = (uNextWord==MP2VDEC_USER_START_CODE)||(uNextWord == MP2VDEC_EXT_START_CODE)||(uNextWord ==MP2VDEC_SLICE_START_CODE);
                if((bFlag1==1) ||(bFlag2==1))
                {
                    bFindStartCodeFlag = 1;
                    *pStartCode = uNextWord;
                     i += 1;
                     break;
                }
            }
        }
    }
    if(bUpdataFlag == 1)
    {   
        Mpeg2SbmUpdateReadPointer(pMpeg2Dec, i-3);
    }
    return bFindStartCodeFlag;
}
//**************************************************************************************************************//
//**************************************************************************************************************//
int8_t Mpeg2SwSearchStartcode(Mpeg2DecodeContext* mpg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec, uint32_t* pStartCode)
{
    uint8_t  aCoverData[7] 	= {0};
    uint8_t  bSearchFlag 	= 0;
    uint32_t nRemainDataSize = 0;

re_sw_search_startCode:
    if(pMpeg2Dec->sbmInfo.nSbmDataSize < 32)
    {
       if(Mpeg2RequestBitstreamData(mpg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
        {   
             return VDECODE_RESULT_NO_BITSTREAM;
        }
    }

    if((pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize)<= pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {   
        bSearchFlag = Mpeg2SearchStartcode(pMpeg2Dec, pMpeg2Dec->sbmInfo.pReadPtr,pMpeg2Dec->sbmInfo.nSbmDataSize,pStartCode, 1);
    }
    else
    {   
        nRemainDataSize = pMpeg2Dec->sbmInfo.pSbmBufEnd - pMpeg2Dec->sbmInfo.pReadPtr + 1;
        if(nRemainDataSize >= 4)
        {   
            bSearchFlag = Mpeg2SearchStartcode(pMpeg2Dec, pMpeg2Dec->sbmInfo.pReadPtr,nRemainDataSize,pStartCode, 1);
        }
        if(pMpeg2Dec->sbmInfo.nSbmDataSize < 32)
        {
            if(Mpeg2RequestBitstreamData(mpg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
            {   
                 return VDECODE_RESULT_NO_BITSTREAM;
            }
        }
        if(bSearchFlag == 0)
        {
            nRemainDataSize = pMpeg2Dec->sbmInfo.pSbmBufEnd - pMpeg2Dec->sbmInfo.pReadPtr + 1;
            memcpy(aCoverData, pMpeg2Dec->sbmInfo.pReadPtr, nRemainDataSize);
            memcpy(aCoverData+nRemainDataSize, pMpeg2Dec->sbmInfo.pSbmBuf, 7-nRemainDataSize);
            bSearchFlag = Mpeg2SearchStartcode(pMpeg2Dec, aCoverData,7,pStartCode, 1);
        }
        if(bSearchFlag == 0)
        {   
            bSearchFlag = Mpeg2SearchStartcode(pMpeg2Dec, pMpeg2Dec->sbmInfo.pReadPtr,pMpeg2Dec->sbmInfo.nSbmDataSize,pStartCode, 1);
        }
    }
    
    if(bSearchFlag == 1)
    {
        return VDECODE_RESULT_OK;
    }
goto re_sw_search_startCode;
}


//*********************************************************************************************************************//
//********************************************************************************************************************//
int8_t Mpeg2JudgeStartcode(Mpeg2DecodeInfo* pMpeg2Dec, uint32_t nStartcode)
{   
    uint8_t  eCurPicType 	= 0;
    uint8_t  nRemainDataSize = 0;
    uint64_t nPicDuration 	= 0;
    
     if(pMpeg2Dec->picInfo.bRepeatFstFieldFlag == 1)
    {
        nPicDuration = pMpeg2Dec->picInfo.nPicDuration*3/2;
    }
    else
    {
        nPicDuration = pMpeg2Dec->picInfo.nPicDuration;
    } 
    
    if(pMpeg2Dec->picInfo.eCurPicStructure != MP2VDEC_FRAME)
    {
        nPicDuration /= 2;
    }
    if(pMpeg2Dec->picInfo.bGetSeqFlag == 0)
    {
    	if(nStartcode != MP2VDEC_SEQ_START_CODE)
    	{
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            return VDECODE_RESULT_UNSUPPORTED;
    	}
    }


    if((nStartcode==MP2VDEC_SEQ_START_CODE)||(nStartcode== MP2VDEC_GOP_START_CODE))
    {   
        if((pMpeg2Dec->sbmInfo.nSbmDataPts==MP2VDEC_ERROR_PTS_VALUE)&&(pMpeg2Dec->picInfo.nFrmNum==0))
        {
            if(pMpeg2Dec->sbmInfo.nValidDataPts != MP2VDEC_ERROR_PTS_VALUE)
            {
                pMpeg2Dec->picInfo.nNextPicPts = (pMpeg2Dec->sbmInfo.nValidDataPts+pMpeg2Dec->nSearchStcdTime*nPicDuration);
            }
        }
        pMpeg2Dec->bFieldSeachNextPicFlag = 0;
        pMpeg2Dec->picInfo.bFstFieldFlag = 0;
        return VDECODE_RESULT_OK;
    }
    else if(nStartcode==MP2VDEC_PIC_START_CODE)
    {   
        pMpeg2Dec->nSearchStcdTime += 1;
        
        if((pMpeg2Dec->sbmInfo.pReadPtr+5)<=pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
            eCurPicType = (pMpeg2Dec->sbmInfo.pReadPtr[5]>>3) & 0x07;
        }
        else
        {
            nRemainDataSize = pMpeg2Dec->sbmInfo.pSbmBufEnd - pMpeg2Dec->sbmInfo.pReadPtr+1;
            eCurPicType = (pMpeg2Dec->sbmInfo.pSbmBuf[5-nRemainDataSize]>>3) & 0x07;
        }
        if(pMpeg2Dec->picInfo.bGetForFrmFlag == 0)
        {
        	if((pMpeg2Dec->sbmInfo.nSbmDataPts==MP2VDEC_ERROR_PTS_VALUE)&&(pMpeg2Dec->picInfo.nFrmNum==0))
        	{
        		if(pMpeg2Dec->sbmInfo.nValidDataPts != MP2VDEC_ERROR_PTS_VALUE)
        		{
        			pMpeg2Dec->picInfo.nNextPicPts = (pMpeg2Dec->sbmInfo.nValidDataPts+pMpeg2Dec->nSearchStcdTime*nPicDuration);
        		}
        	}

            if(eCurPicType != MP2VDEC_I_PIC)
            {
                Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
                return VDECODE_RESULT_UNSUPPORTED;
            }
            return VDECODE_RESULT_OK;
        }
        else if(pMpeg2Dec->picInfo.bGetBacFrmFlag == 0)
        {
        	if((pMpeg2Dec->sbmInfo.nSbmDataPts==MP2VDEC_ERROR_PTS_VALUE)&&(pMpeg2Dec->picInfo.nFrmNum==0))
        	{
        		if(pMpeg2Dec->sbmInfo.nValidDataPts != MP2VDEC_ERROR_PTS_VALUE)
        		{
        			pMpeg2Dec->picInfo.nNextPicPts = (pMpeg2Dec->sbmInfo.nValidDataPts+pMpeg2Dec->nSearchStcdTime*nPicDuration);
        		}
        	}

            if((eCurPicType!= MP2VDEC_I_PIC) &&(eCurPicType!=MP2VDEC_P_PIC))
             {
                Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
                return VDECODE_RESULT_UNSUPPORTED;
             }
             return VDECODE_RESULT_OK;
        }
        if(pMpeg2Dec->bFieldSeachNextPicFlag == 1)
        {
             Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
             pMpeg2Dec->bFieldSeachNextPicFlag = 0;
             pMpeg2Dec->picInfo.bFstFieldFlag = 0;
             return VDECODE_RESULT_UNSUPPORTED;
        }
    }
    else
    {
        Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
        return VDECODE_RESULT_UNSUPPORTED;
    }
    
    return VDECODE_RESULT_OK;
}

//**********************************************************************************************************************//
//***********************************************************************************************************************//

int8_t Mpeg2HwSearchStartcode(Mpeg2DecodeContext* mpg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint8_t   uInrCode 	 = 0;
    uint32_t  nByteOffset = 0;
    uint32_t  nByteLen 	 = 0;
    uint32_t  nRemainLen  = 0;
    uint32_t  uHwDwPtr 	 = 0;
    uint32_t  nHwOff 	 = 0;
    uint8_t*  pHwPtr 	 = 0;
    uint32_t  nStartcode  =0;
    uint32_t  nUpdateSize = 0;
        
hw_search_startcode:

    if(pMpeg2Dec->sbmInfo.nSbmHwDataSize < 64)
    {
       if(Mpeg2RequestBitstreamData(mpg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
        {    
             return VDECODE_RESULT_NO_BITSTREAM;
        }
    }
    nByteOffset =  pMpeg2Dec->sbmInfo.pHwReadPtr - pMpeg2Dec->sbmInfo.pSbmBuf;
    nRemainLen = (int32_t)( pMpeg2Dec->sbmInfo.pHwReadPtr+pMpeg2Dec->sbmInfo.nSbmHwDataSize) & 0x03;
    nByteLen   = pMpeg2Dec->sbmInfo.nSbmHwDataSize - nRemainLen;
        
    if(pMpeg2Dec->bFstSetVbvFlag == 1)
    {   
        Mpeg2SetHwStartCodeInfo(pMpeg2Dec);
        Mpeg2SetSbmRegister(pMpeg2Dec, pMpeg2Dec->bFstSetVbvFlag, 0, (nByteOffset<<3), (nByteLen<<3), 0, 1);
    }
    else
    {   
        Mpeg2SetSbmRegister(pMpeg2Dec, pMpeg2Dec->bFstSetVbvFlag, 0, (nByteOffset<<3), (nByteLen<<3), 0, 0);
    }
    //IVE.ve_wait_intr();
    AdapterVeWaitInterrupt();
    uInrCode = Mpeg2VeIsr(pMpeg2Dec); 
    
    if(uInrCode == 0)
    {   
        pMpeg2Dec->bFstSetVbvFlag = 1;
        ResetVeInternal(mpg2Ctx->pVideoEngine);  
        goto hw_search_startcode;
    }

    if((uInrCode&1) != 1)     // uInrCode= 2,4,6
    {  
        if((pMpeg2Dec->sbmInfo.pHwReadPtr+nByteLen) < pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
            pMpeg2Dec->sbmInfo.pHwReadPtr += nByteLen;
            pMpeg2Dec->sbmInfo.nSbmHwDataSize -= nByteLen;
        }
        else
        {
            pMpeg2Dec->sbmInfo.pHwReadPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pHwReadPtr+nByteLen-pMpeg2Dec->sbmInfo.nSbmBufSize);
            pMpeg2Dec->sbmInfo.nSbmHwDataSize -= nByteLen;
        }
        pMpeg2Dec->bFstSetVbvFlag = 0;
        if(Mpeg2RequestBitstreamData(mpg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
        {
            return VDECODE_RESULT_NO_BITSTREAM;
        } 
    }
    else
    {  
       nHwOff = Mpeg2GetStartcodeBitOffet(pMpeg2Dec);
       if(nHwOff & 7)
          nHwOff  = (nHwOff+7)&0xfffffff8;
       uHwDwPtr   = (nHwOff+31)&0xffffffe0;
       uHwDwPtr >>= 3;
       nHwOff   >>= 3;
       while(nHwOff >= pMpeg2Dec->sbmInfo.nSbmBufSize)
       {
            nHwOff -= pMpeg2Dec->sbmInfo.nSbmBufSize;
       }
       pHwPtr =(uint8_t*)(pMpeg2Dec->sbmInfo.pSbmBuf + nHwOff);
       if(pHwPtr >= pMpeg2Dec->sbmInfo.pHwReadPtr)
       {
            nHwOff = pHwPtr - pMpeg2Dec->sbmInfo.pHwReadPtr;
       }
       else 
       {
            nHwOff = 0;
            if((pMpeg2Dec->sbmInfo.pHwReadPtr+pMpeg2Dec->sbmInfo.nSbmHwDataSize) > pMpeg2Dec->sbmInfo.pSbmBufEnd)
            {
                nHwOff = pHwPtr+pMpeg2Dec->sbmInfo.nSbmBufSize-pMpeg2Dec->sbmInfo.pHwReadPtr;
            }
       }
       
       pMpeg2Dec->bFstSetVbvFlag = 1;

      pMpeg2Dec->sbmInfo.nSbmHwDataSize -= nHwOff;
      nUpdateSize = pMpeg2Dec->sbmInfo.nSbmDataSize-pMpeg2Dec->sbmInfo.nSbmHwDataSize;
      if(nUpdateSize >= 8)
      {
        nUpdateSize -= 8;
      }
      else
      {
         nUpdateSize  = 0;
      }
        
      Mpeg2SbmUpdateReadPointer(pMpeg2Dec,nUpdateSize);
      
       if(Mpeg2SwSearchStartcode(mpg2Ctx, pMpeg2Dec, &nStartcode)==VDECODE_RESULT_OK)
       {    
            if(Mpeg2JudgeStartcode(pMpeg2Dec,nStartcode) == VDECODE_RESULT_OK)
            {
                return VDECODE_RESULT_OK;
            }
       }
       pMpeg2Dec->sbmInfo.pHwReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;
       pMpeg2Dec->sbmInfo.nSbmHwDataSize = pMpeg2Dec->sbmInfo.nSbmDataSize;
    }
    goto hw_search_startcode;
}


/****************************************************************************************************/
/****************************************************************************************************/

int8_t Mpeg2RequestBitstreamData(Mpeg2DecodeContext* pMpeg2Context, Mpeg2DecodeInfo* pMpeg2Dec)
{   
    int16_t nCurUpdateStrmIdx = 0;
    VideoStreamDataInfo* newStreamData = NULL;
    int64_t diffPts = 0;

Mpeg2RequestBitstreamDataAgain:
    newStreamData = SbmRequestStream(pMpeg2Dec->sbmInfo.pSbm);
    if(newStreamData == NULL)
    {   
        Mpeg2ReleaseBitstreamData(pMpeg2Dec);
        if(pMpeg2Dec->bEndOfStream == 1)
        {
        	Mpeg2FlushPictures(pMpeg2Dec,pMpeg2Context->pFbm,1);
        }
        return VDECODE_RESULT_NO_BITSTREAM;
    }
    else if((newStreamData->bValid==0) &&(pMpeg2Dec->sbmInfo.bGetFstDataFlag==0))
    {   
        SbmFlushStream(pMpeg2Dec->sbmInfo.pSbm,newStreamData);
        newStreamData->nLength = 0;
        return VDECODE_RESULT_NO_BITSTREAM;
    }
    else if((newStreamData->nLength==0)&&(pMpeg2Dec->sbmInfo.bGetFstDataFlag==0))
    {
        SbmFlushStream(pMpeg2Dec->sbmInfo.pSbm,newStreamData);
        newStreamData->nLength = 0;
        return VDECODE_RESULT_NO_BITSTREAM;
    }
    
    
    if((newStreamData->nPts!= MP2VDEC_ERROR_PTS_VALUE)&& (pMpeg2Dec->sbmInfo.nLastDataPts!=MP2VDEC_ERROR_PTS_VALUE))
    {
    	diffPts = pMpeg2Dec->sbmInfo.nLastDataPts - newStreamData->nPts;

    	if(diffPts>=2000000 || diffPts<=-2000000)
    	{
    		pMpeg2Dec->sbmInfo.nLastDataPts = MP2VDEC_ERROR_PTS_VALUE;
    		Mpeg2DecoderReset((DecoderInterface*)pMpeg2Context);
    		SbmReturnStream(pMpeg2Dec->sbmInfo.pSbm,newStreamData);
    		return VDECODE_RESULT_NO_BITSTREAM;
    	}
    }
    pMpeg2Dec->sbmInfo.nLastDataPts = newStreamData->nPts;
    if(pMpeg2Dec->picInfo.bOnlyDispKeyFrmFlag == 1)
    {
    	pMpeg2Dec->sbmInfo.nLastDataPts = MP2VDEC_ERROR_PTS_VALUE;
    }

    nCurUpdateStrmIdx = pMpeg2Dec->sbmInfo.nCurUpdateStrmIdx;
    nCurUpdateStrmIdx += 1;
    if(nCurUpdateStrmIdx == MPEG2VDEC_MAX_STREAM_NUM)
    {
        nCurUpdateStrmIdx = 0;
    }
   
    pMpeg2Dec->sbmInfo.vbvStreamData[nCurUpdateStrmIdx] = newStreamData;
    pMpeg2Dec->sbmInfo.nCurUpdateStrmIdx = nCurUpdateStrmIdx;
    pMpeg2Dec->sbmInfo.nSbmDataSize += newStreamData->nLength;
    pMpeg2Dec->sbmInfo.nSbmDataPts  = newStreamData->nPts;
    pMpeg2Dec->sbmInfo.nStrmLen += newStreamData->nLength;
    pMpeg2Dec->sbmInfo.nSbmHwDataSize += newStreamData->nLength;
    
    if((newStreamData->nLength==4)&&(newStreamData->pData[0]==0)&&(newStreamData->pData[1]==0)
        &&(newStreamData->pData[2]==0)&&(newStreamData->pData[3]==0))
    {
        pMpeg2Dec->picInfo.nNextPicPts += pMpeg2Dec->picInfo.nPicDuration;
      
    }
    
    if(newStreamData->nPts!= MP2VDEC_ERROR_PTS_VALUE)
    { 
        pMpeg2Dec->sbmInfo.nValidDataPts = newStreamData->nPts;
        pMpeg2Dec->nSearchStcdTime = 0;
    }
    
    if(pMpeg2Dec->sbmInfo.bGetFstDataFlag == 0)
    {  
        pMpeg2Dec->sbmInfo.pReadPtr = (uint8_t*)newStreamData->pData;
        pMpeg2Dec->sbmInfo.bGetFstDataFlag = 1;
        pMpeg2Dec->sbmInfo.pHwReadPtr =  pMpeg2Dec->sbmInfo.pReadPtr;
 
    }
    if(newStreamData->nLength == 0)
    {
    	logw("mpeg2 decoder get stream data length == 0, try to requeset again......");
    	goto Mpeg2RequestBitstreamDataAgain;
    }
    return VDECODE_RESULT_OK;
}
  
//**************************************************************************************************/
//***************************************************************************************************/
int8_t Mpeg2ReleaseBitstreamData(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    int16_t nCurReleaseStrmIdx = 0;
    uint32_t nRemainLen = 0;
    
    while(pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx != pMpeg2Dec->sbmInfo.nCurUpdateStrmIdx)
    {
        nCurReleaseStrmIdx = pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx;
        nCurReleaseStrmIdx += 1;  
        if(nCurReleaseStrmIdx == MPEG2VDEC_MAX_STREAM_NUM)
        {
            nCurReleaseStrmIdx = 0;
        }
        if(pMpeg2Dec->sbmInfo.vbvStreamData[nCurReleaseStrmIdx] == NULL)
        {
            break;
        }
        
        nRemainLen = pMpeg2Dec->sbmInfo.nStrmLen - pMpeg2Dec->sbmInfo.vbvStreamData[nCurReleaseStrmIdx]->nLength;
        if(pMpeg2Dec->sbmInfo.nSbmDataSize > nRemainLen)
        {   
            break;
        }
        else
        {
           SbmFlushStream(pMpeg2Dec->sbmInfo.pSbm,pMpeg2Dec->sbmInfo.vbvStreamData[nCurReleaseStrmIdx]);
           pMpeg2Dec->sbmInfo.nStrmLen = nRemainLen;
           pMpeg2Dec->sbmInfo.vbvStreamData[nCurReleaseStrmIdx] = NULL;
           pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx = nCurReleaseStrmIdx;
        }
    }
    return VDECODE_RESULT_OK;
}

    
/****************************************************************************************************/
/****************************************************************************************************/
int8_t Mpeg2ParsePictureHeader(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec)
{  
    uint8_t  uExtensionId    = 0;
    uint8_t  bFlag1 			= 0;
    uint8_t  bFlag2 			= 0;
    uint8_t  bFlag3 			= 0;
    int8_t  eRet 			= 0;
    uint32_t nStartcode 		= 0;
    uint32_t nRemainDataSize = 0;
    uint8_t *pDataBuf        = NULL;

    // search the nStartcode,sequence/picture/slice nStartcode
    if(pMpeg2Dec->sbmInfo.nSbmDataSize < 140)
    {
       if(Mpeg2RequestBitstreamData(pMpeg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
       {
            return VDECODE_RESULT_NO_BITSTREAM;
       }
    }
    
    while(1)
    {    
        if(Mpeg2SwSearchStartcode(pMpeg2Ctx, pMpeg2Dec, &nStartcode) != VDECODE_RESULT_OK) // cannot find the nStartcode
        {   
            return VDECODE_RESULT_NO_BITSTREAM;
        }
        if(pMpeg2Dec->sbmInfo.nSbmDataSize < 140)    // avoid data is not enough
        {
            if(Mpeg2RequestBitstreamData(pMpeg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
            {
                 return VDECODE_RESULT_NO_BITSTREAM;
            }
        }  
        pMpeg2Dec->picInfo.eCurStartCode = nStartcode; 
        //check whether is mpeg1 stream or mpeg2 stream
        
        nRemainDataSize = pMpeg2Dec->sbmInfo.pSbmBufEnd-pMpeg2Dec->sbmInfo.pReadPtr+1;
        if(nRemainDataSize < 140)
        {
            memcpy(pMpeg2Dec->sbmInfo.aSbmCopyBuf, pMpeg2Dec->sbmInfo.pReadPtr, nRemainDataSize);
            memcpy(pMpeg2Dec->sbmInfo.aSbmCopyBuf+nRemainDataSize,pMpeg2Dec->sbmInfo.pSbmBuf, 140-nRemainDataSize);
            pDataBuf = pMpeg2Dec->sbmInfo.aSbmCopyBuf;
        }
        else
        {
            pDataBuf = pMpeg2Dec->sbmInfo.pReadPtr;
        }
            
        uExtensionId = (pDataBuf[4]>>4);
        if(pMpeg2Dec->picInfo.eLastStartCode == MP2VDEC_SEQ_START_CODE)
        {
            bFlag1 = (pMpeg2Dec->picInfo.eCurStartCode!=MP2VDEC_EXT_START_CODE);
            bFlag2 = (uExtensionId != MP2VDEC_SEQ_EXT_ID);
            if((bFlag1==1) || (bFlag2==1))
            {
                pMpeg2Dec->picInfo.bIsMpeg1Flag = 1;
                pMpeg2Dec->picInfo.bGetSeqFlag  = 1;
                pMpeg2Dec->picInfo.eLastPicStructure = MP2VDEC_FRAME;
            }
        }
        else if(pMpeg2Dec->picInfo.eLastStartCode == MP2VDEC_PIC_START_CODE)
        {
            bFlag1 = (pMpeg2Dec->picInfo.eCurStartCode!=MP2VDEC_EXT_START_CODE);
            bFlag2 = (uExtensionId != MP2VDEC_PIC_EXT_ID);
            if((bFlag1==1) || (bFlag2==1))
            {
                if((pMpeg2Dec->picInfo.bGetSeqFlag==1) && (pMpeg2Dec->picInfo.bIsMpeg1Flag==0))
                {
                    pMpeg2Dec->picInfo.eLastStartCode = pMpeg2Dec->picInfo.eCurStartCode;
                }
                else
                {
                    pMpeg2Dec->picInfo.bIsMpeg1Flag = 1;
                    Mpeg2SetDefaultForMpeg1(pMpeg2Dec);
                }
            }
        }
        
        eRet = Mpeg2ProcessStartCode(pMpeg2Ctx, pMpeg2Dec, pDataBuf);
        if(eRet != VDECODE_RESULT_OK)
        {
            return eRet;
        }
            
        if(pMpeg2Dec->picInfo.eCurStartCode==MP2VDEC_SLICE_START_CODE)
        {
            if(pMpeg2Dec->picInfo.bGetPicFlag == 1)
            {
                break;
            }
            else
            {
                loge("cannot find the picture start code before find the slice start code.\n");
                Mpeg2SbmUpdateReadPointer(pMpeg2Dec, 4);
            }
                
        }
        if((pMpeg2Dec->picInfo.bGetPicFlag==1) && (pMpeg2Dec->picInfo.eCurStartCode==MP2VDEC_SLICE_START_CODE))
        {
            break;
        }
        pMpeg2Dec->picInfo.eLastStartCode = pMpeg2Dec->picInfo.eCurStartCode;
    }
    
    if(pMpeg2Dec->picInfo.bGetForFrmFlag == 0)
    {   
        bFlag1 = (pMpeg2Dec->picInfo.bGetSeqFlag==1);
        bFlag2 = (pMpeg2Dec->picInfo.bGetPicFlag==1);
        bFlag3 = ((pMpeg2Dec->picInfo.bFstFieldFlag==1) &&(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_I_PIC));
        if(bFlag1 == 0)
        {
           if(pMpeg2Ctx->videoStreamInfo.pCodecSpecificData!= NULL)
            {   
                Mpeg2ParseSequenceInfo(pMpeg2Ctx,(Mpeg2DecodeInfo*)pMpeg2Ctx->pMpeg2Dec, (uint8_t*)pMpeg2Ctx->videoStreamInfo.pCodecSpecificData, MP2VDEC_PARSE_SEQ_MODE2);
            }
        }
            
        if((bFlag1==1) && (bFlag2==1) && (bFlag3==1))
        {
            pMpeg2Dec->picInfo.bGetForFrmFlag = 1;
        }
    }
    else if(pMpeg2Dec->picInfo.bGetBacFrmFlag == 0)
    {
        bFlag1 = (pMpeg2Dec->picInfo.bGetPicFlag==1);
        bFlag2 = ((pMpeg2Dec->picInfo.bFstFieldFlag==1) &&(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_I_PIC));
        bFlag3 = ((pMpeg2Dec->picInfo.bFstFieldFlag==1) &&(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_P_PIC));
        if((bFlag1==1) && ((bFlag2==1)||(bFlag3==1)))
        {
            pMpeg2Dec->picInfo.bGetBacFrmFlag = 1;
        }
    }
    return VDECODE_RESULT_OK;
}



/****************************************************************************************************/
/****************************************************************************************************/
void Mpeg2PutPictureOut(Mpeg2DecodeInfo* pMpeg2Dec, Fbm* pMpeg2DecFrm, uint8_t bDecKeyFrmFlag)
{
    if((pMpeg2Dec->picInfo.eCurPicType == MP2VDEC_B_PIC) ||(bDecKeyFrmFlag==1))
    {
    	FbmReturnBuffer(pMpeg2DecFrm,pMpeg2Dec->frmBufInfo.pCurFrm, VALID);
    	pMpeg2Dec->frmBufInfo.pCurFrm = NULL;
    }
    else
    {    
         if(pMpeg2Dec->picInfo.nFrmNum == 1)
         {
        	 FbmShareBuffer(pMpeg2DecFrm,pMpeg2Dec->frmBufInfo.pCurFrm);
         }
         else if(pMpeg2Dec->picInfo.nFrmNum > 2)
         {
        	 if(pMpeg2Dec->frmBufInfo.pLastRefFrm != NULL)
             {
        		 FbmReturnBuffer(pMpeg2DecFrm,pMpeg2Dec->frmBufInfo.pLastRefFrm, INVALID);
                 pMpeg2Dec->frmBufInfo.pLastRefFrm = NULL;
             }
        	 if(pMpeg2Dec->frmBufInfo.pForRefFrm != NULL)
        	 {
        		 FbmShareBuffer(pMpeg2DecFrm,pMpeg2Dec->frmBufInfo.pForRefFrm);
        	 }
         }
    }
}

//***************************************************************************************************//
void Mpeg2RevertSomeData(Mpeg2DecodeInfo* pMpeg2Dec, uint32_t reverDataLen)
{   
    int16_t nCurReleaseStrmIdx = 0;
    int32_t nRemainLen 		   = 0;

    nCurReleaseStrmIdx = pMpeg2Dec->sbmInfo.nCurReleaseStrmIdx;
    nCurReleaseStrmIdx += 1;  
    
    if(nCurReleaseStrmIdx == MPEG2VDEC_MAX_STREAM_NUM)
    {
        nCurReleaseStrmIdx = 0;
    }

    nRemainLen =  pMpeg2Dec->sbmInfo.pReadPtr - (uint8_t*)pMpeg2Dec->sbmInfo.vbvStreamData[nCurReleaseStrmIdx]->pData;

    
    if(nRemainLen > 0)
    {    
        if(nRemainLen >=(int32_t)reverDataLen)
        {
            nRemainLen = (int32_t)reverDataLen;
        }
        pMpeg2Dec->sbmInfo.pReadPtr -= nRemainLen;
        pMpeg2Dec->sbmInfo.nSbmDataSize += nRemainLen;
    }
    else
    {
        nRemainLen =  pMpeg2Dec->sbmInfo.pReadPtr - pMpeg2Dec->sbmInfo.pSbmBuf;
        if(nRemainLen >= (int32_t)reverDataLen)
        {
            nRemainLen = (int32_t)reverDataLen;
        }
        pMpeg2Dec->sbmInfo.pReadPtr -= nRemainLen;
        pMpeg2Dec->sbmInfo.nSbmDataSize += nRemainLen;
    }

 
}


/****************************************************************************************************/
/****************************************************************************************************/

void Mpeg2UpdateSbmBuffer(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t uInrCode)
{   
    uint32_t nHwOffet = 0;
    uint32_t uHwDwPtr = 0;
    uint8_t *pNewPtr  = NULL;
    
    if((uInrCode&0x1) == 0)
    {
        loge("the result of the interrupt is error.\n");
    }
    else
    {
        nHwOffet = Mpeg2GetDecodeBitOffset(pMpeg2Dec);
        if((nHwOffet&7)==1)
        {
            nHwOffet = (nHwOffet+7)&0xfffffff8;
        }
       uHwDwPtr = ((nHwOffet+31)&0xffffffe0);
       uHwDwPtr >>= 3;
       nHwOffet >>= 3;

        while(nHwOffet > pMpeg2Dec->sbmInfo.nSbmBufSize)
       {
            nHwOffet -= pMpeg2Dec->sbmInfo.nSbmBufSize;
       }
       uHwDwPtr = (uint32_t)(pMpeg2Dec->sbmInfo.pSbmBuf + nHwOffet);
    
       if(uHwDwPtr >= (uint32_t)pMpeg2Dec->sbmInfo.pReadPtr)
       {   
           nHwOffet = uHwDwPtr - (uint32_t)(pMpeg2Dec->sbmInfo.pReadPtr);
       }
       else
       {  
          if((pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize)<pMpeg2Dec->sbmInfo.pSbmBufEnd)
          {
                nHwOffet = 0;
          }
          else
          {     
                pNewPtr = pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize-pMpeg2Dec->sbmInfo.nSbmBufSize;
                if((uint32_t)pNewPtr < uHwDwPtr) 
                {
                    nHwOffet = 0;
                }
                else
                {
                    nHwOffet = uHwDwPtr+pMpeg2Dec->sbmInfo.nSbmBufSize-(uint32_t)pMpeg2Dec->sbmInfo.pReadPtr;
                }
          }
       }


#if 0
       Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nHwOffet);
       Mpeg2RevertSomeData(pMpeg2Dec, 8);
#else
        if(nHwOffet >= 8)
        {
            nHwOffet -= 8;
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nHwOffet);
        }
        else if(nHwOffet > 0)
        {
            nHwOffet = 0;
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nHwOffet);
        }
        else
        {
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nHwOffet);
            Mpeg2RevertSomeData(pMpeg2Dec, 8);
        }
#endif
    }
}

/********************************************************************************************************/
/********************************************************************************************************/

void mpeg2_exchange_values(uint32_t* nWidth,uint32_t* nHeight)
{
    uint32_t nMidValue = 0;
   
    nMidValue   = (*nWidth);
    (*nWidth)   = (*nHeight);
    (*nHeight)  = nMidValue;
}

void Mpeg2ProcessPictureFinish(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec)
{
    VideoPicture* pCurFrmInfo;
    int32_t nTopOffset = 0;
    int32_t nLeftOffset = 0;
    int32_t nRightOffset = 0;
    int32_t nBottomOffset = 0;

    pCurFrmInfo = pMpeg2Dec->frmBufInfo.pCurFrm;
    pCurFrmInfo->nTopOffset      = pMpeg2Dec->picInfo.nTopOffset;
    pCurFrmInfo->nLeftOffset     = pMpeg2Dec->picInfo.nLeftOffset;
    pCurFrmInfo->nRightOffset    = pMpeg2Dec->picInfo.nPicWidth;
    pCurFrmInfo->nBottomOffset   = pMpeg2Dec->picInfo.nPicHeight;

    nTopOffset    = pMpeg2Dec->picInfo.nTopOffset;
    nRightOffset  = pMpeg2Dec->picInfo.nRightOffset;
    nBottomOffset = pMpeg2Dec->picInfo.nBottomOffset;
    nLeftOffset   = pMpeg2Dec->picInfo.nLeftOffset;


    pCurFrmInfo->nTopOffset    = nTopOffset;
    pCurFrmInfo->nRightOffset  = pMpeg2Dec->picInfo.nDispWidth-nRightOffset;
    pCurFrmInfo->nBottomOffset = pMpeg2Dec->picInfo.nDispHeight-nBottomOffset;
    pCurFrmInfo->nLeftOffset   = nLeftOffset;

    pCurFrmInfo->nFrameRate              = pMpeg2Dec->picInfo.nFrmRate;
    pCurFrmInfo->nAspectRatio            = 1000;

    pCurFrmInfo->bIsProgressive          = pMpeg2Dec->picInfo.uProgressiveFlag;
    pCurFrmInfo->bTopFieldFirst         = pMpeg2Dec->picInfo.bTopFieldFstFlag;
    pCurFrmInfo->bRepeatTopField        = pMpeg2Dec->picInfo.bRepeatFstFieldFlag;
    pCurFrmInfo->ePixelFormat            = pMpeg2Ctx->vconfig.eOutputPixelFormat;
    pCurFrmInfo->nPts                    = MP2VDEC_ERROR_PTS_VALUE;

    if((pMpeg2Dec->picInfo.bHasPtsFlag==1) ||(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_B_PIC))
    {
    	pCurFrmInfo->nPts = pMpeg2Dec->picInfo.nCurPicPts;
    }
    pMpeg2Dec->picInfo.nFrmNum += 1;
}


/******************************************************************************************/
/******************************************************************************************/
int8_t Mpeg2GetEmptyFrameBuf(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bKeyFramOnlyFlag,Fbm* pMpeg2DecFrm)
{  
    uint8_t bbKeyFramOnlyFlag = 0;
	bbKeyFramOnlyFlag = bKeyFramOnlyFlag;
	
    if((pMpeg2Dec->picInfo.eCurPicStructure!= MP2VDEC_FRAME)&&(pMpeg2Dec->picInfo.bFstFieldFlag==0))
    {
        return VDECODE_RESULT_OK;
    }
        
    pMpeg2Dec->frmBufInfo.pCurFrm = FbmRequestBuffer(pMpeg2DecFrm);
    if(pMpeg2Dec->frmBufInfo.pCurFrm == NULL)
    {
    	return VDECODE_RESULT_NO_FRAME_BUFFER;
    }
    return VDECODE_RESULT_OK;
}


/**********************************************************************************/
/**********************************************************************************/

int8_t  Mpeg2SearchNextPicStartcode( Mpeg2DecodeContext* mpg2Ctx,Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint32_t nStartcode = 0;
    
    if(pMpeg2Dec->bNormalChipFlag == 1)
    {   
        return Mpeg2HwSearchStartcode(mpg2Ctx, pMpeg2Dec);
    }
    else 
    {
  sw_search_startcode:
        if(Mpeg2SwSearchStartcode(mpg2Ctx, pMpeg2Dec, &nStartcode)!= VDECODE_RESULT_OK)
        {   
            if(Mpeg2RequestBitstreamData(mpg2Ctx, pMpeg2Dec) != VDECODE_RESULT_OK)
            {   
                return VDECODE_RESULT_NO_BITSTREAM;
            }
            goto sw_search_startcode;
        }
        if(Mpeg2JudgeStartcode(pMpeg2Dec, nStartcode) == VDECODE_RESULT_OK)
        {
            return VDECODE_RESULT_OK;
        }
        goto sw_search_startcode;
    }
}


/**********************************************************************************/
/**********************************************************************************/


void Mpeg2ResetDecodeParams(Mpeg2DecodeInfo* pMpeg2Dec)
{
    pMpeg2Dec->nDecStep = MP2VDEC_PARSE_HEADER;
    pMpeg2Dec->picInfo.eLastStartCode  = 0;
    pMpeg2Dec->picInfo.bGetForFrmFlag  = 0;
    pMpeg2Dec->picInfo.bGetBacFrmFlag  = 0;
    pMpeg2Dec->picInfo.bGetPicFlag 	   = 0;
    pMpeg2Dec->picInfo.eCurPicType 	   = 0;
    pMpeg2Dec->picInfo.bHasPtsFlag     = 0;
    pMpeg2Dec->picInfo.bFstFieldFlag   =1;
    pMpeg2Dec->bFstSetVbvFlag 		   = 1;
    pMpeg2Dec->picInfo.nFrmNum 		   = 0;
    pMpeg2Dec->nSearchStcdTime 		   = 0;
    pMpeg2Dec->sbmInfo.bGetFstDataFlag = 0;
    pMpeg2Dec->picInfo.eLastPicType    = MP2VDEC_B_PIC;
    pMpeg2Dec->picInfo.nCurPicPts      = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->picInfo.nNextPicPts 	   = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->sbmInfo.nValidDataPts   = MP2VDEC_ERROR_PTS_VALUE; 
    pMpeg2Dec->sbmInfo.nLastDataPts    = MP2VDEC_ERROR_PTS_VALUE;
    pMpeg2Dec->bSearchFstStartCodeFlag = 1;
}

/**********************************************************************************/
/**********************************************************************************/


int8_t Mpeg2SwCalculatePicLength(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint8_t  bSearchFlag 	= 0;
    uint32_t nStartcode 		= 0;
    uint32_t nCheckLen 		= 0;
    uint32_t nRemainDataSize = 0;
    uint8_t  *pCurPtr 		= NULL;
    uint8_t  *pOrgPtr 		= NULL;
    uint8_t  aCoverData[7]    = {0};

    pCurPtr = pMpeg2Dec->sbmInfo.pRecordPtr;
    pOrgPtr = pMpeg2Dec->sbmInfo.pReadPtr;
    nCheckLen = pMpeg2Dec->sbmInfo.nSbmDataSize;
    nCheckLen -= (pCurPtr>=pOrgPtr)?(pCurPtr-pOrgPtr) :(pCurPtr+pMpeg2Dec->sbmInfo.nSbmBufSize-pOrgPtr);
    
    if((pCurPtr+nCheckLen) <= pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {  
       Mpeg2SearchStartcode(pMpeg2Dec, pCurPtr,nCheckLen,&nStartcode, 0);
       if((nStartcode==MP2VDEC_SEQ_START_CODE)||(nStartcode==MP2VDEC_GOP_START_CODE)||(nStartcode==MP2VDEC_PIC_START_CODE))
       {
            bSearchFlag = 1;
       }
    }
    else 
    {
        nRemainDataSize = pMpeg2Dec->sbmInfo.pSbmBufEnd - pCurPtr + 1;
        if(nRemainDataSize >= 4)
        {   
            Mpeg2SearchStartcode(pMpeg2Dec, pCurPtr,nRemainDataSize,&nStartcode, 0);
            if((nStartcode==MP2VDEC_SEQ_START_CODE)||(nStartcode==MP2VDEC_GOP_START_CODE)||(nStartcode==MP2VDEC_PIC_START_CODE))
            {
                bSearchFlag = 1;
            }
        }
        
        if(bSearchFlag == 0)
        {
            memcpy(aCoverData, pMpeg2Dec->sbmInfo.pSbmBufEnd-2, 3);
            memcpy(aCoverData+3, pMpeg2Dec->sbmInfo.pSbmBuf, 4);
            Mpeg2SearchStartcode(pMpeg2Dec, aCoverData,7,&nStartcode, 0);
            if((nStartcode==MP2VDEC_SEQ_START_CODE)||(nStartcode==MP2VDEC_GOP_START_CODE)||(nStartcode==MP2VDEC_PIC_START_CODE))
            {
                bSearchFlag = 1;
            }
        }
        if(bSearchFlag == 0)
        {   
            pCurPtr   = pMpeg2Dec->sbmInfo.pSbmBuf;
            nCheckLen -= nRemainDataSize;
            Mpeg2SearchStartcode(pMpeg2Dec, pCurPtr, nCheckLen,&nStartcode, 0);
            if((nStartcode==MP2VDEC_SEQ_START_CODE)||(nStartcode==MP2VDEC_GOP_START_CODE)||(nStartcode==MP2VDEC_PIC_START_CODE))
            {
                bSearchFlag = 1;
            }
        }
    }
    
    if(bSearchFlag == 1)
    {   
        return VDECODE_RESULT_OK;
    }
    else
    {   
        pMpeg2Dec->sbmInfo.pRecordPtr = pCurPtr+nCheckLen-4;
        if(pMpeg2Dec->sbmInfo.pRecordPtr > pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
            pMpeg2Dec->sbmInfo.pRecordPtr -= (uint32_t)(pMpeg2Dec->sbmInfo.nSbmBufSize);
        }
        return VDECODE_RESULT_UNSUPPORTED;
    }
}


/**********************************************************************************/
/**********************************************************************************/

int8_t Mpeg2JudgePictureEnd(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    uint32_t uNextCode 		= 0;
    uint8_t  buf[4]			={0};
    uint8_t* pCurPtr 		= NULL;
    
    if(pMpeg2Dec->sbmInfo.nSbmDataSize  < 4)
    {
        return VDECODE_RESULT_UNSUPPORTED;
    }
    pCurPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize-4);
    if(pCurPtr > pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {
        pCurPtr -= pMpeg2Dec->sbmInfo.nSbmBufSize;
    }
    buf[0] = pCurPtr[0];
    pCurPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize-3);
    if(pCurPtr > pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {
        pCurPtr -= pMpeg2Dec->sbmInfo.nSbmBufSize;
    }
    buf[1] = pCurPtr[0];
    pCurPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize-2);
    if(pCurPtr > pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {
        pCurPtr -= pMpeg2Dec->sbmInfo.nSbmBufSize;
    }
    buf[2] = pCurPtr[0];
    pCurPtr = (uint8_t*)(pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize-1);
    if(pCurPtr > pMpeg2Dec->sbmInfo.pSbmBufEnd)
    {
        pCurPtr -= pMpeg2Dec->sbmInfo.nSbmBufSize;
    }
    buf[3] = pCurPtr[0];
    uNextCode = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
    if(uNextCode == 0x000001B7)
    {
        return VDECODE_RESULT_OK;
    }
    return VDECODE_RESULT_UNSUPPORTED;
}
