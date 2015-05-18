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

#include "h264_hal.h"
#include "h264_dec.h"

#define H264_WEIGHT_PREDPARA_LEN	768		//32*3*2*4 bytes
#define H264_QUANT_MATRIX_LEN		224		//(4*6+16*2)*4 bytes
#define H264_REFLIST_LEN			72		//36*2 bytes
#define H264_FRMBUF_INFO_LEN		576		//8*4*18 bytes

#define H264_WEIGHT_PREDPARA_ADDR	0x000
#define H264_FRMBUF_INFO_ADDR		0x400
#define H264_QUANT_MATRIX_ADDR		0x800
#define H264_REFLIST_ADDR			H264_FRMBUF_INFO_ADDR+H264_FRMBUF_INFO_LEN



void H264PrintRegister(H264DecCtx *h264DecCtx)
{   
    uint32_t i = 0;
    uint32_t* reg_base = NULL;
    H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

#if 0
    //logv("**************top register\n");
    reg_base = ve_get_reglist( REG_GROUP_VETOP);

    for(i=0;i<16;i++)
    {
        //logv("%02x, 0x%08x 0x%08x 0x%08x 0x%08x \n",i, reg_base[4*i],reg_base[4*i+1],reg_base[4*i+2],reg_base[4*i+3]);
    }
    //logv("\n");
#endif

    reg_base = (uint32_t*)h264Dec->nRegisterBaseAddr;
    //logv("*********	reg_base =%p, %p\n", reg_base, ve_get_reglist( REG_GROUP_H264_DECODER));
    
    for(i=0;i<16;i++)
    {
        logd("%02x, 0x%08x 0x%08x 0x%08x 0x%08x \n",i, reg_base[4*i],reg_base[4*i+1], reg_base[4*i+2],reg_base[4*i+3]);
    }
    logd("\n");
}


void H264InitFuncCtrlRegister(H264DecCtx *h264DecCtx)
{
    H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    
    vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL, (1<<10)|(1<<25));
}

void H264CheckBsDmaBusy(H264DecCtx *h264DecCtx)
{  
    #define BS_DMA_BUSY (1<<22)
    
    volatile uint32_t dwVal;
    H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    
    while(1)
    {   
        vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
        if(!(dwVal & BS_DMA_BUSY))
        {
            break;
        } 
    }
    return;
}



void H264EnableIntr(H264DecCtx *h264DecCtx)
{   
	volatile uint32_t dwVal;
	H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL);
	dwVal |= 7;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL, dwVal);
}

uint32_t H264GetbitOffset(H264DecCtx* h264DecCtx, uint8_t offsetMode)
{
	H264Dec* h264Dec = NULL;
	volatile uint32_t dwVal;
	
	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	if(offsetMode ==  H264_GET_VLD_OFFSET)
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_VLD_OFFSET);
	}
	else if(offsetMode ==  H264_GET_STCD_OFFSET)
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_STCD_OFFSET);
	}
	return dwVal;
}

int32_t H264UpdateDataPointer(H264DecCtx* h264DecCtx, H264Context* hCtx, uint8_t offsetMode)
{
	H264Dec* h264Dec = NULL;
	uint32_t bitOffset = 0;
	
	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	bitOffset = H264GetbitOffset(h264DecCtx, offsetMode);
    
	if(offsetMode==H264_GET_VLD_OFFSET /*&& newPtr[0]==0x01*/)
	{
		bitOffset = (bitOffset>=24)? (bitOffset-24) : bitOffset;
	}
	
	if((bitOffset>>3) > (uint32_t)(hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pVbvBuf+1))
	{
		bitOffset -= (hCtx->vbvInfo.pVbvBufEnd-hCtx->vbvInfo.pVbvBuf+1)*8;
		//logv("offset maybe error:x\n", bitOffset>>3);
	}
	if((hCtx->vbvInfo.pVbvBuf+(bitOffset>>3)) < hCtx->vbvInfo.pVbvDataStartPtr)
	{
		hCtx->vbvInfo.nVbvDataSize = hCtx->vbvInfo.pVbvDataEndPtr-hCtx->vbvInfo.pVbvBufEnd-(bitOffset>>3)-1;
	}
	else
	{
		hCtx->vbvInfo.nVbvDataSize = hCtx->vbvInfo.pVbvDataEndPtr-hCtx->vbvInfo.pVbvBuf-(bitOffset>>3);
	}

	hCtx->vbvInfo.pReadPtr = hCtx->vbvInfo.pVbvBuf+ (bitOffset>>3);

	if(hCtx->vbvInfo.nVbvDataSize <= (H264_EXTENDED_DATA_LEN+4))
	{   
		return VDECODE_RESULT_NO_BITSTREAM;
	}
	return VDECODE_RESULT_OK;
}

/****************************************************************************************************/
/****************************************************************************************************/


