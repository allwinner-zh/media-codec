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
#include "log.h"

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

#if 0 //Don't HAVE_NEON
static void map32x32_to_yuv_Y(unsigned char* srcY, unsigned char* tarY, unsigned int coded_width, unsigned int coded_height)
{
	unsigned int i,j,l,m,n;
	unsigned int mb_width,mb_height,twomb_line,recon_width;
	unsigned long offset;
	unsigned char *ptr;

	ptr = srcY;
	mb_width = (coded_width+15)>>4;
	mb_height = (coded_height+15)>>4;
	twomb_line = (mb_height+1)>>1;
	recon_width = (mb_width+1)&0xfffffffe;

	for(i=0;i<twomb_line;i++)
	{
		for(j=0;j<recon_width;j+=2)
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;
				n= j*16;
				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					memcpy(tarY+offset,ptr,16);
					ptr += 16;
				}
				else
					ptr += 16;

				//second mb
				n= j*16+16;
				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					memcpy(tarY+offset,ptr,16);
					ptr += 16;
				}
				else
					ptr += 16;
			}
		}
	}
}

static void map32x32_to_yuv_C(int mode,unsigned char* srcC,unsigned char* tarCb,unsigned char* tarCr,unsigned int coded_width,unsigned int coded_height)
{
	unsigned int i,j,l,m,n,k;
	unsigned int mb_width,mb_height,fourmb_line,recon_width;
	unsigned char line[16];
	unsigned long offset;
	unsigned char *ptr;

	ptr = srcC;
	mb_width = (coded_width+7)>>3;
	mb_height = (coded_height+7)>>3;
	fourmb_line = (mb_height+3)>>2;
	recon_width = (mb_width+1)&0xfffffffe;

	for(i=0;i<fourmb_line;i++)
	{
		for(j=0;j<recon_width;j+=2)
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;
				n= j*8;
				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					memcpy(line,ptr,16);
					for(k=0;k<8;k++)
					{
						*(tarCb + offset + k) = 0xaa;//line[2*k];
						*(tarCr + offset + k) = 0x55; //line[2*k+1];
					}
					ptr += 16;
				}
				else
					ptr += 16;

				//second mb
				n= j*8+8;
				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					memcpy(line,ptr,16);
					for(k=0;k<8;k++)
					{
						*(tarCb + offset + k) = 0xaa;//line[2*k];
						*(tarCr + offset + k) = 0x55;//line[2*k+1];
					}
					ptr += 16;
				}
				else
					ptr += 16;
			}
		}
	}
}

static void map32x32_to_yuv_C_422(int mode,unsigned char* srcC,unsigned char* tarCb,unsigned char* tarCr,unsigned int coded_width,unsigned int coded_height) {
	;
}

#else


static void map32x32_to_yuv_Y(unsigned char* srcY,
		                      unsigned char* tarY,
		                      unsigned int   coded_width,
		                      unsigned int   coded_height)
{
	unsigned int i,j,l,m,n;
	unsigned int mb_width,mb_height,twomb_line,recon_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst_asm,*src_asm;

	ptr = srcY;
	mb_width = (coded_width+15)>>4;
	mb_height = (coded_height+15)>>4;
	twomb_line = (mb_height+1)>>1;
	recon_width = (mb_width+1)&0xfffffffe;

	for(i=0;i<twomb_line;i++)
	{
		for(j=0;j<mb_width/2;j++)
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;
				n= j*32;
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

				ptr += 32;
			}
		}

		//LOGV("mb_width:%d",mb_width);
		if(mb_width & 1)
		{
			j = mb_width-1;
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;
				n= j*16;
				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					//memcpy(tarY+offset,ptr,16);
					dst_asm = tarY+offset;
					src_asm = ptr;
					asm volatile (
					        "vld1.8         {d0 - d1}, [%[src_asm]]              \n\t"
					        "vst1.8         {d0 - d1}, [%[dst_asm]]              \n\t"
					        : [dst_asm] "+r" (dst_asm), [src_asm] "+r" (src_asm)
					        :  //[srcY] "r" (srcY)
					        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
					        );
				}

				ptr += 16;
				ptr += 16;
			}
		}
	}
}

