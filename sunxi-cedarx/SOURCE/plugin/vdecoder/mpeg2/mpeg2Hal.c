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
#include "mpeg2Hal.h"

//***************************************************************************************/
//**************************************************************************************//
void Mpeg2SetQuantMatrix(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bIntraFlag)
{
    uint8_t i = 0;
    uint32_t uValue = 0;

    for(i=0; i<64; i++)
    {
        uValue = (i << 8);
        if(bIntraFlag == 1)
        {
            uValue |= pMpeg2Dec->picInfo.aIntraQuartMatrix[i];
            uValue |= 0x4000;
        }
        else
        {
            uValue |= pMpeg2Dec->picInfo.aNonIntraQuartMatrix[i];
        }
        vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+IQMINPUT_REG, uValue);
    }
}

/*******************************************************************************************/
/*******************************************************************************************/

int8_t Mpeg2SetHwStartCodeInfo(Mpeg2DecodeInfo* pMpeg2Dec)
 {
     volatile uint32_t* pPdwTemp;
     uint32_t uSbmBufPhyAddr   = 0;
     uint32_t uSbmByteHighAddr = 0;
     uint32_t uSbmByteLowAddr  = 0;
     // set MB address information (Macro Block Address Register)
     pPdwTemp = (uint32_t*)(&mbaddr_reg10);
     *pPdwTemp = 0;
 
     //clear error case register
     pPdwTemp = (uint32_t*)(&errflag_regc4);
     *pPdwTemp = 0;
 
     //clear correct mb counter register
     pPdwTemp = (uint32_t*)(&crtmb_regc8);
     *pPdwTemp = 0;
 
     // set video engine start trigger information
     pPdwTemp = (uint32_t*)(&vetrigger_reg18);
     *pPdwTemp = 0;
     if(pMpeg2Dec->picInfo.bIsMpeg1Flag == 1)
         vetrigger_reg18.dec_format = 1;                 // MPEG-1 format
     else
         vetrigger_reg18.dec_format = 2;                 // MPEG-2 format
     vetrigger_reg18.chrom_format = 0;                 // 4:2:0 format
 
     // set VLD VBV start address information
     pPdwTemp = (uint32_t*)(&vldbaddr_reg28);
     *pPdwTemp = 0;
     pPdwTemp = (uint32_t*)(&vldoffset_reg2c);
     *pPdwTemp = 0;
     pPdwTemp = (uint32_t*)(&vldlen_reg30);
     *pPdwTemp = 0;
     pPdwTemp = (uint32_t*)(&vbvsize_reg34);
 
     uSbmBufPhyAddr = (uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBuf);
     uSbmByteHighAddr = (uSbmBufPhyAddr & 0x0fffffff) >> 4;
     uSbmByteLowAddr =  (uSbmBufPhyAddr & 0xf0000000) >> 28;

     vldbaddr_reg28.vld_byte_start_addr = (uSbmByteHighAddr<<4) | uSbmByteLowAddr;
     vbvsize_reg34.vld_byte_end_addr  =  (uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBufEnd);
     // write register to hardware
     pPdwTemp = (uint32_t*)(&mphr_reg00);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+MPHR_REG,*pPdwTemp);
     pPdwTemp = (uint32_t*)(&mbaddr_reg10);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+MBADDR_REG,*pPdwTemp);
     pPdwTemp = (uint32_t*)(&vectrl_reg14);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VECTRL_REG,*pPdwTemp);
 
     pPdwTemp = (uint32_t*)(&errflag_regc4);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+ERRFLAG_REG,*pPdwTemp);
     pPdwTemp = (uint32_t*)(&crtmb_regc8);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+CRTMBADDR_REG,*pPdwTemp);
 
     vetrigger_reg18.ve_start_type = 0;
     pPdwTemp = (uint32_t*)(&vetrigger_reg18);
     vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VETRIGGER_REG,*pPdwTemp);
     return 0;
 }

