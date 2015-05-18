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

#include "vdecoder.h"
#include "videoengine.h"
#include "log.h"

static int  Mpeg2DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo);
void Mpeg2DecoderReset(DecoderInterface* pSelf);
static int  Mpeg2DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex);
static int  Mpeg2DecoderGetFbmNum(DecoderInterface* pSelf);
static Fbm* Mpeg2DecoderGetFbm(DecoderInterface* pSelf, int nIndex);
static int  Mpeg2DecoderDecode(DecoderInterface* pSelf, 
                            int               bEndOfStream, 
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay, 
                            int64_t           nCurrentTimeUs);
static void Destroy(DecoderInterface* pSelf);
extern void Mpeg2ResetTopRegister(void);


DecoderInterface* CreateMpeg2Decoder(VideoEngine* p)
{
    Mpeg2DecodeContext* pMpeg2Context;
    
    pMpeg2Context = (Mpeg2DecodeContext*) malloc(sizeof(Mpeg2DecodeContext));
    if(pMpeg2Context == NULL)
        return NULL;
    
    memset(pMpeg2Context, 0, sizeof(Mpeg2DecodeContext));
    
    pMpeg2Context->pVideoEngine        = p;
    pMpeg2Context->interface.Init      = Mpeg2DecoderInit;
    pMpeg2Context->interface.Reset     = Mpeg2DecoderReset;
    pMpeg2Context->interface.SetSbm    = Mpeg2DecoderSetSbm;
    pMpeg2Context->interface.GetFbmNum = Mpeg2DecoderGetFbmNum;
    pMpeg2Context->interface.GetFbm    = Mpeg2DecoderGetFbm;
    pMpeg2Context->interface.Decode    = Mpeg2DecoderDecode;
    pMpeg2Context->interface.Destroy   = Destroy;
    
    return &pMpeg2Context->interface;
}

void CedarPluginVDInit(void)
{
    int ret;
    ret = VDecoderRegister(VIDEO_CODEC_FORMAT_MPEG2, "mpeg2", CreateMpeg2Decoder);

    if (0 == ret)
    {
        logi("register mpeg2 decoder success!");
    }
    else
    {
        loge("register mpeg2 decoder failure!!!");
    }
    return ;
}

//************************************************************************************/
//      Name:	    Mpeg2DecoderInit                                                          //
//      Prototype:	Handle ve_open (VConfig* config, VideoStreamInfo* stream_info); //
//      Function:	Start up the VE CSP.                                             //
//      Return:	A handle of the VE device.                                           //
//      Input:	VConfig* config, the configuration for the VE CSP.                 //
//************************************************************************************/

