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
#include "stdio.h"

extern uint32_t H264GetUeGolomb(H264DecCtx* h264DecCtx);
extern int32_t H264GetSeGolomb(H264DecCtx* h264DecCtx);
extern uint32_t H264GetBits(H264DecCtx* h264DecCtx, uint8_t len);
extern uint32_t H264GetFunctionStatus(H264DecCtx* h264DecCtx);
extern int32_t H264DecodeSps(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern int32_t H264UpdateDataPointer(H264DecCtx* h264DecCtx,  H264Context* hCtx, uint8_t offsetMode);
extern uint32_t H264GetbitOffset(H264DecCtx* h264DecCtx, uint8_t offsetMode);
extern int32_t H264DecodePps(H264DecCtx* h264DecCtx, H264Context* hCtx, int32_t sliceDataLen);
extern int32_t H264DecodeSliceHeader(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern uint32_t H264GetDecodeMbNum(H264DecCtx* h264DecCtx);
extern uint32_t H264VeIsr(H264DecCtx* h264DecCtx);

extern void H264ConfigureBitstreamRegister(H264DecCtx *h264DecCtx, H264Context* hCtx, uint32_t nBitLens);
extern void H264DisableStartcodeDetect(H264DecCtx *h264DecCtx);
extern void H264SyncByte(H264DecCtx *h264DecCtx);
extern void H264CheckBsDmaBusy(H264DecCtx *h264DecCtx);
extern void H264EnableIntr(H264DecCtx *h264DecCtx);
extern void H264DisableStartcodeDetect(H264DecCtx *h264DecCtx);
extern void H264EnableStartcodeDetect(H264DecCtx *h264DecCtx);
extern void H264ReferenceRefresh(H264Context* hCtx);
extern void H264ConfigureSliceRegister(H264DecCtx *h264DecCtx, H264Context* hCtx, uint8_t decStreamIndex);
extern void  H264DecoderRest(DecoderInterface* pSelf);
extern int H264DecodeSei(H264DecCtx* h264DecCtx, H264Context* hCtx,int32_t nSliceDataLen);
extern void H264FlushDelayedPictures(H264Context* hCtx);

#ifndef AV_RB16
#   define AV_RB16(x)                           \
((((const uint8_t*)(x))[0] << 8) |          \
 ((const uint8_t*)(x))[1])
#endif

/****************************************************************************************************/
/****************************************************************************************************/

void H264ResetDecoderParams(H264Context* hCtx)
{
    int32_t i = 0;

    for(i=0; i<18; i++)
    {
        if(hCtx->frmBufInf.pDelayedPic[i] != NULL)
        {
            hCtx->frmBufInf.pDelayedPic[i]->nReference = 0;
            hCtx->frmBufInf.pDelayedPic[i]->pVPicture = NULL;
        }
    }
	
    if(hCtx->frmBufInf.pDelayedOutPutPic != NULL)
    {
        hCtx->frmBufInf.pDelayedOutPutPic->nReference = 0;
        hCtx->frmBufInf.pDelayedOutPutPic->pVPicture = NULL;
    }
    
    if(hCtx->frmBufInf.pCurPicturePtr != NULL)
    {
        hCtx->frmBufInf.pCurPicturePtr->nReference = 0;
        hCtx->frmBufInf.pCurPicturePtr->pVPicture = NULL;
    }
	
    H264ReferenceRefresh(hCtx);
    hCtx->bNeedFindIFrm = 1;
    hCtx->nPicStructure = 0xFF;
    hCtx->bFstField = 0xFF;
    hCtx->nCurSliceNum = 0;
    memset(hCtx->frmBufInf.defaultRefList, 0, 2*MAX_PICTURE_COUNT*	sizeof(H264PicInfo));
    memset(hCtx->frmBufInf.refList, 0, 2*32*sizeof(H264PicInfo));
    hCtx->nDecStep = H264_STEP_CONFIG_VBV;
    hCtx->bCanResetHw = 0;

    hCtx->bProgressice = 1;
    
    hCtx->vbvInfo.nLastValidPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.nValidDataPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.nVbvDataPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.nNextPicPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.pVbvStreamData = NULL;

    hCtx->nCurFrmNum = 0 ;
    hCtx->nMinDispPoc = 0;
    hCtx->nDelayedPicNum = 0;
    hCtx->nFrmNumOffset = 0;
    hCtx->nPrevFrmNumOffset = 0;
    hCtx->nPrevPocMsb = 0;
    hCtx->nPrevPocLsb = 0;
    hCtx->nDeltaPocBottom = 0;
    hCtx->frmBufInf.pLastPicturePtr = NULL;
    hCtx->frmBufInf.pNextPicturePtr = NULL;
    hCtx->vbvInfo.nNextPicPts = 0;
    hCtx->vbvInfo.nPrePicPts = 0;
    hCtx->vbvInfo.nPrePicPoc = 0;
}

/****************************************************************************************************/
/****************************************************************************************************/
int32_t H264InitDecode(H264DecCtx* h264DecCtx, H264Dec* h264Dec, H264Context* hCtx)
{   
	H264Dec* ph264Dec = NULL;
	ph264Dec = h264Dec;

    memset(hCtx, 0, sizeof(H264Context));

    hCtx->pMbFieldIntraBuf = (*MemPalloc)(0x20000);
    if(hCtx->pMbFieldIntraBuf == NULL)
    {
    	return VDECODE_RESULT_UNSUPPORTED;
    }
    (*MemSet)(hCtx->pMbFieldIntraBuf, 0, 0x20000);
    (*MemFlushCache)(hCtx->pMbFieldIntraBuf, 0x20000);

    hCtx->pMbNeighborInfoBuf = (*MemPalloc)(0x4000);
    if(hCtx->pMbNeighborInfoBuf == NULL)
    {
    	return VDECODE_RESULT_UNSUPPORTED;
    }
    (*MemSet)(hCtx->pMbNeighborInfoBuf, 0, 0x4000);
    (*MemFlushCache)(hCtx->pMbNeighborInfoBuf, 0x4000);


    hCtx->bNeedFindPPS = 1;
    hCtx->bNeedFindSPS = 1;
    hCtx->bNeedFindIFrm = 1;
    hCtx->vbvInfo.nVbvDataPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.nValidDataPts = H264VDEC_ERROR_PTS_VALUE;
    hCtx->vbvInfo.nLastValidPts = H264VDEC_ERROR_PTS_VALUE;
	
    hCtx->vbvInfo.nFrameRate =  h264DecCtx->videoStreamInfo.nFrameRate;
    if(hCtx->vbvInfo.nFrameRate != 0)
    {
    	hCtx->vbvInfo.nPicDuration = 1000;
    	hCtx->vbvInfo.nPicDuration *= (1000*1000);
    	hCtx->vbvInfo.nPicDuration /= hCtx->vbvInfo.nFrameRate;
    }
    hCtx->nMinDispPoc = 0;
    hCtx->nDelayedPicNum = 0;
    hCtx->nPicStructure = 0xFF;
    hCtx->bFstField = 0xFF;
    hCtx->bIsAvc = 0;

    return 0;
}


/****************************************************************************************************/
/****************************************************************************************************/
int32_t H264DecodeExtraData(H264Context* hCtx, uint8_t* extraDataPtr)
{   
    int8_t i = 0;
    int8_t cnt = 0;
    uint8_t* buf = NULL;
    uint16_t nalSize = 0;
    
    buf = hCtx->pExtraDataBuf;
    //store the nal length size, that will be use to parse all other nals
	hCtx->nNalLengthSize = (extraDataPtr[4]&0x03) + 1;
    cnt = *(extraDataPtr+5) & 0x1f;      // Number of pSps
    
    extraDataPtr += 6;
    while(i<2)
    {
        for(;cnt>0; cnt--)
        {
         	buf[0] = 0x00;
         	buf[1] = 0x00;
         	buf[2] = 0x00;
         	buf[3] = 0x01;
            nalSize = AV_RB16(extraDataPtr);
            
            extraDataPtr += 2;
            //(*MemWrite)(buf+4,extraDataPtr, nalSize);
            (*MemWrite)(extraDataPtr,buf+4, nalSize);
            buf += 4+nalSize;
            extraDataPtr += nalSize;
        }
	
    	cnt = *(extraDataPtr) & 0xff;      // Number of pPps
        extraDataPtr += 1;
        i++;
    }
	
    hCtx->nExtraDataLen = buf - hCtx->pExtraDataBuf+8;
    if(hCtx->nExtraDataLen > H264VDEC_MAX_EXTRA_DATA_LEN)
    {
    	//LOGD("the hCtx->nExtraDataLen is %d, larger than the H264Vdec_MAX_EXTRA_DATA_LEN\n", hCtx->nExtraDataLen);
    }
    return 0;
}


int32_t H264RequestBitstreamData(H264Context* hCtx)
{   
    int64_t nDiffPts = 0;
    uint32_t nLastDecStep = 0;
    VideoStreamDataInfo* newStreamData = NULL;

    while(1)
    {
        newStreamData = SbmRequestStream(hCtx->vbvInfo.vbv);
        if(newStreamData == NULL)
        {   
        	if(hCtx->bEndOfStream == 1)
        	{
        		H264FlushDelayedPictures(hCtx);
        	}
            return VDECODE_RESULT_NO_BITSTREAM;
        }

        if(newStreamData->nLength == 0)
        {
            SbmFlushStream(hCtx->vbvInfo.vbv, newStreamData);
            continue;
        }

        if((newStreamData->nPts!= H264VDEC_ERROR_PTS_VALUE)&&(hCtx->vbvInfo.nLastValidPts>0))
        {
        	nDiffPts = newStreamData->nPts-hCtx->vbvInfo.nLastValidPts;
        	if((nDiffPts>=2000000)||(nDiffPts<=-2000000))
        	{
        		nLastDecStep = hCtx->nDecStep;
        	    //H264DecoderRest((DecoderInterface*)h264DecCtx);
        	    H264FlushDelayedPictures(hCtx);
        	    H264ResetDecoderParams(hCtx);

        	    SbmReturnStream(hCtx->vbvInfo.vbv, newStreamData);
        	    hCtx->vbvInfo.nLastValidPts = H264VDEC_ERROR_PTS_VALUE;
        	    hCtx->nDecStep = nLastDecStep;
        	    logd("diff pts is large than nDiffPts=%lld\n", nDiffPts);
        	    return VDECODE_RESULT_NO_BITSTREAM;
        	}
        }
        break;
    }

	
    hCtx->vbvInfo.pVbvStreamData = newStreamData;
    hCtx->vbvInfo.nVbvDataSize = newStreamData->nLength+H264_EXTENDED_DATA_LEN;
    
    hCtx->vbvInfo.pVbvDataEndPtr = (uint8_t*)(newStreamData->pData+hCtx->vbvInfo.nVbvDataSize);
    hCtx->vbvInfo.pVbvDataStartPtr = (uint8_t*)newStreamData->pData;
    hCtx->vbvInfo.pReadPtr = (uint8_t*)newStreamData->pData;
    hCtx->vbvInfo.nValidDataPts  = H264VDEC_ERROR_PTS_VALUE;
    if((newStreamData->nPts!=H264VDEC_ERROR_PTS_VALUE) &&(hCtx->vbvInfo.nVbvDataPts!=newStreamData->nPts))
    {
    	hCtx->vbvInfo.nLastValidPts =  newStreamData->nPts;
        hCtx->vbvInfo.nValidDataPts = newStreamData->nPts;
    }
    hCtx->vbvInfo.nVbvDataPts  = newStreamData->nPts;
    return VDECODE_RESULT_OK;
}


void H264SetVbvParams(H264Context* hCtx,uint8_t* startBuf, uint8_t* endBuf, uint32_t dataLen, uint32_t dataCtrlFlag)
{   
    uint32_t phyAddr = 0;
    uint32_t highPhyAddr = 0;
    uint32_t lowPhyAddr = 0;
       
    hCtx->vbvInfo.pVbvDataStartPtr = startBuf;
    hCtx->vbvInfo.pVbvBuf = startBuf;

    hCtx->vbvInfo.pVbvBufEnd = endBuf;
    hCtx->vbvInfo.pReadPtr = startBuf;
    hCtx->vbvInfo.nVbvDataSize = dataLen+H264_EXTENDED_DATA_LEN;
    
    hCtx->vbvInfo.pVbvDataEndPtr = startBuf+ hCtx->vbvInfo.nVbvDataSize;
    hCtx->vbvInfo.bVbvDataCtrlFlag = dataCtrlFlag;


    phyAddr = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->vbvInfo.pVbvBuf)&0x7fffffff;
    highPhyAddr = (phyAddr>>28) & 0x0f;
    lowPhyAddr =  phyAddr & 0x0ffffff0;

    hCtx->vbvInfo.nVbvBufPhyAddr = lowPhyAddr+highPhyAddr;
    hCtx->vbvInfo.nVbvBufEndPhyAddr = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->vbvInfo.pVbvBufEnd)&0x7fffffff;
}
/****************************************************************************************************/
/****************************************************************************************************/

