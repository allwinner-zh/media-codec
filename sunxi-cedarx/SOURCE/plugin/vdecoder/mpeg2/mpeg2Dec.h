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
#ifndef MPEG2DEC_H
#define MPEG2DEC_H

#include "mpeg2.h"

#ifdef __cplusplus
extern "C" {
#endif

    #define MAGIC_VER_A 0
    #define MAGIC_VER_B 1

    #define MPEG2VDEC_MAX_STREAM_NUM  500
    #define MP2VDEC_ERROR_PTS_VALUE  (int64_t)-1 //0xFFFFFFFFFFFFFFFF
    #define MP2VDEC_RECORD_MAX_SEQ_INFO 240

    #define MP2VDEC_PARSE_SEQ_MODE1      1
    #define MP2VDEC_PARSE_SEQ_MODE2      2
    #define MP2VDEC_SEQ_START_CODE   0x000001B3
    #define MP2VDEC_GOP_START_CODE   0x000001B8
    #define MP2VDEC_PIC_START_CODE   0x00000100
    #define MP2VDEC_USER_START_CODE  0x000001B2
    #define MP2VDEC_EXT_START_CODE   0x000001B5
    #define MP2VDEC_SLICE_START_CODE 0x00000101
    
    #define	MP2VDEC_SEQ_EXT_ID					 1
    #define	MP2VDEC_PIC_EXT_ID					 8
    #define	MP2VDEC_SEQ_DISPLAY_EXT_ID			 2
    #define	MP2VDEC_SEQ_SCALABLE_EXT_ID			 5
    #define	MP2VDEC_PIC_QUANT_MATRIX_EXT_ID		 3
    #define	MP2VDEC_PIC_DISPLAY_EXT_ID			 7
    #define	MP2VDEC_PIC_SPATIAL_SCALABLE_EXT_ID	 9
    #define	MP2VDEC_PIC_TEMPORAL_SCALABLE_EXT_ID 10
    
   
    enum  MPEG2_DECODE_STATUS
    {
        MP2VDEC_PARSE_HEADER = 0,
        MP2VDEC_CHECK_HWENGINE_FREE,
        MP2VDEC_CHECK_EMPTY_FRAME_BUFFER,
	    MP2VDEC_SET_VBV_CONFIG,
	    MP2VDEC_FINISH_CHECK,
	    MP2VDEC_CAL_RESULT,
    };
    enum MPEG2_PICTURE_TYPE
    {   
        MP2VDEC_NON_PIC = 0,
        MP2VDEC_I_PIC = 1,
        MP2VDEC_P_PIC = 2,
        MP2VDEC_B_PIC = 3
    };

    enum mp2_picture_structure
    {
		MP2VDEC_TOP_FIELD = 1,
		MP2VDEC_BOT_FIELD = 2,
		MP2VDEC_FRAME
    };
    
    enum
    {
        MPEG_CODED_FRAME_RATIO_NONE  = 0,
        MPEG_CODED_FRAME_RATIO_4_3   = 1,
        MPEG_CODED_FRAME_RATIO_14_9  = 2,
        MPEG_CODED_FRAME_RATIO_16_9  = 3,
        MPEG_CODED_FRAME_RATIO_OTHER = 4,
        MPEG_CODED_FRAME_RATIO_
    };


    typedef struct MPEG2_SBM
    {   
        Sbm*    pSbm;
        VideoStreamDataInfo* vbvStreamData[MPEG2VDEC_MAX_STREAM_NUM];
        int16_t  	nCurUpdateStrmIdx;
        int16_t  	nCurReleaseStrmIdx;
        uint32_t 	nStrmLen;
        uint8_t* 	pSbmBuf;           // the start address of the vbv buffer
        uint8_t* 	pReadPtr;          // the current read pointer of the vbv buffer
        uint8_t* 	pSbmBufEnd;        // the end address of the vbv buffer
        uint8_t* 	pRecordPtr;
        uint32_t 	nSbmBufSize;       // the total size of the vbv buffer
        uint32_t 	nSbmDataSize;      // the valid data size of the vbv buffer
        int64_t nSbmDataPts;
        int64_t nValidDataPts;
        uint8_t  	bGetFstDataFlag;
        uint8_t* 	pHwReadPtr;
        uint32_t 	nSbmHwDataSize;
        uint8_t  	aSbmCopyBuf[148];
        int64_t nLastDataPts;
    }MpegSbmInfo;
    
    typedef struct MPEG2_FRAMEBUF
    {
        VideoPicture* pCurFrm;          // current original frame info
        VideoPicture* pLastRefFrm;
        VideoPicture* pForRefFrm;       // foreward reference frame 
        VideoPicture* pBacRefFrm;       // backward reference frame 
    }Mpeg2FrmBufInfo;

    typedef struct
    {   
        uint8_t  bMallocFrmBufFlag;
        uint32_t nFrmNum;
        uint8_t  bOnlyDispKeyFrmFlag;
        uint8_t  eLastPicStructure;  // the structure of the last decoded picture
        uint8_t  bGetSeqFlag;        // has got the sequence nStartcode
        uint8_t  bGetGopFlag;
        uint8_t  bGetPicFlag;
        uint8_t  bGetForFrmFlag;
        uint8_t  bGetBacFrmFlag;
        uint8_t  bIsMpeg1Flag;       // whether is mpeg1 stream
        uint8_t  bFstFieldFlag;      // the first field of the decode picture
        uint8_t  bDropFrmFlag;       // whether drop the current frame  
        uint8_t  eCurPicType;        // has decoded the frame type 1: I, 2: P, 3:B
        uint8_t  eLastPicType;
        uint8_t  bLoadIntraQuartMatrixFlag;
        uint8_t  bLoadNonIntraQuartMatrixFlag;
        uint8_t	aIntraQuartMatrix[64];
	    uint8_t	aNonIntraQuartMatrix[64];
        
        uint32_t uTemporalRef;
        uint32_t uFullPerBacVector;
        uint32_t uFullPerForVector;
        uint32_t uForFCode;
        uint32_t uBacFCode;

        uint32_t uFCode00;
        uint32_t uFCode01;
        uint32_t uFCode10;
        uint32_t uFCode11;
        uint32_t uIntraDcPrecision;
        uint32_t uFrmPredFrmDet;
        uint32_t uQScaleType;
        uint32_t uIntraVlcFormat;
        uint32_t uAlternateScan;
        uint32_t uChroma420Type;
        uint32_t uProgressiveFlag;
        uint32_t uProgressiveSequence;

    
        uint32_t 	nAspectRatio;
        uint32_t 	eLastStartCode;     // has checked the start code
        uint32_t 	eCurStartCode;      // current nStartcode
        uint32_t 	nPicWidth;
        uint32_t 	nPicHeight;
        uint32_t 	nMbXNum;
        uint32_t 	nMbYNum;
        uint32_t 	nMbWidth;
        uint32_t 	nMbHeight;
        uint32_t 	nFrmRate;
        uint64_t 	nPicDuration;
        uint32_t 	uConcealMotionVectors;
        uint32_t 	eCurPicStructure;
        uint32_t 	nIntraDcPresion;
        uint32_t 	bTopFieldFstFlag;
        uint32_t 	bRepeatFstFieldFlag;
        uint32_t 	uChromaFormat;
        int64_t nCurPicPts;
        int64_t nNextPicPts;
        uint8_t  	bHasPtsFlag;
        int32_t     nLeftOffset;
        int32_t     nRightOffset;
        int32_t     nTopOffset;
        int32_t     nBottomOffset;
        int8_t      bCodedFrameRatio;
        int32_t 	nDispWidth;
        int32_t 	nDispHeight;
    }Mpeg2PicInfo;

    typedef struct MPEG2_DECODER
    {   
        uint32_t       		nVeVersion;
        uint8_t        		nSearchStcdTime;
        uint32_t       		uRegisterBaseAddr;
        uint8_t        		nDecStep;           // the status of the mpeg2 decoder
        uint8_t        		bDecPicOkFlag;      // has decoded a frame successfully
        uint8_t        		bSearchStdFlag;     // search the start code flag
        uint8_t        		bResetHwFlag;       // reset the hardware flag
        uint8_t        		bDecEndFlag;        // end decode the mpeg2 stream
        uint8_t        		bFstSetVbvFlag;     // first set the vbv configure flag
        Mpeg2PicInfo 	picInfo;           // picture information
        MpegSbmInfo 	sbmInfo;           // the information of the mpeg2 vbv buffer
        Mpeg2FrmBufInfo frmBufInfo;     // the information of the frame buffer 
        uint8_t         		eIcVersion;
        uint8_t         		bNormalChipFlag;
        uint8_t         		bSearchFstStartCodeFlag;
        uint8_t         		bFieldSeachNextPicFlag;
        int32_t             bEndOfStream;
        int32_t             bFrameDataComplete;
    }Mpeg2DecodeInfo;


//vresult_e mpeg2_reset(uint8_t flush_pictures,vdecoder_t* p);


void Mpeg2InitDecode(Mpeg2DecodeInfo* pMpeg2Dec);
void Mpeg2ResetDecodeParams(Mpeg2DecodeInfo* pMpeg2Dec);

void Mpeg2SetQuantMatrix(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bIntraFlag);
void Mpeg2DecodeSetSbmBuf(uint8_t* pSbmBase, uint32_t nSbmSize, Mpeg2DecodeInfo* pMpeg2Dec);
void Mpeg2FlushPictures(Mpeg2DecodeInfo* pMpeg2Dec, Fbm* pMpeg2Fbm, uint8_t bFlushPictureFlag);

void Mpeg2SetRotateScaleBuf(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec);
void Mpeg2SetReconstructBuf( Mpeg2DecodeInfo* pMpeg2Dec);
void Mpeg2SetSbmRegister(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bFstSetFlag,uint8_t bBoundFlag, uint32_t uStartBitPos, uint32_t uBitLen, uint8_t bDataEndFlag, uint8_t bCheckStartCodeFlag);
void Mpeg2SbmUpdateReadPointer(Mpeg2DecodeInfo* pMpeg2Dec, uint32_t nUpdateDataSize);
void Mpeg2UpdateSbmBuffer(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t uInrCode);
void Mpeg2ProcessPictureFinish(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec);
void Mpeg2PutPictureOut(Mpeg2DecodeInfo* pMpeg2Dec, Fbm* pMpeg2DecFrm, uint8_t bDecKeyFrmFlag);
void Mpeg2RevertSomeData(Mpeg2DecodeInfo* pMpeg2Dec, uint32_t nRevertDataLen);



int8_t Mpeg2RequestBitstreamData(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2HwSearchStartcode(Mpeg2DecodeContext* pMpeg2Ctx,Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2CheckVeIdle(Mpeg2DecodeInfo* pMpeg2Dec);

int8_t Mpeg2SetPictureInfo(Mpeg2DecodeInfo* pMpeg2Dec);

int8_t   Mpeg2SetPictureSize(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2SetPictureSize(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2ParsePictureHeader(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2SetHwStartCodeInfo(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2GetEmptyFrameBuf(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bKeyFramOnlyFlag,Fbm* pMpeg2DecFrm);
int8_t  Mpeg2SearchNextPicStartcode(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2SwCalculatePicLength(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2HwCalculatePicLength(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2ReleaseBitstreamData(Mpeg2DecodeInfo* pMpeg2Dec);



uint32_t Mpeg2GetStartcodeBitOffet(Mpeg2DecodeInfo* pMpeg2Dec);
uint32_t Mpeg2GetDecodeBitOffset(Mpeg2DecodeInfo* pMpeg2Dec);
uint32_t Mpeg2VeIsr(Mpeg2DecodeInfo* pMpeg2Dec);
uint32_t Mpeg2GetRegVal(uint32_t uRegAddr);
uint32_t Mpeg2GetVersion(Mpeg2DecodeInfo* pMpeg2Dec);


int32_t  Mpeg2SetupAnaglaghTransform(Mpeg2DecodeContext* pMpeg2Ctx);
void  Mpeg2CloseAnaglaghTransform(Mpeg2DecodeContext* pMpeg2Ctx);
int8_t Mpeg2JudgePictureEnd(Mpeg2DecodeInfo* pMpeg2Dec);
int8_t Mpeg2ParseSequenceInfo(Mpeg2DecodeContext* pMpeg2Ctx, Mpeg2DecodeInfo* pMpeg2Dec, uint8_t* pDataBuf, uint8_t uParseMode);

int8_t Mpeg2SearchStartcode(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t* pStartPtr, uint32_t nSearchSize, uint32_t* pStartCode, uint8_t bUpdataFlag);
//uint32_t  get_1623_chip_version(void);






#ifdef __cplusplus
}
#endif

#endif