static int  Mpeg2DecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo)
{
    Mpeg2DecodeContext* pMpeg2Context   = NULL;
    Mpeg2DecodeInfo* 	pMpeg2DecHandle = NULL;

    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;
    memcpy(&pMpeg2Context->vconfig, pConfig, sizeof(VConfig));

    pMpeg2Context->videoStreamInfo.eCodecFormat            = pVideoInfo->eCodecFormat;
    pMpeg2Context->videoStreamInfo.nWidth                  = pVideoInfo->nWidth;
    pMpeg2Context->videoStreamInfo.nHeight                 = pVideoInfo->nHeight;
    pMpeg2Context->videoStreamInfo.nFrameRate              = pVideoInfo->nFrameRate;
    pMpeg2Context->videoStreamInfo.nFrameDuration          = pVideoInfo->nFrameDuration;
    pMpeg2Context->videoStreamInfo.nAspectRatio            = pVideoInfo->nAspectRatio;
    pMpeg2Context->videoStreamInfo.nCodecSpecificDataLen   = pVideoInfo->nCodecSpecificDataLen;
    pMpeg2Context->videoStreamInfo.pCodecSpecificData      = pVideoInfo->pCodecSpecificData;
    
    pMpeg2DecHandle = (Mpeg2DecodeInfo*)malloc(sizeof(Mpeg2DecodeInfo));
    if(pMpeg2DecHandle == NULL)
    {
        free(pMpeg2Context);
        loge("mpeg2_open, malloc memory fail.");
        return VDECODE_RESULT_UNSUPPORTED;
    }
    memset(pMpeg2DecHandle, 0, sizeof(Mpeg2DecodeInfo));

    pMpeg2Context->pMpeg2Dec = (void*)pMpeg2DecHandle;
    Mpeg2InitDecode(pMpeg2Context->pMpeg2Dec);
    ResetVeInternal(pMpeg2Context->pVideoEngine);

    
    if(pMpeg2Context->videoStreamInfo.pCodecSpecificData!= NULL)
    {
        if((pMpeg2Context->videoStreamInfo.pCodecSpecificData[0]==0x00) &&(pMpeg2Context->videoStreamInfo.pCodecSpecificData[1]==0x00)
                     &&(pMpeg2Context->videoStreamInfo.pCodecSpecificData[2]==0x01)&&(pMpeg2Context->videoStreamInfo.pCodecSpecificData[3]==0xB3))
        {
        	pMpeg2DecHandle->picInfo.bIsMpeg1Flag = (pMpeg2Context->videoStreamInfo.eCodecFormat==VIDEO_CODEC_FORMAT_MPEG1);
            Mpeg2ParseSequenceInfo(pMpeg2Context, pMpeg2DecHandle, (uint8_t*)pMpeg2Context->videoStreamInfo.pCodecSpecificData, MP2VDEC_PARSE_SEQ_MODE2);
        }
    }
    
    return VDECODE_RESULT_OK;
}



/**********************************************************************   *******************/
//          Name:	ve_reset                                                                //
//          Prototype: vresult_e ve_reset (uint8_t flush_pictures)                               //
//          Function:  Reset the VE CSP.                                                    //
//          Return:    VDECODE_RESULT_OK: success.                                                 //
//	                   VDECODE_RESULT_UNSUPPORTED: the CSP is not opened yet.             //
/********************************************************************************************/
 
void Mpeg2DecoderReset(DecoderInterface* pSelf)
{   
    Mpeg2DecodeInfo* pMpeg2Dec = NULL;
    Mpeg2DecodeContext* pMpeg2Context = NULL;
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;
    
    if(pMpeg2Context == NULL)
    {
        return;
    }
    else
    {   
        pMpeg2Dec = (Mpeg2DecodeInfo*)pMpeg2Context->pMpeg2Dec;
        Mpeg2FlushPictures(pMpeg2Dec, pMpeg2Context->pFbm, 0);
        Mpeg2DecodeSetSbmBuf(pMpeg2Context->pStreamBufferBase, pMpeg2Context->nStreamBufferSize, pMpeg2Context->pMpeg2Dec);
        ResetVeInternal(pMpeg2Context->pVideoEngine);
        Mpeg2ResetDecodeParams(pMpeg2Dec);
        return;        
    }
}

   
/*********************************************************************************************************/
//             Name:	ve_set_vbv                                                                       //
//             Prototype: vresult_e ve_set_vbv (uint8_t* vbv_buf, uint32_t nStreamBufferSize, Handle ve);                   //
//             Function: Set pSbm's bitstream buffer base address and buffer size to the CSP..            //
//             Return:   VDECODE_RESULT_OK: success.                                                            //
//	                     VDECODE_RESULT_UNSUPPORTED: the CSP is not opened yet.                        //
/*********************************************************************************************************/

