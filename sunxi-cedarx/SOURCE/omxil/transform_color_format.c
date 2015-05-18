/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
* Copyright (c) 2014 Ning Fang <fangning@allwinnertech.com>
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

#include <log.h>
#include "transform_color_format.h"

#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

enum FORMAT_CONVERT_COLORFORMAT
{
	CONVERT_COLOR_FORMAT_NONE = 0,
	CONVERT_COLOR_FORMAT_YUV420PLANNER,
	CONVERT_COLOR_FORMAT_YUV422PLANNER,
	CONVERT_COLOR_FORMAT_YUV420MB,
	CONVERT_COLOR_FORMAT_YUV422MB,
};

typedef struct ScalerParameter
{
	int mode; //0: YV12 1:thumb yuv420p
	int format_in;
	int format_out;

	int width_in;
	int height_in;

	int width_out;
	int height_out;

	void *addr_y_in;
	void *addr_c_in;
	unsigned int addr_y_out;
	unsigned int addr_u_out;
	unsigned int addr_v_out;
}ScalerParameter;

/*******************************************************************************
Function name: map32x32_to_yuv_Y
Description:
    1. we should know : vdecbuf is 32*32 align
    2. must match gpuBuf size.
    3. we guarantee: vdecbufSize>=gpuBufSize
    4. if coded_width is stride, we can support gpu_buf_width 32byte align and 16byte align!
    5. A20_GPU:
        YV12: y_width_align=16, u_width_align=16, v_width_align=16,
              y/u/v_height can be any number(not even).

Parameters:

Return:

Time: 2013/4/15
*******************************************************************************/
static void map32x32_to_yuv_Y(unsigned char* srcY, unsigned char* tarY,unsigned int coded_width,unsigned int coded_height)
{
	unsigned int i,j,l,m,n;
	unsigned int mb_width,mb_height,twomb_line, twomb_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst_asm,*src_asm;
    unsigned vdecbuf_width, vdecbuf_height;
    int nWidthMatchFlag;
    int nLeftValidLine;  //in the bottom macroblock(32*32), the valid line is < 32.
	ptr = srcY;

    mb_width = ((coded_width+31)&~31) >>4;
	mb_height = ((coded_height+31)&~31) >>4;
	twomb_line = (mb_height+1)>>1;
    twomb_width = (mb_width+1)>>1;
    if(twomb_line < 1 || twomb_width < 1)
    {
        loge("fatal error! twomb_line=%d, twomb_width=%d", twomb_line, twomb_width);
    }
    vdecbuf_width = twomb_width*32;
    vdecbuf_height = twomb_line*32;
    if(vdecbuf_width > coded_width)
    {
        nWidthMatchFlag = 0;
        if((vdecbuf_width - coded_width) != 16)
        {
            logw("fatal error! vdecbuf_width=%d, gpubuf_width=%d,  the program will crash!", vdecbuf_width, coded_width);
        }
        else
        {
            //LOGV("(f:%s, l:%d) Be careful! vdecbuf_width=%d, gpubuf_width=%d", __FUNCTION__, __LINE__, vdecbuf_width, coded_width);
        }
    }
    else if(vdecbuf_width == coded_width)
    {
        nWidthMatchFlag = 1;
    }
    else
    {
        logw("fatal error! vdecbuf_width=%d <= gpubuf_width=%d, the program will crash!", vdecbuf_width, coded_width);
        nWidthMatchFlag = 0;
    }
	for(i=0;i<twomb_line-1;i++)   //twomb line number
	{
		for(j=0;j<twomb_width-1;j++)   //macroblock(32*32) number in one line
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;     //line num
				n= j*32;        //byte num in one line
				offset = m*coded_width + n;
				//memcpy(tarY+offset,ptr,32);
				dst_asm = tarY+offset;
				src_asm = ptr;
				asm volatile (
				        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
				        "vst1.8         {d0 - d3}, [%[dst_asm]]              \n\t"
				        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
				        :  //[srcY] "r" (srcY)
				        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				        );

				ptr += 32;  //32 byte in one process.
			}
		}
        //process last macroblock of one line, gpu buf must be 16byte align or 32 byte align
        { //last mb of one line
            for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;     //line num
				n= j*32;        //byte num in one line
				offset = m*coded_width + n;
				//memcpy(tarY+offset,ptr,32);
				dst_asm = tarY+offset;
				src_asm = ptr;
                if(nWidthMatchFlag)
                {
    				asm volatile (
    				        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
    				        "vst1.8         {d0 - d3}, [%[dst_asm]]              \n\t"
    				        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
    				        :  //[srcY] "r" (srcY)
    				        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    				        );
                }
                else
                {
                    asm volatile (
    				        "vld1.8         {d0,d1}, [%[src_asm]]              \n\t"
    				        "vst1.8         {d0,d1}, [%[dst_asm]]              \n\t"
    				        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
    				        :  //[srcY] "r" (srcY)
    				        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
    				        );
                }

				ptr += 32;  //32 byte in one process.
			}
        }
	}
    //last twomb line, we process it alone
    nLeftValidLine = coded_height - (twomb_line-1)*32;
    if(nLeftValidLine!=32)
    {
        //LOGV("(f:%s, l:%d)hehehaha,gpuBufHeight[%d] is not 32 align", __FUNCTION__, __LINE__, nLeftValidLine);
    }
    for(j=0;j<twomb_width-1;j++)   //macroblock(32*32) number in one line
	{
		for(l=0;l<nLeftValidLine;l++)
		{
			//first mb
			m=i*32 + l;     //line num
			n= j*32;        //byte num in one line
			offset = m*coded_width + n;
			//memcpy(tarY+offset,ptr,32);
			dst_asm = tarY+offset;
			src_asm = ptr;
			asm volatile (
			        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
			        "vst1.8         {d0 - d3}, [%[dst_asm]]              \n\t"
			        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
			        :  //[srcY] "r" (srcY)
			        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
			        );

			ptr += 32;  //32 byte in one process.
		}
        ptr += (32-nLeftValidLine)*32;
	}
    //process last macroblock of last line, gpu buf must be 16byte align or 32 byte align
    { //last mb of last line
        for(l=0;l<nLeftValidLine;l++)
		{
			//first mb
			m=i*32 + l;     //line num
			n= j*32;        //byte num in one line
			offset = m*coded_width + n;
			//memcpy(tarY+offset,ptr,32);
			dst_asm = tarY+offset;
			src_asm = ptr;
            if(nWidthMatchFlag)
            {
				asm volatile (
				        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
				        "vst1.8         {d0 - d3}, [%[dst_asm]]              \n\t"
				        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
				        :  //[srcY] "r" (srcY)
				        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				        );
            }
            else
            {
                asm volatile (
				        "vld1.8         {d0,d1}, [%[src_asm]]              \n\t"
				        "vst1.8         {d0,d1}, [%[dst_asm]]              \n\t"
				        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
				        :  //[srcY] "r" (srcY)
				        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				        );
            }

			ptr += 32;  //32 byte in one process.
		}
        ptr += (32-nLeftValidLine)*32;
    }

}

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