static void map32x32_to_yuv_C(int mode,
		                      unsigned char* srcC,
		                      unsigned char* tarCb,
		                      unsigned char* tarCr,
		                      unsigned int coded_width,
		                      unsigned int coded_height)
{
	unsigned int i,j,l,m,n,k;
	unsigned int mb_width,mb_height,fourmb_line,recon_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst0_asm,*dst1_asm,*src_asm;
	unsigned char line[16];
	int dst_stride = mode==0 ? (coded_width + 15) & (~15) : coded_width;

	ptr = srcC;
	mb_width = (coded_width+7)>>3;
	mb_height = (coded_height+7)>>3;
	fourmb_line = (mb_height+3)>>2;
	recon_width = (mb_width+1)&0xfffffffe;

	for(i=0;i<fourmb_line;i++)
	{
		for(j=0;j<mb_width/2;j++)
		{
			for(l=0;l<32;l++)
			{
				//first mb
				m=i*32 + l;
				n= j*16;
				if(m<coded_height && n<coded_width)
				{
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
					        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
							"vuzp.8         d0, d1              \n\t"
							"vuzp.8         d2, d3              \n\t"
							"vst1.8         {d0}, [%[dst0_asm]]!              \n\t"
							"vst1.8         {d2}, [%[dst0_asm]]!              \n\t"
							"vst1.8         {d1}, [%[dst1_asm]]!              \n\t"
							"vst1.8         {d3}, [%[dst1_asm]]!              \n\t"
					         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
					         :  //[srcY] "r" (srcY)
					         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
					         );
				}

				ptr += 32;
			}
		}

		if(mb_width & 1)
		{
			j= mb_width-1;
			for(l=0;l<32;l++)
			{
				m=i*32 + l;
				n= j*8;

				if(m<coded_height && n<coded_width)
				{
					offset = m*dst_stride + n;
					memcpy(line,ptr,16);
					for(k=0;k<8;k++)
					{
						*(tarCb + offset + k) = line[2*k];
						*(tarCr + offset + k) = line[2*k+1];
					}
				}

				ptr += 16;
				ptr += 16;
			}
		}
	}
}

static void map32x32_to_yuv_C_422(int mode,
		                          unsigned char* srcC,
		                          unsigned char* tarCb,
		                          unsigned char* tarCr,
		                          unsigned int coded_width,
		                          unsigned int coded_height)
{
	unsigned int i,j,l,m,n,k;
	unsigned int mb_width,mb_height,twomb_line,recon_width;
	unsigned long offset;
	unsigned char *ptr;
	unsigned char *dst0_asm,*dst1_asm,*src_asm;
	unsigned char line[16];

	CEDARX_UNUSE(mode);

	ptr = srcC;
	mb_width = (coded_width+7)>>3;
	mb_height = (coded_height+7)>>3;
	twomb_line = (mb_height+1)>>1;
	recon_width = (mb_width+1)&0xfffffffe;

	for(i=0;i<twomb_line;i++)
	{
		for(j=0;j<mb_width/2;j++)
		{
			for(l=0;l<16;l++)
			{
				//first mb
				m=i*16 + l;
				n= j*16;
				if(m<coded_height && n<coded_width)
				{
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
					        "vld1.8         {d0 - d3}, [%[src_asm]]              \n\t"
							"vuzp.8         d0, d1              \n\t"
							"vuzp.8         d2, d3              \n\t"
							"vst1.8         {d0}, [%[dst0_asm]]!              \n\t"
							"vst1.8         {d2}, [%[dst0_asm]]!              \n\t"
							"vst1.8         {d1}, [%[dst1_asm]]!              \n\t"
							"vst1.8         {d3}, [%[dst1_asm]]!              \n\t"
					         : [dst0_asm] "+r" (dst0_asm), [dst1_asm] "+r" (dst1_asm), [src_asm] "+r" (src_asm)
					         :  //[srcY] "r" (srcY)
					         : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
					         );
				}

				ptr += 32;
				ptr += 32;
			}
		}

		if(mb_width & 1)
		{
			j= mb_width-1;
			for(l=0;l<16;l++)
			{
				m=i*32 + l;
				n= j*8;

				if(m<coded_height && n<coded_width)
				{
					offset = m*coded_width + n;
					memcpy(line,ptr,16);
					for(k=0;k<8;k++)
					{
						*(tarCb + offset + k) = line[2*k];
						*(tarCr + offset + k) = line[2*k+1];
					}
				}

				ptr += 32;
				ptr += 32;
			}
		}
	}
}
#endif