static int  Mpeg2DecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex)
{   
    Mpeg2DecodeInfo* pMpeg2DecHandle = NULL;
    Mpeg2DecodeContext* pMpeg2Context = NULL;
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;

    if(nIndex != 0)
    {
        loge("multiple video stream unsupported.");
        return VDECODE_RESULT_UNSUPPORTED;
    }
    
    if(pMpeg2Context == NULL)
    {
        return VDECODE_RESULT_UNSUPPORTED;
    }
    else
    {   
        pMpeg2DecHandle = (Mpeg2DecodeInfo*)pMpeg2Context->pMpeg2Dec;
        pMpeg2Context->pSbm    = pSbm;
        pMpeg2Context->pStreamBufferBase = (uint8_t*)SbmBufferAddress(pSbm);
        pMpeg2Context->nStreamBufferSize =      SbmBufferSize(pSbm);
        
        pMpeg2DecHandle->sbmInfo.pSbm         = pSbm;
        pMpeg2DecHandle->sbmInfo.pReadPtr      = pMpeg2Context->pStreamBufferBase;
        pMpeg2DecHandle->sbmInfo.pSbmBuf      = pMpeg2Context->pStreamBufferBase;
        pMpeg2DecHandle->sbmInfo.nSbmBufSize  = pMpeg2Context->nStreamBufferSize;
        pMpeg2DecHandle->sbmInfo.nSbmDataSize = 0;
        pMpeg2DecHandle->sbmInfo.pSbmBufEnd   = pMpeg2Context->pStreamBufferBase+pMpeg2Context->nStreamBufferSize-1;
        return VDECODE_RESULT_OK;
    }
}

static int  Mpeg2DecoderGetFbmNum(DecoderInterface* pSelf)
{
    Mpeg2DecodeContext* pMpeg2Context;
    
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;
    
    if(pMpeg2Context->pFbm != NULL)
        return 1;
    else
        return 0;
}

/****************************************************************************************************/
//             Name:	Mpeg2DecoderGetFbm                                                                  //
//             Prototype: Handle get_fbm (void);                                                    //
//             Function: Get a handle of the pFbm instance, in which pictures for display are stored.//
//             Return:   Not NULL Handle: handle of the pFbm instance.                               //
//                       NULL Handle: pFbm module is not initialized yet.                            //            
/****************************************************************************************************/

static Fbm* Mpeg2DecoderGetFbm(DecoderInterface* pSelf, int nIndex)
{   
    Mpeg2DecodeInfo*    pMpeg2Dec 	  = NULL;
    Mpeg2DecodeContext* pMpeg2Context = NULL;
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;

    if(nIndex != 0)
        return NULL;
    
    if(pMpeg2Context == NULL)
    {
        return NULL;
    }
    else
    {   
        pMpeg2Dec = (Mpeg2DecodeInfo*)pMpeg2Context->pMpeg2Dec;
        if(pMpeg2Dec == NULL)
        {
            return NULL;
        }
        else if(pMpeg2Dec->picInfo.bMallocFrmBufFlag == 0)
        {
            return NULL;
        }
        else
        {
        	return pMpeg2Context->pFbm;
        }
    }
}


/**********************************************************************************************************/
//   Name:	ve_decode                                                                                      //
//   Prototype: vresult_e decode (vstream_data_t* stream, uint8_t keyframe_only, uint8_t bSkipBFrameIfDelay, uint32_t nCurrentTimeUs); //
//   Function: Decode one bitstream frame.                                                                 //
//   INput:    vstream_data_t* stream: start address of the stream frame.                                  //
//             uint8_t keyframe_only:   tell the CSP to decode key frame only;                                  //
//             uint8_t bSkipBFrameIfDelay:  tell the CSP to skip B frame if it is overtime;                            //
//             uint32_t nCurrentTimeUs:current time, used to compare with PTS when decoding B frame;                  //
//   Return:   VDECODE_RESULT_OK:             decode stream success but no frame decoded;                         //
//             VDECODE_RESULT_FRAME_DECODED:  one common frame decoded;                                           //
//             VRESULT_KEYFRAME_DECODED:    one key frame decoded;                                         //
//             VDECODE_RESULT_UNSUPPORTED:           decode stream fail;                                             //
//             VRESULT_ERR_INVALID_PARAM:  either stream or ve is NULL;                                    //
//             VRESULT_ERR_INVALID_STREAM: some error data in the stream, decode fail;                     //
//             VRESULT_ERR_NO_MEMORY:      allocate memory fail in this method;                            //
//             VRESULT_ERR_NO_FRAMEBUFFER: request empty frame buffer fail in this method;                 //
//             VRESULT_ERR_UNSUPPORTED:    stream format is unsupported by this version of VE CSP;         //    
//             VDECODE_RESULT_UNSUPPORTED:  'open' has not been successfully called yet.                 //
/***********************************************************************************************************/