Time: 2013/4/15
*******************************************************************************/
//#define USE_VLD2_8  1
static void map32x32_to_yuv_C(int mode, unsigned char* srcC,unsigned char* tarCb,unsigned char* tarCr,unsigned int coded_width,unsigned int coded_height)
{
	unsigned int i,j,l,m,n,k;
	unsigned int mb_width,mb_height,twomb_line,twomb_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst0_asm,*dst1_asm,*src_asm;
    unsigned vdecbuf_width, vdecbuf_height; //unit: pixel
    int         nWidthMatchFlag;
    int         nLeftValidLine;  //in the bottom macroblock(32*32), the valid line is < 32.
	unsigned char line[16];

    int dst_stride = mode==0 ? (coded_width + 15) & (~15) : coded_width;

	ptr = srcC;

//	mb_width = (coded_width+7)>>3;
//	mb_height = (coded_height+7)>>3;
//	fourmb_line = (mb_height+3)>>2;
//	recon_width = (mb_width+1)&0xfffffffe;

    mb_width = ((coded_width+15)&~15)>>4;   //vdec's uvBuf is 32byte align, so uBuf and vBuf is 16byte align!
	mb_height = ((coded_height+31)&~31)>>4;
	twomb_line = (mb_height+1)>>1;
	//recon_width = (mb_width+1)&0xfffffffe;
    twomb_width = mb_width; //vdec mb32 is uv interleave, so uv_32 byte == u_16byte
    if(twomb_line < 1 || twomb_width < 1)
    {
        loge("map32x32_to_yuv_C() fatal error! twomb_line=%d, twomb_width=%d", twomb_line, twomb_width);
    }
    //vdec mb32 uvBuf, one vdec_macro_block, extract u component, u's width and height.
    vdecbuf_width = twomb_width*16;
    vdecbuf_height = twomb_line*32;
    if(vdecbuf_width > coded_width)
    {
        nWidthMatchFlag = 0;
        if((vdecbuf_width - coded_width) != 8)
        {
            logw("fatal error! vdec_UVbuf_width=%d, gpu_UVbuf_width=%d,  the program will crash!", vdecbuf_width, coded_width);
        }
        else
        {
            //LOGV("(f:%s, l:%d) vdec_UVbuf_width=%d, gpu_UVbuf_width=%d, not match, gpu_uvBuf is 8byte align?", __FUNCTION__, __LINE__, vdecbuf_width, coded_width);
        }
    }
    else if(vdecbuf_width == coded_width)
    {
        nWidthMatchFlag = 1;
    }
    else
    {
        logw("fatal error! vdec_UVbuf_width=%d <= gpu_UVbuf_width=%d, the program will crash!", vdecbuf_width, coded_width);
        nWidthMatchFlag = 0;
    }

	for(i=0;i<twomb_line-1;i++)
	{
		for(j=0;j<twomb_width-1;j++)
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l; //line num
				n= j*16;    //byte num in dst_one_line
				offset = m*dst_stride + n;

				dst0_asm = tarCb + offset;
				dst1_asm = tarCr+offset;
				src_asm = ptr;
//					for(k=0;k<16;k++)
//					{
//						dst0_asm[k] = src_asm[2*k];
//						dst1_asm[k] = src_asm[2*k+1];
//					}
            asm volatile (
                    "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                    "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                    "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                     : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                     :  //[srcY] "r" (srcY)
                     : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                     );

				ptr += 32;
			}
		}
        //process last twomb_macroblock of one line, gpu buf must be 16 byte align or 8 byte align.
        for(l=0;l<32;l++)
		{
			//first mb
			m=i*32 + l; //line num
			n= j*16;    //byte num in dst_one_line
			offset = m*dst_stride + n;

			dst0_asm = tarCb + offset;
			dst1_asm = tarCr+offset;
			src_asm = ptr;
//					for(k=0;k<16;k++)
//					{
//						dst0_asm[k] = src_asm[2*k];
//						dst1_asm[k] = src_asm[2*k+1];
//					}

            if(nWidthMatchFlag)
            {
                asm volatile (
                        "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                         :  //[srcY] "r" (srcY)
                         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                         );
            }
            else
            {
                asm volatile (
                        "vld2.8         {d0,d1}, [%[src_asm]]              \n\t"
                        "vst1.8         {d0}, [%[dst0_asm]]              \n\t"
                        "vst1.8         {d1}, [%[dst1_asm]]              \n\t"
                         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                         :  //[srcY] "r" (srcY)
                         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                         );
            }
			ptr += 32;
		}
	}

    //last twomb line, we process it alone
    nLeftValidLine = coded_height - (twomb_line-1)*32;  //uv height can be odd number,must be very careful!
    if(nLeftValidLine!=32)
    {
        //LOGV("(f:%s, l:%d) hehehaha,gpu_UVBuf_extra_Height[%d] is not 32 align, coded_height[%d], twomb_line[%d]",
         //   __FUNCTION__, __LINE__, nLeftValidLine, coded_height, twomb_line);
    }
    for(j=0;j<twomb_width-1;j++)   //macroblock(32*32) number in one line
	{
		for(l=0;l<nLeftValidLine;l++)
		{
			//first mb
			m=i*32 + l;     //line num
			n= j*16;        //byte num in dst_one_line
			offset = m*dst_stride + n;

			dst0_asm = tarCb + offset;
			dst1_asm = tarCr+offset;
			src_asm = ptr;
//              for(k=0;k<16;k++)
//              {
//                  dst0_asm[k] = src_asm[2*k];
//                  dst1_asm[k] = src_asm[2*k+1];
//              }
            asm volatile (
                    "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                    "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                    "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                     : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                     :  //[srcY] "r" (srcY)
                     : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                     );
			ptr += 32;  //32 byte in one process.
		}
        ptr += (32-nLeftValidLine)*32;
	}
    //process last macroblock of last line, gpu UVbuf must be 16byte align or 8 byte align
    { //last mb of last line
        for(l=0;l<nLeftValidLine;l++)
		{
			//first mb
			m=i*32 + l;     //line num
			n= j*16;        //byte num in one line
			offset = m*dst_stride + n;

			dst0_asm = tarCb + offset;
			dst1_asm = tarCr+offset;
			src_asm = ptr;
            if(nWidthMatchFlag)
            {
                asm volatile (
                        "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                         :  //[srcY] "r" (srcY)
                         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                         );
            }
            else
            {
                asm volatile (
                        "vld2.8         {d0,d1}, [%[src_asm]]              \n\t"
                        "vst1.8         {d0}, [%[dst0_asm]]              \n\t"
                        "vst1.8         {d1}, [%[dst1_asm]]              \n\t"
                         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                         :  //[srcY] "r" (srcY)
                         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                         );
            }

			ptr += 32;  //32 byte in one process.
		}
        ptr += (32-nLeftValidLine)*32;
    }
}