static void SoftwarePictureScaler(ScalerParameter *cdx_scaler_para)
{
	map32x32_to_yuv_Y((unsigned char*)cdx_scaler_para->addr_y_in,
			          (unsigned char*)cdx_scaler_para->addr_y_out,
			          (unsigned int)cdx_scaler_para->width_out,
			          (unsigned int)cdx_scaler_para->height_out);

	if (cdx_scaler_para->format_in == CONVERT_COLOR_FORMAT_YUV422MB)
		map32x32_to_yuv_C_422(cdx_scaler_para->mode,
				              (unsigned char*)cdx_scaler_para->addr_c_in,
				              (unsigned char*)cdx_scaler_para->addr_u_out,
				              (unsigned char*)cdx_scaler_para->addr_v_out,
				              (unsigned int)cdx_scaler_para->width_out / 2,
				              (unsigned int)cdx_scaler_para->height_out / 2);
	else
		map32x32_to_yuv_C(cdx_scaler_para->mode,
				          (unsigned char*)cdx_scaler_para->addr_c_in,
				          (unsigned char*)cdx_scaler_para->addr_u_out,
				          (unsigned char*)cdx_scaler_para->addr_v_out,
				          (unsigned int)cdx_scaler_para->width_out / 2,
				          (unsigned int)cdx_scaler_para->height_out / 2);

	return;
}

void TransformMB32ToYV12(VideoPicture* pict, void* ybuf)
{
	ScalerParameter cdx_scaler_para;
	int             display_height_align;
	int             display_width_align;
	int             dst_c_stride;
	int             dst_y_size;
	int             dst_c_size;
	int             alloc_size;

	if(pict == NULL)
		return;

	pict->nHeight = (pict->nHeight + 7) & (~7);
	display_height_align = (pict->nHeight + 1) & (~1);
	
#if(CONFIG_CHIP == OPTION_CHIP_1673)    
    display_width_align  = (pict->nWidth+ 31) & (~31);//on chip-1673, gpu is 32pixel align
#else
    display_width_align  = (pict->nWidth+ 15) & (~15);//other chip, gpu buffer is 16 align
#endif

	dst_y_size           = display_width_align * display_height_align;
	dst_c_stride         = (pict->nWidth/2 + 15) & (~15);
	dst_c_size           = dst_c_stride * (display_height_align/2);
	alloc_size           = dst_y_size + dst_c_size * 2;

	cdx_scaler_para.mode       = 0;
	cdx_scaler_para.format_in  = (pict->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_422) ? PIXEL_FORMAT_YUV_MB32_422 : PIXEL_FORMAT_YUV_MB32_420;
	cdx_scaler_para.format_out = CONVERT_COLOR_FORMAT_YUV420PLANNER;
	cdx_scaler_para.width_in   = pict->nWidth;
	cdx_scaler_para.height_in  = pict->nHeight;
	cdx_scaler_para.addr_y_in  = (void*)pict->pData0;
	cdx_scaler_para.addr_c_in  = (void*)pict->pData1;
#if 0
	cedarx_cache_op(cdx_scaler_para.addr_y_in, cdx_scaler_para.addr_y_in+pict->size_y, CEDARX_DCACHE_FLUSH);
	cedarx_cache_op(cdx_scaler_para.addr_c_in, cdx_scaler_para.addr_c_in+pict->size_u, CEDARX_DCACHE_FLUSH);
#endif
	cdx_scaler_para.width_out  = display_width_align;
	cdx_scaler_para.height_out = display_height_align;

	cdx_scaler_para.addr_y_out = (unsigned int)ybuf;
	cdx_scaler_para.addr_v_out = cdx_scaler_para.addr_y_out + dst_y_size;
	cdx_scaler_para.addr_u_out = cdx_scaler_para.addr_v_out + dst_c_size;

    //* use neon accelarator instruction to transform the pixel format, slow if buffer is not cached(DMA mode).
	SoftwarePictureScaler(&cdx_scaler_para);

	return;
}