/**********************************************************************************/
/**********************************************************************************/
int8_t Mpeg2SetPictureSize(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    volatile uint32_t* pPdwTemp;
    uint32_t uValue = 0;
    
    if(pMpeg2Dec->picInfo.eCurPicStructure != MP2VDEC_FRAME)
    {
        uValue = (pMpeg2Dec->picInfo.nMbXNum<<8)|(pMpeg2Dec->picInfo.nMbYNum>>1);
    }
    else
    {
        uValue = (pMpeg2Dec->picInfo.nMbXNum<<8)|(pMpeg2Dec->picInfo.nMbYNum);
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+FSIZE_REG,uValue);
    pPdwTemp = (uint32_t*)(&picsize_reg0c);
   *pPdwTemp = 0;
    picsize_reg0c.pic_boundary_width = pMpeg2Dec->picInfo.nMbWidth;
	picsize_reg0c.pic_boundary_height = pMpeg2Dec->picInfo.nMbHeight;
	if(pMpeg2Dec->picInfo.eCurPicStructure != MP2VDEC_FRAME)
	{
        picsize_reg0c.pic_boundary_height >>= 1;
	}
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+PICSIZE_REG,*pPdwTemp);
    return 0;
}


/************************************************************************************************/
/************************************************************************************************/
void Mpeg2SetDisplayBuf(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    if(pMpeg2Dec->picInfo.bOnlyDispKeyFrmFlag == 0)
    {
        if(pMpeg2Dec->picInfo.eCurPicType != MP2VDEC_B_PIC)
        {   
            pMpeg2Dec->frmBufInfo.pLastRefFrm = pMpeg2Dec->frmBufInfo.pForRefFrm;
            pMpeg2Dec->frmBufInfo.pForRefFrm = pMpeg2Dec->frmBufInfo.pBacRefFrm;
            pMpeg2Dec->frmBufInfo.pBacRefFrm = pMpeg2Dec->frmBufInfo.pCurFrm;
        }
    }
}
/************************************************************************************************/
/************************************************************************************************/


