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

extern int32_t InitJpegHw(JpegDec* jpgctx);
extern int32_t SetJpegFormat(JpegDec* jpgctx);
extern int32_t JpegHwDec(JpegDec* jpgctx);

__inline uint32_t GetByte(JpegDec* p, uint32_t nBits)
{
	uint32_t i;
	int32_t ret = 0;
	for(i=0; i<nBits; i+=8)
	{
		ret <<= 8;
		ret |= *p->data++;
		if(p->data >= (p->pVbvBase + p->nVbvSize))
			p->data = p->pVbvBase;
		p->nFrameSize--;
    }
    
	return ret;
}

#define INPUT_BYTE(x) 	(x=GetByte(jpgctx,1*8))
#define INPUT_2BYTES(x)	(x=GetByte(jpgctx,2*8))

__inline void JpegSkipVariable (JpegDec* jpgctx)
{
	int32_t length;
	
	INPUT_2BYTES(length);
	length -= 2;
	
	if (length > 0)
	{
		int32_t i;

		for(i=0;i<(length>>2);i++)
		{
			GetByte(jpgctx, 4*8);
		}

		for(i=0;i<(length&3);i++)
		{
			GetByte(jpgctx, 1*8);
		}
	}
}

int32_t JpegFirstMarker(JpegDec* jpgctx)
{
	uint8_t c, c2;
	
	INPUT_BYTE(c);
	INPUT_BYTE(c2);
	if (c != 0xFF || c2 != M_SOI)
		return 0;
		
	jpgctx->unreadMarker = c2;
	
	return 1;
}

int32_t JpegNextMarker(JpegDec* jpgctx)
{
	uint8_t c;
	
	for (;;) 
	{
		INPUT_BYTE(c);
		while (c != 0xFF) 
		{
			INPUT_BYTE(c);
		}
		do
		{
			INPUT_BYTE(c);
		} while (c == 0xFF);
		if (c != 0)
			break;			/* found a valid marker, exit loop */
	}
	jpgctx->unreadMarker = c;
	
	return 1;
}

__inline int32_t JpegGetSoi(JpegDec* jpgctx)
{
	if (jpgctx->sawSOI)
		return 0;
	
	jpgctx->sawSOI = 1;
	
	return 1;
}



int32_t JpegMallocFrmBuffer(MjpegDecodeContext* pMjpegContext, JpegDec* jpgctx)
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t jpegFormat = 0;
	uint32_t lineStride = 0;
    uint32_t nRefPixelFormat = 0;
    uint32_t nDispPixelFormat = 0;
	uint8_t  JpegFormat[6] = {PIXEL_FORMAT_YUV_MB32_420, PIXEL_FORMAT_YUV_MB32_422,
			            PIXEL_FORMAT_YUV_MB32_422, PIXEL_FORMAT_YUV_MB32_422,
			             PIXEL_FORMAT_YUV_MB32_422, PIXEL_FORMAT_YUV_MB32_420};
	int32_t nAlignValue = 0;
    int32_t nProgressiveFlag = 1;
    FbmCreateInfo mFbmCreateInfo;

	#define MJPEG_FRM_BUF_NUM 8

	width = jpgctx->nWidth;
	height = jpgctx->nHeight;

    nRefPixelFormat = JpegFormat[jpgctx->jpegFormat];
    nDispPixelFormat = JpegFormat[jpgctx->jpegFormat];

    jpgctx->pFbm = NULL;

    memset(&mFbmCreateInfo, 0, sizeof(FbmCreateInfo));
    mFbmCreateInfo.nFrameNum          = MJPEG_FRM_BUF_NUM;
    mFbmCreateInfo.nWidth             = width;
	mFbmCreateInfo.nHeight            = height;
	mFbmCreateInfo.ePixelFormat       = nRefPixelFormat;
	mFbmCreateInfo.nBufferType        = BUF_TYPE_REFERENCE_DISP;
	mFbmCreateInfo.bProgressiveFlag   = nProgressiveFlag;

	jpgctx->pFbm = FbmCreate(&mFbmCreateInfo);

	if(jpgctx->pFbm == NULL)
	{
		return VDECODE_RESULT_UNSUPPORTED;
	}
    if(jpgctx->nDecStramIndex == 0)
    {
    	pMjpegContext->pFbm = jpgctx->pFbm;
    }
    return VDECODE_RESULT_OK;
}