void H264ConfigureBitstreamRegister(H264DecCtx *h264DecCtx, H264Context* hCtx, uint32_t nBitLens)
{
    H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
    H264CheckBsDmaBusy(h264DecCtx);
    
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_OFFSET, (hCtx->vbvInfo.pReadPtr-hCtx->vbvInfo.pVbvBuf)*8);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_BIT_LENGTH, nBitLens);
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_END_ADDR, hCtx->vbvInfo.nVbvBufEndPhyAddr);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_BITSTREAM_ADDR, (hCtx->vbvInfo.bVbvDataCtrlFlag<<28)|hCtx->vbvInfo.nVbvBufPhyAddr);
}

void  H264SearchStartcode(H264DecCtx* h264DecCtx, H264Context* hCtx)
{
	volatile uint32_t dwVal;
	H264Dec* h264Dec = NULL;

	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

	H264CheckBsDmaBusy(h264DecCtx);

	vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL);
	dwVal &= ~STARTCODE_DETECT_E;
	dwVal |= EPTB_DETECTION_BY_PASS;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL, dwVal);

	H264ConfigureBitstreamRegister(h264DecCtx,  hCtx, hCtx->vbvInfo.nVbvDataSize*8);
	H264EnableIntr(h264DecCtx);
    vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, INIT_SWDEC);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, START_CODE_DETECT);
}

void H264ConfigureEptbDetect(H264DecCtx* h264DecCtx, H264Context* hCtx, uint32_t sliceDataBits, uint8_t eptbDetectEnable)
{
	volatile uint32_t dwVal;
	H264Dec* h264Dec = NULL;

	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

	H264CheckBsDmaBusy(h264DecCtx);

	if(eptbDetectEnable == 1)
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL);
		dwVal &= ~EPTB_DETECTION_BY_PASS;
	    vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL, dwVal);
	}

	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_OFFSET, (hCtx->vbvInfo.pReadPtr-hCtx->vbvInfo.pVbvBuf)*8);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_BIT_LENGTH, sliceDataBits);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_END_ADDR, hCtx->vbvInfo.nVbvBufEndPhyAddr);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_VLD_BITSTREAM_ADDR, (hCtx->vbvInfo.bVbvDataCtrlFlag<<28)|hCtx->vbvInfo.nVbvBufPhyAddr);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, INIT_SWDEC);
}
/****************************************************************************************************/
/****************************************************************************************************/


void H264ConfigureReconMvcolBufRegister(H264Dec* h264Dec, H264PicInfo* pCurPicturePtr)
{   
	volatile uint32_t* pdwTemp;
	
	//0xe0
	pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
	*pdwTemp = 0;
	sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+pCurPicturePtr->nDecodeBufIndex*32+12;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
	//logd("0xe0: %x\n", *pdwTemp);
	
	// configure luma buffer addr, chroma buffer addr, topmv_coladdr, botmv_coladdr
	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = (uint32_t)pCurPicturePtr->pVPicture->phyYBufAddr;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logv("0xe4: %x\n", *pdwTemp);
	
	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = (uint32_t)pCurPicturePtr->pVPicture->phyCBufAddr;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logv("0xe4: %x\n", *pdwTemp);
	
	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = (uint32_t)(*MemGetPhysicAddress)((void*)pCurPicturePtr->pTopMvColBuf);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logv("0xe4:%x\n", *pdwTemp);
	
	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = (uint32_t)(*MemGetPhysicAddress)((void*)pCurPicturePtr->pBottomMvColBuf);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logv("0xe4:%x\n", *pdwTemp);
}



void H264ConfigureFrameInfoRegister(H264Dec* h264Dec, H264PicInfo* pCurPicturePtr)
{
	volatile uint32_t* pdwTemp;
	H264Context* hCtx = NULL;
	hCtx = 	h264Dec->pHContext;


	if(hCtx->nPicStructure == PICT_FRAME)
	{   
		//0xe0
		pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
		*pdwTemp = 0;
		sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+pCurPicturePtr->nDecodeBufIndex*32;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
		//logv("0xe0:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
		*pdwTemp = 0;
		sram_port_rw_data_rege4.sram_data = pCurPicturePtr->nFieldPoc[0];
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
		*pdwTemp = 0;
		sram_port_rw_data_rege4.sram_data = pCurPicturePtr->nFieldPoc[1];
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&frame_struct_ref_info_rege4);
		*pdwTemp = 0;
		frame_struct_ref_info_rege4.frm_struct  = hCtx->bMbAffFrame? 2: 0;
		frame_struct_ref_info_rege4.top_ref_type = hCtx->nNalRefIdc ? 0: 2;
		frame_struct_ref_info_rege4.bot_ref_type = hCtx->nNalRefIdc ? 0: 2;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
	}
	else if(hCtx->nPicStructure == PICT_TOP_FIELD)
	{   
		//0xe0
		pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
		*pdwTemp = 0;
		sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+pCurPicturePtr->nDecodeBufIndex*32;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
		//logv("0xe0:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
		*pdwTemp = 0;
		sram_port_rw_data_rege4.sram_data = pCurPicturePtr->nFieldPoc[0];
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
		*pdwTemp = 0;
		sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+pCurPicturePtr->nDecodeBufIndex*32+8;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
		//logv("0xe0:%x\n", *pdwTemp);

		pdwTemp = (uint32_t*)(&frame_struct_ref_info_rege4);
		*pdwTemp = 0;
		frame_struct_ref_info_rege4.frm_struct = 1;
		frame_struct_ref_info_rege4.top_ref_type = hCtx->nNalRefIdc ? 0: 2;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
	}
	else if(hCtx->nPicStructure == PICT_BOTTOM_FIELD)
	{   
		//0xe0
		pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
		*pdwTemp = 0;
		sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+pCurPicturePtr->nDecodeBufIndex*32+4;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
		//logv("0xe0:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
		*pdwTemp = 0;
		sram_port_rw_data_rege4.sram_data = pCurPicturePtr->nFieldPoc[1];
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
		
		pdwTemp = (uint32_t*)(&frame_struct_ref_info_rege4);
		*pdwTemp = 0;
		frame_struct_ref_info_rege4.frm_struct = 1;
		frame_struct_ref_info_rege4.bot_ref_type = hCtx->nNalRefIdc ? 0: 2;;
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
		//logv("0xe4:%x\n", *pdwTemp);
	}
}
  