/*******************************************************************************
Function name: map32x32_to_yuv_C_422
Description:
    1. mb32_422 to yv12_420
    2. so we discard one line in every two lines.
    3. mb32_yuv422, vdec_macroblock is also 32*32.
Parameters:

Return:

Time: 2013/4/16
*******************************************************************************/
static void map32x32_to_yuv_C_422(int mode, unsigned char* srcC,unsigned char* tarCb,unsigned char* tarCr,unsigned int coded_width,unsigned int coded_height)
{
	unsigned int i,j,l,m,n,k;
	unsigned int mb_width,mb_height,twomb_line,twomb_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst0_asm,*dst1_asm,*src_asm;
	unsigned char line[16];
    unsigned vdecbuf_width, vdecbuf_height;
    int nWidthMatchFlag;
    int nLeftValidLine;  //in the bottom macroblock(32*32), the valid line is < 32.

	ptr = srcC;
//	mb_width = (coded_width+7)>>3;
//	mb_height = (coded_height+7)>>3;
//	twomb_line = (mb_height+1)>>1;
//	recon_width = (mb_width+1)&0xfffffffe;

    mb_width = ((coded_width+15)&~15)>>4;   //vdec's uvBuf is 32byte align, so uBuf and vBuf is 16byte align!
	mb_height = ((coded_height*2+31)&~31)>>4;   //coded_height is gpu_buf's uv_height of yuv420, so uv_height of yuv422 must *2!
	twomb_line = (mb_height+1)>>1;
	//recon_width = (mb_width+1)&0xfffffffe;
    twomb_width = mb_width; //vdec mb32 is uv interleave, so uv_32 byte == u_16byte
    if(twomb_line < 1 || twomb_width < 1)
    {
        loge("map32x32_to_yuv_C_422() fatal error! twomb_line=%d, twomb_width=%d", twomb_line, twomb_width);
    }
    //vdec mb32 uvBuf, one vdec_macro_block, extract u component, u's width and height.
    vdecbuf_width = twomb_width*16;
    vdecbuf_height = twomb_line*32;
    if(vdecbuf_width > coded_width)
    {
        nWidthMatchFlag = 0;
        if((vdecbuf_width - coded_width) != 8)
        {
            logw("fatal error! vdec_UVbuf_width=%d, gpu_UVbuf_width=%d,  the program will crash!", vdecbuf_width, coded_width);
        }
        else
        {
            logv("vdec_UVbuf_width=%d, gpu_UVbuf_width=%d, not match, gpu_uvBuf is 8byte align?", vdecbuf_width, coded_width);
        }
    }
    else if(vdecbuf_width == coded_width)
    {
        nWidthMatchFlag = 1;
    }
    else
    {
        logw("fatal error! vdec_UVbuf_width=%d <= gpu_UVbuf_width=%d, the program will crash!", vdecbuf_width, coded_width);
        nWidthMatchFlag = 0;
    }

	for(i=0;i<twomb_line-1;i++)
	{
		for(j=0;j<twomb_width-1;j++)
		{
			for(l=0;l<16;l++)
			{
				//first mb
				m=i*16 + l; //line num, because yub422->yuv420, so discard one line in every two lines, so is 16!
				n= j*16;    //byte num in dst_one_line
				offset = m*coded_width + n;

				dst0_asm = tarCb + offset;
				dst1_asm = tarCr+offset;
				src_asm = ptr;
//					for(k=0;k<16;k++)
//					{
//						dst0_asm[k] = src_asm[2*k];
//						dst1_asm[k] = src_asm[2*k+1];
//					}
                 asm volatile (
                         "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                         "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                         "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                          : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                          :  //[srcY] "r" (srcY)
                          : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                          );
				ptr += 32;
				ptr += 32;
			}
		}
        //process last twomb_macroblock of one line, gpu buf must be 16 byte align or 8 byte align.
        for(l=0;l<16;l++)
        {
            //first mb
            m=i*16 + l; //line num, because yub422->yuv420, so discard one line in every two lines, so is 16!
            n= j*16;    //byte num in dst_one_line
            offset = m*coded_width + n;

            dst0_asm = tarCb + offset;
            dst1_asm = tarCr+offset;
            src_asm = ptr;
//                  for(k=0;k<16;k++)
//                  {
//                      dst0_asm[k] = src_asm[2*k];
//                      dst1_asm[k] = src_asm[2*k+1];
//                  }
            if(nWidthMatchFlag)
            {
                 asm volatile (
                         "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                         "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                         "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                          : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                          :  //[srcY] "r" (srcY)
                          : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                          );
            }
            else
            {
                 asm volatile (
                         "vld2.8         {d0,d1}, [%[src_asm]]              \n\t"
                         "vst1.8         {d0}, [%[dst0_asm]]              \n\t"
                         "vst1.8         {d1}, [%[dst1_asm]]              \n\t"
                          : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                          :  //[srcY] "r" (srcY)
                          : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                          );
            }
            ptr += 32;
            ptr += 32;
        }
	}

    //last twomb line, we process it alone
    nLeftValidLine = coded_height - (twomb_line-1)*16;  //uv height can be odd number,must be very careful! because yuv422, so 32line equal to 16 line of yuv420
    if(nLeftValidLine!=16)
    {
        //LOGV("(f:%s, l:%d) hehehaha,gpu_UVBuf_extra_Height[%d] is not 16 align, coded_height[%d], twomb_line[%d]",
          //  __FUNCTION__, __LINE__, nLeftValidLine, coded_height, twomb_line);
    }
    for(j=0;j<twomb_width-1;j++)
	{
		for(l=0;l<nLeftValidLine;l++)
		{
			//first mb
			m=i*16 + l; //line num, because yub422->yuv420, so discard one line in every two lines, so is 16!
			n= j*16;    //byte num in dst_one_line
			offset = m*coded_width + n;

			dst0_asm = tarCb + offset;
			dst1_asm = tarCr+offset;
			src_asm = ptr;
//					for(k=0;k<16;k++)
//					{
//						dst0_asm[k] = src_asm[2*k];
//						dst1_asm[k] = src_asm[2*k+1];
//					}
             asm volatile (
                     "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                     "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                     "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                      : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                      :  //[srcY] "r" (srcY)
                      : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                      );
			ptr += 32;
			ptr += 32;
		}
        ptr += (16-nLeftValidLine)*32*2;
	}
    //process last twomb_macroblock of one line, gpu buf must be 16 byte align or 8 byte align.
    { //last mb of last line
        for(l=0;l<nLeftValidLine;l++)
        {
            //first mb
            m=i*16 + l; //line num, because yub422->yuv420, so discard one line in every two lines, so is 16!
            n= j*16;    //byte num in dst_one_line
            offset = m*coded_width + n;

            dst0_asm = tarCb + offset;
            dst1_asm = tarCr+offset;
            src_asm = ptr;
    //                  for(k=0;k<16;k++)
    //                  {
    //                      dst0_asm[k] = src_asm[2*k];
    //                      dst1_asm[k] = src_asm[2*k+1];
    //                  }
            if(nWidthMatchFlag)
            {
                 asm volatile (
                         "vld2.8         {d0 - d3}, [%[src_asm]]              \n\t"
                         "vst1.8         {d0,d1}, [%[dst0_asm]]              \n\t"
                         "vst1.8         {d2,d3}, [%[dst1_asm]]              \n\t"
                          : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                          :  //[srcY] "r" (srcY)
                          : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                          );
            }
            else
            {
                 asm volatile (
                         "vld2.8         {d0,d1}, [%[src_asm]]              \n\t"
                         "vst1.8         {d0}, [%[dst0_asm]]              \n\t"
                         "vst1.8         {d1}, [%[dst1_asm]]              \n\t"
                          : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
                          :  //[srcY] "r" (srcY)
                          : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                          );
            }
            ptr += 32;
            ptr += 32;
        }
        ptr += (16-nLeftValidLine)*32*2;
    }
}


