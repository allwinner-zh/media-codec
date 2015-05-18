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

int H264DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo);
void Destroy(DecoderInterface* pSelf);
void H264DecoderRest(DecoderInterface* pSelf);
int  H264DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex);
Fbm* H264DecoderGetFbm(DecoderInterface* pSelf,int index);
int H264DecoderGetFbmNum(DecoderInterface* pSelf);
int  H264DecoderDecode(DecoderInterface* pSelf, 
                       int32_t               bEndOfStream, 
		               int32_t               bDecodeKeyFrameOnly,
                       int32_t               bSkipBFrameIfDelay, 
                       int64_t               nCurrentTimeUs);
void H264FreeMemory(H264Context* hCtx);


extern int32_t H264InitDecode(H264DecCtx* h264DecCtx, H264Dec* h264Dec, H264Context* hCtx);
extern int32_t H264DecodeExtraData(H264Context* hCtx, uint8_t* extraDataPtr);
extern int32_t H264RequestBitstreamData(H264Context* hCtx);
extern int32_t H264ProcessNaluUnit(H264DecCtx* h264DecCtx, H264Context* h264Ctx , uint8_t nal_unit_type, int32_t sliceDataLen, uint8_t decStreamIndex);
extern int32_t H264ExecuteRefPicMarking(H264Context* hCtx, H264MmcoInfo* mmco, int32_t mmcoCount);
extern uint32_t H264GetBits(H264DecCtx* h264DecCtx, uint8_t len);
extern int32_t H264UpdateDataPointer(H264DecCtx* h264DecCtx, H264Context* hCtx, uint8_t offsetMode);
extern uint32_t H264VeIsr(H264DecCtx* h264DecCtx);
extern uint32_t H264GetbitOffset(H264DecCtx* h264DecCtx, uint8_t offsetMode);