void H264CalCulateRefFrameRegisterValue(H264FrmRegisterInfo *pFrmRegisterInfo, H264PicInfo* pCurPicturePtr, uint8_t pPicStructure, uint8_t nReference)
{
	if(nReference == PICT_FRAME)
	{
		pFrmRegisterInfo->nFirstFieldPoc = pCurPicturePtr->nFieldPoc[0];
		pFrmRegisterInfo->nSecondFieldPoc = pCurPicturePtr->nFieldPoc[1];
		pFrmRegisterInfo->nFrameStruct = 1;

		if(pPicStructure == PICT_FRAME)
		{
			pFrmRegisterInfo->nFrameStruct  = pCurPicturePtr->bMbAffFrame? 2: 0; //hCtx->bMbAffFrame? 2: 0;
		}
		pFrmRegisterInfo->nTopRefType = pCurPicturePtr->nFieldNalRefIdc[0]? 0: 2; //hCtx->nNalRefIdc ? 0: 2;
		pFrmRegisterInfo->nBottomRefType = pCurPicturePtr->nFieldNalRefIdc[1]? 0: 2; //hCtx->nNalRefIdc ? 0: 2;
	}
	else if(nReference== PICT_TOP_FIELD)
	{
		pFrmRegisterInfo->nFirstFieldPoc = pCurPicturePtr->nFieldPoc[0];
		pFrmRegisterInfo->nFrameStruct = 1;
		pFrmRegisterInfo->nTopRefType = pCurPicturePtr->nFieldNalRefIdc[0]? 0: 2; //hCtx->nNalRefIdc ? 0: 2;
	}
	else if(nReference == PICT_BOTTOM_FIELD)
	{
		pFrmRegisterInfo->nSecondFieldPoc = pCurPicturePtr->nFieldPoc[1];
		pFrmRegisterInfo->nFrameStruct = 1;
		pFrmRegisterInfo->nBottomRefType = pCurPicturePtr->nFieldNalRefIdc[1]? 0: 2; //hCtx->nNalRefIdc ? 0: 2;
	}
}

void H264ConfigureRefFrameRegister(H264Dec* h264Dec, H264FrmRegisterInfo *pFrmRegisterInfo)
{
	volatile uint32_t* pdwTemp;
	//0xe0
	pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
	*pdwTemp = 0;
	sram_port_rw_offset_rege0.sram_addr = H264_FRMBUF_INFO_ADDR+ pFrmRegisterInfo->nFrameBufIndex*32;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);

	//logd("0xe0:%x\n", *pdwTemp);


	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = pFrmRegisterInfo->nFirstFieldPoc;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logd("0xe4:%x\n", *pdwTemp);

	pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
	*pdwTemp = 0;
	sram_port_rw_data_rege4.sram_data = pFrmRegisterInfo->nSecondFieldPoc;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logd("0xe4:%x\n", *pdwTemp);

	pdwTemp = (uint32_t*)(&frame_struct_ref_info_rege4);
	*pdwTemp = 0;
	frame_struct_ref_info_rege4.frm_struct  = pFrmRegisterInfo->nFrameStruct;
	frame_struct_ref_info_rege4.top_ref_type = pFrmRegisterInfo->nTopRefType;
	frame_struct_ref_info_rege4.bot_ref_type = pFrmRegisterInfo->nBottomRefType;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,*pdwTemp);
	//logd("0xe4:%x\n", *pdwTemp);
}