int8_t H264ComputeScaleRatio(uint32_t orgSize, uint32_t dstSize)
{   
    uint8_t scaleRatio = 0;
    
    if(dstSize == 0)
    {
        scaleRatio = 0;
    }
    else if(orgSize > dstSize)
    {
        scaleRatio = orgSize / dstSize;
        switch(scaleRatio)
        {
            case 0:
            case 1:
            case 2:
            {
                scaleRatio = 1;
                break;
            }
            case 3:
            case 4:
            {
                scaleRatio = 2;
                break;
            }
            default:
            {
                scaleRatio = 2; //* 1/8 scale down is not support.
                break;
            }
        }
    }
    return scaleRatio;
}

int32_t H264MallocBuffer(H264DecCtx* h264DecCtx, H264Context* hCtx , uint32_t H264_FRM_BUF_NUM)
{   
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t validFrmNum = 0;
    uint32_t fieldMvColBufSize= 0;
    uint32_t j = 0;
    uint32_t nRefFrameFormat = 0;
    uint32_t nDispFrameFormat = 0;
    H264Dec* h264Dec = NULL;
    FbmCreateInfo mFbmCreateInfo;
    
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    
    nRefFrameFormat = PIXEL_FORMAT_YUV_MB32_420;
    nDispFrameFormat = PIXEL_FORMAT_YUV_MB32_420;

    width = hCtx->nFrmMbWidth;
    height = hCtx->nFrmMbHeight;

    hCtx->pFbm  = NULL;
    hCtx->pFbmScaledown = NULL;

    if(hCtx->pFbm == NULL) 
    {
    	memset(&mFbmCreateInfo, 0, sizeof(FbmCreateInfo));
        mFbmCreateInfo.nFrameNum          = hCtx->nRefFrmCount+H264_FRM_BUF_NUM;
        mFbmCreateInfo.nWidth             = width;
        mFbmCreateInfo.nHeight            = height;
        mFbmCreateInfo.ePixelFormat       = nRefFrameFormat;
        mFbmCreateInfo.nBufferType        = BUF_TYPE_REFERENCE_DISP;
        mFbmCreateInfo.bProgressiveFlag   = hCtx->bProgressice;

        hCtx->pFbm = FbmCreate(&mFbmCreateInfo);
        if(hCtx->pFbm == NULL)
        {
        	return VDECODE_RESULT_UNSUPPORTED;
        }
        validFrmNum = hCtx->nRefFrmCount+H264_FRM_BUF_NUM;
    }

    if(hCtx->pFbm == NULL)
    {
        return VDECODE_RESULT_UNSUPPORTED;
    }

	//****************************construct_fbm*******************************************//
    hCtx->frmBufInf.nMaxValidFrmBufNum = validFrmNum;
    memset(hCtx->frmBufInf.picture, 0, sizeof(H264PicInfo)*MAX_PICTURE_COUNT);

    fieldMvColBufSize = hCtx->nMbHeight*(2-hCtx->bFrameMbsOnlyFlag);
    fieldMvColBufSize = (fieldMvColBufSize+1)/2;
    fieldMvColBufSize = hCtx->nMbWidth*fieldMvColBufSize*32;
    
    if(hCtx->bDirect8x8InferenceFlag == 0)
    {
        fieldMvColBufSize *= 2;
    }

    hCtx->frmBufInf.pMvColBuf = (*MemPalloc)(fieldMvColBufSize*hCtx->frmBufInf.nMaxValidFrmBufNum*2);
    if(hCtx->frmBufInf.pMvColBuf == NULL)
    {
    	logd("malloc buffer for hCtx->frmBufInf.pMvColBuf failed\n");
    	return VDECODE_RESULT_UNSUPPORTED;
    }
    (*MemSet)(hCtx->frmBufInf.pMvColBuf, 0, fieldMvColBufSize*hCtx->frmBufInf.nMaxValidFrmBufNum*2);
    
    for(j=0; j<(uint32_t)hCtx->frmBufInf.nMaxValidFrmBufNum; j++)
    {
        hCtx->frmBufInf.picture[j].pTopMvColBuf = hCtx->frmBufInf.pMvColBuf+(j*2+0)*fieldMvColBufSize;
        hCtx->frmBufInf.picture[j].pBottomMvColBuf = hCtx->frmBufInf.pMvColBuf+(j*2+1)*fieldMvColBufSize;
        hCtx->frmBufInf.picture[j].pVPicture = NULL;
    }
    (*MemFlushCache)(hCtx->frmBufInf.pMvColBuf, fieldMvColBufSize*hCtx->frmBufInf.nMaxValidFrmBufNum*2);

    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

    if(hCtx->nFrmMbWidth > 2048)
    {
    	hCtx->bUseDramBufFlag = 1;
    	hCtx->pDeblkDramBuf = (*MemPalloc)((hCtx->nMbWidth+31)*16*12);
    	if(hCtx->pDeblkDramBuf == NULL)
    	{
    		return VDECODE_RESULT_UNSUPPORTED;
    	}
    	hCtx->pIntraPredDramBuf = (*MemPalloc)((hCtx->nMbWidth+63)*16*5);
    	if(hCtx->pIntraPredDramBuf == NULL)
    	{
    		return VDECODE_RESULT_UNSUPPORTED;
    	}
    	(*MemFlushCache)(hCtx->pDeblkDramBuf, (hCtx->nMbWidth+31)*16*12);
    	(*MemFlushCache)(hCtx->pIntraPredDramBuf, (hCtx->nMbWidth+63)*16*5);
    }
    return VDECODE_RESULT_OK;
}

