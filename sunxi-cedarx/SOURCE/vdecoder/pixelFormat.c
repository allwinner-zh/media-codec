/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 BZ Chen <bzchen@allwinnertech.com>
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
#include <unistd.h>
#include <stdlib.h>
#include "vdecoder.h"
#include "adapter.h"
#include "log.h"

/*******************************************************************************
Function name: map32x32_to_yuv_Y
Description:
    1. we should know : vdecbuf_uv is 32*32 align too
    2. must match gpuBuf_uv size.
    3. gpuBuf_uv size is half of gpuBuf_y size
    4. we guarantee: vdecbufSize>=gpuBufSize
    5. uv's macroblock is also 16*16. Vdec_macroblock is also twomb(32*32).
    6. if coded_width is stride/2, we can support gpu_buf_width 16byte align and 8byte align!
       But need outer set right value of coded_width, must meet gpu_uv_buf_width align's request!
Parameters:
    1. mode = 0:yv12, 1:thumb yuv420p
    2. coded_width and coded_height is uv size, already half of y_size
Return:


*******************************************************************************/



#if 0
void ConvertMb32420ToNv21Y(char* pSrc,char* pDst,int nWidth, int nHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int n = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char *dstAsm = NULL;
    char *srcAsm = NULL;
    char bufferU[32];
    int num1 = 0;
    int num2 = 0;

    nLineStride = (nWidth + 15) &~15;
    nMbWidth = (nWidth+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;
    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nMbWidth; j++)
    	{
    		num1 = nHeight-i*32;
    		num1 = (num1>=32)? 32: num1;

    		//for(m=0; m<32; m++)
    		for(m=0; m<num1; m++)
    		{
#if 0
    			if((i*32 + m) >= nHeight)
    			{
    				ptr += 32;
    				continue;
        		}
#endif

    			dstAsm = bufferU;
    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;

    			 asm volatile (
    					        "vld1.8         {d0 - d3}, [%[srcAsm]]              \n\t"
    					        "vst1.8         {d0 - d3}, [%[dstAsm]]              \n\t"
    					       	: [dstAsm] "+r" (dstAsm), [srcAsm] "+r" (srcAsm)
    					       	:  //[srcY] "r" (srcY)
    					       	: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31");
    			ptr += 32;

#if 0
    			for(k=0; k<32; k++)
    			{
    				if((j*32+ k) >= nLineStride)
    				{
    					break;
    				}
    				pDst[offset+k] = bufferU[k];
    			}
#endif
    			num2 = nLineStride-j*32;
    		   	num2 = (num2>=32)? 32: num2;
    		   	for(k=0; k<num2; k++)
    		   	{
    		  		pDst[offset+k] = bufferU[k];
    		   	}
    		}
    	    ptr += (32-num1)*32;
    	}
    }
}
#endif

void ConvertMb32420ToNv21Y(char* pSrc,char* pDst,int nWidth, int nHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    char* ptr = NULL;
    char *dstAsm = NULL;
    char *srcAsm = NULL;
    char bufferU[32];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nLineStride = (nWidth + 15) &~15;
    nMbWidth = (nWidth+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;
    ptr = pSrc;

    nWidthMatchFlag = 0;
	nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*32 == nLineStride)
    {
    	nWidthMatchFlag = 1;
    	nCopyMbWidth = nMbWidth;

    }
    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nCopyMbWidth; j++)
    	{
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    		  	{
    				ptr += 32;
    		    	continue;
    		  	}
    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;
    			dstAsm = pDst+ offset;

    			 asm volatile (
    					        "vld1.8         {d0 - d3}, [%[srcAsm]]              \n\t"
    					        "vst1.8         {d0 - d3}, [%[dstAsm]]              \n\t"
    					       	: [dstAsm] "+r" (dstAsm), [srcAsm] "+r" (srcAsm)
    					       	:  //[srcY] "r" (srcY)
    					       	: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31");
    			ptr += 32;
    		}
    	}

    	if(nWidthMatchFlag == 1)
    	{
    		continue;
    	}
    	for(m=0; m<32; m++)
    	{
    		if((i*32 + m) >= nHeight)
    		{
    			ptr += 32;
    	    	continue;
    	   	}
    		dstAsm = bufferU;
    		srcAsm = ptr;
    	 	lineNum = i*32 + m;           //line num
    		offset =  lineNum*nLineStride + j*32;

    	   	 asm volatile (
    	    	      "vld1.8         {d0 - d3}, [%[srcAsm]]              \n\t"
    	              "vst1.8         {d0 - d3}, [%[dstAsm]]              \n\t"
    	         	    : [dstAsm] "+r" (dstAsm), [srcAsm] "+r" (srcAsm)
    	    	     	:  //[srcY] "r" (srcY)
    	    	    	: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31");
    	   	ptr += 32;
    	   	for(k=0; k<32; k++)
    	   	{
    	   		if((j*32+ k) >= nLineStride)
    	   	   	{
    	   			break;
    	   	  	}
    	   	 	pDst[offset+k] = bufferU[k];
    	   	}
    	}
    }
}


