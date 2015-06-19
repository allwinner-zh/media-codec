#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include <time.h>

int yu12_nv12(unsigned int width, unsigned int height, unsigned char *addr_uv, unsigned char *addr_tmp_uv)
{
	unsigned int i, chroma_bytes;
	unsigned char *u_addr = NULL;
	unsigned char *v_addr = NULL;
	unsigned char *tmp_addr = NULL;

	chroma_bytes = width*height/4;

	u_addr = addr_uv;
	v_addr = addr_uv + chroma_bytes;
	tmp_addr = addr_tmp_uv;

	for(i=0; i<chroma_bytes; i++)
	{
		*(tmp_addr++) = *(u_addr++);
		*(tmp_addr++) = *(v_addr++);
	}

	memcpy(addr_uv, addr_tmp_uv, chroma_bytes*2);	

	return 0;
}

int main()
{
	VencBaseConfig baseConfig;
	VencAllocateBufferParam bufferParam;
	VideoEncoder* pVideoEnc = NULL;
	VencInputBuffer inputBuffer;
	VencOutputBuffer outputBuffer;
	VencHeaderData sps_pps_data;
	VencH264Param h264Param;
	VencH264FixQP fixQP;
	EXIFInfo exifinfo;
	VencCyclicIntraRefresh sIntraRefresh;
	unsigned int src_width,src_height,dst_width,dst_height;
	unsigned char *uv_tmp_buffer = NULL;
	

	VencROIConfig sRoiConfig[4];

	src_width = 1920;
	src_height = 1080;
	dst_width = 1920;
	dst_height = 1080;

	// roi
	sRoiConfig[0].bEnable = 1;
	sRoiConfig[0].index = 0;
	sRoiConfig[0].nQPoffset = 10;
	sRoiConfig[0].sRect.nLeft = 320;
	sRoiConfig[0].sRect.nTop = 180;
	sRoiConfig[0].sRect.nWidth = 320;
	sRoiConfig[0].sRect.nHeight = 180;


	sRoiConfig[1].bEnable = 1;
	sRoiConfig[1].index = 1;
	sRoiConfig[1].nQPoffset = 10;
	sRoiConfig[1].sRect.nLeft = 320;
	sRoiConfig[1].sRect.nTop = 180;
	sRoiConfig[1].sRect.nWidth = 320;
	sRoiConfig[1].sRect.nHeight = 180;


	sRoiConfig[2].bEnable = 1;
	sRoiConfig[2].index = 2;
	sRoiConfig[2].nQPoffset = 10;
	sRoiConfig[2].sRect.nLeft = 320;
	sRoiConfig[2].sRect.nTop = 180;
	sRoiConfig[2].sRect.nWidth = 320;
	sRoiConfig[2].sRect.nHeight = 180;


	sRoiConfig[3].bEnable = 1;
	sRoiConfig[3].index = 3;
	sRoiConfig[3].nQPoffset = 10;
	sRoiConfig[3].sRect.nLeft = 320;
	sRoiConfig[3].sRect.nTop = 180;
	sRoiConfig[3].sRect.nWidth = 320;
	sRoiConfig[3].sRect.nHeight = 180;


	//intraRefresh
	
	sIntraRefresh.bEnable = 1;
	sIntraRefresh.nBlockNumber = 10;

	//fix qp mode
	fixQP.bEnable = 1;
	fixQP.nIQp = 20;
	fixQP.nPQp = 30;
	
	exifinfo.ThumbWidth = 176;
	exifinfo.ThumbHeight = 144;

	//* h264 param
	h264Param.bEntropyCodingCABAC = 1;
	h264Param.nBitrate = 4*1024*1024; /* bps */
	h264Param.nFramerate = 30; /* fps */
	h264Param.nCodingMode = VENC_FRAME_CODING;
//	h264Param.nCodingMode = VENC_FIELD_CODING;
	
	h264Param.nMaxKeyInterval = 30;
	h264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
	h264Param.sProfileLevel.nLevel = VENC_H264Level31;
	h264Param.sQPRange.nMinqp = 10;
	h264Param.sQPRange.nMaxqp = 40;


	int codecType = VENC_CODEC_H264;
	int testNumber = 70;

	strcpy((char*)exifinfo.CameraMake,		"allwinner make test");
	strcpy((char*)exifinfo.CameraModel,		"allwinner model test");
	strcpy((char*)exifinfo.DateTime, 		"2014:02:21 10:54:05");
	strcpy((char*)exifinfo.gpsProcessingMethod,  "allwinner gps");

	exifinfo.Orientation = 0;
	
	exifinfo.ExposureTime.num = 2;
	exifinfo.ExposureTime.den = 1000;

	exifinfo.FNumber.num = 20;
	exifinfo.FNumber.den = 10;
	exifinfo.ISOSpeed = 50;


	exifinfo.ExposureBiasValue.num= -4;
	exifinfo.ExposureBiasValue.den= 1;

	exifinfo.MeteringMode = 1;
	exifinfo.FlashUsed = 0;

	exifinfo.FocalLength.num = 1400;
	exifinfo.FocalLength.den = 100;

	exifinfo.DigitalZoomRatio.num = 4;
	exifinfo.DigitalZoomRatio.den = 1;

	exifinfo.WhiteBalance = 1;
	exifinfo.ExposureMode = 1;

	exifinfo.enableGpsInfo = 1;

	exifinfo.gps_latitude = 23.2368;
	exifinfo.gps_longitude = 24.3244;
	exifinfo.gps_altitude = 1234.5;
	
	exifinfo.gps_timestamp = (long)time(NULL);

	FILE *in_file = NULL;
	FILE *out_file = NULL;

	if(codecType == VENC_CODEC_H264)
	{
		in_file = fopen("/root/mnt/repos/codec-lte/demo/data/stream/1080p.yuv", "r");
		if(in_file == NULL)
		{
			loge("open in_file fail\n");
			return -1;
		}
		
		out_file = fopen("./1080p.264", "wb");
		if(out_file == NULL)
		{
			loge("open out_file fail\n");
			return -1;
		}
	}
	else
	{
		in_file = fopen("/data/camera/720p-30zhen.yuv", "r");
		if(in_file == NULL)
		{
			loge("open in_file fail\n");
			return -1;
		}
		
		out_file = fopen("/data/camera/test.jpg", "wb");
		if(out_file == NULL)
		{
			loge("open out_file fail\n");
			return -1;
		}
	}

	memset(&baseConfig, 0 ,sizeof(VencBaseConfig));
	memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));

	baseConfig.nInputWidth= src_width;
	baseConfig.nInputHeight = src_height;
	baseConfig.nStride = src_width;
	
	baseConfig.nDstWidth = dst_width;
	baseConfig.nDstHeight = dst_height;
	baseConfig.eInputFormat = VENC_PIXEL_YUV420SP;

	bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
	bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
	bufferParam.nBufferNum = 4;
	
	pVideoEnc = VideoEncCreate(codecType);


	if(codecType == VENC_CODEC_JPEG)
	{
	  	VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
	}
	else if(codecType == VENC_CODEC_H264)
	{
		int value;
		
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264Param);

		value = 0;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamIfilter, &value);

		value = 0; //degree
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamRotation, &value);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FixQP, &fixQP);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264CyclicIntraRefresh, &sIntraRefresh);

		value = 720/4;
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamSliceHeight, &value);

		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[0]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[1]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[2]);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[3]);
	}

	VideoEncInit(pVideoEnc, &baseConfig);

	if(codecType == VENC_CODEC_H264)
	{
		unsigned int head_num = 0;
		VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
		fwrite(sps_pps_data.pBuffer, 1, sps_pps_data.nLength, out_file);
		logd("sps_pps_data.nLength: %d", sps_pps_data.nLength);
		for(head_num=0; head_num<sps_pps_data.nLength; head_num++)
			logd("the sps_pps :%02x\n", *(sps_pps_data.pBuffer+head_num));
	}

	if(codecType == VENC_CODEC_JPEG)
	{
		testNumber = 1;
	}

	AllocInputBuffer(pVideoEnc, &bufferParam);

	if(baseConfig.eInputFormat == VENC_PIXEL_YUV420SP)
	{
		uv_tmp_buffer = (unsigned char*)malloc(baseConfig.nInputWidth*baseConfig.nInputHeight/2);
		if(uv_tmp_buffer == NULL)
		{
			loge("malloc uv_tmp_buffer fail\n");
			return -1;
		}
	}
	
	while(testNumber > 0)
	{
		GetOneAllocInputBuffer(pVideoEnc, &inputBuffer);
		{
			unsigned int size1, size2;
			
			size1 = fread(inputBuffer.pAddrVirY, 1, baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
			size2 = fread(inputBuffer.pAddrVirC, 1, baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);

			if((size1!= baseConfig.nInputWidth*baseConfig.nInputHeight) || (size2!= baseConfig.nInputWidth*baseConfig.nInputHeight/2))
			{
				fseek(in_file, 0L, SEEK_SET);
				size1 = fread(inputBuffer.pAddrVirY, 1, baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
				size2 = fread(inputBuffer.pAddrVirC, 1, baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);
			}

			
			if(baseConfig.eInputFormat == VENC_PIXEL_YUV420SP)
			{
				yu12_nv12(baseConfig.nInputWidth, baseConfig.nInputHeight, inputBuffer.pAddrVirC, uv_tmp_buffer);
			}
		}
		
		inputBuffer.bEnableCorp = 0;
		inputBuffer.sCropInfo.nLeft =  240;
		inputBuffer.sCropInfo.nTop  =  240;
		inputBuffer.sCropInfo.nWidth  =  240;
		inputBuffer.sCropInfo.nHeight =  240;

		FlushCacheAllocInputBuffer(pVideoEnc, &inputBuffer);

		AddOneInputBuffer(pVideoEnc, &inputBuffer);
		VideoEncodeOneFrame(pVideoEnc);

		AlreadyUsedInputBuffer(pVideoEnc,&inputBuffer);
		ReturnOneAllocInputBuffer(pVideoEnc, &inputBuffer);

		GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
		//logi("size: %d,%d", outputBuffer.nSize0,outputBuffer.nSize1);

		fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

		if(outputBuffer.nSize1)
		{
			fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
		}
			
		FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);

		if(h264Param.nCodingMode==VENC_FIELD_CODING && codecType==VENC_CODEC_H264)
		{
			GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
			//logi("size: %d,%d", outputBuffer.nSize0,outputBuffer.nSize1);

			fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

			if(outputBuffer.nSize1)
			{
				fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
			}
				
			FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
		}

		testNumber--;
	}

out:	
	fclose(out_file);
	fclose(in_file);
	if(uv_tmp_buffer)
		free(uv_tmp_buffer);

	return 0;
}