int32_t H264MallocFrmBuffer(H264DecCtx* h264DecCtx, H264Context* hCtx)
{
    uint8_t H264_FRM_BUF_NUM = 0;
    int32_t ret = 0;

    H264_FRM_BUF_NUM = 8;
    ret = H264MallocBuffer(h264DecCtx, hCtx,H264_FRM_BUF_NUM);
    return ret;
}
/****************************************************************************************************/
/****************************************************************************************************/


void H264SortDisplayFrameOrder(H264DecCtx* h264DecCtx,H264Context* hCtx, uint8_t bDecodeKeyFrameOnly)
{
	int32_t i = 0;
    int32_t minPoc = 0x7fffffff;
    int32_t delayedBufIndex = 0xff;
    int32_t anciDelayedBufIndex = 0xff;
    int32_t nonRefFrameNum = 0;
    int32_t refFrameNum = 0;
    uint8_t  findDispFlag = 0;
    uint8_t  flag2 = 0;
    uint8_t  flag3 = 0;
    uint8_t  bDispFrameFlag = 0;
	H264DecCtx* ph264DecCtx = NULL;
	uint8_t bbDecodeKeyFrameOnly;

	H264PicInfo* curPicInf = NULL;
	H264PicInfo* outPicInf = NULL;
	
	ph264DecCtx = h264DecCtx;
	bbDecodeKeyFrameOnly = bDecodeKeyFrameOnly;

    curPicInf = hCtx->frmBufInf.pCurPicturePtr;
    if(curPicInf->nDecodeBufIndex != curPicInf->nDispBufIndex)
    {
    	hCtx->frmBufInf.picture[curPicInf->nDecodeBufIndex].pVPicture = NULL;
    	hCtx->frmBufInf.picture[curPicInf->nDecodeBufIndex].nReference = 0;
    }

    hCtx->frmBufInf.pDelayedPic[hCtx->nDelayedPicNum] = curPicInf;
    hCtx->frmBufInf.pDelayedPic[hCtx->nDelayedPicNum+1] = NULL;
    hCtx->nDelayedPicNum++;
    hCtx->frmBufInf.pCurPicturePtr = NULL;

    for(i=0; i<hCtx->nDelayedPicNum; i++)
    {
    	if(hCtx->frmBufInf.pDelayedPic[i]->nReference==0 || hCtx->frmBufInf.pDelayedPic[i]->nReference==4)
    	{
    		nonRefFrameNum++;
    	}
    	else
    	{
    		refFrameNum++;
    	}

    	if(hCtx->frmBufInf.pDelayedPic[i]->nPoc<minPoc)
    	{
    		minPoc = hCtx->frmBufInf.pDelayedPic[i]->nPoc;
    		anciDelayedBufIndex = i;

    		if(findDispFlag ==1)
    		{
    			continue;
    		}
    		if(hCtx->frmBufInf.pDelayedPic[i]->nPoc<=hCtx->nMinDispPoc)
    		{
    			if(i != 0)
    			{
    				findDispFlag = 1;
    				continue;
    			}
    		}
    		outPicInf = hCtx->frmBufInf.pDelayedPic[i];
    	    delayedBufIndex = i;
    	}
    }

    findDispFlag = 1;

    if(hCtx->frmBufInf.pDelayedOutPutPic != NULL)
    {
    	if(outPicInf->nPoc == 0)
    	{
    		if(minPoc < 0)
    		{
    			outPicInf = hCtx->frmBufInf.pDelayedPic[anciDelayedBufIndex];
    			delayedBufIndex = anciDelayedBufIndex;
    		}
    	}

    	flag2 = (outPicInf->nPoc==0);
        flag3 = ((outPicInf->nPoc-hCtx->nMinDispPoc)==hCtx->nPicPocDeltaNum);

        if(flag2==0 && flag3==0 && nonRefFrameNum==0)
    	{
        	if(refFrameNum < hCtx->nRefFrmCount)
        	{
        		return;
        	}
    	}
        if((flag3==1) && (hCtx->nCurFrmNum==2) && (hCtx->nPicPocDeltaNum==2))
        {
        	return;
        }
    }

    if(outPicInf != NULL)
    {
    	outPicInf->bHasDispedFlag = 1;
    	bDispFrameFlag = 1;
    	if(outPicInf->nReference==0 || outPicInf->nReference==4)
        {
    		FbmReturnBuffer(hCtx->pFbm, outPicInf->pVPicture, bDispFrameFlag);
            hCtx->frmBufInf.picture[outPicInf->nDispBufIndex].pVPicture = NULL;
			outPicInf->pVPicture = NULL;
    		outPicInf->nReference = 0;
    		outPicInf->bHasDispedFlag = 0;
        }
    	else if(bDispFrameFlag==1)
    	{
    		FbmShareBuffer(hCtx->pFbm, outPicInf->pVPicture);
    	}
    	for(i=delayedBufIndex; i<hCtx->nDelayedPicNum; i++)
    	{
    		hCtx->frmBufInf.pDelayedPic[i] = hCtx->frmBufInf.pDelayedPic[i+1];
    	}
    	hCtx->frmBufInf.pDelayedPic[i] = NULL;
    	hCtx->nMinDispPoc = outPicInf->nPoc;
    	hCtx->nDelayedPicNum--;
  		hCtx->frmBufInf.pDelayedOutPutPic = outPicInf;
    }
}