int32_t JpegGetSof(JpegDec* jpgctx)
{
	int32_t 		length = 0;
	uint8_t			data_precision = 0;
	int32_t 		ci = 0;
	int32_t   	    c = 0;
	int32_t         ret = 0;
	JpegComponentInfo*	compptr;
	
	INPUT_2BYTES(length);
	INPUT_BYTE(data_precision);
	
	INPUT_2BYTES(jpgctx->nHeight);
	INPUT_2BYTES(jpgctx->nWidth);

	INPUT_BYTE(jpgctx->nNumComponents);
	length -= 8;
	
	if (jpgctx->sawSOF)
		return 0;
	
	if (data_precision != BITS_IN_JSAMPLE)
		return 0;
	
	if (jpgctx->nNumComponents > MAX_COMPONENTS)
		return 0;

	if (length != (jpgctx->nNumComponents * 3))
		return 0;
	
	for (ci = 0; ci < jpgctx->nNumComponents; ci++)
	{
		compptr = &jpgctx->curCompInfo[ci];
		compptr->componentIndex = ci;
		INPUT_BYTE(compptr->componentId);
		INPUT_BYTE(c);
		compptr->hSampFactor = (c >> 4) & 15;
		compptr->vSampFactor = (c     ) & 15;
		INPUT_BYTE(compptr->quantTblNo);
	}
	
	jpgctx->sawSOF = 1;
	return 1;
}

int32_t JpegGetDqt(JpegDec* jpgctx)
{
	int32_t length;
	int32_t n, i,prec;
	uint32_t tmp;
	uint16_t *QtabPtr;
	
	INPUT_2BYTES(length);
	length -= 2;
	
	while (length > 0)
	{
		INPUT_BYTE(n);
		prec = n >> 4;
		n &= 0x0F;
		
		if (n >= NUM_QUANT_TBLS)
			return 0;
		
		QtabPtr = jpgctx->QTab[n];
		
		for (i = 0; i < DCTSIZE2; i++)
		{
			if (prec)
			{
				INPUT_2BYTES(tmp);
			}
			else 
			{
				INPUT_BYTE(tmp);
			}
			QtabPtr[i] = (uint16_t) tmp;
		}
		
		length -= DCTSIZE2+1;
		if (prec)
			length -= DCTSIZE2;
	}
	
	if (length != 0)
		return 0;
	
	return 1;
}

int32_t JpegGetDht(JpegDec* jpgctx)
{
	int32_t 		length;
	uint8_t 			bits[17];
	int32_t 		i;
	int32_t 		index;
	int32_t 		count;
	int32_t			v;
	int32_t			code_max;
	JHuffTable*	htblptr;
	int32_t 		nb;
	int32_t			code;
	
	INPUT_2BYTES(length);
	length -= 2;
	jpgctx->hasDht = 1;
	
	while (length > 16)
	{
		INPUT_BYTE(index);
		if (index & 0x10)
		{	/* AC table definition */
			index -= 0x10;
			htblptr = &jpgctx->acHuffTbl[index];
		}
		else
		{	/* DC table definition */
			htblptr = &jpgctx->dcHuffTbl[index];
		}
		
		if (index < 0 || index >= NUM_HUFF_TBLS)
			return 0;
		
		bits[0] = 0;
		count = 0;
		for (i = 1; i <= 16; i++)
		{
			htblptr->offset[i-1] = (uint8_t)count;
			INPUT_BYTE(bits[i]);
			count += bits[i];
		}
		
		length -= 1 + 16;
		
		if (count > 256 || count > length)
			return 0;
		
		code_max=0;
		for(i=0;i<count;i++)
		{
            INPUT_BYTE(v);
            if (v > code_max)
                code_max = v;
			htblptr->symbol[i] = (uint8_t)v;
        }
        
		length -= count;
		{
			code = 0;
			for(i=1;i<=16;i++)
			{
				nb = bits[i];
				htblptr->startCode[i-1] = (uint16_t)code;
                code+=nb;
                code <<= 1;
            }
			
			for(i=16;i>=1;i--)
			{
				if(bits[i]==0)
					htblptr->startCode[i-1] = 0xffff;
				else
					break;
			}
        }
	}
	
	if (length != 0)
		return 0;

	return 1;
}