extern void H264CheckBsDmaBusy(H264DecCtx *h264DecCtx);
extern void H264SearchStartcode(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern void H264SortDisplayFrameOrder(H264DecCtx* h264DecCtx, H264Context* hCtx, uint8_t bDecodeKeyFrameOnly);
extern void H264CongigureDisplayParameters(H264DecCtx* h264DecCtx, H264Context* hCtx);
extern void H264ResetDecoderParams(H264Context* hCtx);
extern void H264SetVbvParams(H264Context* hCtx,uint8_t* startBuf, uint8_t* endBuf, uint32_t dataLen, uint32_t dataCtrlFlag);
extern void H264ConfigureAvcRegister( H264DecCtx* h264DecCtx, H264Context* hCtx, uint8_t eptbDetectEnable, uint32_t nBitLens);
extern void H264ConfigureEptbDetect(H264DecCtx* h264DecCtx, H264Context* hCtx, uint32_t sliceDataBits, uint8_t eptbDetectEnable);
//extern void H264InitRegister(H264DecCtx *h264DecCtx);
extern void H264InitFuncCtrlRegister(H264DecCtx *h264DecCtx);
extern void H264ProcessDecodeFrameBuffer(H264DecCtx* h264DecCtx, H264Context* hCtx);

//*******************************************************************//
//*******************************************************************//

DecoderInterface* CreateH264Decoder(VideoEngine* p)
{
    H264DecCtx* h264DecCtx = NULL;
    
    h264DecCtx = (H264DecCtx*)malloc(sizeof(H264DecCtx));
    if(h264DecCtx == NULL)
    {
        return NULL;
    }

    MemPalloc           = AdapterMemPalloc;
    MemPfree   		    = AdapterMemPfree;
    MemFlushCache       = AdapterMemFlushCache;
    MemGetPhysicAddress = AdapterMemGetPhysicAddress;
    MemSet              = AdapterMemSet;
    MemCopy             = AdapterMemCopy;
    MemRead             = AdapterMemRead;
    MemWrite            = AdapterMemWrite;
    
    memset(h264DecCtx, 0, sizeof(H264DecCtx));

    h264DecCtx->pVideoEngine        = p;
    h264DecCtx->interface.Init      = H264DecoderInit;
    h264DecCtx->interface.Reset     = H264DecoderRest;
    h264DecCtx->interface.SetSbm    = H264DecoderSetSbm;
    h264DecCtx->interface.GetFbmNum = H264DecoderGetFbmNum;
    h264DecCtx->interface.GetFbm    = H264DecoderGetFbm;
    h264DecCtx->interface.Decode    = H264DecoderDecode;
    h264DecCtx->interface.Destroy   = Destroy;
    return &h264DecCtx->interface;
}

void CedarPluginVDInit(void)
{
    int ret;
    ret = VDecoderRegister(VIDEO_CODEC_FORMAT_H264, "h264", CreateH264Decoder);

    if (0 == ret)
    {
        logi("register h264 decoder success!");
    }
    else
    {
        loge("register h264 decoder failure!!!");
    }
    return ;
}

//*******************************************************************//
//*******************************************************************//

//************************************************************************************/
//      Name:	    ve_open                                                          //
//      Prototype:	Handle ve_open (vconfig_t* config, videoStreamInfo_t* stream_info); //
//      Function:	Start up the VE CSP.                                             //
//      Return:	A handle of the VE device.                                           //
//      Input:	vconfig_t* config, the configuration for the VE CSP.                 //
//************************************************************************************/

int H264ProcessExtraData(H264DecCtx* h264DecCtx,  H264Context* hCtx)
{
	// process extra data
	hCtx->bDecExtraDataFlag = 1;
	hCtx->pExtraDataBuf = (uint8_t*)(*MemPalloc)(H264VDEC_MAX_EXTRA_DATA_LEN);
	if(hCtx->pExtraDataBuf == NULL)
	{
		return VDECODE_RESULT_UNSUPPORTED;
	}
	(*MemSet)(hCtx->pExtraDataBuf, 0, H264VDEC_MAX_EXTRA_DATA_LEN);
	if(h264DecCtx->videoStreamInfo.pCodecSpecificData[0] == 0x01)
	{
		hCtx->bIsAvc = 1;
		if(h264DecCtx->videoStreamInfo.nCodecSpecificDataLen >= 7)
		{
			H264DecodeExtraData(hCtx, (uint8_t*)h264DecCtx->videoStreamInfo.pCodecSpecificData);
		}
	}
	else
	{
		hCtx->bIsAvc = 0;
		if(h264DecCtx->videoStreamInfo.nCodecSpecificDataLen > H264VDEC_MAX_EXTRA_DATA_LEN)
		{
			//LOGD("the extra data len is %d, larger than the H264VDEC_MAX_EXTRA_DATA_LEN", h264DecCtx->vStreamInfo.init_data_len);
		}
		(*MemWrite)(h264DecCtx->videoStreamInfo.pCodecSpecificData, hCtx->pExtraDataBuf, h264DecCtx->videoStreamInfo.nCodecSpecificDataLen);
		hCtx->nExtraDataLen = h264DecCtx->videoStreamInfo.nCodecSpecificDataLen;
	}
	return VDECODE_RESULT_OK;
}

int H264DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo)
{
    H264DecCtx* h264DecCtx = NULL;
    H264Dec*h264DecHandle = NULL;
    
    h264DecCtx = (H264DecCtx*)pSelf;
    memcpy(&h264DecCtx->vconfig, pConfig, sizeof(VConfig));
    memcpy(&h264DecCtx->videoStreamInfo, pVideoInfo, sizeof(VideoStreamInfo));

    h264DecHandle = (H264Dec*)malloc(sizeof(H264Dec));
    if(h264DecHandle == NULL)
    {   
        goto h264_open_error;
    }
    memset(h264DecHandle, 0, sizeof(H264Dec));
    	
    h264DecCtx->pH264Dec = (void*)h264DecHandle;

    h264DecHandle->nRegisterBaseAddr = (uint32_t)ve_get_reglist(REG_GROUP_H264_DECODER);
    h264DecHandle->nVeVersion =  get_ve_version_id();

    h264DecHandle->pHContext = (H264Context*)malloc(sizeof(H264Context));
    if(h264DecHandle->pHContext == NULL)
    {
        //LOGD("malloc memory for h264Dec->pHContext failed\n");
    	goto h264_open_error;
    }

    if(H264InitDecode(h264DecCtx, h264DecHandle, h264DecHandle->pHContext) < 0)
    {
        goto h264_open_error;
    }
    
    H264ResetDecoderParams(h264DecHandle->pHContext);
    if(h264DecCtx->videoStreamInfo.pCodecSpecificData!= NULL)
    {
    	if(H264ProcessExtraData(h264DecCtx, h264DecHandle->pHContext)< 0)
    	{
    		goto h264_open_error;
    	}
    }
    return VDECODE_RESULT_OK;
	
h264_open_error:
    if(h264DecHandle != NULL)
    {
    	if(h264DecHandle->pHContext != NULL)
    	{
    		H264FreeMemory(h264DecHandle->pHContext);
    	}
    	free(h264DecHandle);
    	h264DecHandle = NULL;
    	h264DecCtx->pH264Dec = NULL;
    }
    if(h264DecCtx != NULL)
    {
        free(h264DecCtx);
        h264DecCtx = NULL;
    }
    return VDECODE_RESULT_UNSUPPORTED;
}

/******************************************************************************************/
//       Name:	ve_close                                                                  //
//       Prototype: vresult_e ve_close (uint8_t flush_pictures);                               //
//       Function:	Close the VE CSP.                                                     //
//       Return:    VDECODE_RESULT_OK: success.                                                  //
//	     VRESULT_ERR_LIBRARY_NOT_OPEN: the CSP is not opened yet.                         //
/******************************************************************************************/