void ConvertMb32420ToNv21C(char* pSrc,char* pDst,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;
    char bufferV[16], bufferU[16];
    int nWidth = 0;
    int nHeight = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth*2 + 15) &~15;
    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;


    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nMbWidth; j++)
    	{
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    			{
    				ptr += 32;
    				continue;
        		}

    			dst0Asm = bufferU;
    			dst1Asm = bufferV;
    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;

    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 32;


    			for(k=0; k<16; k++)
    			{
    				if((j*32+ 2*k) >= nLineStride)
    				{
    					break;
    				}
    				pDst[offset+2*k]   = bufferV[k];
    			   	pDst[offset+2*k+1] = bufferU[k];
    			}
    		}
    	}
    }
}


#if 0
void ConvertMb32420ToNv21C(char* pSrc,char* pDst,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int n = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;
    char bufferV[16], bufferU[16];
    int nWidth = 0;
    int nHeight = 0;
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth*2 + 15) &~15;
    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;


    ptr = pSrc;

    nWidthMatchFlag = 0;
	nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*32 == nLineStride)
    {
    	nWidthMatchFlag = 1;
    	nCopyMbWidth = nMbWidth;
    }

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nCopyMbWidth; j++)
    	{
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    			{
    				ptr += 32;
    				continue;
        		}
    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;
    			dst0Asm = bufferU+offset;
    		   	dst1Asm = bufferV+offset;

    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 32;
    		}

    		if(nWidthMatchFlag == 1)
    		{
    			continue;
    		}
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    		   	{
    				ptr += 32;
    		    	continue;
    		   	}

    		    dst0Asm = bufferU;
    		   	dst1Asm = bufferV;
    		    srcAsm = ptr;
    		   	lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;

    		   	asm volatile(
    		    			"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    		    		 	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    		    		  	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    		   			     : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    		    		     :  //[srcY] "r" (srcY)
    		    	         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    		    		   );
    		   	ptr += 32;
    		   	for(k=0; k<16; k++)
    		    {
    		   		if((j*32+ 2*k) >= nLineStride)
    		    	{
    		   			break;
    		    	}
    		    	pDst[offset+2*k]   = bufferV[k];
    		       	pDst[offset+2*k+1] = bufferU[k];
    		   	}
    		}
    	}
    }
}
#endif

void ConvertMb32422ToNv21C(char* pSrc,char* pDst,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;
    char bufferV[16], bufferU[16];
    int nWidth = 0;
    int nHeight = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth*2 + 15) &~15;
    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight*2+31)&~31;
    nMbHeight /= 32;


    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nMbWidth; j++)
    	{
    		for(m=0; m<16; m++)
    		{
    			if((i*16 + m) >= nHeight)
    			{
    				ptr += 64;
    				continue;
        		}

    			dst0Asm = bufferU;
    			dst1Asm = bufferV;
    			srcAsm = ptr;
    			lineNum = i*16 + m;           //line num
    			offset =  lineNum*nLineStride + j*32;

    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 64;


    			for(k=0; k<16; k++)
    			{
    				if((j*32+ 2*k) >= nLineStride)
    				{
    					break;
    				}
    				pDst[offset+2*k]   = bufferV[k];
    			   	pDst[offset+2*k+1] = bufferU[k];
    			}
    		}
    	}
    }

}


