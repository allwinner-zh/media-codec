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
#ifndef H264_V2_DEC_H
#define H264_V2_DEC_H

#include "h264.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PICT_TOP_FIELD     1
#define PICT_BOTTOM_FIELD  2
#define PICT_FRAME         3
	
#define MAX_SPS_COUNT 32
#define MAX_PPS_COUNT 256
#define H264VDEC_MAX_STREAM_NUM 8
#define H264VDEC_MAX_EXTRA_DATA_LEN 1024
#define H264VDEC_ERROR_PTS_VALUE  (int64_t)-1 //0xFFFFFFFFFFFFFFFF

#define H264_MVC_PARAM_BUF_SIZE 32*1024
#define FIRST_SLICE_DATA 4
#define LAST_SLICE_DATA  2
#define SLICE_DATA_VALID 1
	

#define H264_P_TYPE  0 ///< Predicted
#define H264_B_TYPE  1 ///< Bi-dir predicted
#define H264_I_TYPE  2 ///< Intra
#define H264_SP_TYPE 3 ///< Switching Predicted
#define H264_SI_TYPE 4 ///< Switching Intra

#define DELAYED_PIC_REF 4

#define H264_IR_NONE  	 0
#define H264_IR_FINISH	 1
#define H264_IR_ERR	     2
#define H264_IR_DATA_REQ 4
	
#define H264_GET_VLD_OFFSET 0
#define H264_GET_STCD_OFFSET 1
#define MAX_MMCO_COUNT 66
#define MAX_PICTURE_COUNT 32
//#define H264_EXTENDED_DATA_LEN 32
#define H264_EXTENDED_DATA_LEN 0
#define H264_START_DEC_FRAME 1
#define H264_END_DEC_FRAME 2
#define VRESULT_DEC_FRAME_ERROR 0xFF
#define VRESULT_ERR_FAIL 0xFB

#define H264_MVC_PARAM_BUF_LEN 4096

enum
{   
    H264_STEP_CONFIG_VBV  = 0,
    H264_STEP_UPDATE_DATA = 1,
    H264_STEP_SEARCH_NALU = 2, 
    H264_STEP_SEARCH_AVC_NALU = 3,
    H264_STEP_PROCESS_DECODE_RESULT = 4,
    H264_STEP
};

enum 
{
    NAL_SLICE 			= 1,
    NAL_DPA   			= 2,
    NAL_DPB   			= 3,
    NAL_DPC   			= 4,
    NAL_IDR_SLICE 		= 5,
    NAL_SEI				= 6,
    NAL_SPS        	 	= 7,
    NAL_PPS        	 	= 8,
    NAL_AUD         	= 9,
    NAL_END_SEQUENCE 	= 10,
    NAL_END_STREAM   	= 11,
    NAL_FILLER_DATA  	= 12,
    NAL_HEADER_EXT1  	= 14,
    NAL_SPS_EXT         = 15,
    NAL_AUXILIARY_SLICE = 19,
    NAL_HEADER_EXT2     = 20,
};
	
enum
{
    H264_CODED_FRAME_RATIO_NONE  = 0,
    H264_CODED_FRAME_RATIO_4_3   = 1,
    H264_CODED_FRAME_RATIO_14_9  = 2,
    H264_CODED_FRAME_RATIO_16_9  = 3,
    H264_CODED_FRAME_RATIO_OTHER = 4,
    H264_CODED_FRAME_RATIO_
};

	
typedef struct H264_VBV
{   
    Sbm*    vbv;
    VideoStreamDataInfo* pVbvStreamData;
    uint8_t* pVbvBuf;           // the start address of the vbv buffer
    uint8_t* pReadPtr;          // the current read pointer of the vbv buffer
    uint8_t* pVbvBufEnd;        // the end address of the vbv buffer
    uint8_t* pVbvDataEndPtr;
    uint8_t* pVbvDataStartPtr;
    int32_t nVbvDataSize;      // the valid data size of the vbv buffer
    uint32_t nVbvBufPhyAddr;
    uint32_t nVbvBufEndPhyAddr;
    uint32_t bVbvDataCtrlFlag;
    uint32_t nFrameRate;
    int64_t nVbvDataPts;
    int64_t nValidDataPts;
    int64_t nPrePicPts;
    int64_t nNextPicPts;
    int32_t nPrePicPoc;
    int64_t nPicDuration;
    int64_t nLastValidPts;
}H264Vbv;