static void SoftwarePictureScaler(ScalerParameter *cdx_scaler_para)
{
	map32x32_to_yuv_Y((unsigned char*)cdx_scaler_para->addr_y_in,
			          (unsigned char*)cdx_scaler_para->addr_y_out,
			          cdx_scaler_para->width_out,
			          cdx_scaler_para->height_out);

	if (cdx_scaler_para->format_in == CONVERT_COLOR_FORMAT_YUV422MB)
		map32x32_to_yuv_C_422(cdx_scaler_para->mode,
				              (unsigned char*)cdx_scaler_para->addr_c_in,
				              (unsigned char*)cdx_scaler_para->addr_u_out,
				              (unsigned char*)cdx_scaler_para->addr_v_out,
				              cdx_scaler_para->width_out / 2,
				              cdx_scaler_para->height_out / 2);
	else
		map32x32_to_yuv_C(cdx_scaler_para->mode,
				          (unsigned char*)cdx_scaler_para->addr_c_in,
				          (unsigned char*)cdx_scaler_para->addr_u_out,
				          (unsigned char*)cdx_scaler_para->addr_v_out,
				          cdx_scaler_para->width_out / 2,
				          cdx_scaler_para->height_out / 2);

	return;
}

void TransformToYUVPlaner(VideoPicture *pict, void* ybuf)
{
	ScalerParameter cdx_scaler_para;
	int display_height;
	int display_width;
	int display_height_align;
	int display_width_align;
	int dst_c_stride;
	int dst_y_size;
	int dst_c_size;
	int alloc_size;
	int m_UseGPUbufferFlag = 0;

	if(pict == NULL)
		return;
		
	display_height = pict->nBottomOffset - pict->nTopOffset;
	display_width = pict->nRightOffset - pict->nLeftOffset;

    if (pict->nHeight%2 != 0)
    {
        logw("display_height[%d] is odd number!!!", pict->nHeight);
    }
    
	//pict->display_height = (pict->display_height + 7) & (~7);
	display_height_align = (display_height + 1) & (~1);
	display_width_align  = (display_width + 15) & (~15);
    //#if (defined(__CHIP_VERSION_F23) || defined(__CHIP_VERSION_F51) || defined(__CHIP_VERSION_F50))
#if (1 == ADAPT_A10_GPU_RENDER)
    //A10's GPU has a bug, we can avoid it
    if(display_height_align%8 != 0)
    {
        if((display_width_align*display_height_align)%256 != 0)
        {
            display_height_align = (display_height_align+7)&~7;
        }
    }
#endif
	dst_y_size           = display_width_align * display_height_align;

    if(m_UseGPUbufferFlag==1)
    {
        dst_c_stride         = (display_width/2 + 15) & (~15);
        cdx_scaler_para.mode = 0;
    }
	else
	{
        dst_c_stride         = display_width/2;
        cdx_scaler_para.mode = 1;
    }

	dst_c_size           = dst_c_stride * (display_height_align/2);
	alloc_size           = dst_y_size + dst_c_size * 2;

	cdx_scaler_para.format_in  = CONVERT_COLOR_FORMAT_YUV420MB;
	cdx_scaler_para.format_out = CONVERT_COLOR_FORMAT_YUV420PLANNER;
	cdx_scaler_para.width_in   = pict->nWidth;
	cdx_scaler_para.height_in  = pict->nHeight;
	cdx_scaler_para.addr_y_in  = (void*)pict->pData0; /*y*/
	cdx_scaler_para.addr_c_in  = (void*)pict->pData1; /*uv*/
#if 0
	cedarx_cache_op(cdx_scaler_para.addr_y_in, cdx_scaler_para.addr_y_in+pict->size_y, CEDARX_DCACHE_FLUSH);
	cedarx_cache_op(cdx_scaler_para.addr_c_in, cdx_scaler_para.addr_c_in+pict->size_u, CEDARX_DCACHE_FLUSH);
#endif
	cdx_scaler_para.width_out  = display_width_align;
	cdx_scaler_para.height_out = display_height_align;

	cdx_scaler_para.addr_y_out = (unsigned int)ybuf;

    if(m_UseGPUbufferFlag==1)
    {
        cdx_scaler_para.addr_v_out = cdx_scaler_para.addr_y_out + dst_y_size;
	    cdx_scaler_para.addr_u_out = cdx_scaler_para.addr_v_out + dst_c_size;
    }
	else
	{
        cdx_scaler_para.addr_u_out = cdx_scaler_para.addr_y_out + dst_y_size;
	    cdx_scaler_para.addr_v_out = cdx_scaler_para.addr_u_out + dst_c_size;
    }



    //* use neon accelarator instruction to transform the pixel format, slow if buffer is not cached(DMA mode).
	SoftwarePictureScaler(&cdx_scaler_para);

	return;
}