int8_t Mpeg2SetPictureInfo(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    volatile uint32_t* pPdwTemp;
    uint32_t uSbmBufPhyAddr;
    uint32_t uSbmByteHighAddr;
    uint32_t uSbmByteLowAddr;
    pPdwTemp = (uint32_t*)(&mphr_reg00);
	*pPdwTemp = 0;

	mphr_reg00.pic_coding_type = pMpeg2Dec->picInfo.eCurPicType;

    if(pMpeg2Dec->picInfo.bIsMpeg1Flag == 1)
    {
        mphr_reg00.f_code00          = pMpeg2Dec->picInfo.uForFCode;
		mphr_reg00.f_code01          = pMpeg2Dec->picInfo.uForFCode;
		mphr_reg00.f_code10          = pMpeg2Dec->picInfo.uBacFCode;
		mphr_reg00.f_code11          = pMpeg2Dec->picInfo.uBacFCode;
		mphr_reg00.intra_dc_precision = 0;		// 8-bits precision
		mphr_reg00.pic_structure     = 3;			// frame picture
		mphr_reg00.frame_pred_dct    = 1;			// frame pred frame dct
		mphr_reg00.top_field_first   = 1;			// regardless bit
		mphr_reg00.con_motion_vec    = 0;			// no concealment motion vectors
		mphr_reg00.q_scale_type      = 0;			// linear quantization scale
		mphr_reg00.intra_vlc_format  = 0;		// select Table B-14 as DCT vlc table
		mphr_reg00.alter_scan        = 0;				// Zig-zag scan
		mphr_reg00.fp_f_vec          = pMpeg2Dec->picInfo.uFullPerForVector;
		mphr_reg00.fp_b_vec          = pMpeg2Dec->picInfo.uFullPerBacVector;
    }
    else
    {
        mphr_reg00.f_code00          = pMpeg2Dec->picInfo.uFCode00;
		mphr_reg00.f_code01          = pMpeg2Dec->picInfo.uFCode01;
		mphr_reg00.f_code10          = pMpeg2Dec->picInfo.uFCode10;
		mphr_reg00.f_code11          = pMpeg2Dec->picInfo.uFCode11;
		mphr_reg00.intra_dc_precision = pMpeg2Dec->picInfo.uIntraDcPrecision;
		mphr_reg00.pic_structure     = pMpeg2Dec->picInfo.eCurPicStructure;
		if(pMpeg2Dec->picInfo.eCurPicStructure == MP2VDEC_FRAME)
		{
            mphr_reg00.top_field_first = pMpeg2Dec->picInfo.bTopFieldFstFlag;
		}
		else if(pMpeg2Dec->picInfo.eCurPicStructure == MP2VDEC_TOP_FIELD)
		{
            mphr_reg00.top_field_first = (pMpeg2Dec->picInfo.bFstFieldFlag==1)?1:0;
		}
		else if(pMpeg2Dec->picInfo.eCurPicStructure == MP2VDEC_BOT_FIELD)
		{
            mphr_reg00.top_field_first = (pMpeg2Dec->picInfo.bFstFieldFlag==1)?0:1;
        }
		mphr_reg00.frame_pred_dct     = pMpeg2Dec->picInfo.uFrmPredFrmDet;
		mphr_reg00.con_motion_vec     = pMpeg2Dec->picInfo.uConcealMotionVectors;
		mphr_reg00.q_scale_type       = pMpeg2Dec->picInfo.uQScaleType;
		mphr_reg00.intra_vlc_format   = pMpeg2Dec->picInfo.uIntraVlcFormat;
		mphr_reg00.alter_scan         = pMpeg2Dec->picInfo.uAlternateScan;
		mphr_reg00.fp_f_vec           = 0;				// half pixels unit
		mphr_reg00.fp_b_vec           = 0;				// half pixels unit
    }
    // set MB address information (Macro Block Address Register)
	pPdwTemp = (uint32_t*)(&mbaddr_reg10);
	*pPdwTemp = 0;

	// set video engine control information
	pPdwTemp = (uint32_t*)(&vectrl_reg14);
	*pPdwTemp &= 0x78;
    
    vectrl_reg14.mc_cache_enable = 0;
    if(pMpeg2Dec->nVeVersion > 0x1619)
    {
        vectrl_reg14.mc_cache_enable = 1;
    }

    vectrl_reg14.not_write_recons_flag = 1;

	//set frame buffer control register
	if((pMpeg2Dec->picInfo.eCurPicStructure==MP2VDEC_FRAME)||(pMpeg2Dec->picInfo.bFstFieldFlag==1))
	{
        Mpeg2SetDisplayBuf(pMpeg2Dec);
    }
	
	//clear error case register
	pPdwTemp = (uint32_t*)(&errflag_regc4);
	*pPdwTemp = 0;

	//clear correct mb counter register
	pPdwTemp = (uint32_t*)(&crtmb_regc8);
	*pPdwTemp = 0;

	// set video engine start trigger information
	pPdwTemp = (uint32_t*)(&vetrigger_reg18);
	*pPdwTemp = 0;
	if(pMpeg2Dec->picInfo.bIsMpeg1Flag == 1)
	{
        vetrigger_reg18.dec_format = 1;                 // MPEG-1 format
	}
	else
	{
        vetrigger_reg18.dec_format = 2;                 // MPEG-2 format
	}
	vetrigger_reg18.chrom_format = 0;				   // 4:2:0 format

	// set VLD VBV start address information
	pPdwTemp = (uint32_t*)(&vldbaddr_reg28);
	*pPdwTemp = 0;
	pPdwTemp = (uint32_t*)(&vldoffset_reg2c);
	*pPdwTemp = 0;
	pPdwTemp = (uint32_t*)(&vldlen_reg30);
	*pPdwTemp = 0;
	pPdwTemp = (uint32_t*)(&vbvsize_reg34);
    
    if(pMpeg2Dec->nVeVersion == 0x1618)
    {
       *pPdwTemp = 0;
       vldbaddr_reg28.vld_byte_start_addr = (uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBuf) & 0x7ffffffffLL;
       vbvsize_reg34.vld_byte_end_addr = (uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBufEnd) & 0x7ffffffffLL;
    }
    else
    {   
        uSbmBufPhyAddr = (uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBuf);
        uSbmByteHighAddr = (uSbmBufPhyAddr & 0x0fffffff) >> 4;
        uSbmByteLowAddr =  (uSbmBufPhyAddr & 0x70000000) >> 28;
        vldbaddr_reg28.vld_byte_start_addr = (uSbmByteHighAddr<<4) | uSbmByteLowAddr;
        vbvsize_reg34.vld_byte_end_addr  = ((uint32_t)AdapterMemGetPhysicAddress((void*)pMpeg2Dec->sbmInfo.pSbmBufEnd) & 0x7fffffffLL);
    }
    
	// write register to hardware
	pPdwTemp = (uint32_t*)(&mphr_reg00);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+MPHR_REG,*pPdwTemp);
	pPdwTemp = (uint32_t*)(&mbaddr_reg10);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+MBADDR_REG,*pPdwTemp);
	pPdwTemp = (uint32_t*)(&vectrl_reg14);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VECTRL_REG,*pPdwTemp);

	pPdwTemp = (uint32_t*)(&errflag_regc4);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+ERRFLAG_REG,*pPdwTemp);
	pPdwTemp = (uint32_t*)(&crtmb_regc8);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+CRTMBADDR_REG,*pPdwTemp);

	vetrigger_reg18.ve_start_type = 0;
	pPdwTemp = (uint32_t*)(&vetrigger_reg18);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VETRIGGER_REG,*pPdwTemp);

    return 0;
}