void TransformYV12ToYUV420(VideoPicture* pict, void* ybuf)
{
    int i;
    int nPicRealWidth;
    int nPicRealHeight;
    int nSrcBufWidth;
    int nSrcBufHeight;
    int nDstBufWidth;
    int nDstBufHeight;
    int nCopyDataWidth;
    int nCopyDataHeight;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    dstPtr      = (unsigned char*)ybuf;
    srcPtr      = (unsigned char*)pict->pData0;
    
#if 0
    memcpy(dstPtr, srcPtr, 640*480);
    //memcpy(dstPtr+640*480, srcPtr+640*480, 640*480/2);
    memcpy(dstPtr+640*480, pict->pData1, 640*480/2);
#endif

    nPicRealWidth  = pict->nRightOffset - pict->nLeftOffset;
    nPicRealHeight = pict->nBottomOffset - pict->nTopOffset;

    //* if the offset is not right, we should not use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = pict->nWidth;
        nPicRealHeight = pict->nHeight;
    }

    nSrcBufWidth  = (pict->nWidth + 15) & ~15;
    nSrcBufHeight = (pict->nHeight + 15) & ~15;

    //* On chip-1673, the gpu is 32 align ,but here is not copy to gpu, so also 16 align.
    //* On other chip, gpu buffer is 16 align.
    //nDstBufWidth = (nPicRealWidth + 15)&~15;
    nDstBufWidth = nPicRealWidth;
    nDstBufHeight = nPicRealHeight;

    nCopyDataWidth  = nPicRealWidth;
    nCopyDataHeight = nPicRealHeight;

    logi("nPicRealWidth & H = %d, %d, nSrcBufWidth & H = %d, %d, nDstBufWidth & H = %d, %d, nCopyDataWidth & H = %d, %d",
          nPicRealWidth,nPicRealHeight,
          nSrcBufWidth,nSrcBufHeight,
          nDstBufWidth,nDstBufHeight,
          nCopyDataWidth,nCopyDataHeight);
    
    //*copy y
    for(i=0; i < nCopyDataHeight; i++)
    {
    	memcpy(dstPtr, srcPtr, nCopyDataWidth);
    	dstPtr += nDstBufWidth;
    	srcPtr += nSrcBufWidth;
    }

    //*copy u
    srcPtr = ((unsigned char*)pict->pData0) + nSrcBufWidth*nSrcBufHeight*5/4;
    nCopyDataWidth  = (nCopyDataWidth+1)/2;
    nCopyDataHeight = (nCopyDataHeight+1)/2;
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;
        srcPtr += nSrcBufWidth/2;
    }

    //*copy v
    srcPtr = ((unsigned char*)pict->pData0) + nSrcBufWidth*nSrcBufHeight;
    for(i=0; i<nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth/2;    //a31 gpu, uv is half of y
        srcPtr += nSrcBufWidth/2;
    }
    
    //logd("xxxxxxxxxxxxxxxxxxxxxxxxx  cp data len=%d", (dstPtr-(unsigned char*)ybuf));
    return;
}

void TransformYV12ToYUV420Soft(VideoPicture* pict, void* ybuf)
{
    int i;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    int copyHeight;
    int copyWidth;
    int dstWidth;
    int dstHeight;

    dstPtr = (unsigned char*)ybuf;
    srcPtr = (unsigned char*)pict->pData0 + (pict->nWidth * pict->nTopOffset + pict->nLeftOffset);
    copyHeight = pict->nBottomOffset - pict->nTopOffset;
    copyWidth  = pict->nRightOffset - pict->nLeftOffset;
    
    //* On chip-1673, the gpu is 32 align ,but here is not copy to gpu, so also 16 align.
    //* On other chip, gpu buffer is 16 align.
    dstWidth   = (copyWidth+15)&~15; 
    dstHeight  = copyHeight;

    logv("c dst w: %d,  dst h: %d;;   src w: %d, src h: %d; top: %d, left: %d, b:%d, r:%d",
          dstWidth, dstHeight, copyWidth, copyHeight,
          pict->nTopOffset, pict->nLeftOffset, pict->nBottomOffset, pict->nRightOffset);

    //*copy y
    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth;
        dstPtr += dstWidth;
    }

    //*copy v
    srcPtr = ((unsigned char*)pict->pData0) + pict->nWidth * pict->nHeight * 5/4 + \
               pict->nWidth * pict->nTopOffset / 4 + pict->nLeftOffset / 2 ;

    dstHeight = (dstHeight+1)/2;
    dstWidth  = dstWidth/2;
    copyWidth = copyWidth/2;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth/2;
        dstPtr += dstWidth;
    }

    //copy u
    srcPtr =((unsigned char*)pict->pData0) + pict->nWidth * pict->nHeight + \
              pict->nWidth * pict->nTopOffset / 4 + pict->nLeftOffset / 2 ;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth/2;
        dstPtr += dstWidth;
    }  
    
    return;
}