#if 0
void ConvertMb32420ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int n = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;
    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    //nLineStride = ((nPicWidth+ 15) &~15)/2;
    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;


    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<(nMbWidth-1); j++)
    	{
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    			{
    				ptr += 32;
    				continue;
        		}

    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*16;
    			dst0Asm = pDstU+offset;
    		    dst1Asm = pDstV+offset;
    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 32;
    		}
    	}

    	for(m=0; m<32; m++)
    	{
    		if((i*32 + m) >= nHeight)
    		{
    			ptr += 32;
    			continue;
    		}


    	   	srcAsm = ptr;
    	   	lineNum = i*32 + m;           //line num
    		offset =  lineNum*nLineStride + j*16;
    	   	dst0Asm = bufferU;
        	dst1Asm = bufferV;
    	   	asm volatile(
    	   			"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    	    		"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    	    	 	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    	    	   	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    	            :  //[srcY] "r" (srcY)
    	            : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    	         );
    	   	ptr += 32;

    	   	for(k=0; k<16; k++)
    	   	{
    	   		if((j*16+ k) >= nLineStride)
    	   		{
    	   			break;
    	   		}
    	   		pDstV[offset+k] = bufferV[k];
    	   		pDstU[offset+k] = bufferU[k];
    	   	}
    	}
    }
}


void ConvertMb32422ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int n = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;

    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight*2+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    if(nMbWidth*32 == nLineStride)
    {

    }
    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<(nMbWidth-1); j++)
    	{
    		for(m=0; m<16; m++)
    		{
    			if((i*16 + m) >= nHeight)
    			{
    				ptr += 64;
    				continue;
        		}

    			srcAsm = ptr;
    			lineNum = i*16 + m;           //line num
    			offset =  lineNum*nLineStride + j*16;
    			dst0Asm = pDstU+offset;
    			dst1Asm = pDstV+offset;


    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 64;
    		}
    	}

    	for(m=0; m<16; m++)
    	{
    		if((i*16 + m) >= nHeight)
    	    {
    			ptr += 64;
    			continue;
    	    }

    		dst0Asm = bufferU;
    	    dst1Asm = bufferV;
    	   	srcAsm = ptr;
    	   	lineNum = i*16 + m;           //line num
    		offset =  lineNum*nLineStride + j*16;

    	   	asm volatile(
    	    			"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    	    		 	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    	    	    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    	    	    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    	    	        :  //[srcY] "r" (srcY)
    	    	        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    	   		     );
    	   	ptr += 64;

    	   	for(k=0; k<16; k++)
    	    {
    	   		if((j*16+ k) >= nLineStride)
    	   		{
    	   			break;
    	   		}
    	    	pDstV[offset+k] = bufferV[k];
    	   	   	pDstU[offset+k] = bufferU[k];
    		}
    	}
    }
}
#endif

void ConvertMb32420ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;
    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    //nLineStride = ((nPicWidth+ 15) &~15)/2;
    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;


    ptr = pSrc;

    nWidthMatchFlag = 0;
	nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*16 == nLineStride)
    {
    	nWidthMatchFlag = 1;
    	nCopyMbWidth = nMbWidth;
    }

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nCopyMbWidth; j++)
    	{
    		for(m=0; m<32; m++)
    		{
    			if((i*32 + m) >= nHeight)
    			{
    				ptr += 32;
    				continue;
        		}

    			srcAsm = ptr;
    			lineNum = i*32 + m;           //line num
    			offset =  lineNum*nLineStride + j*16;
    			dst0Asm = pDstU+offset;
    		    dst1Asm = pDstV+offset;
    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 32;
    		}
    	}

    	if(nWidthMatchFlag == 1)
    	{
    		continue;
    	}
    	for(m=0; m<32; m++)
    	{
    		if((i*32 + m) >= nHeight)
    		{
    			ptr += 32;
    			continue;
    		}


    	   	srcAsm = ptr;
    	   	lineNum = i*32 + m;           //line num
    		offset =  lineNum*nLineStride + j*16;
    	   	dst0Asm = bufferU;
        	dst1Asm = bufferV;
    	   	asm volatile(
    	   			"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    	    		"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    	    	 	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    	    	   	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    	            :  //[srcY] "r" (srcY)
    	            : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    	         );
    	   	ptr += 32;

    	   	for(k=0; k<16; k++)
    	   	{
    	   		if((j*16+ k) >= nLineStride)
    	   		{
    	   			break;
    	   		}
    	   		pDstV[offset+k] = bufferV[k];
    	   		pDstU[offset+k] = bufferU[k];
    	   	}
    	}
    }
}