void H264ConfigureRefListRegister(H264Dec* h264Dec, H264Context* hCtx)
{
	uint32_t value = 0;
	uint32_t i= 0;
	int32_t j = 0;
	int32_t k = 0;
	int32_t n = 0;
	int32_t refPicNum[18]={0};
	volatile uint32_t* pdwTemp;
	H264PicInfo* pCurPicturePtr = NULL;
	H264FrmRegisterInfo pFrmRegisterInfo[32];
	uint8_t validFrameBufIndex[32];
	uint8_t bRefFrmErrorFlag = 0;

	
	memset(pFrmRegisterInfo, 0, 32*sizeof(H264FrmRegisterInfo));
	//logv("hCtx->nListCount=%d, nRefCount[0]=%d, nRefCount[1]=%d\n", hCtx->nListCount, hCtx->nRefCount[0], hCtx->nRefCount[1]);

	for(i=0; i<hCtx->nListCount; i++)
	{
		j = 0;
		for(; j<hCtx->nRefCount[i]; j++)
		{
			if(hCtx->frmBufInf.refList[i][j].pVPicture != NULL)
			{
				bRefFrmErrorFlag += hCtx->frmBufInf.refList[i][j].bDecErrorFlag;
				k = hCtx->frmBufInf.refList[i][j].nDecodeBufIndex;
				pCurPicturePtr = &hCtx->frmBufInf.refList[i][j];
				pFrmRegisterInfo[k].nFrameBufIndex = k;

				H264CalCulateRefFrameRegisterValue(&pFrmRegisterInfo[k],pCurPicturePtr,
						       hCtx->frmBufInf.refList[i][j].nPicStructure, hCtx->frmBufInf.refList[i][j].nReference);

				validFrameBufIndex[n] = k;
				if(pFrmRegisterInfo[k].bBufUsedFlag == 0)
				{
					n++;
				}
				pFrmRegisterInfo[k].bBufUsedFlag = 1;
			}
		}
	}

	hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag = (bRefFrmErrorFlag>0)? 1: hCtx->frmBufInf.pCurPicturePtr->bDecErrorFlag;


	for(j=0; j<n; j++)
	{
		k = validFrameBufIndex[j];
		H264ConfigureRefFrameRegister(h264Dec, &pFrmRegisterInfo[k]);
	}

	for(i=0; i<hCtx->nListCount; i++)
	{   
		memset(refPicNum, 0, 18*sizeof(uint32_t));
		j = hCtx->nRefCount[i]-1;

		for(;j>=0; j--)
		{
			if(hCtx->frmBufInf.refList[i][j].pVPicture != NULL)
			{
				k = hCtx->frmBufInf.refList[i][j].nDecodeBufIndex;
				value = (k<<1)|(hCtx->frmBufInf.refList[i][j].nPicStructure==PICT_BOTTOM_FIELD);
				refPicNum[j/4] = (refPicNum[j/4]<<8) |value;
			}
		}
		
		if(hCtx->nRefCount[i]>0)
		{
			//0xe0
			pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
			*pdwTemp = 0;
			vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,H264_REFLIST_ADDR+36*i);
			//logd("0xe0:%x\n", H264_REFLIST_ADDR+36*i);
			
			for(k=0; k<=(hCtx->nRefCount[i]/4); k++)
			{
				pdwTemp = (uint32_t*)(&sram_port_rw_data_rege4);
				*pdwTemp = 0;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,refPicNum[k]);
				//logd("0xe4:%x\n", refPicNum[k]);
			}
		}
	}
}

void H264ConvertScalingMatrices( H264Context* hCtx, uint8_t (*nScalingMatrix4)[16], uint8_t (*nScalingMatrix8)[64])
{   
	int32_t i = 0;
	int32_t j = 0;
	
	for(i=0; i<6; i++)
	{   
		for(j=0; j<4; j++)
		{
			hCtx->nScalingMatrix4[i][j] = (nScalingMatrix4[i][4*j+3]<<24) | (nScalingMatrix4[i][4*j+2]<<16) | (nScalingMatrix4[i][4*j+1]<<8)| (nScalingMatrix4[i][4*j+0]);
		}
	}
	
	for(i=0; i<2; i++)
	{
		for(j=0; j<16; j++)
		{
			hCtx->nScalingMatrix8[i][j] = (nScalingMatrix8[i][4*j+3]<<24) | (nScalingMatrix8[i][4*j+2]<<16) | (nScalingMatrix8[i][4*j+1]<<8)| (nScalingMatrix8[i][4*j+0]);
		}
	}
}