void H264FlushDelayedPictures(H264Context* hCtx)
{   
    int32_t outIdx=0;
    int32_t i = 0;

    H264PicInfo* outPicPtr = NULL;
    
    while(1)
    {
        outPicPtr = hCtx->frmBufInf.pDelayedPic[0];
        outIdx = 0;

        if(outPicPtr != NULL)
        {
        	for(i=1; hCtx->frmBufInf.pDelayedPic[i] && !hCtx->frmBufInf.pDelayedPic[i]->bKeyFrame; i++)
        	{
        		if(hCtx->frmBufInf.pDelayedPic[i]->pVPicture==NULL)
        		{
        			hCtx->frmBufInf.pDelayedPic[i] = NULL;
        		}
        		if(hCtx->frmBufInf.pDelayedPic[i]->nPoc < outPicPtr->nPoc)
        		{
        			outPicPtr = hCtx->frmBufInf.pDelayedPic[i];
        			outIdx = i;
        		}
        	}
        }
        hCtx->frmBufInf.pDelayedPic[i] = NULL;
        for(i=outIdx; hCtx->frmBufInf.pDelayedPic[i]; i++)
        {
            hCtx->frmBufInf.pDelayedPic[i] = hCtx->frmBufInf.pDelayedPic[i+1];
        }
    
        if(outPicPtr != NULL)
        {   
            if(outPicPtr->pVPicture != NULL)
            {
                FbmReturnBuffer(hCtx->pFbm, outPicPtr->pVPicture, 1);
                hCtx->frmBufInf.picture[outPicPtr->nDispBufIndex].pVPicture = NULL;
                outPicPtr->pVPicture = NULL;
            }
        }
        else
        {
            break;
        }
    }  
    for(i=0; i<MAX_PICTURE_COUNT; i++)
    {
    	hCtx->frmBufInf.pDelayedPic[i] = NULL;
    }
    hCtx->nDelayedPicNum = 0;
    for(i=0; i<hCtx->frmBufInf.nMaxValidFrmBufNum; i++)
    {
       	if(hCtx->frmBufInf.picture[i].pVPicture!=NULL)
       	{
       		FbmReturnBuffer(hCtx->pFbm, hCtx->frmBufInf.picture[i].pVPicture, 0);
       	}
       	hCtx->frmBufInf.picture[i].pVPicture = NULL;
       	hCtx->frmBufInf.picture[i].nReference = 0;
       	hCtx->frmBufInf.picture[i].nDecFrameOrder = 0;
    }
    return;
}

void H264FreeMemory(H264Context* hCtx)
{   
    int32_t i = 0;

    if(hCtx->pDeblkDramBuf != NULL)
    {
    	(*MemPfree)(hCtx->pDeblkDramBuf);
    	hCtx->pDeblkDramBuf = NULL;
    }
    if(hCtx->pIntraPredDramBuf != NULL)
    {
    	(*MemPfree)(hCtx->pIntraPredDramBuf);
    	hCtx->pIntraPredDramBuf = NULL;
    }
    if(hCtx->pExtraDataBuf != NULL)
    {
    	(*MemPfree)(hCtx->pExtraDataBuf);
    	hCtx->pExtraDataBuf = NULL;
    }
    if(hCtx->pMbFieldIntraBuf !=NULL)
    {
    	(*MemPfree)(hCtx->pMbFieldIntraBuf);
        hCtx->pMbFieldIntraBuf = NULL;
    }
    if(hCtx->pMbNeighborInfoBuf != NULL)
    {
    	(*MemPfree)(hCtx->pMbNeighborInfoBuf);
    	hCtx->pMbNeighborInfoBuf = NULL;
    }

      
    if(hCtx->frmBufInf.pMvColBuf != NULL)
    {
    	(*MemPfree)(hCtx->frmBufInf.pMvColBuf);
    	hCtx->frmBufInf.pMvColBuf = NULL;
    }

    if(hCtx->pFbm != NULL)
    {
    	FbmDestroy(hCtx->pFbm);
    	hCtx->pFbm = NULL;
    }
    if(hCtx->pFbmScaledown != NULL)
    {
    	FbmDestroy(hCtx->pFbmScaledown);
    	hCtx->pFbmScaledown = NULL;
    }
	for(i=0; i<hCtx->nSpsBufferNum; i++)
	{
		free(hCtx->pSpsBuffers[hCtx->nSpsBufferIndex[i]]);
		hCtx->pSpsBuffers[hCtx->nSpsBufferIndex[i]] = NULL;
	}
	for(i=0; i<hCtx->nPpsBufferNum; i++)
	{
		free(hCtx->pPpsBuffers[hCtx->nPpsBufferIndex[i]]);
		hCtx->pPpsBuffers[hCtx->nPpsBufferIndex[i]] = NULL;
	}
    free(hCtx);
    hCtx = NULL;
}

void Destroy(DecoderInterface* pSelf)
{   
    H264DecCtx* h264DecCtx = NULL;
    H264Dec* h264DecHandle = NULL;
    h264DecCtx = (H264DecCtx*)pSelf;

    if(h264DecCtx == NULL)
    {
        return;
    }

    h264DecHandle = (H264Dec*)h264DecCtx->pH264Dec;

    if(h264DecHandle != NULL)
    {
    	if(h264DecHandle->pHContext != NULL)
    	{
    		H264FreeMemory(h264DecHandle->pHContext);
    	}
    	free(h264DecHandle);
        h264DecHandle = NULL;
    }
    if(h264DecCtx != NULL)
    {
        free(h264DecCtx);
        h264DecCtx = NULL;
    }
}


/**********************************************************************   *******************/
//          Name:	ve_reset                                                                //
//          Prototype: vresult_e ve_reset (uint8_t flush_pictures)                               //
//          Function:  Reset the VE CSP.                                                    //
//          Return:    VDECODE_RESULT_OK: success.                                                 //
//	                   VRESULT_ERR_LIBRARY_NOT_OPEN: the CSP is not opened yet.             //
/********************************************************************************************/