void ConvertMb32422ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
	int nMbWidth = 0;
	int nMbHeight = 0;
	int i = 0;
	int j = 0;
	int m = 0;
	int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    char* ptr = NULL;
    char *dst0Asm = NULL;
    char *dst1Asm = NULL;
    char *srcAsm = NULL;

    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight*2+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    nWidthMatchFlag = 0;
	nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*16 == nLineStride)
    {
    	nWidthMatchFlag = 1;
    	nCopyMbWidth = nMbWidth;
    }

    for(i=0; i<nMbHeight; i++)
    {
    	for(j=0; j<nCopyMbWidth; j++)
    	{
    		for(m=0; m<16; m++)
    		{
    			if((i*16 + m) >= nHeight)
    			{
    				ptr += 64;
    				continue;
        		}

    			srcAsm = ptr;
    			lineNum = i*16 + m;           //line num
    			offset =  lineNum*nLineStride + j*16;
    			dst0Asm = pDstU+offset;
    			dst1Asm = pDstV+offset;


    			asm volatile(
    					"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    			    	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    			    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    			    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    			        :  //[srcY] "r" (srcY)
    			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    			     );
    			ptr += 64;
    		}
    	}

    	if(nWidthMatchFlag==1)
    	{
    		continue;
    	}
    	for(m=0; m<16; m++)
    	{
    		if((i*16 + m) >= nHeight)
    	    {
    			ptr += 64;
    			continue;
    	    }

    		dst0Asm = bufferU;
    	    dst1Asm = bufferV;
    	   	srcAsm = ptr;
    	   	lineNum = i*16 + m;           //line num
    		offset =  lineNum*nLineStride + j*16;

    	   	asm volatile(
    	    			"vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
    	    		 	"vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
    	    	    	"vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
    	    	    	: [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
    	    	        :  //[srcY] "r" (srcY)
    	    	        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    	   		     );
    	   	ptr += 64;

    	   	for(k=0; k<16; k++)
    	    {
    	   		if((j*16+ k) >= nLineStride)
    	   		{
    	   			break;
    	   		}
    	    	pDstV[offset+k] = bufferV[k];
    	   	   	pDstU[offset+k] = bufferU[k];
    		}
    	}
    }
}


//**********************************************************************************************************************//
//**********************************************************************************************************************//