typedef struct H264_PIC_INF 
{
    int32_t nFieldPoc[2];           ///< h264 top/bottom POC
    int32_t nPoc;                    ///< h264 frame POC
    int32_t nFrmNum;              ///< h264 frame_num (raw frame_num from slice header)
    int32_t nPicId;                 /**< h264 pic_num (short -> no wrap version of pic_num,
    														 pic_num & max_pic_num; long -> long_pic_num) */
    int32_t bLongRef;               ///< 1->long term nReference 0->short term nReference
    int32_t nRefPoc[2][16];         ///< h264 POCs of the frames used as nReference
    int32_t nRefCount[2];           ///< number of entries in ref_nPoc
    int32_t bFrameScore;          /* */
    int32_t nReference;
    uint8_t  bKeyFrame;
    int32_t nPictType;
    int32_t nPicStructure;
    int32_t nFieldPicStructure[2];
    uint32_t nDecodeBufIndex;
    uint32_t nDispBufIndex;
    uint8_t* pTopMvColBuf;
    uint8_t* pBottomMvColBuf;
    VideoPicture* pVPicture;
    uint8_t nNalRefIdc;
    uint8_t nFieldNalRefIdc[2];
    uint8_t bMbAffFrame;
    uint32_t nDecFrameOrder;
    uint8_t  bHasDispedFlag;
    uint8_t  bDecErrorFlag;
//****************for mvc************************//
}H264PicInfo;

typedef struct H264_FRM_BUF_INF
{   
    int32_t nMaxValidFrmBufNum;
    uint8_t* pMvColBuf;
    H264PicInfo picture[MAX_PICTURE_COUNT];
    H264PicInfo* pLongRef[MAX_PICTURE_COUNT];
    H264PicInfo* pShortRef[MAX_PICTURE_COUNT];
    H264PicInfo  defaultRefList[2][MAX_PICTURE_COUNT];
    H264PicInfo  refList[2][48];
    H264PicInfo* pDelayedPic[MAX_PICTURE_COUNT];
    H264PicInfo* pDelayedOutPutPic;
    H264PicInfo *pLastPicturePtr;     ///< pointer to the previous picture.
    H264PicInfo *pNextPicturePtr;     ///< pointer to the next picture (for bidir pred)
    H264PicInfo *pCurPicturePtr;      ///< pointer to the current picture
}H264FrmBufInfo;


typedef struct H264_SPS_INFO
{   
    uint8_t  bMbAff;
    uint8_t  nPocType;
    uint8_t  bDeltaPicOrderAlwaysZeroFlag;
    uint8_t  bFrameMbsOnlyFlag;
    uint8_t  bDirect8x8InferenceFlag;
    int32_t nRefFrmCount;
    uint32_t nMbWidth;
    uint32_t nMbHeight;
    uint32_t nFrmMbWidth;
    uint32_t nFrmMbHeight;
    uint32_t nFrmRealWidth;
    uint32_t nFrmRealHeight;
    uint8_t bCodedFrameRatio;
    uint32_t nAspectRatio;
    
    int32_t nLog2MaxFrmNum;
    int32_t bScalingMatrixPresent;
    uint32_t nLog2MaxPocLsb;
    int32_t nOffsetForNonRefPic;
    int32_t nOffsetForTopToBottomField;
    uint32_t nPocCycleLength;
    uint32_t nProfileIdc;
    uint8_t nScalingMatrix4[6][16];
    uint8_t nScalingMatrix8[2][64];
    uint16_t nOffsetForRefFrm[256];
    int32_t bBitstreamRestrictionFlag;
    int32_t nNumReorderFrames;
    uint8_t bLowDelayFlag;
    uint8_t bPicStructPresentFlag;
    uint32_t bNalHrdParametersPresentFlag;
    uint32_t bVclHrdParametersPresentFlag;
    uint32_t nCpbRemovalDelayLength;
    uint32_t nDpbOutputDelayLength;
    uint32_t nPicDuration;
}H264SpsInfo;