int32_t Mpeg2JudgeNextStartCode(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bEndOfStream)
{
    uint8_t* pReadPtr = NULL;
    uint8_t* pWritePtr = NULL;
    uint32_t nByteLen = 0;
    uint32_t nByteOffset = 0;
    uint32_t uInrCode = 0;
    uint32_t nHwOff = 0;
    uint8_t* pHwPtr = NULL;
    uint32_t startCode = 0;
    uint32_t nRemainDataLen = 0;

    if(bEndOfStream ==1)
    {
    	return VDECODE_RESULT_OK;
    }

    pReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;

    while(1)
    {
    	pWritePtr = (uint8_t*)SbmBufferWritePointer(pMpeg2Dec->sbmInfo.pSbm);
    	if(pWritePtr >= pReadPtr)
    	{
    		nByteLen = pWritePtr-pReadPtr;
    	}
    	else
    	{
    		nByteLen =	pWritePtr+ pMpeg2Dec->sbmInfo.nSbmBufSize-pReadPtr;
    	}
        if(nByteLen <= 64)
        {
            return VDECODE_RESULT_NO_BITSTREAM;
        }

    	nByteOffset =  pReadPtr - pMpeg2Dec->sbmInfo.pSbmBuf;
    	Mpeg2SetHwStartCodeInfo(pMpeg2Dec);
    	Mpeg2SetSbmRegister(pMpeg2Dec, 1, 0, (nByteOffset<<3), (nByteLen<<3), 1, 1);
    	AdapterVeWaitInterrupt();
        uInrCode = Mpeg2VeIsr(pMpeg2Dec);
            
        if(uInrCode != 1)
        {
    	   return VDECODE_RESULT_NO_BITSTREAM;
        }
        nHwOff = Mpeg2GetStartcodeBitOffet(pMpeg2Dec);
        if(nHwOff & 7)
        	nHwOff  = (nHwOff+7)&0xfffffff8;
        nHwOff >>= 3;
        while(nHwOff >= pMpeg2Dec->sbmInfo.nSbmBufSize)
        {
        	nHwOff -= pMpeg2Dec->sbmInfo.nSbmBufSize;
        }
        pHwPtr =(uint8_t*)(pMpeg2Dec->sbmInfo.pSbmBuf + nHwOff);
        if(pHwPtr >= pReadPtr)
        {
        	nHwOff = pHwPtr - pReadPtr;
        }
        else
        {
        	nHwOff = 0;
        	if((pReadPtr+nByteLen) > pMpeg2Dec->sbmInfo.pSbmBufEnd)
        	{
        		nHwOff = pHwPtr+nByteLen-pReadPtr;
        	}
        }

        if((pHwPtr+8) <= pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
        	pHwPtr -= 8;
        	nHwOff -= 8;
        }
        else
        {
        	nHwOff -= (pHwPtr-pMpeg2Dec->sbmInfo.pSbmBuf);
        	pHwPtr = pMpeg2Dec->sbmInfo.pSbmBuf;
        }
        nByteLen -= nHwOff;
        pReadPtr = pHwPtr;
        if(pReadPtr+ nByteLen <=  pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
        	 Mpeg2SearchStartcode(pMpeg2Dec, pReadPtr,nByteLen,&startCode, 0);
        	 if((startCode=0x00000100) || (startCode=0x000001B3)||(startCode=0x000001B8))
        	 {
        		 return VDECODE_RESULT_OK;
        	 }
        }
        else
        {
        	nRemainDataLen = pMpeg2Dec->sbmInfo.pSbmBufEnd-pReadPtr+1;
            startCode = 0;
        	Mpeg2SearchStartcode(pMpeg2Dec, pReadPtr,nRemainDataLen,&startCode, 0);
        	if((startCode=0x00000100) || (startCode=0x000001B3)||(startCode=0x000001B8))
        	{
        		return VDECODE_RESULT_OK;
        	}
        	Mpeg2SearchStartcode(pMpeg2Dec, pMpeg2Dec->sbmInfo.pSbmBuf,nByteLen-nRemainDataLen,&startCode, 0);
        	if((startCode=0x00000100) || (startCode=0x000001B3)||(startCode=0x000001B8))
        	{
        		return VDECODE_RESULT_OK;
        	}
        }
        pReadPtr += 8;
        if(pReadPtr >  pMpeg2Dec->sbmInfo.pSbmBufEnd)
        {
        	pReadPtr = pMpeg2Dec->sbmInfo.pSbmBuf;
        }
    }
}