void ConvertPixelFormat(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
    int   nMemSizeY = 0;
    int   nMemSizeC = 0;
    int   nLineStride = 0;
    int   nHeight16Align = 0;
    int   nHeight32Align = 0;
    int   nHeight64Align = 0;
    int   nHeight = 0;
    int   i = 0;
    int	  j = 0;


    nHeight        = pPictureIn->nHeight;
    nLineStride    = (pPictureIn->nWidth+15) &~15;
    nHeight16Align = (nHeight+15) & ~15;
    nHeight32Align = (nHeight+31) & ~31;
    nHeight64Align = (nHeight+63) & ~63;

	pPictureOut->nLineStride = (pPictureIn->nWidth+15) & ~15;
	pPictureOut->nHeight     = pPictureIn->nHeight;
	pPictureOut->nWidth      = pPictureIn->nWidth;
	pPictureOut->nTopOffset  = pPictureIn->nTopOffset;
	pPictureOut->nBottomOffset  = pPictureIn->nBottomOffset;
	pPictureOut->nLeftOffset  = pPictureIn->nLeftOffset;
	pPictureOut->nRightOffset  = pPictureIn->nRightOffset;

	if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
	{
		if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			nMemSizeC = nMemSizeY>>2;
		//	ConvertMb32ToYv12Y(pPictureIn->pData0,pPictureOut->pData0, pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0,pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32420ToYv12C(pPictureIn->pData1,(char*)(pPictureOut->pData0+nMemSizeY+nMemSizeC),
					(char*)(pPictureOut->pData0+nMemSizeY), pPictureIn->nWidth, pPictureIn->nHeight);

		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			nMemSizeC = nMemSizeY>>2;
			ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32422ToYv12C(pPictureIn->pData1,(char*)(pPictureOut->pData0+nMemSizeY+nMemSizeC),
	    		               (char*)(pPictureOut->pData0+nMemSizeY), pPictureIn->nWidth, pPictureIn->nHeight);
		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_NV21)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
			nMemSizeC = nMemSizeY>>2;
			pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;
			pPictureOut->pData2 = pPictureOut->pData1 + nMemSizeC;
			pPictureIn->pData1  = pPictureIn->pData0 + nMemSizeY;

            for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
            {
            	for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
            	{
            		pPictureOut->pData1[i*nLineStride/2+j] = pPictureIn->pData1[i*nLineStride+2*j];
            		pPictureOut->pData2[i*nLineStride/2+j] = pPictureIn->pData1[i*nLineStride+2*j+1];
            	}
            }
		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
			nMemSizeC = nMemSizeY>>2;
			memcpy(pPictureOut->pData0+nMemSizeY, pPictureIn->pData0+nMemSizeY+nMemSizeC, nMemSizeC);
			memcpy(pPictureOut->pData0+nMemSizeY+nMemSizeC, pPictureIn->pData0+nMemSizeY, nMemSizeC);
		}
	}
	else if(pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)
	{
		if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			//nMemSizeY = nLineStride*nHeight;
			nMemSizeC = nMemSizeY>>2;
			//ConvertMb32ToYv12Y(pPictureIn->pData0,pPictureOut->pData0, pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0,pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32420ToNv21C(pPictureIn->pData1,pPictureOut->pData0+nMemSizeY,pPictureIn->nWidth, pPictureIn->nHeight);
		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			nMemSizeC = nMemSizeY>>2;
	//		ConvertMb32ToYv12Y(pPictureIn->pData0,pPictureOut->pData0, pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, pPictureIn->nWidth, pPictureIn->nHeight);
			ConvertMb32422ToNv21C(pPictureIn->pData1,(char*)(pPictureOut->pData0+nMemSizeY), pPictureIn->nWidth, pPictureIn->nHeight);
		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YV12)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
			nMemSizeC = nMemSizeY>>2;
			pPictureIn->pData1 = pPictureIn->pData0 + nMemSizeY;
			pPictureIn->pData2 = pPictureIn->pData1 + nMemSizeC;
		    pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;

		    for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
			{
		    	for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
		    	{
		    		pPictureOut->pData1[i*nLineStride+2*j]   = pPictureIn->pData1[i*nLineStride/2+j];
		    		pPictureOut->pData1[i*nLineStride+2*j+1] = pPictureIn->pData2[i*nLineStride/2+j];
		    	}
			}
		}
		else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420)
		{
			nMemSizeY = nLineStride*nHeight16Align;
			memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
			nMemSizeC = nMemSizeY>>2;
			pPictureIn->pData1 = pPictureIn->pData0 + nMemSizeY;
			pPictureIn->pData2 = pPictureIn->pData1 + nMemSizeC;
		    pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;

		    for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
			{
		    	for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
				{
		    		pPictureOut->pData1[i*nLineStride+2*j]   = pPictureIn->pData2[i*nLineStride/2+j];
					pPictureOut->pData1[i*nLineStride+2*j+1] = pPictureIn->pData1[i*nLineStride/2+j];
			 	}
			}
		}
	}
	//AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
}


