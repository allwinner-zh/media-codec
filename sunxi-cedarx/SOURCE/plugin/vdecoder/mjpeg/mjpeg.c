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

#include "mjpeg_dec_lib.h"

static int  MjpegDecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo);
static int  MjpegDecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex);
static int  MjpegDecoderGetFbmNum(DecoderInterface* pSelf);
static Fbm* MjpegDecoderGetFbm(DecoderInterface* pSelf, int nIndex);
static int  MjpegDecoderDecode(DecoderInterface* pSelf,
                            int               bEndOfStream,
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay,
                            int64_t           nCurrentTimeUs);
static void Destroy(DecoderInterface* pSelf);
void MjpegDecoderReset(DecoderInterface* pSelf);
extern void MjpegSetDispParams(MjpegDecodeContext* pMjpegContext, JpegDec* mJpegDec, VideoPicture* pCurPicture);

DecoderInterface* CreateMjpegDecoder(VideoEngine* p)
{
    MjpegDecodeContext* pMjpegContext;

    pMjpegContext = (MjpegDecodeContext*) malloc(sizeof(MjpegDecodeContext));
    if(pMjpegContext == NULL)
        return NULL;

    memset(pMjpegContext, 0, sizeof(MjpegDecodeContext));

    pMjpegContext->pVideoEngine        = p;
    pMjpegContext->interface.Init      = MjpegDecoderInit;
    pMjpegContext->interface.Reset     = MjpegDecoderReset;
    pMjpegContext->interface.SetSbm    = MjpegDecoderSetSbm;
    pMjpegContext->interface.GetFbmNum = MjpegDecoderGetFbmNum;
    pMjpegContext->interface.GetFbm    = MjpegDecoderGetFbm;
    pMjpegContext->interface.Decode    = MjpegDecoderDecode;
    pMjpegContext->interface.Destroy   = Destroy;

    return &pMjpegContext->interface;
}

void CedarPluginVDInit(void)
{
    int ret;
    ret = VDecoderRegister(VIDEO_CODEC_FORMAT_MJPEG, "mjpeg", CreateMjpegDecoder);

    if (0 == ret)
    {
        logi("register mjpeg decoder success!");
    }
    else
    {
        loge("register mjpeg decoder failure!!!");
    }
    return ;
}

//************************************************************************************/
//      Name:	    MjpegDecoderInit                                                          //
//      Prototype:	Handle ve_open (VConfig* config, VideoStreamInfo* stream_info); //
//      Function:	Start up the VE CSP.                                             //
//      Return:	A handle of the VE device.                                           //
//      Input:	VConfig* config, the configuration for the VE CSP.                 //
//************************************************************************************/

static int  MjpegDecoderInit(DecoderInterface* pSelf, VConfig* pConfig, VideoStreamInfo* pVideoInfo)
{
    MjpegDecodeContext* pMjpegContext   = NULL;
    JpegDec* pMjpegDec = NULL;

    pMjpegContext = (MjpegDecodeContext*)pSelf;

    memcpy(&pMjpegContext->vconfig, pConfig, sizeof(VConfig));

    pMjpegContext->videoStreamInfo.eCodecFormat            = pVideoInfo->eCodecFormat;
    pMjpegContext->videoStreamInfo.nWidth                  = pVideoInfo->nWidth;
    pMjpegContext->videoStreamInfo.nHeight                 = pVideoInfo->nHeight;
    pMjpegContext->videoStreamInfo.nFrameRate              = pVideoInfo->nFrameRate;
    pMjpegContext->videoStreamInfo.nFrameDuration          = pVideoInfo->nFrameDuration;
    pMjpegContext->videoStreamInfo.nAspectRatio            = pVideoInfo->nAspectRatio;
    pMjpegContext->videoStreamInfo.nCodecSpecificDataLen   = pVideoInfo->nCodecSpecificDataLen;
    pMjpegContext->videoStreamInfo.pCodecSpecificData      = pVideoInfo->pCodecSpecificData;

    pMjpegDec = malloc(sizeof(JpegDec));
    if(pMjpegDec == NULL)
    {
    	goto _mjpeg_decode_init_err;
    }
    memset(pMjpegDec, 0, sizeof(JpegDec));
    pMjpegContext->pMjpegDec = (void*)pMjpegDec;
    return VDECODE_RESULT_OK;
_mjpeg_decode_init_err:
	if(pMjpegDec != NULL)
	{
		free(pMjpegDec);
		pMjpegDec = NULL;
	}
    return VDECODE_RESULT_UNSUPPORTED;
}