/***********************************************************************************************/
/***********************************************************************************************/

void Mpeg2SetReconstructBuf(Mpeg2DecodeInfo* pMpeg2Dec)
{
    volatile uint32_t* pPdwTemp;
    pPdwTemp = (uint32_t*)(&reyaddr_reg48);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pCurFrm != NULL)
    {
    	reyaddr_reg48.re_ybuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyYBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+REYADDR_REG,*pPdwTemp);
    
    pPdwTemp = (uint32_t*)(&recaddr_reg4c);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pCurFrm != NULL)
    {
    	recaddr_reg4c.re_cbuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyCBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+RECADDR_REG,*pPdwTemp);
    
    //forward ref luma/chroma frame buffer address
    pPdwTemp = (uint32_t*)(&frefyaddr_reg50);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pForRefFrm != NULL)
    {
    	frefyaddr_reg50.fref_ybuf_addr = pMpeg2Dec->frmBufInfo.pForRefFrm->phyYBufAddr;
    }
    else
    {
    	frefyaddr_reg50.fref_ybuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyYBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+FREFYADDR_REG,*pPdwTemp);
    
    pPdwTemp = (uint32_t*)(&frefcaddr_reg54);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pForRefFrm != NULL)
    {
    	frefcaddr_reg54.fref_cbuf_addr = pMpeg2Dec->frmBufInfo.pForRefFrm->phyCBufAddr;
    }
    else
    {
    	frefcaddr_reg54.fref_cbuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyCBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+FREFCADDR_REG,*pPdwTemp);
    
    //backward ref luma/chroma frame buffer address
    pPdwTemp = (uint32_t*)(&brefyaddr_reg58);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pBacRefFrm != NULL)
    {
    	brefyaddr_reg58.bref_ybuf_addr = pMpeg2Dec->frmBufInfo.pBacRefFrm->phyYBufAddr;
    }
    else
    {
    	brefyaddr_reg58.bref_ybuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyYBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+BREFYADDR_REG,*pPdwTemp);
    
    pPdwTemp = (uint32_t*)(&brefcaddr_reg5c);
    *pPdwTemp = 0;
    if(pMpeg2Dec->frmBufInfo.pBacRefFrm != NULL)
    {
    	 brefcaddr_reg5c.bref_cbuf_addr = pMpeg2Dec->frmBufInfo.pBacRefFrm->phyCBufAddr;
    }
    else
    {
    	brefcaddr_reg5c.bref_cbuf_addr = pMpeg2Dec->frmBufInfo.pCurFrm->phyCBufAddr;
    }
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+BREFCADDR_REG,*pPdwTemp);
}
/************************************************************************************************/
/***********************************************************************************************/
void Mpeg2SetSbmRegister(Mpeg2DecodeInfo* pMpeg2Dec, uint8_t bFstSetFlag,uint8_t bBoundFlag, uint32_t uStartBitPos, uint32_t uBitLen, uint8_t bDataEndFlag, uint8_t bCheckStartCodeFlag)
{
    volatile uint32_t* pPdwTemp;
    
    vldoffset_reg2c.vld_bit_offset = uStartBitPos;
	pPdwTemp = (uint32_t*)(&vldoffset_reg2c);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VLDOFFSET_REG,*pPdwTemp);

	vldlen_reg30.vld_bit_len = uBitLen;
	pPdwTemp = (uint32_t*)(&vldlen_reg30);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VLDLEN_REG,*pPdwTemp);

    pPdwTemp = (uint32_t*)(&vbvsize_reg34);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VBVENDADDR_REG,*pPdwTemp);
        
	vldbaddr_reg28.vbv_buff_data_first = bFstSetFlag;
	if(bDataEndFlag == 1)
	{
        vldbaddr_reg28.vbv_buff_data_last = 1;
	}
	else
	{
        vldbaddr_reg28.vbv_buff_data_last = 0;
	}

	vldbaddr_reg28.vbv_buff_data_valid = 1;
	pPdwTemp = (uint32_t*)(&vldbaddr_reg28);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VLDBADDR_REG,*pPdwTemp);

  
    vectrl_reg14.ve_error_int_en = 1;
    vectrl_reg14.ve_finish_int_en = 1;
    vectrl_reg14.vld_mem_req_int_en = 1;
    pPdwTemp = (uint32_t*)(&vectrl_reg14);
	vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VECTRL_REG,*pPdwTemp);    
    
    if(bCheckStartCodeFlag == 1)
    {
        vetrigger_reg18.ve_start_type = 0x8;	//start hardware VLD, picture level
        vetrigger_reg18.stcd_type   =  0x1;
		vetrigger_reg18.mb_boundary = bBoundFlag;
		pPdwTemp = (uint32_t*)(&vetrigger_reg18);
		vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VETRIGGER_REG,*pPdwTemp);
    }
	else if(bFstSetFlag == 1)
	{   
        #if 0
        {   
		    int i;
		    uint32_t *reg_base = NULL;

            reg_base = (uint32_t*)ve_get_reglist(REG_GROUP_VETOP);

            for(i=0;i<16;i++)
            {
			  logd("%d, 0x%08x 0x%08x 0x%08x 0x%08x \n",i, reg_base[4*i],reg_base[4*i+1],
					reg_base[4*i+2],reg_base[4*i+3]);
		     }
            
            reg_base = (uint32_t*)pMpeg2Dec->uRegisterBaseAddr;

            logd("*******************\n");
            logd("*******************\n");
		    for(i=0;i<16;i++)
            {
			  logd("%d, 0x%08x 0x%08x 0x%08x 0x%08x \n",i, reg_base[4*i],reg_base[4*i+1],
					reg_base[4*i+2],reg_base[4*i+3]);
		     }
            logd("*******************\n");
            logd("*******************\n");
        }
        #endif
        
		vetrigger_reg18.ve_start_type = 0xf;			//start hardware VLD, picture level
		vetrigger_reg18.mb_boundary = bBoundFlag;
		pPdwTemp = (uint32_t*)(&vetrigger_reg18);
		vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VETRIGGER_REG,*pPdwTemp);
	}
}