int RotatePicture0Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut, int nGpuYAlign, int nGpuCAlign)
{
	int nMemSizeY = 0;
	int nMemSizeC = 0;
	int nLineStride = 0;
    int ePixelFormat = 0;
    int nHeight16Align = 0;
    int nHeight32Align = 0;
    int nHeight64Align = 0;

	nLineStride    = pPictureIn->nLineStride;
	ePixelFormat   = pPictureIn->ePixelFormat;
	nHeight16Align = (pPictureIn->nHeight+15) &~15;
	nHeight32Align = (pPictureIn->nHeight+31) &~31;
	nHeight64Align = (pPictureIn->nHeight+63) &~63;

	switch(pPictureOut->ePixelFormat)
	{
		case PIXEL_FORMAT_YUV_PLANER_420:
		case PIXEL_FORMAT_YUV_PLANER_422:
	    case PIXEL_FORMAT_YUV_PLANER_444:
	    case PIXEL_FORMAT_YV12:
	    case PIXEL_FORMAT_NV21:
	    	//* for decoder,
	    	//* height of Y component is required to be 16 aligned, for example, 1080 becomes to 1088.
	    	//* width and height of U or V component are both required to be 8 aligned.

	    	//* nLineStride should be 16 aligned.
	        nMemSizeY = nLineStride*nHeight16Align;
	        if(ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
	        		ePixelFormat == PIXEL_FORMAT_YV12 ||
	        		ePixelFormat == PIXEL_FORMAT_NV21)
	        	nMemSizeC = nMemSizeY>>2;
	        else if(ePixelFormat == PIXEL_FORMAT_YUV_PLANER_422)
	        	nMemSizeC = nMemSizeY>>1;
	        else
	        	nMemSizeC = nMemSizeY;  //* PIXEL_FORMAT_YUV_PLANER_444

            //* copy relay on gpuYAlign and gpuCAlign if the format is YV12
            if(pPictureOut->ePixelFormat == PIXEL_FORMAT_YV12)
            {
                //* we can memcpy directly if gpuYAlign is 16 and gpuCAlign is 8
                if(nGpuYAlign == 16 && nGpuCAlign == 8)
                {
    	            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY+nMemSizeC*2);
                }
                else if(nGpuYAlign == 16 && nGpuCAlign == 16)
                {
                    int i;
                    int nCpyWidthSize = pPictureOut->nLineStride;
                    char* pDstData = pPictureOut->pData0;
                    char* pSrcData = pPictureIn->pData0;
                    //* cpy y
                    for(i = 0;i < pPictureIn->nHeight;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride;
                    }
                    //* cpy v
                    pDstData = pPictureOut->pData0 + pPictureOut->nLineStride*pPictureOut->nHeight;
                    pSrcData = pPictureIn->pData0 + nMemSizeY;
                    nCpyWidthSize = (nCpyWidthSize/2 + 15) & ~15;
                    
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                    //* cpy u
                    int nCSize = nCpyWidthSize*pPictureOut->nHeight/2;
                    pDstData = pPictureOut->pData0 + pPictureOut->nLineStride*pPictureOut->nHeight + nCSize;
                    pSrcData = pPictureIn->pData0 + nMemSizeY+nMemSizeC;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride/2;
                    }                
                }
                else if(nGpuYAlign == 32 && nGpuCAlign == 16)
                {
                    int i;
                    int nCpyWidthSize = pPictureOut->nLineStride;
                    char* pDstData = pPictureOut->pData0;
                    char* pSrcData = pPictureIn->pData0;
                    //* cpy y
                    for(i = 0;i < pPictureIn->nHeight;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride;
                    }
                    //* cpy v
                    pDstData = pPictureOut->pData0 + pPictureOut->nLineStride*pPictureOut->nHeight;
                    pSrcData = pPictureIn->pData0 + nMemSizeY;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize/2;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                    //* cpy u
                    pDstData = pPictureOut->pData0 + pPictureOut->nLineStride*pPictureOut->nHeight*5/4;
                    pSrcData = pPictureIn->pData0 + nMemSizeY+nMemSizeC;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize/2;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                }
                else
                {
                    loge("the nGpuYAlign[%d] and nGpuCAlign[%d] is not surpport!",nGpuYAlign,nGpuCAlign);
                    return -1;
                }
            }
            else
            {
                memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY+nMemSizeC*2);
            }
	        //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
	        break;

	    case PIXEL_FORMAT_YUV_MB32_420:
	    case PIXEL_FORMAT_YUV_MB32_422:
	    case PIXEL_FORMAT_YUV_MB32_444:
	    	//* for decoder,
	    	//* height of Y component is required to be 32 aligned.
	    	//* height of UV component are both required to be 32 aligned.
	    	//* nLineStride should be 32 aligned.
	    	nMemSizeY = nLineStride*nHeight32Align;
	    	if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
	    		nMemSizeC = nLineStride*nHeight64Align/4;
	    	else if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
	    		nMemSizeC = nLineStride*nHeight64Align/2;
	    	else
	    		nMemSizeC = nLineStride*nHeight64Align;

	    	memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
	    	memcpy(pPictureOut->pData1, pPictureIn->pData1, nMemSizeC*2);
	    	//AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY);
	    	//AdapterMemFlushCache((void*)pPictureOut->pData1, 2*nMemSizeC);
	    	break;

	    default:
	    	loge("pixel format incorrect, ePixelFormat=%d", ePixelFormat);
	    	return -1;
	}
	return 0;
}