/**********************************************************************   *******************/
//          Name:	ve_reset                                                                //
//          Prototype: vresult_e ve_reset (uint8_t flush_pictures)                               //
//          Function:  Reset the VE CSP.                                                    //
//          Return:    VDECODE_RESULT_OK: success.                                                 //
//	                   VDECODE_RESULT_UNSUPPORTED: the CSP is not opened yet.             //
/********************************************************************************************/

void MjpegDecoderReset(DecoderInterface* pSelf)
{
	JpegDec* pMjpegDec = NULL;
    MjpegDecodeContext* pMjpegContext = NULL;
    pMjpegContext = (MjpegDecodeContext*)pSelf;

    if(pMjpegContext == NULL)
    {
        return;
    }
    else
    {
    	pMjpegDec = (JpegDec*)pMjpegContext->pMjpegDec;
        ResetVeInternal(pMjpegContext->pVideoEngine);
        pMjpegDec->nDecStep = MJPEG_DEC_INIT;
        pMjpegDec->nDecStep = MJPEG_DEC_INIT;
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

static int  MjpegDecoderSetSbm(DecoderInterface* pSelf, Sbm* pSbm, int nIndex)
{
    MjpegDecodeContext* pMjpegContext = NULL;
    pMjpegContext = (MjpegDecodeContext*)pSelf;

    if(pMjpegContext == NULL)
    {
        return VDECODE_RESULT_UNSUPPORTED;
    }
    else
    {
    	if(nIndex == 0)
    	{
    		pMjpegContext->pSbm = pSbm;
    		pMjpegContext->pVbvBase = (uint8_t*)SbmBufferAddress(pSbm);
    		pMjpegContext->nVbvSize = (uint32_t)SbmBufferSize(pSbm);
    	}
        return VDECODE_RESULT_OK;
    }
}

static int  MjpegDecoderGetFbmNum(DecoderInterface* pSelf)
{
    MjpegDecodeContext* pMjpegContext;

    pMjpegContext = (MjpegDecodeContext*)pSelf;

    if(pMjpegContext == NULL)
    {
    	return 0;
    }
    return 1;
}

/****************************************************************************************************/
//             Name:	Mpeg2DecoderGetFbm                                                                  //
//             Prototype: Handle get_fbm (void);                                                    //
//             Function: Get a handle of the pFbm instance, in which pictures for display are stored.//
//             Return:   Not NULL Handle: handle of the pFbm instance.                               //
//                       NULL Handle: pFbm module is not initialized yet.                            //
/****************************************************************************************************/

static Fbm* MjpegDecoderGetFbm(DecoderInterface* pSelf, int nIndex)
{
    MjpegDecodeContext* pMjpegContext = NULL;
    pMjpegContext = (MjpegDecodeContext*)pSelf;

    if(pMjpegContext == NULL)
    {
        return NULL;
    }
    else
    {
        if(nIndex == 0)
        {
        	if(pMjpegContext->pFbm != NULL)
        	{
        		return pMjpegContext->pFbm;
        	}
        }
    }
    return NULL;
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
    int32_t i =0;
    MjpegDecodeContext* pMjpegContext = NULL;

    pMjpegContext = (MjpegDecodeContext*)pSelf;

    if(pMjpegContext == NULL)
    {
        return;
    }
    else
    {
        if(pMjpegContext->pFbm)
        {
            FbmDestroy(pMjpegContext->pFbm);
            pMjpegContext->pFbm = NULL;
        }
        free(pMjpegContext->pMjpegDec);
        pMjpegContext->pMjpegDec = NULL;

        free(pMjpegContext);
        pMjpegContext = NULL;
        return;
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


static int  MjpegDecoderDecode(DecoderInterface* pSelf,
                            int               bEndOfStream,
		                    int               bDecodeKeyFrameOnly,
                            int               bSkipBFrameIfDelay,
                            int64_t           nCurrentTimeUs)
{
	int32_t ret = 0;
	uint8_t* ptr = NULL;
	uint8_t  buffer[4];
	uint32_t frameSize = 0;
	uint32_t remainData = 0;
	uint32_t nInLumaBufAddr = 0;
	uint32_t nInChromaBufAddr = 0;
	uint32_t nOutLumaBufAddr = 0;
	uint32_t nOutChromaBufAddr = 0;
    uint8_t* pInLumaBuf = NULL;
	VideoStreamDataInfo* stream = NULL;
    JpegDec* 	   pMjpegDec = NULL;
	MjpegDecodeContext* pMjpegContext = NULL;
    uint8_t  bDispFrmFlag = 0;
    int32_t bEndOfStremaFlag = 0;
    int32_t bDecodeKeyFrameFlag = 0;
    int32_t bSkipBFrameFlag = 0;
    int64_t nCurTimeUs = 0;

    bEndOfStremaFlag = bEndOfStream;
    bDecodeKeyFrameFlag = bDecodeKeyFrameOnly;
    bSkipBFrameFlag = bSkipBFrameIfDelay;
    nCurTimeUs = nCurrentTimeUs;


	pMjpegContext = (MjpegDecodeContext*)pSelf;
	pMjpegDec = (JpegDec*)pMjpegContext->pMjpegDec;


	// step 1:
	if(pMjpegDec->nDecStep == MJPEG_DEC_INIT)
	{
		if(pMjpegDec->nDecStramIndex == 0)
		{
			pMjpegDec->pSbm = pMjpegContext->pSbm;
			pMjpegDec->pFbm = pMjpegContext->pFbm;
			pMjpegDec->pVbvBase = pMjpegContext->pVbvBase;
			pMjpegDec->nVbvSize = pMjpegContext->nVbvSize;
		}
		pMjpegDec->nDecStep = MJPEG_DEC_GET_FBM_BUFFER;
	}

	//step2
	if(pMjpegDec->nDecStep == MJPEG_DEC_GET_FBM_BUFFER)
	{
		 if(pMjpegDec->pFbm && FbmEmptyBufferNum(pMjpegDec->pFbm) == 0)
		 {
			 return VDECODE_RESULT_NO_FRAME_BUFFER;
		 }
		 pMjpegDec->nDecStep = MJPEG_DEC_GET_STREAM;
	}

	//step3
	if(pMjpegDec->nDecStep == MJPEG_DEC_GET_STREAM)
	{
		stream = SbmRequestStream(pMjpegDec->pSbm);
		if(stream == NULL)
		{
			return VDECODE_RESULT_NO_BITSTREAM;
		}
		pMjpegDec->nPts = stream->nPts;
		if((uint32_t)(stream->pData+4) <= (uint32_t)(pMjpegDec->pVbvBase+pMjpegDec->nVbvSize))
		{
			ptr = (uint8_t*)(stream->pData);
		}
		else
		{
			remainData = ((uint32_t)pMjpegDec->pVbvBase+pMjpegDec->nVbvSize-(uint32_t)stream->pData+1);
			memcpy(buffer, (uint8_t*)stream->pData, remainData);
			memcpy(buffer+remainData,  pMjpegDec->pVbvBase, 4-remainData);
			ptr = buffer;
		}
		frameSize = (ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|(ptr[3]);
		if(frameSize+4 == (uint32_t)stream->nLength)
		{
			pMjpegDec->nFrameSize = frameSize;
			pMjpegDec->data = (uint8_t*)(stream->pData+4);
			if(pMjpegDec->data > pMjpegDec->pVbvBase+pMjpegDec->nVbvSize)
			{
				pMjpegDec->data -= pMjpegDec->nVbvSize;
			}
		}
		else
		{
			pMjpegDec->nFrameSize = stream->nLength;
			pMjpegDec->data = (uint8_t*)stream->pData;
		}
		pMjpegDec->nDecStep = MJPEG_DEC_STREAM_DATA;
	}

	//step4:
	if(pMjpegDec->nDecStep == MJPEG_DEC_STREAM_DATA)
	{
		ResetVeInternal(pMjpegContext->pVideoEngine);
		ret = JpegDecoderMain(pMjpegContext, pMjpegDec);
		if(ret == VDECODE_RESULT_NO_FRAME_BUFFER)
		{
            if(stream != NULL)
			    SbmReturnStream(pMjpegDec->pSbm, stream);
			return VDECODE_RESULT_NO_FRAME_BUFFER;
		}
		else if(ret < 0)
		{
			bDispFrmFlag = 0;
			pMjpegDec->nDecStep = MJPEG_DEC_PROCESS_RESULT;
		}
		else
		{
			bDispFrmFlag = 1;
			pMjpegDec->nDecStep = MJPEG_DEC_PROCESS_RESULT;
		}
	}
	if(pMjpegDec->nDecStep == MJPEG_DEC_PROCESS_RESULT)
	{
        if(stream != NULL)
		    SbmFlushStream(pMjpegDec->pSbm, stream);

		if(pMjpegDec->pRefPicture != NULL)
		{
			MjpegSetDispParams(pMjpegContext, pMjpegDec, pMjpegDec->pRefPicture);
			FbmReturnBuffer(pMjpegDec->pFbm, pMjpegDec->pRefPicture, bDispFrmFlag);
		}

		pMjpegDec->pRefPicture = NULL;
		pMjpegDec->nDecStep = MJPEG_DEC_INIT;
	}
	return (bDispFrmFlag==1)? VDECODE_RESULT_KEYFRAME_DECODED:VDECODE_RESULT_OK;
}