void TransformYV12ToYV12Hw(VideoPicture* pict, void* ybuf)
{
    int i;
    int nPicRealWidth;
    int nPicRealHeight;
    int nSrcBufWidth;
    int nSrcBufHeight;
    int nDstBufWidth;
    int nDstBufHeight;
    int nCopyDataWidth;
    int nCopyDataHeight;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    dstPtr      = (unsigned char*)ybuf;
    srcPtr      = (unsigned char*)pict->pData0;

    nPicRealWidth  = pict->nRightOffset - pict->nLeftOffset;
    nPicRealHeight = pict->nBottomOffset - pict->nTopOffset;

    //* if the offset is not right, we should use them to compute width and height
    if(nPicRealWidth <= 0 || nPicRealHeight <= 0)
    {
        nPicRealWidth  = pict->nWidth;
        nPicRealHeight = pict->nHeight;
    }

    nSrcBufWidth  = (pict->nWidth + 15) & ~15;
    nSrcBufHeight = (pict->nHeight + 15) & ~15;
    
#if(CONFIG_CHIP == OPTION_CHIP_1673)
    nDstBufWidth = (nPicRealWidth + 31)&~31;   //on chip-1673, gpu is 32pixel align
#else
    nDstBufWidth = (nPicRealWidth + 15)&~15;   //other chip, gpu is 16pixel align
#endif

    nDstBufHeight = nPicRealHeight;

    nCopyDataWidth  = nPicRealWidth;
    nCopyDataHeight = nPicRealHeight;

    logv("nPicRealWidth & H = %d, %d, nSrcBufWidth & H = %d, %d, nDstBufWidth & H = %d, %d, nCopyDataWidth & H = %d, %d",
          nPicRealWidth,nPicRealHeight,
          nSrcBufWidth,nSrcBufHeight,
          nDstBufWidth,nDstBufHeight,
          nCopyDataWidth,nCopyDataHeight);
    
    //*copy y
    for(i=0; i < nCopyDataHeight; i++)
    {
    	memcpy(dstPtr, srcPtr, nCopyDataWidth);
    	dstPtr += nDstBufWidth;
    	srcPtr += nSrcBufWidth;
    }

	nCopyDataWidth  = (nCopyDataWidth+1)/2;
    nCopyDataHeight = (nCopyDataHeight+1)/2;

//* the uv is 16 align on 1677 and 1680 

#if(CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667)
    nDstBufWidth = (nDstBufWidth/2 + 15)&~15;
#else
    nDstBufWidth = nDstBufWidth/2;
#endif
    //*copy v
    srcPtr = ((unsigned char*)pict->pData0) + nSrcBufWidth*nSrcBufHeight;
    for(i=0; i < nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth/2;
    }

    //*copy u
    srcPtr = ((unsigned char*)pict->pData0) + nSrcBufWidth*nSrcBufHeight*5/4;
    for(i=0; i<nCopyDataHeight; i++)
    {
        memcpy(dstPtr, srcPtr, nCopyDataWidth);
        dstPtr += nDstBufWidth;
        srcPtr += nSrcBufWidth/2;
    }
    
    return;
}