/***********************************************************************************************/
/***********************************************************************************************/
int8_t Mpeg2CheckVeIdle(Mpeg2DecodeInfo* pMpeg2Dec)
{
    volatile uint32_t uDwVal;
    vdec_readuint32(pMpeg2Dec->uRegisterBaseAddr+VESTAT_REG);
    if(uDwVal&0x3f00)
    {   
        return -1;
    }
    return 0;
}


/**********************************************************************************************/
/**********************************************************************************************/
uint32_t Mpeg2GetStartcodeBitOffet(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    volatile uint32_t uDwVal;
    vdec_readuint32(pMpeg2Dec->uRegisterBaseAddr+STCDdHWBITOFFSET_REG);
    return (uint32_t)uDwVal;
}

/***********************************************************************************************/
/***********************************************************************************************/
uint32_t Mpeg2GetDecodeBitOffset(Mpeg2DecodeInfo* pMpeg2Dec)
{
    volatile uint32_t uDwVal=0;
    vdec_readuint32(pMpeg2Dec->uRegisterBaseAddr+VLDOFFSET_REG);
	return uDwVal;
}

/*******************************************************************************************/
/*******************************************************************************************/
uint32_t Mpeg2VeIsr(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    #define MPEG2_RESULT_OK  	 0
    #define MPEG2_RESULT_FINISH	 1
    #define MPEG2_RESULT_ERR	 2
    #define MPEG2_RESULT_MEM_REQ 4
    uint32_t uDecodeResult;
    volatile uint32_t uDwVal;
    volatile uint32_t* pPdwTemp;
	uDecodeResult  = 0;
    vdec_readuint32(pMpeg2Dec->uRegisterBaseAddr+VESTAT_REG);
    
    vestat_reg1c.ve_error = 0;
    vestat_reg1c.ve_finish = 0;
    vestat_reg1c.vld_mem_req = 0;
    
    if((uDwVal&0x02) > 0)
    {
        uDecodeResult |= MPEG2_RESULT_ERR;
        vestat_reg1c.ve_error = 1;
        logv("ve dec error.\n");
    }
    if((uDwVal&0x01) > 0)
    {
        uDecodeResult |= MPEG2_RESULT_FINISH;
        vestat_reg1c.ve_finish = 1;
    }
    if((uDwVal&0x04) > 0)
    {
        uDecodeResult |= MPEG2_RESULT_MEM_REQ;
        vestat_reg1c.vld_mem_req = 1;
    }
    pPdwTemp = (uint32_t*)(&vestat_reg1c);
    vdec_writeuint32(pMpeg2Dec->uRegisterBaseAddr+VESTAT_REG,*pPdwTemp);
    return uDecodeResult;
}

/*****************************************************************************/
/*****************************************************************************/

uint32_t Mpeg2GetRegVal(uint32_t uRegAddr)
{    
     volatile uint32_t uDwVal;
     vdec_readuint32(uRegAddr);
     return uDwVal;
}

/*****************************************************************************/
/*****************************************************************************/

uint32_t Mpeg2GetVersion(Mpeg2DecodeInfo* pMpeg2Dec)
{   
    volatile uint32_t uDwVal;
    uint32_t uMaccRegsBase = 0;
    uint32_t nVeVersion    = 0;
    
    uMaccRegsBase = (uint32_t)AdapterVeGetBaseAddress();
    vdec_readuint32(uMaccRegsBase+MACC_VE_VERSION);
    nVeVersion = uDwVal;
    
    pMpeg2Dec->nVeVersion = (nVeVersion >> 16);
    return 0;
}




 