static int  Mpeg2DecoderDecode(DecoderInterface* pSelf, 
                            int               bEndOfStream, 
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay, 
                            int64_t           nCurrentTimeUs)
{   
    Mpeg2DecodeContext* pMpeg2Context = NULL;
	Mpeg2DecodeInfo* 	pMpeg2Dec 	  = NULL;
    int8_t  eRet 		= 0;
    int8_t  bSkipFlag1  = 0;
    int8_t  bSkipFlag2  = 0;
    int8_t  bSkipFlag3  = 0;
    uint32_t uInrCode    = 0;
    uint32_t nByteOffset = 0;
    uint32_t nByteRemain = 0;
    uint32_t nByteLen    = 0;
    uint8_t  bPicEndFlag = 0;
    int32_t bEndOfStreamFlag = 0;
	
	bEndOfStreamFlag = bEndOfStream;
   
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;
    pMpeg2Dec = (Mpeg2DecodeInfo*)pMpeg2Context->pMpeg2Dec;
    pMpeg2Dec->bEndOfStream = bEndOfStream;
    
    //************step1: parser pic header informaion
    if(pMpeg2Dec->nDecStep == MP2VDEC_PARSE_HEADER)
    {   
        if(pMpeg2Dec->bSearchFstStartCodeFlag == 1)
        {
            pMpeg2Dec->bDecPicOkFlag = 0;
            pMpeg2Dec->nDecStep = MP2VDEC_CAL_RESULT;    
            pMpeg2Dec->bSearchStdFlag = 1; 
            pMpeg2Dec->bFstSetVbvFlag = 1;
            pMpeg2Dec->bSearchFstStartCodeFlag = 0;
            pMpeg2Dec->sbmInfo.pHwReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;
            pMpeg2Dec->sbmInfo.nSbmHwDataSize = pMpeg2Dec->sbmInfo.nSbmDataSize;
        }
        else
        {
            pMpeg2Dec->bDecPicOkFlag = 0;
            pMpeg2Dec->picInfo.bOnlyDispKeyFrmFlag = bDecodeKeyFrameOnly;
            eRet = Mpeg2ParsePictureHeader(pMpeg2Context, pMpeg2Dec);
            if(eRet != VDECODE_RESULT_OK)
            {  
                return eRet;
            }
            pMpeg2Dec->nDecStep = MP2VDEC_CHECK_HWENGINE_FREE;
            
            bSkipFlag1 = (pMpeg2Dec->picInfo.bDropFrmFlag == 1);
            bSkipFlag2 = (bDecodeKeyFrameOnly==1)&&(pMpeg2Dec->picInfo.bFstFieldFlag==1)&&(pMpeg2Dec->picInfo.eCurPicType!=MP2VDEC_I_PIC);
            bSkipFlag3 = ((bSkipBFrameIfDelay==1)&&(pMpeg2Dec->picInfo.bFstFieldFlag==1)&&(pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_B_PIC)
                        &&(nCurrentTimeUs>=pMpeg2Dec->picInfo.nCurPicPts));
              
            if((bSkipFlag1==1) || (bSkipFlag2==1) || (bSkipFlag3==1))
            {   
                loge("drop the frame.\n");
                pMpeg2Dec->nDecStep = MP2VDEC_CAL_RESULT;  
                pMpeg2Dec->picInfo.bDropFrmFlag = 0;   
                pMpeg2Dec->bSearchStdFlag = 1; 
                pMpeg2Dec->bFstSetVbvFlag = 1;
                pMpeg2Dec->sbmInfo.pHwReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;
                pMpeg2Dec->sbmInfo.nSbmHwDataSize = pMpeg2Dec->sbmInfo.nSbmDataSize;
                if(pMpeg2Dec->picInfo.eCurPicStructure!= MP2VDEC_FRAME)
                {
                    pMpeg2Dec->bFieldSeachNextPicFlag = 1;
                }
            }
        }
    }
    
    //*** step2: check whether the hardware is free
    if(pMpeg2Dec->nDecStep == MP2VDEC_CHECK_HWENGINE_FREE)
    {   
        pMpeg2Dec->nDecStep = MP2VDEC_CHECK_EMPTY_FRAME_BUFFER;
        if(Mpeg2CheckVeIdle(pMpeg2Dec) < 0)
        {
            loge("reset hw.\n");
            ResetVeInternal(pMpeg2Context->pVideoEngine);   
        }
    }
    
    //******step3: get the empty frame buffer
    if(pMpeg2Dec->nDecStep == MP2VDEC_CHECK_EMPTY_FRAME_BUFFER)
    {
        if(Mpeg2GetEmptyFrameBuf(pMpeg2Dec,bDecodeKeyFrameOnly, pMpeg2Context->pFbm)!= VDECODE_RESULT_OK)
        {   
            return VDECODE_RESULT_NO_FRAME_BUFFER;
        }
        pMpeg2Dec->bFstSetVbvFlag = 1;
        pMpeg2Dec->nDecStep = MP2VDEC_SET_VBV_CONFIG;
        pMpeg2Dec->sbmInfo.pRecordPtr = pMpeg2Dec->sbmInfo.pReadPtr;
    }
    

    //***step4: first set the pSbm configure
mpeg2_set_dec_register:
    if(pMpeg2Dec->nDecStep == MP2VDEC_SET_VBV_CONFIG)
    {   
        bPicEndFlag = 1;

        if(pMpeg2Dec->bFstSetVbvFlag == 1)
        {
            if(Mpeg2JudgeNextStartCode(pMpeg2Dec, bEndOfStream)==VDECODE_RESULT_NO_BITSTREAM)
            {   
        	    return VDECODE_RESULT_NO_BITSTREAM;
            }
        }
        if((pMpeg2Dec->bFrameDataComplete==0) &&(Mpeg2JudgePictureEnd(pMpeg2Dec)==VDECODE_RESULT_UNSUPPORTED))
        {
             bPicEndFlag = 0;
             if(pMpeg2Dec->sbmInfo.nSbmDataSize < 64)
             {
                if(Mpeg2RequestBitstreamData(pMpeg2Context, pMpeg2Dec) != VDECODE_RESULT_OK)
                {  
                    return VDECODE_RESULT_NO_BITSTREAM;
                }
             }
        }

        nByteOffset = pMpeg2Dec->sbmInfo.pReadPtr - pMpeg2Dec->sbmInfo.pSbmBuf;
        nByteRemain = (((uint32_t)(pMpeg2Dec->sbmInfo.pReadPtr+pMpeg2Dec->sbmInfo.nSbmDataSize))&0x03); 
        if(bPicEndFlag == 1)
        {
        	nByteLen = pMpeg2Dec->sbmInfo.nSbmDataSize;
        }
        else
        {
           nByteLen    = pMpeg2Dec->sbmInfo.nSbmDataSize-nByteRemain-32;
        }           
       
        if(pMpeg2Dec->bFstSetVbvFlag == 1)        // first set the pSbm configure
        {   
            //set_ve_toplevel_reg(pMpeg2Context->pVideoEngine);
        	ResetVeInternal(pMpeg2Context->pVideoEngine);
            Mpeg2SetQuantMatrix(pMpeg2Dec,1);
            Mpeg2SetQuantMatrix(pMpeg2Dec,0);
            Mpeg2SetPictureSize(pMpeg2Dec);
            Mpeg2SetPictureInfo(pMpeg2Dec);
            Mpeg2SetReconstructBuf(pMpeg2Dec);
            Mpeg2SetSbmRegister(pMpeg2Dec, 1, 1, (nByteOffset<<3), (nByteLen<<3), (bPicEndFlag||pMpeg2Dec->bDecEndFlag), 0);
        }
        else
        {
            Mpeg2SetSbmRegister(pMpeg2Dec, 0, 0, (nByteOffset<<3), (nByteLen<<3), (bPicEndFlag||pMpeg2Dec->bDecEndFlag), 0);
        }
        
        pMpeg2Dec->nDecStep = MP2VDEC_FINISH_CHECK;
    }
    
    //********step5:check the ve finish

    if(pMpeg2Dec->nDecStep == MP2VDEC_FINISH_CHECK)
    {   
        //IVE.ve_wait_intr();
        AdapterVeWaitInterrupt();
        uInrCode = Mpeg2VeIsr(pMpeg2Dec);
        
        if(uInrCode == 0)
        {   
            pMpeg2Dec->nDecStep = MP2VDEC_CAL_RESULT;
            pMpeg2Dec->bResetHwFlag = 1;   
            pMpeg2Dec->bSearchStdFlag = 1;
            pMpeg2Dec->bFstSetVbvFlag = 1;
            pMpeg2Dec->sbmInfo.pHwReadPtr =  pMpeg2Dec->sbmInfo.pReadPtr;
            pMpeg2Dec->sbmInfo.nSbmHwDataSize =  pMpeg2Dec->sbmInfo.nSbmDataSize;
        }

        if((uInrCode&2) && pMpeg2Dec->picInfo.bOnlyDispKeyFrmFlag==1)
        {
        	if(uInrCode&4)
        	{
        		Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nByteLen);
        	}
        	else
        	{
        		Mpeg2UpdateSbmBuffer(pMpeg2Dec,uInrCode);
        	}

        	FbmReturnBuffer(pMpeg2Context->pFbm,pMpeg2Dec->frmBufInfo.pCurFrm, 0);
            pMpeg2Dec->bSearchStdFlag = 1;
            pMpeg2Dec->bFstSetVbvFlag = 1;
            pMpeg2Dec->sbmInfo.pHwReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;
            pMpeg2Dec->sbmInfo.nSbmHwDataSize = pMpeg2Dec->sbmInfo.nSbmDataSize;
            pMpeg2Dec->nDecStep = MP2VDEC_CAL_RESULT;
            pMpeg2Dec->bDecPicOkFlag = 1;
            pMpeg2Dec->picInfo.bFstFieldFlag = 0;
        }
        else if(uInrCode & 4)           // request data
        {   
            Mpeg2SbmUpdateReadPointer(pMpeg2Dec, nByteLen);
            pMpeg2Dec->nDecStep = MP2VDEC_SET_VBV_CONFIG;
            pMpeg2Dec->bFstSetVbvFlag = 0;
            if(Mpeg2RequestBitstreamData(pMpeg2Context, pMpeg2Dec) != VDECODE_RESULT_OK)
            {   
                return VDECODE_RESULT_NO_BITSTREAM;
            }
            pMpeg2Dec->nDecStep = MP2VDEC_SET_VBV_CONFIG;
            usleep(2);//* Here, we sleep 2 us to fix hw.
			          //* If we reset regsiter too fast, hw will cover the previous values.
            goto mpeg2_set_dec_register;
         }
         else if(uInrCode & 1)                       // decode finish
         {  
            Mpeg2UpdateSbmBuffer(pMpeg2Dec,uInrCode);
            if((pMpeg2Dec->picInfo.eCurPicStructure==MP2VDEC_FRAME)||(pMpeg2Dec->picInfo.bFstFieldFlag==0))
            {   
                Mpeg2ProcessPictureFinish(pMpeg2Context, pMpeg2Dec);
                Mpeg2PutPictureOut(pMpeg2Dec,pMpeg2Context->pFbm, bDecodeKeyFrameOnly);
                pMpeg2Dec->bDecPicOkFlag = 1;
                pMpeg2Dec->bSearchStdFlag = 0;
            }
           // if(bDecodeKeyFrameOnly==0)
           // {
                pMpeg2Dec->bFstSetVbvFlag = 1;
                pMpeg2Dec->sbmInfo.pHwReadPtr = pMpeg2Dec->sbmInfo.pReadPtr;
                pMpeg2Dec->sbmInfo.nSbmHwDataSize = pMpeg2Dec->sbmInfo.nSbmDataSize;
            //}
            pMpeg2Dec->nDecStep = MP2VDEC_CAL_RESULT;
        }
    }
    //********step6: calculate the decode result
    if(pMpeg2Dec->nDecStep == MP2VDEC_CAL_RESULT)
    {
        if(pMpeg2Dec->bResetHwFlag==1)
        {   
            ResetVeInternal(pMpeg2Context->pVideoEngine);           
            pMpeg2Dec->bResetHwFlag = 0;
        }
        
        if(pMpeg2Dec->bSearchStdFlag == 1)
        {   
            if(Mpeg2SearchNextPicStartcode(pMpeg2Context,pMpeg2Dec) != VDECODE_RESULT_OK)
            {   
                return VDECODE_RESULT_NO_BITSTREAM;
            }
            pMpeg2Dec->picInfo.eLastStartCode = MP2VDEC_SLICE_START_CODE;
            pMpeg2Dec->bSearchStdFlag = 0;
        }
        pMpeg2Dec->picInfo.bGetPicFlag = 0;
        pMpeg2Dec->nDecStep = MP2VDEC_PARSE_HEADER;
        Mpeg2ReleaseBitstreamData(pMpeg2Dec);
        if(pMpeg2Dec->bDecPicOkFlag == 1)
        {
            if(bDecodeKeyFrameOnly == 1)
            {
                Mpeg2DecodeSetSbmBuf(pMpeg2Context->pStreamBufferBase, pMpeg2Context->nStreamBufferSize, pMpeg2Dec);
                Mpeg2ResetDecodeParams(pMpeg2Dec);
                return VDECODE_RESULT_KEYFRAME_DECODED;
            }
            return (pMpeg2Dec->picInfo.eCurPicType==MP2VDEC_I_PIC)? VDECODE_RESULT_KEYFRAME_DECODED:VDECODE_RESULT_FRAME_DECODED;
        }
    }
    return VDECODE_RESULT_OK;
}