int RotatePicture90Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
	int i = 0;
	int j = 0;
	int nHeight = 0;
	int nWidth = 0;
	int nLineStride = 0;
	int nRotateLineStride = 0;
	int nLeftOffset = 0;
	int nRightOffset = 0;
	int nBottomOffset = 0;
	int nTopOffset = 0;
	int nMemSizeY = 0;
	int nMemSizeC = 0;

	pPictureOut->nLineStride = (pPictureIn->nHeight+15) &~15;
	pPictureOut->nHeight = pPictureIn->nWidth;
	pPictureOut->nWidth = pPictureIn->nHeight;

	if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))
	{

		for(j=0; j<pPictureIn->nHeight; j++)
		{
			for(i=0; i<pPictureIn->nWidth; i++)
			{
				pPictureOut->pData0[i*pPictureOut->nLineStride+pPictureIn->nHeight-j-1] = pPictureIn->pData0[j*pPictureIn->nLineStride+i];
			}
		}

		nHeight           = pPictureIn->nHeight/2;
		nWidth            = pPictureIn->nWidth/2;
		nLineStride       = pPictureIn->nLineStride/2;
	    nRotateLineStride = pPictureOut->nLineStride/2;

		if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
		{
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
            pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;

			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					pPictureOut->pData1[i*nRotateLineStride+nHeight-j-1] = pPictureIn->pData1[j*nLineStride+i];
					pPictureOut->pData2[i*nRotateLineStride+nHeight-j-1] = pPictureIn->pData2[j*nLineStride+i];
				}
			}
		}
		else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)||(pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
		{
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;


            for(j=0; j<nHeight; j++)
            {
            	for(i=0; i<nWidth; i++)
            	{
            		pPictureOut->pData1[2*i*nRotateLineStride+2*nHeight-(2*j+1)-1] = pPictureIn->pData1[2*j*nLineStride+2*i];
            		pPictureOut->pData1[2*i*nRotateLineStride+2*nHeight-(2*j)-1] = pPictureIn->pData1[2*j*nLineStride+2*i+1];
            	}
            }
		}
	}

	//AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);


	nLeftOffset = pPictureIn->nLeftOffset;
	nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
	nTopOffset = pPictureIn->nTopOffset;
	nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

	pPictureOut->nLeftOffset   = nBottomOffset;
	pPictureOut->nBottomOffset = nRightOffset;
	pPictureOut->nRightOffset  = nTopOffset;
	pPictureOut->nTopOffset    = nLeftOffset;
	pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
	pPictureOut->nBottomOffset  = pPictureOut->nHeight -pPictureOut->nBottomOffset;
	return 0;
}

int RotatePicture180Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
	int i = 0;
	int j = 0;
	int nHeight = 0;
	int nWidth = 0;
	int nLineStride = 0;
	int nRotateLineStride = 0;
	int nLeftOffset = 0;
	int nRightOffset = 0;
	int nBottomOffset = 0;
	int nTopOffset = 0;
	int nMemSizeY = 0;
	int nMemSizeC = 0;

	pPictureOut->nLineStride = pPictureIn->nLineStride;

	if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))
	{

		for(j=0; j<pPictureIn->nHeight; j++)
		{
			for(i=0; i<pPictureIn->nWidth; i++)
			{
				pPictureOut->pData0[(pPictureIn->nHeight-1-j)*pPictureIn->nLineStride+(pPictureIn->nWidth-1-i)] = pPictureIn->pData0[j*pPictureIn->nLineStride+i];
			}
		}

		nHeight           = pPictureIn->nHeight/2;
		nWidth            = pPictureIn->nWidth/2;
		nLineStride       = pPictureIn->nLineStride/2;

		if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
		{
	        nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
	        nMemSizeC = nMemSizeY>>2;
	        pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
	        pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;
	        pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
	        pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					pPictureOut->pData1[(nHeight-1-j)*nLineStride+(nWidth-1-i)] = pPictureIn->pData1[j*nLineStride+i];
					pPictureOut->pData2[(nHeight-1-j)*nLineStride+(nWidth-1-i)] = pPictureIn->pData2[j*nLineStride+i];
				}
			}
		}
		else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)||(pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
		{
			 nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
			 nMemSizeC = nMemSizeY>>2;
			 pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
			 pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
			 for(j=0; j<nHeight; j++)
			 {
				 for(i=0; i<nWidth; i++)
				 {
					 pPictureOut->pData1[2*(nHeight-1-j)*nLineStride+2*(nWidth-1-i)] = pPictureIn->pData1[2*j*nLineStride+2*i];
					 pPictureOut->pData1[2*(nHeight-1-j)*nLineStride+2*(nWidth-1-i)+1] = pPictureIn->pData1[2*j*nLineStride+2*i+1];
				 }
			 }
		}
	}

    //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
	pPictureOut->nHeight = pPictureIn->nHeight;
	pPictureOut->nWidth = pPictureIn->nWidth;

	nLeftOffset = pPictureIn->nLeftOffset;
	nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
	nTopOffset = pPictureIn->nTopOffset;
	nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

	pPictureOut->nLeftOffset   = nRightOffset;
	pPictureOut->nBottomOffset = nTopOffset;
	pPictureOut->nRightOffset  = nLeftOffset;
	pPictureOut->nTopOffset    = nBottomOffset;

	pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
	pPictureOut->nBottomOffset = pPictureOut->nHeight -pPictureOut->nBottomOffset;
	return 0;
}