void H264ConfigureQuantMatrixRegister(H264DecCtx *h264DecCtx, H264Context* hCtx)
{   
	int32_t i = 0;
	int32_t j = 0;
	H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;


	if(hCtx->bScalingMatrixPresent == 1)
	{
		if(hCtx->pCurPps->bScalingMatrixPresent == 1)
		{   
			H264ConvertScalingMatrices(hCtx, hCtx->pCurPps->nScalingMatrix4, hCtx->pCurPps->nScalingMatrix8);
		}
		else if(hCtx->pCurSps->bScalingMatrixPresent == 1)
		{   
			H264ConvertScalingMatrices(hCtx, hCtx->pCurSps->nScalingMatrix4, hCtx->pCurSps->nScalingMatrix8);
		}
		hCtx->bScalingMatrixPresent = 0;
	}
	
	for(i=0; i<=1; i++)
	{
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,H264_QUANT_MATRIX_ADDR+i*64);
		//logv("0xe0: %x\n", H264_QUANT_MATRIX_ADDR+i*64);

		for(j=0; j<16; j++)
		{
			vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,hCtx->nScalingMatrix8[i][j]);
			//logv("0xe4: %x\n", hCtx->nScalingMatrix8[i][j]);
		}
	}
	
	for(i=0; i<6; i++)
	{
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,H264_QUANT_MATRIX_ADDR+0x80+i*16);
		//logv("0xe0: %x\n", H264_QUANT_MATRIX_ADDR+0x80+i*16);

		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,hCtx->nScalingMatrix4[i][0]);
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,hCtx->nScalingMatrix4[i][1]);
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,hCtx->nScalingMatrix4[i][2]);
		vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,hCtx->nScalingMatrix4[i][3]);
		//logv("0xe4: %x\n", hCtx->nScalingMatrix4[i][0]);
		//logv("0xe4: %x\n", hCtx->nScalingMatrix4[i][1]);
		//logv("0xe4: %x\n", hCtx->nScalingMatrix4[i][2]);
		//logv("0xe4: %x\n", hCtx->nScalingMatrix4[i][3]);
	}
	return;
}


void H264ConfigTopLeveLRegisters(H264Context* hCtx)
{
	vetop_reglist_t* vetop_reg_list;

	if(hCtx->bUseDramBufFlag == 1)
	{
		vetop_reg_list = (vetop_reglist_t*)ve_get_reglist(REG_GROUP_VETOP);

		if(hCtx->bUseDramBufFlag == 1)
		{
			vetop_reg_list->_50_deblk_intrapred_buf_ctrl.deblk_buf_ctrl = 1;
			vetop_reg_list->_50_deblk_intrapred_buf_ctrl.intrapred_buf_ctrl = 1;
			vetop_reg_list->_54_deblk_dram_buf.addr  = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->pDeblkDramBuf);
			vetop_reg_list->_58_intrapred_dram_buf.addr  = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->pIntraPredDramBuf);
		}
	}
}