void  H264DecoderRest(DecoderInterface* pSelf)
{   
    H264DecCtx* h264DecCtx = NULL;
    H264Dec* h264DecHandle = NULL;

    h264DecCtx = (H264DecCtx*)pSelf;

    if(h264DecCtx == NULL)
    {
        return;
    }

    ResetVeInternal(h264DecCtx->pVideoEngine);

    h264DecHandle = (H264Dec*)h264DecCtx->pH264Dec;
    if(h264DecHandle != NULL)
    {
    	h264DecHandle->nDecStreamIndex = 0;
    	if(h264DecHandle->pHContext != NULL)
    	{
    	    H264FlushDelayedPictures(h264DecHandle->pHContext);
    	    H264ResetDecoderParams(h264DecHandle->pHContext);
    	}
    }
}


/*********************************************************************************************************/
//             Name:	ve_set_vbv                                                                       //
//             Prototype: vresult_e ve_set_vbv (uint8_t* vbv_buf, uint32_t vbv_size, Handle ve);                   //
//             Function: Set VBV's bitstream buffer base address and buffer size to the CSP..            //
//             Return:   VDECODE_RESULT_OK: success.                                                            //
//	                     VRESULT_ERR_LIBRARY_NOT_OPEN: the CSP is not opened yet.                        //
/*********************************************************************************************************/

int  H264DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex)
{   
   	H264Dec*h264DecHandle = NULL;
    H264DecCtx* h264DecCtx = NULL;
    h264DecCtx = (H264DecCtx*)pSelf;
    
    h264DecHandle = (H264Dec*)h264DecCtx->pH264Dec;

    if(nIndex == 0)
    {
    	h264DecHandle->pHContext->pVbv    		= pSbm;
    	h264DecHandle->pHContext->pVbvBase 		= (uint8_t*)SbmBufferAddress(pSbm);
    	h264DecHandle->pHContext->nVbvSize 		= SbmBufferSize(pSbm);
    	h264DecHandle->pHContext->vbvInfo.vbv   = pSbm;
    }
    return VDECODE_RESULT_OK;
}


/****************************************************************************************************/
//             Name:	ve_get_fbm                                                                  //
//             Prototype: Handle get_fbm (void);                                                    //
//             Function: Get a handle of the FBM instance, in which pictures for display are stored.//
//             Return:   Not NULL Handle: handle of the FBM instance.                               //
//                       NULL Handle: FBM module is not initialized yet.                            //            
/****************************************************************************************************/

Fbm* H264DecoderGetFbm(DecoderInterface* pSelf, int index)
{   
    H264DecCtx* h264DecCtx = NULL;
    H264Dec*h264Dec = NULL;
    H264Context* hCtx = NULL;

    
    h264DecCtx = (H264DecCtx*)pSelf;
    	
    if(h264DecCtx == NULL)
    {   
        return NULL;
    }
    else
    {    
        h264Dec =  (H264Dec*)h264DecCtx->pH264Dec;
        if(h264Dec->pHContext->frmBufInf.nMaxValidFrmBufNum == 0)
        {   
            return NULL;
        }
        else
        {
        	if(index == 0)
        	{
        		hCtx = h264Dec->pHContext;
        	}
        	return  hCtx->pFbm;
        }
    }
    return NULL;
}


int H264DecoderGetFbmNum(DecoderInterface* pSelf)
{   
	H264DecCtx* h264DecCtx = NULL;
	h264DecCtx = (H264DecCtx*)pSelf;
	if(h264DecCtx == NULL)
	{   
		return 0;
	}
    return 1;
}


/**********************************************************************************************************/
//   Name:	ve_decode                                                                                      //
//   Prototype: vresult_e decode (VideoStreamDataInfo* stream, uint8_t keyframe_only, uint8_t skip_bframe, uint32_t cur_time); //
//   Function: Decode one bitstream frame.                                                                 //
//   INput:    VideoStreamDataInfo* stream: start address of the stream frame.                                  //
//             uint8_t keyframe_only:   tell the CSP to decode key frame only;                                  //
//             uint8_t skip_bframe:  tell the CSP to skip B frame if it is overtime;                            //
//             uint32_t cur_time:current time, used to compare with PTS when decoding B frame;                  //
//   Return:   VDECODE_RESULT_OK:             decode stream success but no frame decoded;                         //
//             VDECODE_RESULT_FRAME_DECODED:  one common frame decoded;                                           //
//             VDECODE_RESULT_KEYFRAME_DECODED:    one key frame decoded;                                         //
//             VRESULT_ERR_FAIL:           decode stream fail;                                             //
//             VRESULT_ERR_INVALID_PARAM:  either stream or ve is NULL;                                    //
//             VRESULT_ERR_INVALID_STREAM: some error data in the stream, decode fail;                     //
//             VRESULT_ERR_NO_MEMORY:      allocate memory fail in this method;                            //
//             VRESULT_ERR_NO_FRAMEBUFFER: request empty frame buffer fail in this method;                 //
//             VRESULT_ERR_UNSUPPORTED:    stream format is unsupported by this version of VE CSP;         //    
//             VRESULT_ERR_LIBRARY_NOT_OPEN:  'open' has not been successfully called yet.                 //
/***********************************************************************************************************/