void H264exchangeValues(int32_t* param1,int32_t* param2)
{
    int32_t temp = 0;
    temp = * param2;
    *param2 = *param1;
    *param1 = temp;
}


void H264CongigureDisplayParameters(H264DecCtx* h264DecCtx, H264Context* hCtx)
{
    VideoPicture*  curFrmInfo = NULL;

    curFrmInfo = hCtx->frmBufInf.pCurPicturePtr->pVPicture;
    curFrmInfo->nTopOffset      = hCtx->nTopOffset;
    curFrmInfo->nLeftOffset     = hCtx->nLeftOffset;
    curFrmInfo->nRightOffset    = hCtx->pCurSps->nFrmRealWidth - hCtx->nRightOffset;
    curFrmInfo->nBottomOffset   = hCtx->pCurSps->nFrmRealHeight - hCtx->nBottomOffset;

    curFrmInfo->nFrameRate              = hCtx->vbvInfo.nFrameRate;  // need update
    curFrmInfo->nAspectRatio            = 1000;

    curFrmInfo->bIsProgressive         = hCtx->bProgressice;
    curFrmInfo->bTopFieldFirst         = (hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[0]<=hCtx->frmBufInf.pCurPicturePtr->nFieldPoc[1]);
    curFrmInfo->bRepeatTopField        = 0;
    curFrmInfo->ePixelFormat          = h264DecCtx->vconfig.eOutputPixelFormat;
    curFrmInfo->nStreamIndex           = 0;
}