/******************************************************************************************/
//       Name:	ve_close                                                                  //
//       Prototype: vresult_e ve_close (uint8_t flush_pictures);                               //
//       Function:	Close the VE CSP.                                                     //
//       Return:    VDECODE_RESULT_OK: success.                                                  //
//	     VDECODE_RESULT_UNSUPPORTED: the CSP is not opened yet.                         //
/******************************************************************************************/

static void Destroy(DecoderInterface* pSelf)
{    
    Mpeg2DecodeInfo* 	pMpeg2Dec 	  = NULL;
    Mpeg2DecodeContext* pMpeg2Context = NULL;
    pMpeg2Context = (Mpeg2DecodeContext*)pSelf;
    
    if(pMpeg2Context == NULL)
    {
        return;
    }
    else
    {
       //  Mpeg2FlushPictures(pMpeg2Context->pMpeg2Dec, pMpeg2Context->pFbm, pMpeg2Context->pFbm_scaledown, 0);
        //** release mpeg2 interface 
        //** release frame buffer manager
        pMpeg2Dec = (Mpeg2DecodeInfo*)pMpeg2Context->pMpeg2Dec;
        if(pMpeg2Context->pFbm)
        {
            FbmDestroy(pMpeg2Context->pFbm);
            pMpeg2Context->pFbm = NULL;
        }
        free(pMpeg2Context->pMpeg2Dec);
        pMpeg2Context->pMpeg2Dec = NULL;
               
        free(pMpeg2Context);
        pMpeg2Context = NULL;
        return;
    }
}