typedef struct H264_PPS_INFO
{   
    uint8_t  bScalingMatrixPresent;
    uint32_t nSpsId;
    int32_t bCabac;
    int32_t nInitQp;                ///< pic_init_qp_minus26 + 26
    uint32_t nSliceGroupCount;
    uint32_t nMbSliceGroupMapType;
    uint8_t pPicOrderPresent;
    uint8_t pRedundatPicCntPresent;
    uint32_t nRefCount[2];
    int32_t bWeightedPred;          ///< weighted_pred_flag
    int32_t nWeightedBIpredIdc;
    int32_t bDeblockingFilterParamPresent;
    int32_t bTransform8x8Mode;
    int32_t nChromaQpIndexOffset[2];
    int32_t bConstrainedIntraPred; ///< constrained_intra_pred_flag
    uint8_t nScalingMatrix4[6][16];
    uint8_t nScalingMatrix8[2][64];
}H264PpsInfo;


/**
 * Memory management control operation opcode.
 */
typedef enum H264_MMCO_OPCODE
{
    MMCO_END = 0,
    MMCO_SHORT_TO_UNUSED,
    MMCO_LONG_TO_UNUSED,
    MMCO_SHORT_TO_LONG,
    MMCO_SET_MAX_LONG,
    MMCO_RESET,
    MMCO_LONG,
}h264_mmco_opcode;

/**
 * Memory management control operation.
 */
typedef struct H264_MMCO_INFO
{
    h264_mmco_opcode opCode;
    int32_t nShortPicNum;  ///< pic_num without wrapping (pic_num & max_pic_num)
    int32_t bLongArg;       ///< index, pic_num, or num long refs depending on opcode
}H264MmcoInfo;

typedef struct
{
	uint8_t  bBufUsedFlag;
	int32_t nFrameBufIndex;
	int32_t nFirstFieldPoc;
	int32_t nSecondFieldPoc;
    int32_t nFrameStruct;
    int32_t nTopRefType;
    int32_t nBottomRefType;
}H264FrmRegisterInfo;