uint8_t H264CheckNextStartCode(H264Context* hCtx, uint32_t bitOffset, uint8_t* pNewReadPtr)
{
	uint8_t* pReadPtr = NULL;
	uint8_t* pBuffer = NULL;
	uint32_t size = 0;
	uint8_t  i = 0;
	uint8_t buffer[6];
	uint32_t nextCode = 0xffffffff;

	if((bitOffset>>3) > (uint32_t)(hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pVbvBuf+1))
	{
		bitOffset -= (hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pVbvBuf+1)*8;
	}
	pReadPtr = hCtx->vbvInfo.pVbvBuf+ (bitOffset>>3);
	if(pReadPtr >= pNewReadPtr)
	{
		if(!((hCtx->vbvInfo.pReadPtr<=pReadPtr)&&(pReadPtr<hCtx->vbvInfo.pReadPtr)))
		{
			pReadPtr = pNewReadPtr;
		}
	}
	pReadPtr -= 6;
    if(pReadPtr+6 <= hCtx->vbvInfo.pVbvBufEnd)
    {
    	pBuffer = pReadPtr;
    }
    else
    {
    	size = hCtx->vbvInfo.pVbvBufEnd-pReadPtr+1;
    	memcpy(buffer, pReadPtr, size);
    	memcpy(buffer+size, hCtx->vbvInfo.pVbvBuf, 6-size);
    	pBuffer = buffer;
    }

    for(i=0; i<6; i++)
    {
    	nextCode <<= 8;
    	nextCode |= pBuffer[i];
    	if(nextCode == 0x000001)
    	{
    		return 1;
    	}
    }
    return 0;
}