void TransformYV12ToYV12Soft(VideoPicture* pict, void* ybuf)
{
    int i;
    unsigned char* dstPtr;
    unsigned char* srcPtr;
    int copyHeight;
    int copyWidth;
    int dstWidth;
    int dstHeight;

    dstPtr = (unsigned char*)ybuf;
    srcPtr = (unsigned char*)pict->pData0 + (pict->nWidth * pict->nTopOffset + pict->nLeftOffset);
    copyHeight = pict->nBottomOffset - pict->nTopOffset;
    copyWidth  = pict->nRightOffset - pict->nLeftOffset;
#if(CONFIG_CHIP == OPTION_CHIP_1673)
    dstWidth   = (copyWidth+31)&~31;   //on chip-1673, gpu is 32pixel align
#else
    dstWidth   = (copyWidth+15)&~15;   //other chip, gpu is 16pixel align
#endif
    dstHeight  = copyHeight;

    logv("c dst w: %d,  dst h: %d;;   src w: %d, src h: %d; top: %d, left: %d, b:%d, r:%d",
          dstWidth, dstHeight, copyWidth, copyHeight,
          pict->nTopOffset, pict->nLeftOffset, pict->nBottomOffset, pict->nRightOffset);

    //*copy y
    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth;
        dstPtr += dstWidth;
    }

    //*copy v
    srcPtr =((unsigned char*)pict->pData0) + pict->nWidth * pict->nHeight + \
              pict->nWidth * pict->nTopOffset / 4 + pict->nLeftOffset / 2 ;

    dstHeight = (dstHeight+1)/2;

//* the uv is 16 align on 1677 and 1680

#if(CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667)
    dstWidth  = (dstWidth/2+15)&~15;
#else
    dstWidth  = dstWidth/2;
#endif
    copyWidth = copyWidth/2;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth/2;
        dstPtr += dstWidth;
    }

    //copy u
    srcPtr = ((unsigned char*)pict->pData0) + pict->nWidth * pict->nHeight * 5/4 + \
               pict->nWidth * pict->nTopOffset / 4 + pict->nLeftOffset / 2 ;

    for(i = 0; i < dstHeight; i++)
    {
        memcpy(dstPtr, srcPtr, copyWidth);
        srcPtr += pict->nWidth/2;
        dstPtr += dstWidth;
    }  
    
    return;
}

#if 0
void TransformToGPUBuffer0(cedarv_picture_t* pict, void* ybuf)
{
	ScalerParameter cdx_scaler_para;
	int             display_height_align;
	int             display_width_align;
	int             dst_c_stride;
	int             dst_y_size;
	int             dst_c_size;
	int             alloc_size;

//    logd("omx transformtyv12, pict->display_width[%d], pict->display_height[%d], pict:size_y[%d],size_u[%d]",
//        pict->display_width, pict->display_height, pict->size_y, pict->size_u);
    //memcpy(ybuf, pict->y, pict->size_y + pict->size_u); 

    {
    	int i;
    	int widthAlign;
    	int heightAlign;
    	int cHeight;
    	int cWidth;
    	int dstCStride;
        int GPUFBStride;
    	unsigned char* dstPtr;
    	unsigned char* srcPtr;
    	dstPtr = (unsigned char*)ybuf;
    	//srcPtr = (unsigned char*)cedarv_address_phy2vir((void*)pOverlayParam->addr[0]);
        srcPtr = pict->y;
    	//widthAlign = (mWidth + 15) & ~15;
    	//heightAlign = (mHeight + 15) & ~15;
        widthAlign = (pict->display_width+15)&~15;  //hw_decoder is 16pixel align
    	heightAlign = (pict->display_height+15)&~15;
        GPUFBStride = (pict->display_width + 31)&~31;   //gpu is 32pixel align
    	for(i=0; i<heightAlign; i++)
    	{
    		memcpy(dstPtr, srcPtr, widthAlign);
    		dstPtr += GPUFBStride;
    		srcPtr += widthAlign;
    	}
    	//cWidth = (mWidth/2 + 15) & ~15;
    	cWidth = (pict->display_width/2 + 15) & ~15;    //equal to GPUFBStride/2. hw_decoder's uv is 16pixel align
    	cHeight = heightAlign;
    	for(i=0; i<cHeight; i++)
    	{
    		memcpy(dstPtr, srcPtr, cWidth);
    		dstPtr += cWidth;
    		srcPtr += cWidth;
    	}
    }
    return;
}
#endif