int32_t JpegGetSos(JpegDec* jpgctx)
{
	int32_t length;
	int32_t i, ci, n, c, cc;
	JpegComponentInfo* compptr;
	
	if (!jpgctx->sawSOF)
		return 0;
	
	INPUT_2BYTES(length);
	INPUT_BYTE(n); /* Number of components */
	
	if (length != (n * 2 + 6) || n < 1 || n > MAX_COMPS_IN_SCAN)
		return 0;
	
	jpgctx->nCompsInScan = n;
	
	for (i = 0; i < n; i++)
	{
		INPUT_BYTE(cc);
		INPUT_BYTE(c);
		for (ci = 0; ci < jpgctx->nNumComponents; ci++)
		{
			compptr = &jpgctx->curCompInfo[ci];
			if (cc == compptr->componentId)
				goto id_found;
		}
		return 0;
id_found:
		jpgctx->sosNbBlocks[i] = compptr->hSampFactor * compptr->vSampFactor;
		jpgctx->seqCompId[i] = compptr->componentId;
        jpgctx->sosHCount[i] = (uint8_t)compptr->hSampFactor;
        jpgctx->sosVCount[i] = (uint8_t)compptr->vSampFactor;
		
		compptr->dcTblNo = (c >> 4) & 15;
		compptr->acTblNo = (c     ) & 15;
	}
	
	/* Collect the additional scan parameters Ss, Se, Ah/Al. */
	INPUT_BYTE(c);	
	INPUT_BYTE(cc);
	INPUT_BYTE(ci);
	if(c!=0 || cc!=63 || ci!=0)
	{
		return 0;
	}
	jpgctx->nNextRestartNum = 0;

	return 1;
}

int32_t JpegGetDri(JpegDec* jpgctx)
{
	int32_t length;
	uint32_t tmp;
	
	INPUT_2BYTES(length);
	if (length != 4)
		return 0;
	INPUT_2BYTES(tmp);
	jpgctx->nRestartInterval = tmp;
	
	return 1;
}