int  H264DecoderDecode(DecoderInterface* pSelf,
		                    int32_t           bEndOfStream,
		                    int32_t           bDecodeKeyFrameOnly,
                            int32_t           bSkipBFrameIfDelay,
                            int64_t           nCurrentTimeUs)
{
    H264DecCtx* h264DecCtx = NULL;
    H264Dec* h264Dec = NULL;
    H264Context* hCtx = NULL;
    uint8_t  nal_unit_type  = 0;
    uint32_t ve_status_reg = 0;
    int32_t ret = 0;
    int32_t i = 0;
    uint32_t bitOffset1 = 0;
    uint32_t sliceDataBits = 0;
    uint32_t sliceLen = 0;
    uint8_t  NAL_PPS = 8;
    uint8_t* pOldReadPtr = NULL;
    uint32_t nOldVbvDataSize = 0;
    uint8_t* pNewReadPtr = NULL;
	int32_t bbSkipBFrameIfDelay = 0;
    
    h264DecCtx = (H264DecCtx*)pSelf;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

    if(h264Dec->nDecStreamIndex == 0)
    {
    	hCtx = 	h264Dec->pHContext;
    }
	bbSkipBFrameIfDelay = bSkipBFrameIfDelay;
    hCtx->bEndOfStream = bEndOfStream;
    hCtx->nSystemTime = nCurrentTimeUs;

	//**************************************************************//
	//**************step 1*****************************************//
    if(hCtx->nDecStep == H264_STEP_CONFIG_VBV)
    {   
        ResetVeInternal(h264DecCtx->pVideoEngine);
        H264InitFuncCtrlRegister(h264DecCtx);
        
        hCtx->bCanResetHw  = 1;
        hCtx->bNeedCheckFbmNum = 1;
        
        if(hCtx->bDecExtraDataFlag == 1)
        {
            (*MemFlushCache)((uint8_t*)hCtx->pExtraDataBuf,hCtx->nExtraDataLen);
            H264SetVbvParams(hCtx, hCtx->pExtraDataBuf, hCtx->pExtraDataBuf+1024-1,hCtx->nExtraDataLen,
            											FIRST_SLICE_DATA|LAST_SLICE_DATA|SLICE_DATA_VALID);
        }
        else
        {   
            H264SetVbvParams(hCtx, hCtx->pVbvBase, hCtx->pVbvBase+hCtx->nVbvSize-1, 0,
            										 FIRST_SLICE_DATA|LAST_SLICE_DATA|SLICE_DATA_VALID);
            ret = H264RequestBitstreamData(hCtx);
            if(ret == VDECODE_RESULT_NO_BITSTREAM)
            {
                hCtx->nDecStep = H264_STEP_UPDATE_DATA;
                return VDECODE_RESULT_NO_BITSTREAM;
            }
        }
    	
        if((hCtx->bDecExtraDataFlag==1) || (hCtx->bIsAvc==0))
        {
            hCtx->nDecStep = H264_STEP_SEARCH_NALU;
        }
        else
        {
            hCtx->nDecStep = H264_STEP_SEARCH_AVC_NALU;
        }
    }
	//**************************************************************//
	//**************step 2*****************************************//
    if(hCtx->nDecStep == H264_STEP_UPDATE_DATA)
    {   
        if(hCtx->bNeedCheckFbmNum == 1)
        {
        	hCtx->bNeedFindIFrm |= bDecodeKeyFrameOnly;
        	hCtx->bOnlyDecKeyFrame = bDecodeKeyFrameOnly;
            if(hCtx->pFbm && FbmEmptyBufferNum(hCtx->pFbm) == 0)
            {
            	//logd("here1: VDECODE_RESULT_NO_FRAME_BUFFER\n");
  //          	H264ProcessDecodeFrameBuffer(h264DecCtx, hCtx);
                return VDECODE_RESULT_NO_FRAME_BUFFER;
            }
            if(hCtx->pFbmScaledown && FbmEmptyBufferNum(hCtx->pFbmScaledown) == 0)
            {
               	//logd("here2: VDECODE_RESULT_NO_FRAME_BUFFER\n");
               	//printf_fbm_status(hCtx->pFbmScaledown);
                return VDECODE_RESULT_NO_FRAME_BUFFER;
            }
            hCtx->bNeedCheckFbmNum = 0;
        }

        if(hCtx->vbvInfo.nVbvDataSize <= (H264_EXTENDED_DATA_LEN+4))
        {
            if(H264RequestBitstreamData(hCtx) == VDECODE_RESULT_NO_BITSTREAM)
            {   
                return VDECODE_RESULT_NO_BITSTREAM;
            }
        }

        if(hCtx->bCanResetHw == 1)
        {   
            ResetVeInternal(h264DecCtx->pVideoEngine);
            H264InitFuncCtrlRegister(h264DecCtx);
        }

        if((hCtx->bDecExtraDataFlag==1) || (hCtx->bIsAvc==0))
        {
            hCtx->nDecStep = H264_STEP_SEARCH_NALU;
        }
        else
        {
            hCtx->nDecStep = H264_STEP_SEARCH_AVC_NALU;
        }
    }
	
	//**************************************************************//
	//**************step 3.1*****************************************//
    while(hCtx->nDecStep == H264_STEP_SEARCH_NALU)
    {
        pOldReadPtr = hCtx->vbvInfo.pReadPtr;
        nOldVbvDataSize = hCtx->vbvInfo.nVbvDataSize;
        H264SearchStartcode(h264DecCtx, hCtx);
        AdapterVeWaitInterrupt();
        
        H264CheckBsDmaBusy(h264DecCtx);
        ve_status_reg = H264VeIsr(h264DecCtx);
        
        H264UpdateDataPointer(h264DecCtx, hCtx, H264_GET_STCD_OFFSET);

        if(pOldReadPtr+nOldVbvDataSize >= hCtx->vbvInfo.pVbvBufEnd)
        {
        	pNewReadPtr = pOldReadPtr+nOldVbvDataSize-hCtx->nVbvSize;
        }
        else
        {
        	pNewReadPtr = pOldReadPtr+nOldVbvDataSize;
        }

        if(hCtx->vbvInfo.pReadPtr >= pNewReadPtr)
        {
        	if(!((pOldReadPtr<=hCtx->vbvInfo.pReadPtr)&&(pNewReadPtr<pOldReadPtr)))
        	{
            	hCtx->vbvInfo.nVbvDataSize = 0;
                goto h264_judge_vbv_data;
        	}
        }
        bitOffset1 = (hCtx->vbvInfo.pReadPtr-hCtx->vbvInfo.pVbvBuf)*8;
        sliceDataBits = hCtx->vbvInfo.nVbvDataSize*8;
        nal_unit_type = hCtx->vbvInfo.pReadPtr[0] & 0x1f;
        if(hCtx->vbvInfo.nVbvDataSize > (H264_EXTENDED_DATA_LEN+6))
        {
            H264SearchStartcode(h264DecCtx, hCtx);
            AdapterVeWaitInterrupt();
            H264CheckBsDmaBusy(h264DecCtx);
            ve_status_reg = H264VeIsr(h264DecCtx);

            if(ve_status_reg &H264_IR_FINISH)
            {
                uint32_t bitOffset2 = H264GetbitOffset(h264DecCtx, H264_GET_STCD_OFFSET);
                sliceDataBits = (bitOffset2>=bitOffset1)? (bitOffset2-bitOffset1):(bitOffset1-bitOffset2);
			
				
                if((nal_unit_type==NAL_PPS) &&(hCtx->bDecExtraDataFlag==0))
                {
                	if(H264CheckNextStartCode(hCtx, bitOffset2, pNewReadPtr) == 0)
                	{
                		if(SbmStreamFrameNum(hCtx->vbvInfo.vbv) == 1)
                    	{
                			hCtx->vbvInfo.pReadPtr = pOldReadPtr;
                			hCtx->vbvInfo.nVbvDataSize = nOldVbvDataSize;
                			return VDECODE_RESULT_NO_BITSTREAM;
                    	}
                	}
                }
            }
        }
        else if((nal_unit_type==NAL_PPS)&&(hCtx->bDecExtraDataFlag==0))
        {
        	if(SbmStreamFrameNum(hCtx->vbvInfo.vbv) == 1)
        	{
        		hCtx->vbvInfo.pReadPtr = pOldReadPtr;
        		hCtx->vbvInfo.nVbvDataSize = nOldVbvDataSize;
                return VDECODE_RESULT_NO_BITSTREAM;
        	}
        }


        H264ConfigureEptbDetect(h264DecCtx, hCtx, sliceDataBits,1);

        nal_unit_type = H264GetBits(h264DecCtx, 8);
        hCtx->nNalRefIdc = (nal_unit_type & 0x60) ? 1 : 0;
        nal_unit_type &= 0x1f;
        
        //logv("nal_unit_type=%d\n", nal_unit_type);
        
        ret = H264ProcessNaluUnit(h264DecCtx, hCtx, nal_unit_type, sliceDataBits, h264Dec->nDecStreamIndex);
        
        if((ret==VDECODE_RESULT_KEYFRAME_DECODED) || (ret==VDECODE_RESULT_FRAME_DECODED))
        {
            H264UpdateDataPointer(h264DecCtx, hCtx, H264_GET_VLD_OFFSET);
            if(hCtx->vbvInfo.nVbvDataSize <= (H264_EXTENDED_DATA_LEN+4))
            {
                SbmFlushStream(hCtx->vbvInfo.vbv, hCtx->vbvInfo.pVbvStreamData);
            }
            hCtx->nDecStep = H264_STEP_PROCESS_DECODE_RESULT;
            break;
        }
        else if(ret == VRESULT_ERR_FAIL)
        {
        	continue;
        }
        else if(ret!= VDECODE_RESULT_OK)
        {
            if(ret == VRESULT_DEC_FRAME_ERROR)
            {
                hCtx->nDecStep = H264_STEP_PROCESS_DECODE_RESULT;
                ret = VDECODE_RESULT_FRAME_DECODED;
                hCtx->nDecFrameStatus = H264_END_DEC_FRAME;
                hCtx->vbvInfo.pReadPtr -= 4;
                if(hCtx->vbvInfo.pReadPtr < hCtx->vbvInfo.pVbvBuf)
                {
                    hCtx->vbvInfo.pReadPtr += hCtx->vbvInfo.nVbvDataSize;
                } 
                hCtx->vbvInfo.nVbvDataSize += 4;
                break;
            }  
            else if(ret==VDECODE_RESULT_NO_FRAME_BUFFER)
            {
                hCtx->vbvInfo.pReadPtr -= 4;
                if(hCtx->vbvInfo.pReadPtr < hCtx->vbvInfo.pVbvBuf)
                {
                    hCtx->vbvInfo.pReadPtr += hCtx->vbvInfo.nVbvDataSize;
                }
                hCtx->vbvInfo.nVbvDataSize += 4;
                hCtx->nDecStep = H264_STEP_UPDATE_DATA;
            }
            return ret;
        }

        H264UpdateDataPointer(h264DecCtx, hCtx, H264_GET_VLD_OFFSET);
        h264_judge_vbv_data:
        if(hCtx->vbvInfo.nVbvDataSize <= (H264_EXTENDED_DATA_LEN+4))
        {
            if(hCtx->bDecExtraDataFlag == 1)
            {
                hCtx->bDecExtraDataFlag  = 0;
                hCtx->nDecStep = H264_STEP_CONFIG_VBV;
                return VDECODE_RESULT_OK;
            }
            SbmFlushStream(hCtx->vbvInfo.vbv, hCtx->vbvInfo.pVbvStreamData);
            ret = H264RequestBitstreamData(hCtx);
            
            if(ret == VDECODE_RESULT_NO_BITSTREAM)
            {
                hCtx->nDecStep = H264_STEP_UPDATE_DATA;
                hCtx->bCanResetHw = 0;
                return VDECODE_RESULT_NO_BITSTREAM;
            }
            continue;
        }
    }// end while(hCtx->nDecStep = H264_STEP_SEARCH_NALU)

	//**************************************************************//
	//**************step 3.2*****************************************//
    while(hCtx->nDecStep == H264_STEP_SEARCH_AVC_NALU)
    {  
    	sliceLen = 0;
        for(i=0; i<hCtx->nNalLengthSize; i++)
        {
            sliceLen <<= 8;
            sliceLen += hCtx->vbvInfo.pReadPtr[0];

            hCtx->vbvInfo.pReadPtr++;
            if(hCtx->vbvInfo.pReadPtr > hCtx->vbvInfo.pVbvBufEnd)
            {
            	hCtx->vbvInfo.pReadPtr = hCtx->vbvInfo.pVbvBuf;
            }
        }
		
        hCtx->vbvInfo.nVbvDataSize -= hCtx->nNalLengthSize;
        if((sliceLen>(uint32_t)hCtx->vbvInfo.nVbvDataSize)||(sliceLen==0))
        {
        	sliceLen = hCtx->vbvInfo.nVbvDataSize;
        	goto h264_judge_vbv_data_2;
        }
        nal_unit_type = hCtx->vbvInfo.pReadPtr[0];

        hCtx->nNalRefIdc = (nal_unit_type & 0x60) ? 1 : 0;
		
        nal_unit_type &= 0x1f;
        if(nal_unit_type == 6)
        {
            ResetVeInternal(h264DecCtx->pVideoEngine);
        }
        H264ConfigureAvcRegister(h264DecCtx, hCtx, 1, sliceLen*8);
        H264GetBits(h264DecCtx, 8);
        ret = H264ProcessNaluUnit(h264DecCtx, hCtx, nal_unit_type, sliceLen*8,  h264Dec->nDecStreamIndex);
		
        if((ret==VDECODE_RESULT_KEYFRAME_DECODED) || (ret==VDECODE_RESULT_FRAME_DECODED))
        {   
            hCtx->vbvInfo.pReadPtr += sliceLen;
            hCtx->vbvInfo.nVbvDataSize -= sliceLen;
            if(hCtx->vbvInfo.nVbvDataSize <= H264_EXTENDED_DATA_LEN)
            {
                SbmFlushStream( hCtx->vbvInfo.vbv, hCtx->vbvInfo.pVbvStreamData);
            }
            if(hCtx->vbvInfo.pReadPtr > hCtx->vbvInfo.pVbvBufEnd)
            {
            	hCtx->vbvInfo.pReadPtr -= (hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pVbvBuf+1);
            }
            hCtx->nDecStep = H264_STEP_PROCESS_DECODE_RESULT;
            break;
        }
        else if((ret!= VDECODE_RESULT_OK)&&(ret!= VRESULT_ERR_FAIL))
        {   
        	if(ret == VRESULT_DEC_FRAME_ERROR)
        	 {
        		hCtx->nDecStep = H264_STEP_PROCESS_DECODE_RESULT;
        	    ret = VDECODE_RESULT_FRAME_DECODED;
        	    hCtx->nDecFrameStatus = H264_END_DEC_FRAME;
        	    hCtx->vbvInfo.pReadPtr -= hCtx->nNalLengthSize;
        	    if(hCtx->vbvInfo.pReadPtr < hCtx->vbvInfo.pVbvBuf)
        	    {
        	    	hCtx->vbvInfo.pReadPtr += hCtx->vbvInfo.nVbvDataSize;
        	    }
        	    hCtx->vbvInfo.nVbvDataSize += hCtx->nNalLengthSize;
        	    break;
        	 }
        	else if(ret == VDECODE_RESULT_NO_FRAME_BUFFER)
        	{
        		hCtx->vbvInfo.pReadPtr -=  hCtx->nNalLengthSize;
        	    if(hCtx->vbvInfo.pReadPtr < hCtx->vbvInfo.pVbvBuf)
        	    {
        	    	hCtx->vbvInfo.pReadPtr += hCtx->vbvInfo.nVbvDataSize;
        	    }
        	    hCtx->vbvInfo.nVbvDataSize +=  hCtx->nNalLengthSize;
        	    hCtx->nDecStep = H264_STEP_UPDATE_DATA;
        	}
        	return ret;
        }
h264_judge_vbv_data_2:
        hCtx->vbvInfo.pReadPtr += sliceLen;
        hCtx->vbvInfo.nVbvDataSize -= sliceLen;
        if(hCtx->vbvInfo.pReadPtr > hCtx->vbvInfo.pVbvBufEnd)
        {
        	hCtx->vbvInfo.pReadPtr -= (hCtx->vbvInfo.pVbvBufEnd - hCtx->vbvInfo.pVbvBuf+1);
        }
        if(hCtx->vbvInfo.nVbvDataSize <= H264_EXTENDED_DATA_LEN)
        {
            SbmFlushStream(hCtx->vbvInfo.vbv, hCtx->vbvInfo.pVbvStreamData);
            ret = H264RequestBitstreamData(hCtx);
            
            if(ret == VDECODE_RESULT_NO_BITSTREAM)
            {   
                hCtx->nDecStep = H264_STEP_UPDATE_DATA;
                hCtx->bCanResetHw  = 0;
                return VDECODE_RESULT_NO_BITSTREAM;
            }
            continue;
        }
    }
 	//**************************************************************//
	//**************step 4*****************************************//
    if(hCtx->nDecStep == H264_STEP_PROCESS_DECODE_RESULT)
    {   
        if((ret==VDECODE_RESULT_KEYFRAME_DECODED) || (ret==VDECODE_RESULT_FRAME_DECODED))
        {   
            hCtx->bCanResetHw  = 1;
            
            hCtx->nPrevFrmNumOffset = hCtx->nFrmNumOffset;
            hCtx->nPrevFrmNum = hCtx->nFrmNum;
            hCtx->nPrevPocMsb = hCtx->nPocMsb;
            hCtx->nPrevPocLsb = hCtx->nPocLsb;
            hCtx->frmBufInf.pCurPicturePtr->nPictType = hCtx->nSliceType;

            if(hCtx->nNalRefIdc != 0)
            {
                hCtx->nPrevPocMsb = hCtx->nPocMsb;
                hCtx->nPrevPocLsb = hCtx->nPocLsb;
                H264ExecuteRefPicMarking(hCtx, hCtx->mmco, hCtx->nMmcoIndex);
            }

            if(hCtx->bFstField == 1)
            {
                // waiting fot the second field
                ret = VDECODE_RESULT_OK;
                hCtx->bNeedCheckFbmNum = 0;
                hCtx->bCanResetHw  = 0;
            }
            else
            {   
                if(hCtx->frmBufInf.pCurPicturePtr!= NULL)
                {
                	hCtx->frmBufInf.pCurPicturePtr->nDecFrameOrder = hCtx->nCurFrmNum;

                	hCtx->nCurFrmNum++;
                    H264CongigureDisplayParameters(h264DecCtx, hCtx);
                    H264SortDisplayFrameOrder(h264DecCtx, hCtx, bDecodeKeyFrameOnly);
                }
                hCtx->bNeedCheckFbmNum = 1;
                //decode the second frame
            }
            hCtx->nDecStep = H264_STEP_UPDATE_DATA;
        }
    }
    return ret;
}