void H264ConfigureSliceRegister(H264DecCtx *h264DecCtx, H264Context* hCtx, uint8_t decStreamIndex)
{   
	int32_t i = 0;
	int32_t maxFrmNum = 0;

	
	volatile uint32_t* pdwTemp;	
	H264Dec* h264Dec = NULL;
	H264PicInfo* pCurPicturePtr = NULL;
	uint8_t ndecStreamIndex;
	ndecStreamIndex = decStreamIndex;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;

    H264ConfigTopLeveLRegisters(hCtx);

	//congigure 0x00 register
	pdwTemp = (uint32_t*)(&avc_sps_reg00);
	*pdwTemp = 0;
	avc_sps_reg00.pic_height_mb_minus1 = hCtx->pCurSps->nMbHeight-1;
	avc_sps_reg00.pic_width_mb_minus1 = hCtx->pCurSps->nMbWidth-1;
	avc_sps_reg00.direct_8x8_inference_flag = hCtx->pCurSps->bDirect8x8InferenceFlag;
	avc_sps_reg00.mb_adaptive_frame_field_flag = hCtx->pCurSps->bMbAff;  //mb_adaptive_frame_field_flag
	avc_sps_reg00.frame_mbs_only_flag = hCtx->pCurSps->bFrameMbsOnlyFlag;
	avc_sps_reg00.chroma_format_idc = 1; //000:monochroma 001:4;2;0 010:4:2:2 011:4:4:4 others:reserved
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SPS,*pdwTemp);
	//logv("0x00: %x\n", *pdwTemp);
	
	//congigure 0x04 register
	pdwTemp = (uint32_t*)(&avc_pps_reg04);
	*pdwTemp = 0;
	avc_pps_reg04.transform_8x8_mode_flag = hCtx->pCurPps->bTransform8x8Mode;
	avc_pps_reg04.constrained_intra_pred_flag = hCtx->pCurPps->bConstrainedIntraPred;
	avc_pps_reg04.weighted_bipred_idc = hCtx->pCurPps->nWeightedBIpredIdc;
	avc_pps_reg04.weighted_pred_idc = hCtx->pCurPps->bWeightedPred;
	avc_pps_reg04.num_ref_idx_l1_active_minus1_pic = hCtx->pCurPps->nRefCount[1]-1;
	avc_pps_reg04.num_ref_idx_l0_active_minus1_pic = hCtx->pCurPps->nRefCount[0]-1;
	avc_pps_reg04.entropy_coding_mode_flag = hCtx->pCurPps->bCabac;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_PPS,*pdwTemp);
	//logv("0x04: %x\n", *pdwTemp);
	
	//congigure 0x08 register
	pdwTemp = (uint32_t*)(&shs_reg08);
	*pdwTemp = 0;
    shs_reg08.bCabac_init_idc = hCtx->nCabacInitIdc;
	shs_reg08.direct_spatial_mv_pred_flag = hCtx->bDirectSpatialMvPred;
	shs_reg08.bottom_field_flag = (hCtx->nPicStructure==PICT_BOTTOM_FIELD);
	shs_reg08.field_pic_flag = (hCtx->nPicStructure!=PICT_FRAME);
    shs_reg08.first_slice_in_pic = (hCtx->nCurSliceNum==1)? 1: 0;
    shs_reg08.slice_type = hCtx->nLastSliceType;
	shs_reg08.nal_ref_flag =  hCtx->nNalRefIdc;
	shs_reg08.first_mb_y = hCtx->nMbY;
	shs_reg08.first_mb_x = hCtx->nMbX;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SHS,*pdwTemp);
	//logv("0x08: %x\n", *pdwTemp);
	
	//congigure 0x0c register
	pdwTemp = (uint32_t*)(&shs2_reg0c);
	*pdwTemp = 0;
    shs2_reg0c.slice_beta_offset_div2 = hCtx->nSliceBetaoffset;
    shs2_reg0c.slice_alpha_c0_offset_div2 = hCtx->nSliceAlphaC0offset;
	shs2_reg0c.disable_deblocking_filter_idc = hCtx->bDisableDeblockingFilter;
	shs2_reg0c.num_ref_idx_active_override_flag = hCtx->bNumRefIdxActiveOverrideFlag;
	if(hCtx->nSliceType==H264_B_TYPE)
	{
		shs2_reg0c.num_ref_idx_l1_active_minus1_slice  = hCtx->nRefCount[1]-1;
	}
	shs2_reg0c.num_ref_idx_l0_active_minus1_slice  =  hCtx->nRefCount[0]-1;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SHS2,*pdwTemp);
	//logv("0x0c: %x\n", *pdwTemp);
	
	//configure 0x1c register
	pdwTemp = (uint32_t*)(&shs_qp_reg1c);
	*pdwTemp = 0;
	shs_qp_reg1c.slice_qpy	  = hCtx->nQscale;
	shs_qp_reg1c.chroma_qp_index_offset = hCtx->pCurPps->nChromaQpIndexOffset[0];
	shs_qp_reg1c.second_chroma_qp_index_offset =  hCtx->pCurPps->nChromaQpIndexOffset[1];
	shs_qp_reg1c.scaling_matix_flat_flag = 1;

	//logd("hCtx->pCurSps->nProfileIdc=%d, hCtx->pCurSps->bScalingMatrixPresent=%d, hCtx->pCurPps->bScalingMatrixPresent=%d\n",
	//		hCtx->pCurSps->nProfileIdc, hCtx->pCurSps->bScalingMatrixPresent, hCtx->pCurPps->bScalingMatrixPresent);

	if(hCtx->pCurSps->nProfileIdc==100 || hCtx->pCurSps->nProfileIdc==118 || hCtx->pCurSps->nProfileIdc==128)
	{   
		if(hCtx->pCurSps->bScalingMatrixPresent==1 || hCtx->pCurPps->bScalingMatrixPresent==1)
		{   
			H264ConfigureQuantMatrixRegister(h264DecCtx, hCtx);
			shs_qp_reg1c.scaling_matix_flat_flag = 0;
		}
	}
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SHS_QP,*pdwTemp);
	//logv("0x1c: %x\n", *pdwTemp);
	
	// configure 0x50 register
	pdwTemp = (uint32_t*)(&mb_field_intra_info_addr_reg50);
	*pdwTemp = 0;
	mb_field_intra_info_addr_reg50.mb_field_intra_info_addr = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->pMbFieldIntraBuf);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_MB_FIELD_INTRA_INFO_ADDR,*pdwTemp);
	//logv("0x50: %x\n", *pdwTemp);
	
	// configure 0x54 register
	pdwTemp = (uint32_t*)(&mb_neighbor_info_addr_reg54);
	*pdwTemp = 0;
	mb_neighbor_info_addr_reg54.mb_neighbor_info_addr = (uint32_t)(*MemGetPhysicAddress)((void*)hCtx->pMbNeighborInfoBuf);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_MB_NEIGHBOR_INFO_ADDR,*pdwTemp);
	//logv("0x54: %x\n", *pdwTemp);
	
	if(hCtx->nCurSliceNum > 1)
	{   
		goto start_decode_slice;
	}
	
	//configure 0x2c register
	pdwTemp = (uint32_t*)(&cur_mb_num_reg2c);
	*pdwTemp = 0;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_CUR_MB_NUM,*pdwTemp);
	//logv("0x2c: %x\n", *pdwTemp);

	// configure 0x60 register
	pdwTemp = (uint32_t*)(&mb_addr_reg60);
	*pdwTemp = 0;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_MB_ADDR,*pdwTemp);
	//logv("0x60: %x\n", *pdwTemp);
	
	pCurPicturePtr = hCtx->frmBufInf.pCurPicturePtr;
	//0x4c
	pdwTemp = (uint32_t*)(&shs_recon_frmbuf_index_reg4c);
	*pdwTemp = 0;
    shs_recon_frmbuf_index_reg4c.cur_reconstruct_frame_buf_index  = pCurPicturePtr->nDecodeBufIndex;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SHS_RECONSTRUCT_FRM_BUF_INDEX,*pdwTemp);
	//logv("0x4c: %x\n", *pdwTemp);

	
    maxFrmNum = hCtx->frmBufInf.nMaxValidFrmBufNum;
    if(maxFrmNum >= 18)
    {
    	maxFrmNum = 18;
    }

    for(i=0; i<maxFrmNum; i++)
    {
    	//logv("H264ConfigureReconMvcolBufRegister:i=%d\n", i);
    	pCurPicturePtr = &(hCtx->frmBufInf.picture[i]);
    	if(pCurPicturePtr->pVPicture != NULL)
    	{
    		//logd("configure decode frame: H264ConfigureReconMvcolBufRegister\n");
    		H264ConfigureReconMvcolBufRegister(h264Dec,pCurPicturePtr);
    	}
    }

	pCurPicturePtr = hCtx->frmBufInf.pCurPicturePtr;
    H264ConfigureFrameInfoRegister(h264Dec,pCurPicturePtr);

    H264ConfigureRefListRegister(h264Dec, hCtx);