int32_t JpegReadMarkers(JpegDec* jpgctx)
{
	jpgctx->hasDht = 0;

	for (;;) 
	{
		if (jpgctx->unreadMarker == 0)
		{
			if (!jpgctx->sawSOI)
			{
				if (!JpegFirstMarker(jpgctx))
					return JPEG_MARKER_ERROR;
			}else
			{
				if(!JpegNextMarker(jpgctx))//not found next mark
					return JPEG_MARKER_ERROR;
			}
		}
		
		switch(jpgctx->unreadMarker) 
		{
		case M_SOI:
			if (!JpegGetSoi(jpgctx))
				return JPEG_MARKER_ERROR;
			break;
			
		case M_SOF0:                /* Baseline */
		case M_SOF1:                /* Extended sequential, Huffman */
			if (!JpegGetSof(jpgctx))
				return JPEG_MARKER_ERROR;
			break;
			/* Currently unsupported SOFn types */
		case M_SOF2:                /* Progressive, Huffman */
		case M_SOF3:                /* Lossless, Huffman */
		case M_SOF5:                /* Differential sequential, Huffman */
		case M_SOF6:                /* Differential progressive, Huffman */
		case M_SOF7:                /* Differential lossless, Huffman */
		case M_JPG:                 /* Reserved for JPEG extensions */
		case M_SOF9:                /* Extended sequential, arithmetic */
		case M_SOF10:               /* Progressive, arithmetic */
		case M_SOF11:               /* Lossless, arithmetic */
		case M_SOF13:               /* Differential sequential, arithmetic */
		case M_SOF14:               /* Differential progressive, arithmetic */
		case M_SOF15:               /* Differential lossless, arithmetic */
			return JPEG_FORMAT_UNSUPPORT;//?
			
		case M_SOS:
			if (!JpegGetSos(jpgctx))
				return JPEG_MARKER_ERROR;
			//no break here
		case M_EOI:
			jpgctx->unreadMarker = 0; /* processed the marker */
			return JPEG_PARSER_OK;
		case M_DHT:
			if (!JpegGetDht(jpgctx))
				return JPEG_MARKER_ERROR;
			break;

		case M_DQT:	
			if (!JpegGetDqt(jpgctx))
				return JPEG_MARKER_ERROR;
			break;

		case M_DRI:
			if (! JpegGetDri(jpgctx))
				return JPEG_MARKER_ERROR;
			break;

		case M_COM : //skip com
		case M_DAC : //D_ARITH_CODING_NOT_SUPPORTED ?
		case M_APP0:
		case M_APP1: //no need to process EXIF for jpeg decoder
		case M_APP2:
		case M_APP3:
		case M_APP4:
		case M_APP5:
		case M_APP6:
		case M_APP7:
		case M_APP8:
		case M_APP9:
		case M_APP10:
		case M_APP11:
		case M_APP12:
		case M_APP13:
		case M_APP14:
		case M_APP15:
		case M_DNL:
			//! may be some marker need process
			JpegSkipVariable (jpgctx);
			break;
			
		case M_RST0: /* these are all parameterless */
		case M_RST1:
		case M_RST2:
		case M_RST3:
		case M_RST4:
		case M_RST5:
		case M_RST6:
		case M_RST7:
		case M_TEM:
			break;
			
		default:  /* must be DHP, EXP, JPGn, or RESn */
			return JPEG_MARKER_ERROR;//?
			//break;
		}
		jpgctx->unreadMarker = 0;
	}
}

int32_t JpegDecoderMain(MjpegDecodeContext* pMjpegContext,  JpegDec* jpgctx)
{	
	int32_t ret;

	jpgctx->sawSOI = 0;
	jpgctx->sawSOF = 0;
	jpgctx->unreadMarker = 0;

	ret = JpegReadMarkers(jpgctx);
	if(JPEG_PARSER_OK != ret)
	{
		return -1;//DEC_ERROR;
	}

	if(!SetJpegFormat(jpgctx))
	{
		return -1;//DEC_ERROR;
	}
	if(jpgctx->pFbm == NULL)
	{
		ret = JpegMallocFrmBuffer(pMjpegContext,jpgctx);
		if(ret !=  VDECODE_RESULT_OK)
		{
			return ret;
		}
	}

	jpgctx->pRefPicture =  FbmRequestBuffer(jpgctx->pFbm);
	if(jpgctx->pRefPicture == NULL)
	{
		return VDECODE_RESULT_NO_FRAME_BUFFER;
	}

	// step: decode the stream data
	jpgctx->nRefPicLumaAddr = jpgctx->pRefPicture->phyYBufAddr;
	jpgctx->nRefPicChromaAddr = jpgctx->pRefPicture->phyCBufAddr;

	if(!InitJpegHw(jpgctx))
	{
		return -1;//DEC_ERROR;
	}
    if(!JpegHwDec(jpgctx))
    {
    	return -1;//DEC_ERROR;
    }
	return ret;
}


void MjpegSetDispParams(MjpegDecodeContext* pMjpegContext, JpegDec* mJpegDec, VideoPicture* pCurPicture)
{

    pCurPicture->nFrameRate  = pMjpegContext->videoStreamInfo.nFrameRate;
    pCurPicture->nAspectRatio  = 1000;
    pCurPicture->nPts = mJpegDec->nPts;
    pCurPicture->bIsProgressive = 1;
}