typedef struct H264_CONTEXT
{   
    int8_t  bIsAvc;                    ///< this flag is != 0 if codec is avc1
    int32_t nNalLengthSize;             ///< Number of bytes used for nal length (1, 2 or 4)
    uint8_t* pExtraDataBuf;       
    uint32_t nExtraDataLen;
    uint8_t  bDecExtraDataFlag;
    uint8_t  nNalRefIdc;
    uint8_t  nalUnitType;
    
    uint8_t  bFstField;
    uint8_t  nPicStructure;
    uint8_t  nRedundantPicCount;
    uint8_t  nSliceType;
    
    int32_t  nDeltaPoc[2];
    uint32_t nListCount;
    int32_t nLongRefCount;  ///< number of actual long term nReferences
    int32_t nShortRefCount; ///< number of actual short term nReferences
    int32_t nRefCount[2];
        
    uint32_t nCurSliceNum;                     // current slice number
    uint32_t nLastSliceType; 
    int32_t nFrmNum;
    int32_t nLastFrmNum;
    uint32_t nCurPicNum;
    uint32_t nMaxPicNum;
    uint32_t nMbX;
    uint32_t nMbY;
    int32_t bDirectSpatialMvPred;
    int32_t bMbAffFrame;

    int32_t nCabacInitIdc;
    int32_t nLastQscaleDiff;
    int32_t nQscale;
    int32_t bDisableDeblockingFilter;         ///< disable_deblocking_filter_idc with 1<->0
    int32_t nSliceAlphaC0offset;
    int32_t nSliceBetaoffset;
    int32_t bNumRefIdxActiveOverrideFlag;
    int32_t nChromaQp[2];
    
    int32_t nDeltaPocBottom;
    int32_t bFstMbInSlice;
    int32_t bLastMbInSlice;
    int32_t nFrmNumOffset;
    int32_t nPrevFrmNumOffset;
    int32_t nPrevFrmNum;
    int32_t nPrevPocLsb;
    int32_t nPrevPocMsb;
    int32_t nPocMsb;
    int32_t nPocLsb;
    int32_t nPocType;
    int32_t nMmcoIndex;
    
    uint8_t  bFrameMbsOnlyFlag;
    uint8_t  bDirect8x8InferenceFlag;
    int32_t nRefFrmCount;
    uint32_t nMbWidth;
    uint32_t nMbHeight;
    uint32_t nFrmMbWidth;
    uint32_t nFrmMbHeight;
    	
#if 1
    H264SpsInfo* pCurSps;                ///< current pSps
    H264PpsInfo* pCurPps;               // current pPps
	uint8_t          nSpsBufferNum;
	uint8_t          nPpsBufferNum;
    uint8_t          nSpsBufferIndex[32];
    uint8_t          nPpsBufferIndex[32];

    H264SpsInfo *pSpsBuffers[MAX_SPS_COUNT];
    H264PpsInfo *pPpsBuffers[MAX_PPS_COUNT];
    uint32_t          nNewSpsId;
#else
    H264SpsInfo pSps;               ///< current pSps
    H264PpsInfo pPps;               // current pPps

    uint32_t  nSpsId;               ///< current pSps
    uint32_t  nPpsId;
#endif

    H264MmcoInfo mmco[MAX_MMCO_COUNT];
    uint32_t nScalingMatrix4[6][4];
    uint32_t nScalingMatrix8[2][16];
    	
    H264FrmBufInfo frmBufInf;     // the information of the frame buffer 
    H264Vbv vbvInfo;           // the information of the h264 vbv buffer

    uint8_t* pMbFieldIntraBuf;
    uint8_t* pMbNeighborInfoBuf;
    uint32_t bNeedFindIFrm;
    uint32_t bNeedFindPPS;
    uint32_t bNeedFindSPS;
    uint32_t bScalingMatrixPresent;
    uint8_t  nDecFrameStatus;

    uint32_t nCurFrmNum;
    int32_t nDelayedPicNum;
    int32_t nMinDispPoc;
    uint8_t  bOnlyDecKeyFrame;
    uint8_t  bProgressice;
    int64_t nEstimatePicDuration;
    uint8_t  nPicPocDeltaNum;
    uint8_t*     pVbvBase;
    uint32_t     nVbvSize;
    Sbm*    pVbv;
    Fbm*    pFbm;
    Fbm*    pFbmScaledown;
    uint8_t      nDecStep;           // the status of the mpeg2 decoder
    uint8_t	    bCanResetHw;
    uint8_t      bNeedCheckFbmNum;
    int32_t     nPoc;
    uint8_t      bIdrFrmFlag;
    int32_t	    nLeftOffset;
    int32_t 	nRightOffset;
    int32_t 	nTopOffset;
    int32_t 	nBottomOffset;
    uint8_t  	bUseDramBufFlag;
    uint8_t*     pDeblkDramBuf;
    uint8_t*     pIntraPredDramBuf;
    int32_t     bEndOfStream;
    int64_t     nSystemTime;
}H264Context;
		
typedef struct H264_DECODER
{   
    uint32_t          nVeVersion;
    uint32_t          nRegisterBaseAddr;
    H264Context* pHContext;
    uint8_t           nDecStreamIndex;   // 0: major stream, 1: minor stream
}H264Dec;



//* memory methods.

void* (*MemPalloc)(int nSize);
void  (*MemPfree)(void* pMem);
void  (*MemFlushCache)(void* pMem, int nSize);
void* (*MemGetPhysicAddress)(void* pVirtualAddress);
void  (*MemSet)(void* pMem, int nSize, int nValue);
void  (*MemCopy)(void* pMemDst, void* pMemSrc, int nSize);
int  (*MemRead)(void* pMemSrc, void* pMemDst, int nSize);
int  (*MemWrite)(void* pMemSrc, void* pMemDst, int nSize);


#ifdef __cplusplus
}
#endif

#endif