start_decode_slice:

    H264EnableIntr(h264DecCtx);

#if 0
    if(decStreamIndex==0 && hCtx->nCurFrmNum==0)
    {
    	H264PrintRegister(h264DecCtx);
    }
#endif
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, DECODE_SLICE);
}
/****************************************************************************************************/
/****************************************************************************************************/


void H264ConfigureAvcRegister( H264DecCtx* h264DecCtx, H264Context* hCtx, uint8_t eptbDetectEnable, uint32_t nBitsLen)
{   
    volatile uint32_t dwVal;
	H264Dec* h264Dec = NULL;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
    vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL);
	dwVal &= ~STARTCODE_DETECT_E;
	dwVal |= EPTB_DETECTION_BY_PASS;

	if(eptbDetectEnable ==1)
	{
		dwVal &= ~EPTB_DETECTION_BY_PASS;
	}
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_CTRL, dwVal);
	
	H264ConfigureBitstreamRegister(h264DecCtx, hCtx, nBitsLen);
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, INIT_SWDEC);
}

void H264SyncByte(H264DecCtx *h264DecCtx)
{ 
    H264Dec* h264Dec = NULL;
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, SYNC_BYTE);
}

uint32_t H264GetBits(H264DecCtx* h264DecCtx, uint8_t len)
{
    volatile uint32_t dwVal;
    H264Dec* h264Dec = NULL;
	uint32_t value = 0;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, (len<<8)|GET_BITS);
	
	while(1) 
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
		if(dwVal & VLD_BUSY)
		{
			if(dwVal&0x4)
			{
				value = 0;
				break;
			}
		}
		else
		{   
			vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_BASIC_BITS_RETURN_DATA);
			value = dwVal;
			break;
		}
	}
	return value;
}


uint32_t H264ShowBits(H264DecCtx *h264DecCtx, uint32_t len)
{
	volatile uint32_t dwVal;
    H264Dec* h264Dec = NULL;
	uint32_t value = 0;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, (len<<8)|SHOW_BITS);
	
	while(1) 
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
		if(dwVal & VLD_BUSY)
		{
			if(dwVal&0x4)
			{
				value = 0;
				break;
			}
		}
		else
		{   
			vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_BASIC_BITS_RETURN_DATA);
			value = dwVal;
			break;
		}
	}
	return value;
}

uint32_t H264GetUeGolomb(H264DecCtx* h264DecCtx)
{
    volatile uint32_t dwVal;
    H264Dec* h264Dec = NULL;
	uint32_t value = 0;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, GET_VLCUE);
	
	while(1) 
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
		if(dwVal & VLD_BUSY)
		{
			if(dwVal&0x4)
			{   
				value = 0;
				break;
			}
		}
		else
		{   
			vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_BASIC_BITS_RETURN_DATA);
			value = dwVal;
			break;
		}
	}
	return value;
}

int32_t H264GetSeGolomb(H264DecCtx* h264DecCtx)
{
    volatile uint32_t dwVal;
    H264Dec* h264Dec = NULL;
	uint32_t value = 0;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_TRIGGER_TYPE, GET_VLCSE);
	
	while(1) 
	{
		vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
		if(dwVal & VLD_BUSY)
		{
			if(dwVal&0x4)
			{
				value = 0;
				break;
			}
		}
		else
		{   
			vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_BASIC_BITS_RETURN_DATA);
			value = dwVal;
			break;
		}
	}
	return value;
}