//******************************************************************************************************//
//*****************************************************************************************************//

int32_t H264ProcessNaluUnit(H264DecCtx* h264DecCtx, H264Context* hCtx , uint8_t nalUnitType, int32_t sliceDataLen, uint8_t decStreamIndex)
{   
    int32_t ret = VDECODE_RESULT_OK;	
    uint32_t ve_status_reg = 0;
    uint32_t decMbNum = 0;
    uint32_t picSizeInMb  = 0;
    H264Dec* h264Dec = NULL;
    
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    hCtx->nalUnitType = nalUnitType;

    if(h264Dec->nDecStreamIndex == 0)
    {
    	if(hCtx->bNeedFindPPS==1 || hCtx->bNeedFindSPS==1)
    	{
    		if(nalUnitType!=NAL_SPS&& hCtx->bNeedFindSPS==1)
    		{
    			return VDECODE_RESULT_OK;
    		}
    		if((nalUnitType!=NAL_PPS&&nalUnitType!=NAL_SPS) && hCtx->bNeedFindPPS==1)
    		{
    			return VDECODE_RESULT_OK;
    		}
    	}
    }
    else if(h264Dec->nDecStreamIndex == 1)
    {
    	if(hCtx->bNeedFindSPS==1)
    	{
    		if(nalUnitType!=NAL_SPS_EXT&& hCtx->bNeedFindSPS==1)
    		{
    			return VDECODE_RESULT_OK;
    		}
    		hCtx->bNeedFindSPS = 0;
    		hCtx->bNeedFindPPS = 0;
    	}
    }

    hCtx->bIdrFrmFlag = 0;

    switch(nalUnitType)
    {
        case NAL_IDR_SLICE:
        {   
            hCtx->bNeedFindIFrm = 0;
            H264ReferenceRefresh(hCtx);
            hCtx->bIdrFrmFlag = 1;
        }
        case NAL_SLICE:
        {   
            if(hCtx->frmBufInf.nMaxValidFrmBufNum == 0)
            {
                ret = H264MallocFrmBuffer(h264DecCtx, hCtx);
                if(ret != VDECODE_RESULT_OK)
                {   
                    logd("malloc buffer error\n");
                    return ret;
                }
            }
            ret = H264DecodeSliceHeader(h264DecCtx, hCtx);
            if(ret != VDECODE_RESULT_OK)
            {
                if(ret == VRESULT_ERR_FAIL)
                {
                	hCtx->vbvInfo.nValidDataPts = H264VDEC_ERROR_PTS_VALUE;
                }
                return ret;
            }       
            hCtx->nDecFrameStatus = H264_START_DEC_FRAME;
            H264ConfigureSliceRegister(h264DecCtx, hCtx, decStreamIndex);
            break;
        }
        case NAL_SPS:
        {   
            ret = H264DecodeSps(h264DecCtx, hCtx);
            if(ret != VDECODE_RESULT_OK)
            {   
                logv("decode pSps ret=%d\n", ret);
                return ret;
            }
            hCtx->bNeedFindSPS = 0;
            break;
        }
        case NAL_PPS:
        {   
            ret = H264DecodePps(h264DecCtx, hCtx, sliceDataLen);
            if(ret != VDECODE_RESULT_OK)
            {
                return ret;
            }
            hCtx->bNeedFindPPS = 0;
            break;
        }
        case NAL_SEI:
        {
            ret = H264DecodeSei(h264DecCtx, hCtx, sliceDataLen);
        	break;
        }
        //case NAL_DPA:
        //case NAL_DPB:
        //case NAL_DPC:
        default:
        {
            return VDECODE_RESULT_OK;
        }
    }
	
    if(((hCtx->nalUnitType>=NAL_SLICE)&&(hCtx->nalUnitType<=NAL_IDR_SLICE))||
    		(hCtx->nalUnitType==NAL_HEADER_EXT1|| hCtx->nalUnitType==NAL_HEADER_EXT2))

    {
        AdapterVeWaitInterrupt();

        H264CheckBsDmaBusy(h264DecCtx);
        ve_status_reg = H264VeIsr(h264DecCtx);
		
        decMbNum = H264GetDecodeMbNum(h264DecCtx);
        picSizeInMb = hCtx->nMbWidth*hCtx->nMbHeight*(2-hCtx->bFrameMbsOnlyFlag);
        if(hCtx->nPicStructure != PICT_FRAME)
        {
            picSizeInMb >>= 1;
        }
        hCtx->bLastMbInSlice = decMbNum;

        //logd("decMbNum=%d, picSizeInMb=%d\n", decMbNum, picSizeInMb);

        if(decMbNum >= picSizeInMb)
        {   
            // decode the frame end
        	//logv("dec the frame end\n");
            hCtx->nCurSliceNum  = 0;
            hCtx->bLastMbInSlice = 0;
            hCtx->nDecFrameStatus = H264_END_DEC_FRAME;

        	if(hCtx->frmBufInf.pCurPicturePtr == NULL)
            {
        		return VRESULT_ERR_FAIL;
            }
            ret = (hCtx->frmBufInf.pCurPicturePtr->bKeyFrame==1)? VDECODE_RESULT_KEYFRAME_DECODED : VDECODE_RESULT_FRAME_DECODED;
        }
    }
    return ret;
}