int RotatePicture270Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
	int i = 0;
	int j = 0;
	int nHeight = 0;
	int nWidth = 0;
	int nLineStride = 0;
	int nRotateLineStride = 0;
	int nLeftOffset = 0;
	int nRightOffset = 0;
	int nBottomOffset = 0;
	int nTopOffset = 0;
	int nMemSizeY = 0;
	int nMemSizeC = 0;

	pPictureOut->nLineStride = (pPictureIn->nHeight+15) &~15;
	pPictureOut->nHeight = pPictureIn->nWidth;
	pPictureOut->nWidth = pPictureIn->nHeight;

	if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
			||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))

	{

		for(j=0; j<pPictureIn->nHeight; j++)
		{
			for(i=0; i<pPictureIn->nWidth; i++)
			{
				pPictureOut->pData0[(pPictureIn->nWidth-1-i)*pPictureOut->nLineStride+j] = pPictureIn->pData0[j*pPictureIn->nLineStride+i];
			}
		}

		nHeight           = pPictureIn->nHeight/2;
		nWidth            = pPictureIn->nWidth/2;
		nLineStride       = pPictureIn->nLineStride/2;
		nRotateLineStride = pPictureOut->nLineStride/2;

		if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
		{
		    nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
			nMemSizeC = nMemSizeY>>2;

			pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
			pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

		    nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
	        nMemSizeC = nMemSizeY>>2;
			pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
			pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;


			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					pPictureOut->pData1[(nWidth-1-i)*nRotateLineStride+j] = pPictureIn->pData1[j*nLineStride+i];
					pPictureOut->pData2[(nWidth-1-i)*nRotateLineStride+j] = pPictureIn->pData2[j*nLineStride+i];
				}
			}
		}
		else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21) || (pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
		{
			nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
			pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;

			nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
			nMemSizeC = nMemSizeY>>2;

			for(j=0; j<nHeight; j++)
			{
				for(i=0; i<nWidth; i++)
				{
					pPictureOut->pData1[2*(nWidth-1-i)*nRotateLineStride+2*j] = pPictureIn->pData1[2*j*nLineStride+2*i];
					pPictureOut->pData1[2*(nWidth-1-i)*nRotateLineStride+2*j+1] = pPictureIn->pData1[2*j*nLineStride+2*i+1];
				}
			}
		}
	}

	//AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
	nLeftOffset = pPictureIn->nLeftOffset;
	nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
	nTopOffset = pPictureIn->nTopOffset;
	nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

	pPictureOut->nLeftOffset   = nTopOffset;
	pPictureOut->nBottomOffset = nLeftOffset;
	pPictureOut->nRightOffset  = nBottomOffset;
	pPictureOut->nTopOffset    = nRightOffset;

	pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
	pPictureOut->nBottomOffset = pPictureOut->nHeight -pPictureOut->nBottomOffset;
	return 0;
}