uint32_t H264GetDecodeMbNum(H264DecCtx* h264DecCtx)
{
	H264Dec* h264Dec = NULL;
	volatile uint32_t dwVal;
	
	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_CUR_MB_NUM);
	return dwVal;
}
	
uint32_t H264GetFunctionStatus(H264DecCtx* h264DecCtx)
{
	H264Dec* h264Dec = NULL;
	volatile uint32_t dwVal;
	
	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);
	return dwVal;
}

uint32_t H264VeIsr(H264DecCtx* h264DecCtx)
{   
    H264Dec* h264Dec = NULL;
	volatile uint32_t dwVal;
	uint32_t decode_result = 0;

	h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	decode_result  = 0;
    vdec_readuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS);

    if((dwVal&0x02) > 0)
    {
        decode_result |= H264_IR_ERR;
       // logw("ve dec error.\n");
    }
    if((dwVal&0x01) > 0)
    {
        decode_result |= H264_IR_FINISH;
    }
    if((dwVal&0x04) > 0)
    {
        decode_result |= H264_IR_DATA_REQ;
    }
    
    vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_FUNC_STATUS,dwVal);
    return decode_result;
}


void H264CongigureWeightTableRegisters(H264DecCtx* h264DecCtx, H264Context* hCtx) 
{   
	uint32_t weight = 0;
	int32_t i = 0;
	uint32_t j = 0;
	H264Dec* h264Dec = NULL;
	uint32_t lumaLog2WeightDenom = 0;
	uint32_t chromaLog2WeightDenom = 0;
   	volatile uint32_t* pdwTemp;
	
    h264Dec = (H264Dec*)h264DecCtx->pH264Dec;
	
	if((hCtx->pCurPps->nWeightedBIpredIdc==2)&& (hCtx->nSliceType==H264_B_TYPE))
	{
		lumaLog2WeightDenom = 5;
		chromaLog2WeightDenom = 5;
	}
	else
	{
		lumaLog2WeightDenom = H264GetUeGolomb(h264DecCtx);
		chromaLog2WeightDenom = H264GetUeGolomb(h264DecCtx);
	}
		
	pdwTemp = (uint32_t*)(&shs_wp_reg10);
	*pdwTemp = 0;
	shs_wp_reg10.luma_log2_weight_denom = lumaLog2WeightDenom;
	shs_wp_reg10.chroma_log2_weight_denom = chromaLog2WeightDenom;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SHS_WP,*pdwTemp);
	
	//logv("0x10:%x\n", *pdwTemp);

	pdwTemp = (uint32_t*)(&sram_port_rw_offset_rege0);
	*pdwTemp = 0;
	vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,*pdwTemp);
	//logv("32:0xe0:%x\n", *pdwTemp);
	
	for(j=0; j<2; j++)
	{
		weight = 1<<lumaLog2WeightDenom;
		for(i=0; i<32; i++)
		{
			vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
		}
		//logv("32:0xe4:%x\n", weight);
		weight = 1<<chromaLog2WeightDenom;
		for(i=0; i<64; i++)
		{
			vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
		}
		//logv("64:0xe4:%x\n", weight);
	}
	
	if((hCtx->pCurPps->bWeightedPred&& hCtx->nSliceType==H264_P_TYPE) ||
			(hCtx->pCurPps->nWeightedBIpredIdc==1 && hCtx->nSliceType==H264_B_TYPE))
	{
		for(i=0; i<hCtx->nRefCount[0]; i++)
		{   
			if(H264GetBits(h264DecCtx,1))
			{   
				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,i<<2);
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe0:%x\n", i<<2);
				//logv("0xe4:%x\n", weight);
			}
			
			if(H264GetBits(h264DecCtx,1))
			{   
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,32*4+(i<<3));
				//logv("0xe0:%x\n", 32*4+(i<<3));
				
				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe4:%x\n", weight);
			    ////logv("0xe4:%x\n", weight);
				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe4:%x\n", weight);
			}
		}
	}
	
	if(hCtx->nSliceType==H264_B_TYPE && hCtx->pCurPps->nWeightedBIpredIdc==1)
	{
		for(i=0; i<hCtx->nRefCount[1]; i++)
		{   
			if(H264GetBits(h264DecCtx,1))
			{   
				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,32*3*4+(i<<2));
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe0:%x\n", 32*3*4+(i<<2));
				//logv("0xe4:%x\n", weight);
			}
			if(H264GetBits(h264DecCtx,1))
			{   
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_OFFSET,32*3*4+32*4+(i<<3));
				//logv("0xe0:%x\n",32*3*4+32*4+(i<<3));
		
				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe4:%x\n", weight);

				weight = H264GetSeGolomb(h264DecCtx)&0xffff;
				weight |= (H264GetSeGolomb(h264DecCtx)&0xffff)<<16;
				vdec_writeuint32(h264Dec->nRegisterBaseAddr+REG_SRAM_PORT_RW_DATA,weight);
				//logv("0xe4:%x\n", weight);
			}
		}
	}
	return;
}



