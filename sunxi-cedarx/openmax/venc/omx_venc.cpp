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

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

*//** @file omx_venc.cpp
  This module contains the implementation of the OpenMAX core & component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include "omx_venc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryAdapter.h"

#if CONFIG_OS == OPTION_OS_ANDROID
#include "MetadataBufferType.h"
#include <ion/ion.h>
#include <HardwareAPI.h>
#include <utils/CallStack.h>
#include <binder/IPCThreadState.h>
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>

#if (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0 && CONFIG_CHIP == OPTION_CHIP_1667)
#include "gralloc_priv.h"
#endif


#if (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4)
#include "hardware/hal_public.h"
#elif (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2)
#else
#include "hardware/hal_public.h"
#endif

using namespace android;

#endif

#define SAVE_BITSTREAM 0

#if SAVE_BITSTREAM
static FILE *OutFile = NULL;
#endif

extern "C" void ImgRGBA2YUV420SP_neon(unsigned char *pu8RgbBuffer, unsigned char** pu8SrcYUV, int *l32Width_stride, int l32Height);

#define DEFAULT_BITRATE 1024*1024*2


/* H.263 Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
  {-1, -1}};
/* AVC Supported Levels & profiles  for cts*/
VIDEO_PROFILE_LEVEL_TYPE CTSSupportedAVCProfileLevels[] ={
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
   {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},

   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
   {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},

   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
   {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},

   {-1,-1}
};
/* AVC Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] ={
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevelMax},

  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevelMax},
  
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },        
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},      
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},     
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},     
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},    
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },    
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},   
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},  
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },  
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevelMax},

  {-1,-1}};

 
/*
 *     M A C R O S
 */

/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_s_, _name_)	\
    memset((_s_), 0x0, sizeof(_name_));	\
    (_s_)->nSize = sizeof(_name_);		\
    (_s_)->nVersion.s.nVersionMajor = 0x1;	\
    (_s_)->nVersion.s.nVersionMinor = 0x1;	\
    (_s_)->nVersion.s.nRevision = 0x0;		\
    (_s_)->nVersion.s.nStep = 0x0


typedef enum VIDENC_CUSTOM_INDEX
{
    VideoEncodeCustomParamStoreMetaDataInBuffers = OMX_IndexVendorStartUnused,
	VideoEncodeCustomParamPrependSPSPPSToIDRFrames,
	VideoEncodeCustomParamEnableAndroidNativeBuffers
} VIDENC_CUSTOM_INDEX;


static VIDDEC_CUSTOM_PARAM sVideoEncCustomParams[] =
{
	{"OMX.google.android.index.storeMetaDataInBuffers", (OMX_INDEXTYPE)VideoEncodeCustomParamStoreMetaDataInBuffers},
	{"OMX.google.android.index.prependSPSPPSToIDRFrames",(OMX_INDEXTYPE)VideoEncodeCustomParamPrependSPSPPSToIDRFrames},
	{"OMX.google.android.index.enableAndroidNativeBuffers", (OMX_INDEXTYPE)VideoEncodeCustomParamEnableAndroidNativeBuffers}
};

typedef enum OMX_VENC_COMMANDTYPE
{
	OMX_Venc_Cmd_Open,
	OMX_Venc_Cmd_Close,
    OMX_Venc_Cmd_Stop,
    OMX_Venc_Cmd_Enc_Idle,
    OMX_Venc_Cmd_ChangeBitrate,
    OMX_Venc_Cmd_ChangeColorFormat,
    OMX_Venc_Cmd_RequestIDRFrame,
} OMX_VENC_COMMANDTYPE;


typedef enum OMX_VENC_INPUTBUFFER_STEP
{
	OMX_VENC_STEP_GET_INPUTBUFFER,
	OMX_VENC_STEP_GET_ALLOCBUFFER,
    OMX_VENC_STEP_ADD_BUFFER_TO_ENC,
} OMX_VENC_INPUTBUFFER_STEP;


static void* ComponentThread(void* pThreadData);
static void* ComponentVencThread(void* pThreadData);


#if CONFIG_OS == OPTION_OS_ANDROID

#define GET_CALLING_PID	(IPCThreadState::self()->getCallingPid())
void getCallingProcessName(char *name)
{
	char proc_node[128];

	if (name == 0)
	{
		loge("error in params");
		return;
	}
	
	memset(proc_node, 0, sizeof(proc_node));
	sprintf(proc_node, "/proc/%d/cmdline", GET_CALLING_PID);
	int fp = ::open(proc_node, O_RDONLY);
	if (fp > 0) 
	{
		memset(name, 0, 128);
		::read(fp, name, 128);
		::close(fp);
		fp = 0;
		logd("Calling process is: %s", name);
	}
	else 
	{
		loge("Obtain calling process failed");
	}
}

#endif

//* factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
	logd("----get_omx_component_factory_fn");
	return (new omx_venc);
}


void post_message_to_venc_and_wait(omx_venc *omx, OMX_S32 id)
{
      int ret_value;
      logv("omx_venc: post_message %d pipe out%d to venc\n", (int)id,omx->m_venc_cmdpipe[1]);
      ret_value = write(omx->m_venc_cmdpipe[1], &id, sizeof(OMX_S32));
      logv("post_message to pipe done %d\n",ret_value);
	  omx_sem_down(&omx->m_msg_sem);
}

void post_message_to_venc(omx_venc *omx, OMX_S32 id)
{
      int ret_value;
      logv("omx_vdec: post_message %d pipe out%d to venc\n", (int)id,omx->m_venc_cmdpipe[1]);
      ret_value = write(omx->m_venc_cmdpipe[1], &id, sizeof(OMX_S32));
      logv("post_message to pipe done %d\n",ret_value);
}

omx_venc::omx_venc()
{
	m_state              = OMX_StateLoaded;
	m_cRole[0]           = 0;
	m_cName[0]           = 0;
	m_eCompressionFormat = OMX_VIDEO_CodingUnused;
	m_pAppData           = NULL;
	m_thread_id          = 0;
	m_venc_thread_id	 = 0;
	m_cmdpipe[0]         = 0;
	m_cmdpipe[1]         = 0;
	m_cmddatapipe[0]     = 0;
	m_cmddatapipe[1]     = 0;
	m_venc_cmdpipe[0]   = 0;
    m_venc_cmdpipe[1]   = 0;
	m_firstFrameFlag     = OMX_FALSE;
	m_framerate          = 30;
	mIsFromCts           = OMX_FALSE;
	mIsFromVideoeditor		= OMX_FALSE;
	m_useAllocInputBuffer = OMX_FALSE;
	mFirstInputFrame     = OMX_TRUE;

    memset(mCallingProcess,0,sizeof(mCallingProcess));
    
#if CONFIG_OS == OPTION_OS_ANDROID	
	getCallingProcessName(mCallingProcess);
	if((strcmp(mCallingProcess, "com.android.cts.media") == 0) || (strcmp(mCallingProcess, "com.android.cts.videoperf") == 0) || (strcmp(mCallingProcess, "com.android.pts.videoperf") == 0))
	{
		mIsFromCts           = OMX_TRUE;
	}
	else if(strcmp(mCallingProcess, "com.android.videoeditor") == 0){
		mIsFromVideoeditor   = OMX_TRUE;
	}
#endif

	m_encoder = NULL;

	memset(&m_Callbacks, 0, sizeof(m_Callbacks));
	memset(&m_sInPortDef, 0, sizeof(m_sInPortDef));
	memset(&m_sOutPortDef, 0, sizeof(m_sOutPortDef));
	memset(&m_sInPortFormat, 0, sizeof(m_sInPortFormat));
	memset(&m_sOutPortFormat, 0, sizeof(m_sOutPortFormat));
	memset(&m_sPriorityMgmt, 0, sizeof(m_sPriorityMgmt));
	memset(&m_sInBufSupplier, 0, sizeof(m_sInBufSupplier));
	memset(&m_sOutBufSupplier, 0, sizeof(m_sOutBufSupplier));
	memset(&m_sInBufList, 0, sizeof(m_sInBufList));
	memset(&m_sOutBufList, 0, sizeof(m_sOutBufList));

	pthread_mutex_init(&m_inBufMutex, NULL);
	pthread_mutex_init(&m_outBufMutex, NULL);
	pthread_mutex_init(&m_pipeMutex, NULL);

	omx_sem_init(&m_msg_sem, 0);
	omx_sem_init(&m_input_sem, 0);
	
    logv("omx_enc Create!");
}


omx_venc::~omx_venc()
{
	OMX_S32 nIndex;
    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.


	pthread_mutex_lock(&m_inBufMutex);

	for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
	{
		if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
		{
			if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
			{
				free(m_sInBufList.pBufArr[nIndex].pBuffer);
				m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
			}
		}
	}

    if (m_sInBufList.pBufArr != NULL)
    	free(m_sInBufList.pBufArr);

    if (m_sInBufList.pBufHdrList != NULL)
    	free(m_sInBufList.pBufHdrList);

	memset(&m_sInBufList, 0, sizeof(struct _BufferList));
	m_sInBufList.nBufArrSize = m_sInPortDef.nBufferCountActual;

	pthread_mutex_unlock(&m_inBufMutex);


	pthread_mutex_lock(&m_outBufMutex);

	for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
	{
		if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
		{
			if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
			{
				free(m_sOutBufList.pBufArr[nIndex].pBuffer);
				m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
			}
		}
	}

    if (m_sOutBufList.pBufArr != NULL)
    	free(m_sOutBufList.pBufArr);

    if (m_sOutBufList.pBufHdrList != NULL)
    	free(m_sOutBufList.pBufHdrList);

	memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
	m_sOutBufList.nBufArrSize = m_sOutPortDef.nBufferCountActual;

	pthread_mutex_unlock(&m_outBufMutex);


    pthread_mutex_destroy(&m_inBufMutex);
    pthread_mutex_destroy(&m_outBufMutex);
	pthread_mutex_destroy(&m_pipeMutex);

	omx_sem_deinit(&m_msg_sem);
	omx_sem_deinit(&m_input_sem);

    logv("~omx_enc done!");
}

OMX_ERRORTYPE omx_venc::component_init(OMX_STRING name)
{
	OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int           err;
    OMX_U32       nIndex;

	logd(" COMPONENT_INIT, name = %s", name);

	strncpy((char*)m_cName, name, OMX_MAX_STRINGNAME_SIZE);
	
	if(!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
	{
		strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
		m_eCompressionFormat = OMX_VIDEO_CodingAVC;
	}
	else if(!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.h263", OMX_MAX_STRINGNAME_SIZE))
	{
		strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
		m_eCompressionFormat = OMX_VIDEO_CodingH263;
	}
	else if(!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
	{
		strncpy((char*)m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
		m_eCompressionFormat = OMX_VIDEO_CodingMPEG4;
	}
	else
	{
		logv("\nERROR:Unknown Component\n");
	    eRet = OMX_ErrorInvalidComponentName;
	    return eRet;
	}

	// init codec type
	if(OMX_VIDEO_CodingAVC == m_eCompressionFormat) 
	{
		m_vencCodecType = VENC_CODEC_H264;
	}
	else if(OMX_VIDEO_CodingVP8 == m_eCompressionFormat)
	{
		m_vencCodecType = VENC_CODEC_VP8;
	}
	else if(OMX_VIDEO_CodingMJPEG == m_eCompressionFormat)
	{
		m_vencCodecType = VENC_CODEC_JPEG;
	}
	else
	{
		logd("need check codec type");
	}

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&m_sPortParam, OMX_PORT_PARAM_TYPE);
    m_sPortParam.nPorts           = 0x2;
    m_sPortParam.nStartPortNumber = 0x0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sInPortDef.nPortIndex 					 = 0x0;
    m_sInPortDef.bEnabled 						 = OMX_TRUE;
    m_sInPortDef.bPopulated 					 = OMX_FALSE;
    m_sInPortDef.eDomain 						 = OMX_PortDomainVideo;
    m_sInPortDef.format.video.nFrameWidth 		 = 0;
    m_sInPortDef.format.video.nFrameHeight 		 = 0;
    m_sInPortDef.eDir 							 = OMX_DirInput;
    m_sInPortDef.nBufferCountMin 				 = NUM_IN_BUFFERS;
    m_sInPortDef.nBufferCountActual 			 = NUM_IN_BUFFERS;
    m_sInPortDef.nBufferSize 					 = (OMX_U32)(m_sOutPortDef.format.video.nFrameWidth*m_sOutPortDef.format.video.nFrameHeight*3/2);
    m_sInPortDef.format.video.eColorFormat 		 = OMX_COLOR_FormatYUV420SemiPlanar; //fix it later

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    m_sOutPortDef.nPortIndex 				= 0x1;
    m_sOutPortDef.bEnabled 					= OMX_TRUE;
    m_sOutPortDef.bPopulated 				= OMX_FALSE;
    m_sOutPortDef.eDomain 					= OMX_PortDomainVideo;
    m_sOutPortDef.format.video.cMIMEType 	= (OMX_STRING)"YUV420";
    m_sOutPortDef.format.video.nFrameWidth 	= 176;
    m_sOutPortDef.format.video.nFrameHeight = 144;
    m_sOutPortDef.eDir 						= OMX_DirOutput;
    m_sOutPortDef.nBufferCountMin 			= NUM_OUT_BUFFERS;
    m_sOutPortDef.nBufferCountActual 		= NUM_OUT_BUFFERS;
    m_sOutPortDef.nBufferSize 				= OMX_VIDEO_ENC_INPUT_BUFFER_SIZE;

	m_sOutPortDef.format.video.eCompressionFormat = m_eCompressionFormat;
    m_sOutPortDef.format.video.cMIMEType 		 = (OMX_STRING)"";

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&m_sInPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sInPortFormat.nPortIndex         = 0x0;
    m_sInPortFormat.nIndex             = 0x1;
	m_sInPortFormat.eColorFormat 	   = OMX_COLOR_FormatYUV420SemiPlanar;

	m_inputcolorFormats[0] = OMX_COLOR_FormatYUV420SemiPlanar;
	m_inputcolorFormats[1] = OMX_COLOR_FormatAndroidOpaque;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&m_sOutPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    m_sOutPortFormat.nPortIndex   = 0x1;
    m_sOutPortFormat.nIndex       = 0x0;
    m_sOutPortFormat.eCompressionFormat = m_eCompressionFormat;

    OMX_CONF_INIT_STRUCT_PTR(&m_sPriorityMgmt, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&m_sInBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sInBufSupplier.nPortIndex = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE );
    m_sOutBufSupplier.nPortIndex = 0x1;

	// Initalize the output bitrate
	OMX_CONF_INIT_STRUCT_PTR(&m_sOutPutBitRateType, OMX_VIDEO_PARAM_BITRATETYPE);
    m_sOutPutBitRateType.nPortIndex = 0x01;
    m_sOutPutBitRateType.eControlRate = OMX_Video_ControlRateDisable;
    m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE; //default bitrate

    // Initalize the output h264param
	OMX_CONF_INIT_STRUCT_PTR(&m_sH264Type, OMX_VIDEO_PARAM_AVCTYPE);
    m_sH264Type.nPortIndex                = 0x01;
    m_sH264Type.nSliceHeaderSpacing       = 0;
    m_sH264Type.nPFrames                  = -1;
    m_sH264Type.nBFrames                  = -1;
    m_sH264Type.bUseHadamard              = OMX_TRUE; /*OMX_FALSE*/
    m_sH264Type.nRefFrames                = 1; /*-1;  */
    m_sH264Type.nRefIdx10ActiveMinus1     = -1;
    m_sH264Type.nRefIdx11ActiveMinus1     = -1;
    m_sH264Type.bEnableUEP                = OMX_FALSE;
    m_sH264Type.bEnableFMO                = OMX_FALSE;
    m_sH264Type.bEnableASO                = OMX_FALSE;
    m_sH264Type.bEnableRS                 = OMX_FALSE;
    m_sH264Type.eProfile                  = OMX_VIDEO_AVCProfileBaseline; /*0x01;*/
    m_sH264Type.eLevel                    = OMX_VIDEO_AVCLevel1; /*OMX_VIDEO_AVCLevel11;*/
    m_sH264Type.nAllowedPictureTypes      = -1;
    m_sH264Type.bFrameMBsOnly             = OMX_FALSE;
    m_sH264Type.bMBAFF                    = OMX_FALSE;
    m_sH264Type.bEntropyCodingCABAC       = OMX_FALSE;
    m_sH264Type.bWeightedPPrediction      = OMX_FALSE;
    m_sH264Type.nWeightedBipredicitonMode = -1;
    m_sH264Type.bconstIpred               = OMX_FALSE;
    m_sH264Type.bDirect8x8Inference       = OMX_FALSE;
    m_sH264Type.bDirectSpatialTemporal    = OMX_FALSE;
    m_sH264Type.nCabacInitIdc             = -1;
    m_sH264Type.eLoopFilterMode           = OMX_VIDEO_AVCLoopFilterDisable;


    // Initialize the input buffer list
    memset(&(m_sInBufList), 0x0, sizeof(BufferList));

    m_sInBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) * m_sInPortDef.nBufferCountActual);
    if(m_sInBufList.pBufArr == NULL)
    {
    	eRet = OMX_ErrorInsufficientResources;
    	goto EXIT;
    }

    memset(m_sInBufList.pBufArr, 0, sizeof(OMX_BUFFERHEADERTYPE) * m_sInPortDef.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sInPortDef.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    m_sInBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) * m_sInPortDef.nBufferCountActual);
    if(m_sInBufList.pBufHdrList == NULL)
    {
    	eRet = OMX_ErrorInsufficientResources;
    	goto EXIT;
    }

    m_sInBufList.nSizeOfList       = 0;
    m_sInBufList.nAllocSize        = 0;
    m_sInBufList.nWritePos         = 0;
    m_sInBufList.nReadPos          = 0;
    m_sInBufList.nAllocBySelfFlags = 0;
    m_sInBufList.nSizeOfList       = 0;
    m_sInBufList.nBufArrSize       = m_sInPortDef.nBufferCountActual;
    m_sInBufList.eDir              = OMX_DirInput;

    // Initialize the output buffer list
    memset(&m_sOutBufList, 0x0, sizeof(BufferList));

    m_sOutBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) * m_sOutPortDef.nBufferCountActual);
    if(m_sOutBufList.pBufArr == NULL)
    {
    	eRet = OMX_ErrorInsufficientResources;
    	goto EXIT;
    }

    memset(m_sOutBufList.pBufArr, 0, sizeof(OMX_BUFFERHEADERTYPE) * m_sOutPortDef.nBufferCountActual);
    for (nIndex = 0; nIndex < m_sOutPortDef.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR(&m_sOutBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    m_sOutBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) * m_sOutPortDef.nBufferCountActual);
    if(m_sOutBufList.pBufHdrList == NULL)
    {
    	eRet = OMX_ErrorInsufficientResources;
    	goto EXIT;
    }

    m_sOutBufList.nSizeOfList       = 0;
    m_sOutBufList.nAllocSize        = 0;
    m_sOutBufList.nWritePos         = 0;
    m_sOutBufList.nReadPos          = 0;
    m_sOutBufList.nAllocBySelfFlags = 0;
    m_sOutBufList.nSizeOfList       = 0;
    m_sOutBufList.nBufArrSize       = m_sOutPortDef.nBufferCountActual;
    m_sOutBufList.eDir              = OMX_DirOutput;


    // Create the pipe used to send commands to the thread
    err = pipe(m_cmdpipe);
    if (err)
    {
    	eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

	// Create the pipe used to send commands to the venc drv thread
    err = pipe(m_venc_cmdpipe);
    if (err)
    {
    	eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the pipe used to send command data to the thread
    err = pipe(m_cmddatapipe);
    if (err)
    {
    	eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create the component thread
    err = pthread_create(&m_thread_id, NULL, ComponentThread, this);
    if( err || !m_thread_id )
    {
    	loge("create ComponentThread error!!!");
    	eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

	// Create venc thread
    err = pthread_create(&m_venc_thread_id, NULL, ComponentVencThread, this);
    if( err || !m_venc_thread_id )
    {
    	loge("create ComponentVencThread error!!!");
    	eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

	// init some param
	m_useAndroidBuffer = OMX_FALSE;
	m_useMetaDataInBuffers = OMX_FALSE;
	m_prependSPSPPSToIDRFrames = OMX_FALSE;

#if SAVE_BITSTREAM
	OutFile = fopen("/data/camera/bitstream.dat", "wb");
#endif

EXIT:
    return eRet;
}


OMX_ERRORTYPE  omx_venc::get_component_version(OMX_IN OMX_HANDLETYPE hComp,
                                               OMX_OUT OMX_STRING componentName,
                                               OMX_OUT OMX_VERSIONTYPE* componentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* specVersion,
                                               OMX_OUT OMX_UUIDTYPE* componentUUID)
{

	logv(" COMPONENT_GET_VERSION");
	CEDARX_UNUSE(componentUUID);

    if (!hComp || !componentName || !componentVersion || !specVersion)
    {
        return OMX_ErrorBadParameter;
    }

    strcpy((char*)componentName, (char*)m_cName);

    componentVersion->s.nVersionMajor = 1;
    componentVersion->s.nVersionMinor = 1;
    componentVersion->s.nRevision     = 0;
    componentVersion->s.nStep         = 0;

    specVersion->s.nVersionMajor = 1;
    specVersion->s.nVersionMinor = 1;
    specVersion->s.nRevision     = 0;
    specVersion->s.nStep         = 0;

	return OMX_ErrorNone;
}


OMX_ERRORTYPE  omx_venc::send_command(OMX_IN OMX_HANDLETYPE  hComp,
                                      OMX_IN OMX_COMMANDTYPE cmd,
                                      OMX_IN OMX_U32         param1,
                                      OMX_IN OMX_PTR         cmdData)
{
    ThrCmdType    eCmd;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

	CEDARX_UNUSE(hComp);

    if(m_state == OMX_StateInvalid)
    {
    	loge("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (cmd == OMX_CommandMarkBuffer && cmdData == NULL)
    {
    	loge("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
    	return OMX_ErrorBadParameter;
    }

    switch (cmd)
    {
        case OMX_CommandStateSet:
        	logv(" COMPONENT_SEND_COMMAND: OMX_CommandStateSet");
            eCmd = SetState;
	        break;

        case OMX_CommandFlush:
        	logv(" COMPONENT_SEND_COMMAND: OMX_CommandFlush");
	        eCmd = Flush;
	        if ((int)param1 > 1 && (int)param1 != -1)
	        {
	        	loge("Error: Send OMX_CommandFlush command but param1 invalid.");
	        	return OMX_ErrorBadPortIndex;
	        }
	        break;

        case OMX_CommandPortDisable:
        	logv(" COMPONENT_SEND_COMMAND: OMX_CommandPortDisable");
	        eCmd = StopPort;
	        break;

        case OMX_CommandPortEnable:
        	logv(" COMPONENT_SEND_COMMAND: OMX_CommandPortEnable");
	        eCmd = RestartPort;
	        break;

        case OMX_CommandMarkBuffer:
        	logv(" COMPONENT_SEND_COMMAND: OMX_CommandMarkBuffer");
	        eCmd = MarkBuf;
 	        if (param1 > 0)
	        {
	        	loge("Error: Send OMX_CommandMarkBuffer command but param1 invalid.");
	        	return OMX_ErrorBadPortIndex;
	        }
            break;

        default:
			return OMX_ErrorBadPortIndex;
            break;
    }

	post_event(eCmd, param1, cmdData);

    return eError;
}


OMX_ERRORTYPE  omx_venc::get_parameter(OMX_IN OMX_HANDLETYPE hComp,
                                       OMX_IN OMX_INDEXTYPE  paramIndex,
                                       OMX_INOUT OMX_PTR     paramData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

	CEDARX_UNUSE(hComp);

    if(m_state == OMX_StateInvalid)
    {
    	logv("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(paramData == NULL)
    {
    	logv("Get Param in Invalid paramData \n");
        return OMX_ErrorBadParameter;
    }
    
    switch(paramIndex)
    {
    	case OMX_IndexParamVideoInit:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoInit");
	        memcpy(paramData, &m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
    		break;
    	}

    	case OMX_IndexParamPortDefinition:
    	{
//          android::CallStack stack;
//          stack.update(1, 100);
//          stack.dump("get_parameter");
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamPortDefinition");
	        if (((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nPortIndex == m_sInPortDef.nPortIndex)
	            memcpy(paramData, &m_sInPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	        else if (((OMX_PARAM_PORTDEFINITIONTYPE*)(paramData))->nPortIndex == m_sOutPortDef.nPortIndex)
	        {
	            memcpy(paramData, &m_sOutPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	            logv(" width = %d, height = %d", (int)m_sOutPortDef.format.video.nFrameWidth, (int)m_sOutPortDef.format.video.nFrameHeight);
	        }
	        else
	            eError = OMX_ErrorBadPortIndex;

    		break;
    	}

    	case OMX_IndexParamVideoPortFormat:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoPortFormat");

			OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(paramData);
 	        if (param_portformat_type->nPortIndex == m_sInPortFormat.nPortIndex)
 	        {
 	            if (param_portformat_type->nIndex > m_sInPortFormat.nIndex)
 	            {
		           	eError = OMX_ErrorNoMore;
 	            }
		        else
		        {
	        		param_portformat_type->eCompressionFormat = (OMX_VIDEO_CODINGTYPE)0;
	        		param_portformat_type->eColorFormat = m_inputcolorFormats[param_portformat_type->nIndex];
		        }
		    }
	        else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(paramData))->nPortIndex == m_sOutPortFormat.nPortIndex)
	        {
	            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(paramData))->nIndex > m_sOutPortFormat.nIndex)
	            {
		            eError = OMX_ErrorNoMore;
	            }
		        else
		        {
		            memcpy(paramData, &m_sOutPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
		        }
		    }
		    else
		        eError = OMX_ErrorBadPortIndex;

	        break;
    	}

    	case OMX_IndexParamStandardComponentRole:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamStandardComponentRole");
    		OMX_PARAM_COMPONENTROLETYPE* comp_role;

    		comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) paramData;
    		comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
    		comp_role->nSize             = sizeof(*comp_role);

    		strncpy((char*)comp_role->cRole, (const char*)m_cRole, OMX_MAX_STRINGNAME_SIZE);
    		break;
    	}

    	case OMX_IndexParamPriorityMgmt:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamPriorityMgmt");
	        memcpy(paramData, &m_sPriorityMgmt, sizeof(OMX_PRIORITYMGMTTYPE));
    		break;
    	}

    	case OMX_IndexParamCompBufferSupplier:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* pBuffSupplierParam = (OMX_PARAM_BUFFERSUPPLIERTYPE*)paramData;

            if (pBuffSupplierParam->nPortIndex == 1)
            {
                pBuffSupplierParam->eBufferSupplier = m_sOutBufSupplier.eBufferSupplier;
            }
            else if (pBuffSupplierParam->nPortIndex == 0)
            {
                pBuffSupplierParam->eBufferSupplier = m_sInBufSupplier.eBufferSupplier;
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

    		break;
    	}
		
        case OMX_IndexParamVideoBitrate:
		{
			logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoBitrate");
            if (((OMX_VIDEO_PARAM_BITRATETYPE*)(paramData))->nPortIndex == m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(paramData,&m_sOutPutBitRateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;			
		}

	    case OMX_IndexParamVideoAvc:
	    {
	    	logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)paramData;
			
            if (pComponentParam->nPortIndex == m_sH264Type.nPortIndex)
            {
                memcpy(pComponentParam, &m_sH264Type, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

	    	break;
	    }


    	case OMX_IndexParamAudioInit:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamAudioInit");
    		OMX_PORT_PARAM_TYPE *audioPortParamType = (OMX_PORT_PARAM_TYPE *) paramData;

    		audioPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
    		audioPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
    		audioPortParamType->nPorts            = 0;
    		audioPortParamType->nStartPortNumber  = 0;

    		break;
    	}

    	case OMX_IndexParamImageInit:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamImageInit");
    		OMX_PORT_PARAM_TYPE *imagePortParamType = (OMX_PORT_PARAM_TYPE *) paramData;

    		imagePortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
    		imagePortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
    		imagePortParamType->nPorts            = 0;
    		imagePortParamType->nStartPortNumber  = 0;

    		break;
    	}

    	case OMX_IndexParamOtherInit:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamOtherInit");
    		OMX_PORT_PARAM_TYPE *otherPortParamType = (OMX_PORT_PARAM_TYPE *) paramData;

    		otherPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
    		otherPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
    		otherPortParamType->nPorts            = 0;
    		otherPortParamType->nStartPortNumber  = 0;

    		break;
    	}

    	case OMX_IndexParamVideoH263:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoH263");
        	logv("get_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
    	}

    	case OMX_IndexParamVideoMpeg4:
    	{
    		logv(" COMPONENT_GET_PARAMETER: OMX_IndexParamVideoMpeg4");
        	logv("get_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
    	}
        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
        	VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
        	OMX_U32 nNumberOfProfiles = 0;
        	OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParamProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)paramData;

        	pParamProfileLevel->nPortIndex = m_sOutPortDef.nPortIndex;

        	/* Choose table based on compression format */
        	switch(m_sOutPortDef.format.video.eCompressionFormat)
        	{

        	case OMX_VIDEO_CodingAVC:
			if(mIsFromCts == true){
				pProfileLevel = CTSSupportedAVCProfileLevels;
				nNumberOfProfiles = sizeof(CTSSupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
			}else{
				pProfileLevel = SupportedAVCProfileLevels;
				nNumberOfProfiles = sizeof(SupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);

			}
			break;

        	case OMX_VIDEO_CodingH263:
        		pProfileLevel = SupportedH263ProfileLevels;
        		nNumberOfProfiles = sizeof(SupportedH263ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
        		break;

        	default:
        		return OMX_ErrorBadParameter;
        	}

        	if(pParamProfileLevel->nProfileIndex >= (nNumberOfProfiles - 1))
        		return OMX_ErrorBadParameter;

        	/* Point to table entry based on index */
        	pProfileLevel += pParamProfileLevel->nProfileIndex;

        	/* -1 indicates end of table */
        	if(pProfileLevel->nProfile != -1)
        	{
        		pParamProfileLevel->eProfile = pProfileLevel->nProfile;
        		pParamProfileLevel->eLevel = pProfileLevel->nLevel;
        		eError = OMX_ErrorNone;
        	}
        	else
        	{
        		eError = OMX_ErrorNoMore;
        	}
			
        	break;
        }
    	default:
    	{
    		loge("get_parameter: unknown param %08x\n", paramIndex);
    		eError =OMX_ErrorUnsupportedIndex;
    		break;
    	}
    }

    return eError;
}


OMX_ERRORTYPE  omx_venc::set_parameter(OMX_IN OMX_HANDLETYPE hComp, OMX_IN OMX_INDEXTYPE paramIndex,  OMX_IN OMX_PTR paramData)
{
    OMX_ERRORTYPE         eError      = OMX_ErrorNone;
    unsigned int          alignment   = 0;
    unsigned int          buffer_size = 0;
    int                   i;

	CEDARX_UNUSE(hComp);

    if(m_state == OMX_StateInvalid)
    {
        logv("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(paramData == NULL)
    {
    	logv("Get Param in Invalid paramData \n");
    	return OMX_ErrorBadParameter;
    }

    switch(paramIndex)
    {
	    case OMX_IndexParamPortDefinition:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamPortDefinition");
			
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nPortIndex == m_sInPortDef.nPortIndex)
            {
            	logv("in, nPortIndex: %d", (int)(((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual));
	        	if(((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual != m_sInPortDef.nBufferCountActual)
	        	{
	        		int nBufCnt;
	        		int nIndex;


	        		pthread_mutex_lock(&m_inBufMutex);

	        		if(m_sInBufList.pBufArr != NULL)
	        			free(m_sInBufList.pBufArr);

	        		if(m_sInBufList.pBufHdrList != NULL)
	        			free(m_sInBufList.pBufHdrList);

	        		nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual;
	        		logv(" allocate %d buffers.", (int)nBufCnt);

	        	    m_sInBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* nBufCnt);
	        	    m_sInBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* nBufCnt);
	        	    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
	        	    {
	        	        OMX_CONF_INIT_STRUCT_PTR (&m_sInBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
	        	    }

	        	    m_sInBufList.nSizeOfList       = 0;
	        	    m_sInBufList.nAllocSize        = 0;
	        	    m_sInBufList.nWritePos         = 0;
	        	    m_sInBufList.nReadPos          = 0;
	        	    m_sInBufList.nAllocBySelfFlags = 0;
	        	    m_sInBufList.nSizeOfList       = 0;
	        	    m_sInBufList.nBufArrSize       = nBufCnt;
	        	    m_sInBufList.eDir              = OMX_DirInput;

	        		pthread_mutex_unlock(&m_inBufMutex);
	        	}

	            memcpy(&m_sInPortDef, paramData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

			    logd("init_input_port: stride = %d, width = %d, height = %d", (int)m_sInPortDef.format.video.nStride, (int)m_sInPortDef.format.video.nFrameWidth, (int)m_sInPortDef.format.video.nFrameHeight);
			}
	        else if (((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nPortIndex == m_sOutPortDef.nPortIndex)
	        {
	        	logv("out, nPortIndex: %d", (int)((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual);
	        	if(((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual != m_sOutPortDef.nBufferCountActual)
	        	{
	        		int nBufCnt;
	        		int nIndex;

	        		pthread_mutex_lock(&m_outBufMutex);

	        		if(m_sOutBufList.pBufArr != NULL)
	        			free(m_sOutBufList.pBufArr);

	        		if(m_sOutBufList.pBufHdrList != NULL)
	        			free(m_sOutBufList.pBufHdrList);

	        		nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(paramData))->nBufferCountActual;
	        		logv(" allocate %d buffers.", (int)nBufCnt);
	        	    // Initialize the output buffer list
	        	    m_sOutBufList.pBufArr = (OMX_BUFFERHEADERTYPE*) malloc(sizeof(OMX_BUFFERHEADERTYPE) * nBufCnt);
	        	    m_sOutBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**) malloc(sizeof(OMX_BUFFERHEADERTYPE*) * nBufCnt);
	        	    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
	        	    {
	        	        OMX_CONF_INIT_STRUCT_PTR (&m_sOutBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
	        	    }

	        	    m_sOutBufList.nSizeOfList       = 0;
	        	    m_sOutBufList.nAllocSize        = 0;
	        	    m_sOutBufList.nWritePos         = 0;
	        	    m_sOutBufList.nReadPos          = 0;
	        	    m_sOutBufList.nAllocBySelfFlags = 0;
	        	    m_sOutBufList.nSizeOfList       = 0;
	        	    m_sOutBufList.nBufArrSize       = nBufCnt;
	        	    m_sOutBufList.eDir              = OMX_DirOutput;

	        		pthread_mutex_unlock(&m_outBufMutex);
	        	}

	            memcpy(&m_sOutPortDef, paramData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	            m_sOutPortDef.nBufferSize = m_sOutPortDef.format.video.nFrameWidth * m_sOutPortDef.format.video.nFrameHeight * 3 / 2;

 				m_framerate = (m_sInPortDef.format.video.xFramerate >> 16);
				
				logd("m_framerate: %d", (int)m_framerate);
	        }
	        else
	            eError = OMX_ErrorBadPortIndex;

	       break;
	    }

	    case OMX_IndexParamVideoPortFormat:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoPortFormat");

			OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(paramData);
 	        if (param_portformat_type->nPortIndex == m_sInPortFormat.nPortIndex)
 	        {
 	            if (param_portformat_type->nIndex > m_sInPortFormat.nIndex)
 	            {
		           	eError = OMX_ErrorNoMore;
 	            }
		        else
	        	{
					m_sInPortFormat.eColorFormat = param_portformat_type->eColorFormat;
					m_sInPortDef.format.video.eColorFormat = m_sInPortFormat.eColorFormat;
	        	}
		    }
	        else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(paramData))->nPortIndex == m_sOutPortFormat.nPortIndex)
	        {
	            if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(paramData))->nIndex > m_sOutPortFormat.nIndex)
		            eError = OMX_ErrorNoMore;
		        else
		            memcpy(&m_sOutPortFormat, paramData, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	        }
	        else
	            eError = OMX_ErrorBadPortIndex;

	        break;
	    }

	    case OMX_IndexParamStandardComponentRole:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamStandardComponentRole");
    		OMX_PARAM_COMPONENTROLETYPE *comp_role;
    		comp_role = (OMX_PARAM_COMPONENTROLETYPE *) paramData;
    		logv("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

    		if((m_state == OMX_StateLoaded)/* && !BITMASK_PRESENT(&m_flags,OMX_COMPONENT_IDLE_PENDING)*/)
    		{
    			logv("Set Parameter called in valid state");
    		}
    		else
    		{
    			logv("Set Parameter called in Invalid State\n");
    			return OMX_ErrorIncorrectStateOperation;
    		}

    		if(!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    		{
    			if(!strncmp((char*)comp_role->cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    			{
    				strncpy((char*)m_cRole,"video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
    			}
    			else
    			{
                  	logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                  	eError =OMX_ErrorUnsupportedSetting;
    			}
    		}
    		else
    		{
    			logv("Setparameter: unknown param %s\n", m_cName);
    			eError = OMX_ErrorInvalidComponentName;
    		}

    		break;
	    }

	    case OMX_IndexParamPriorityMgmt:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamPriorityMgmt");
            if(m_state != OMX_StateLoaded)
            {
            	logv("Set Parameter called in Invalid State\n");
            	return OMX_ErrorIncorrectStateOperation;
            }

            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) paramData;

            m_sPriorityMgmt.nGroupID = priorityMgmtype->nGroupID;
            m_sPriorityMgmt.nGroupPriority = priorityMgmtype->nGroupPriority;

            break;
	    }

	    case OMX_IndexParamCompBufferSupplier:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamCompBufferSupplier");
    		OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType = (OMX_PARAM_BUFFERSUPPLIERTYPE*) paramData;

            logv("set_parameter: OMX_IndexParamCompBufferSupplier %d\n", bufferSupplierType->eBufferSupplier);
            if(bufferSupplierType->nPortIndex == 0)
            	m_sInBufSupplier.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else if(bufferSupplierType->nPortIndex == 1)
            	m_sOutBufSupplier.eBufferSupplier = bufferSupplierType->eBufferSupplier;
        	else
        		eError = OMX_ErrorBadPortIndex;

        	break;
	    }
		
        case OMX_IndexParamVideoBitrate:
        {
			logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoBitrate"); 
            OMX_VIDEO_PARAM_BITRATETYPE* pComponentParam = (OMX_VIDEO_PARAM_BITRATETYPE*)paramData;
            if (pComponentParam->nPortIndex == m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(&m_sOutPutBitRateType,pComponentParam, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));

                if(!m_sOutPutBitRateType.nTargetBitrate)
                {
                    m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE;
                }
				
                m_sOutPortDef.format.video.nBitrate = m_sOutPutBitRateType.nTargetBitrate;

				if(m_state == OMX_StateExecuting && m_encoder)
				{
					post_message_to_venc(this, OMX_Venc_Cmd_ChangeBitrate);
				}
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

	    case OMX_IndexParamVideoAvc:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)paramData;
			
            if (pComponentParam->nPortIndex == m_sH264Type.nPortIndex)
            {
                memcpy(&m_sH264Type,pComponentParam, sizeof(OMX_VIDEO_PARAM_AVCTYPE));

                //CalculateBufferSize(pCompPortOut->pPortDef, pComponentPrivate);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

	    	break;
	    }

	    case OMX_IndexParamVideoH263:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoH263");
     		logv("set_parameter: OMX_IndexParamVideoH263, do nothing.\n");
	    	break;
	    }

	    case OMX_IndexParamVideoMpeg4:
	    {
	    	logv(" COMPONENT_SET_PARAMETER: OMX_IndexParamVideoMpeg4");
     		logv("set_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
	    	break;
	    }
		case OMX_IndexParamVideoIntraRefresh:
		{
			OMX_VIDEO_PARAM_INTRAREFRESHTYPE* pComponentParam = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE*)paramData;
			if(pComponentParam->nPortIndex == 1 && pComponentParam->eRefreshMode == OMX_VIDEO_IntraRefreshCyclic)
			{
				int mbWidth, mbHeight;
				mbWidth = (m_sInPortDef.format.video.nFrameWidth + 15)/16;
				mbHeight = (m_sInPortDef.format.video.nFrameHeight + 15)/16;
				m_vencCyclicIntraRefresh.bEnable = 1;
				m_vencCyclicIntraRefresh.nBlockNumber = mbWidth*mbHeight/pComponentParam->nCirMBs;
				logd("m_vencCyclicIntraRefresh.nBlockNumber: %d", m_vencCyclicIntraRefresh.nBlockNumber);
			}
	    	break;			
		}

	    default:
	    {

#if CONFIG_OS == OPTION_OS_ANDROID

			switch((VIDENC_CUSTOM_INDEX)paramIndex)
	    	{
	    		case VideoEncodeCustomParamStoreMetaDataInBuffers:
				{
					OMX_BOOL bFlag = ((StoreMetaDataInBuffersParams*)paramData)->bStoreMetaData;;
					if(((StoreMetaDataInBuffersParams*)paramData)->nPortIndex == 0)
					{
						m_useMetaDataInBuffers = bFlag;
					}
					else
					{
						if(bFlag)
						{
							eError = OMX_ErrorUnsupportedSetting;
						}
					}

					logd(" COMPONENT_SET_PARAMETER: VideoEncodeCustomParamStoreMetaDataInBuffers %d",
								m_useMetaDataInBuffers);
					
			    	break;			
				}
				
				case VideoEncodeCustomParamPrependSPSPPSToIDRFrames:
				{

					m_prependSPSPPSToIDRFrames = ((PrependSPSPPSToIDRFramesParams*)paramData)->bEnable;
					logd(" COMPONENT_SET_PARAMETER: VideoEncodeCustomParamPrependSPSPPSToIDRFrames %d",
							m_prependSPSPPSToIDRFrames);
			    	break;
				}
				
				case VideoEncodeCustomParamEnableAndroidNativeBuffers:
				{
					OMX_BOOL bFlag = ((EnableAndroidNativeBuffersParams*)paramData)->enable;

					if(((StoreMetaDataInBuffersParams*)paramData)->nPortIndex == 0)
					{
						m_useAndroidBuffer = bFlag;
					}
					else
					{
						eError = OMX_ErrorUnsupportedSetting;
					}

					logd(" COMPONENT_SET_PARAMETER: VideoEncodeCustomParamEnableAndroidNativeBuffers m_useAndroidBuffer %d",
							m_useAndroidBuffer);

			    	break;
				}
			    default:
				{
				    loge("Setparameter: unknown param %x\n", paramIndex);
		    		eError = OMX_ErrorUnsupportedIndex;
		    		break;	
		    	}
			}
#else
			eError = OMX_ErrorUnsupportedIndex;
#endif

	    }
    }

    return eError;
}

OMX_ERRORTYPE  omx_venc::get_config(OMX_IN OMX_HANDLETYPE hComp, OMX_IN OMX_INDEXTYPE configIndex, OMX_INOUT OMX_PTR configData)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	logv(" COMPONENT_GET_CONFIG: index = %d", configIndex);
	CEDARX_UNUSE(hComp);
	CEDARX_UNUSE(configData);

	if (m_state == OMX_StateInvalid)
	{
		logv("get_config in Invalid State\n");
		return OMX_ErrorInvalidState;
	}

	switch (configIndex)
	{
    	default:
    	{
    		logv("get_config: unknown param %d\n",configIndex);
    		eError = OMX_ErrorUnsupportedIndex;
    	}
	}

	return eError;
}



OMX_ERRORTYPE omx_venc::set_config(OMX_IN OMX_HANDLETYPE hComp, OMX_IN OMX_INDEXTYPE configIndex, OMX_IN OMX_PTR configData)
{

	logv(" COMPONENT_SET_CONFIG: index = %d", configIndex);

	CEDARX_UNUSE(hComp);

	if(m_state == OMX_StateInvalid)
	{
		logv("set_config in Invalid State\n");
		return OMX_ErrorInvalidState;
	}

	OMX_ERRORTYPE eError = OMX_ErrorNone;

	switch(configIndex)
	{
		case OMX_IndexConfigVideoIntraVOPRefresh:
		{
			if(m_state == OMX_StateExecuting && m_encoder)
			{
				post_message_to_venc(this, OMX_Venc_Cmd_RequestIDRFrame);
			}
			break;
		}
		case OMX_IndexConfigVideoBitrate:
		{
			OMX_VIDEO_CONFIG_BITRATETYPE* pData = (OMX_VIDEO_CONFIG_BITRATETYPE*)(configData);

			if(pData->nPortIndex == 1)
			{
				if(m_state == OMX_StateExecuting && m_encoder)
				{
					m_sOutPortDef.format.video.nBitrate = pData->nEncodeBitrate;
					post_message_to_venc(this, OMX_Venc_Cmd_ChangeBitrate);
				}
				break;
			}
		}

    	default:
    	{
    		logv("get_config: unknown param %d\n",configIndex);
    		eError = OMX_ErrorUnsupportedIndex;
    	}
	}


	return eError;
}


OMX_ERRORTYPE  omx_venc::get_extension_index(OMX_IN OMX_HANDLETYPE hComp, OMX_IN OMX_STRING paramName, OMX_OUT OMX_INDEXTYPE* indexType)
{
	unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

	logv(" COMPONENT_GET_EXTENSION_INDEX: param name = %s", paramName);
    if(m_state == OMX_StateInvalid)
    {
    	logv("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if(hComp == NULL)
    	return OMX_ErrorBadParameter;

    for(nIndex = 0; nIndex < sizeof(sVideoEncCustomParams)/sizeof(VIDDEC_CUSTOM_PARAM); nIndex++)
    {
        if(strcmp((char *)paramName, (char *)&(sVideoEncCustomParams[nIndex].cCustomParamName)) == 0)
        {
            *indexType = sVideoEncCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }

    return eError;
}



OMX_ERRORTYPE omx_venc::get_state(OMX_IN OMX_HANDLETYPE hComp, OMX_OUT OMX_STATETYPE* state)
{
	logv(" COMPONENT_GET_STATE");

	if(hComp == NULL || state == NULL)
		return OMX_ErrorBadParameter;

	*state = m_state;
	return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_venc::component_tunnel_request(OMX_IN    OMX_HANDLETYPE       hComp,
                                                 OMX_IN    OMX_U32              port,
                                                 OMX_IN    OMX_HANDLETYPE       peerComponent,
                                                 OMX_IN    OMX_U32              peerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup)
{
	logv(" COMPONENT_TUNNEL_REQUEST");

	CEDARX_UNUSE(hComp);
	CEDARX_UNUSE(port);
	CEDARX_UNUSE(peerComponent);
	CEDARX_UNUSE(peerPort);
	CEDARX_UNUSE(tunnelSetup);

	logv("Error: component_tunnel_request Not Implemented\n");
	return OMX_ErrorNotImplemented;
}



OMX_ERRORTYPE  omx_venc::use_buffer(OMX_IN    OMX_HANDLETYPE          hComponent,
                  			        OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                  			        OMX_IN    OMX_U32                 nPortIndex,
                  			        OMX_IN    OMX_PTR                 pAppPrivate,
                  			        OMX_IN    OMX_U32                 nSizeBytes,
                  			        OMX_IN    OMX_U8*                 pBuffer)
{
    OMX_PARAM_PORTDEFINITIONTYPE*   pPortDef;
    OMX_U32                         nIndex = 0x0;

	logv(" COMPONENT_USE_BUFFER");

	// logd("-------nPortIndex: %d, nSizeBytes: %d", (int)nPortIndex, (int)nSizeBytes);

	if(hComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
	{
		return OMX_ErrorBadParameter;
	}

    if (nPortIndex == m_sInPortDef.nPortIndex)
        pPortDef = &m_sInPortDef;
    else if (nPortIndex == m_sOutPortDef.nPortIndex)
        pPortDef = &m_sOutPortDef;
    else
    	return OMX_ErrorBadParameter;

    if (!pPortDef->bEnabled)
    	return OMX_ErrorIncorrectStateOperation;

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
    	return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
    	pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: use_buffer");

    	if((int)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
        	pthread_mutex_unlock(&m_inBufMutex);
        	return OMX_ErrorInsufficientResources;
        }

    	nIndex = m_sInBufList.nAllocSize;
    	m_sInBufList.nAllocSize++;

    	m_sInBufList.pBufArr[nIndex].pBuffer = pBuffer;
    	m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
    	m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
    	m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
    	m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        	pPortDef->bPopulated = OMX_TRUE;

    	pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
    	pthread_mutex_lock(&m_outBufMutex);
        logv("vencOutPort: use_buffer");
    
    	if((int)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
        	pthread_mutex_unlock(&m_outBufMutex);
        	return OMX_ErrorInsufficientResources;
        }

    	nIndex = m_sOutBufList.nAllocSize;
    	m_sOutBufList.nAllocSize++;

    	m_sOutBufList.pBufArr[nIndex].pBuffer = pBuffer;
    	m_sOutBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
    	m_sOutBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
    	m_sOutBufList.pBufArr[nIndex].nInputPortIndex = 0xFFFFFFFE;
    	m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        	pPortDef->bPopulated = OMX_TRUE;

    	pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_venc::allocate_buffer(OMX_IN    OMX_HANDLETYPE         hComponent,
        								OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
        								OMX_IN    OMX_U32                nPortIndex,
        								OMX_IN    OMX_PTR                pAppPrivate,
        								OMX_IN    OMX_U32                nSizeBytes)
{
	OMX_S8                        nIndex = 0x0;
	OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;

	logv(" COMPONENT_ALLOCATE_BUFFER");

	if(hComponent == NULL || ppBufferHdr == NULL)
		return OMX_ErrorBadParameter;

    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        logv("port_in: nPortIndex=%d", (int)nPortIndex);
        pPortDef = &m_sInPortDef;
    }
    else
    {
        logv("port_out: nPortIndex=%d",(int)nPortIndex);
        if (nPortIndex == m_sOutPortDef.nPortIndex)
        {
            logv("port_out: nPortIndex=%d", (int)nPortIndex);
	        pPortDef = &m_sOutPortDef;
        }
        else
        {
            logv("allocate_buffer fatal error! nPortIndex=%d", (int)nPortIndex);
        	return OMX_ErrorBadParameter;
        }
    }

    if (!pPortDef->bEnabled)
    	return OMX_ErrorIncorrectStateOperation;

    if (nSizeBytes != pPortDef->nBufferSize || pPortDef->bPopulated)
    	return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
    	pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: malloc vbs");
		
    	if((int)m_sInBufList.nAllocSize >= m_sInBufList.nBufArrSize)
        {
        	pthread_mutex_unlock(&m_inBufMutex);
        	return OMX_ErrorInsufficientResources;
        }

    	nIndex = m_sInBufList.nAllocSize;

        m_sInBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sInBufList.pBufArr[nIndex].pBuffer)
        {
        	pthread_mutex_unlock(&m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sInBufList.nAllocBySelfFlags |= (1<<nIndex);

    	m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
    	m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
    	m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
    	m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &m_sInBufList.pBufArr[nIndex];

        m_sInBufList.nAllocSize++;
        if (m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        	pPortDef->bPopulated = OMX_TRUE;

    	pthread_mutex_unlock(&m_inBufMutex);
    }
    else
    {
        logv("vencOutPort: malloc frame");
//        android::CallStack stack;
//        stack.update(1, 100);
//        stack.dump("allocate_buffer_for_frame");
    	pthread_mutex_lock(&m_outBufMutex);

    	if((int)m_sOutBufList.nAllocSize >= m_sOutBufList.nBufArrSize)
        {
        	pthread_mutex_unlock(&m_outBufMutex);
        	return OMX_ErrorInsufficientResources;
        }

    	nIndex = m_sOutBufList.nAllocSize;

    	m_sOutBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!m_sOutBufList.pBufArr[nIndex].pBuffer)
        {
        	pthread_mutex_unlock(&m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        m_sOutBufList.nAllocBySelfFlags |= (1<<nIndex);

        m_sOutBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        m_sOutBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        m_sOutBufList.pBufArr[nIndex].nInputPortIndex = 0xFFFFFFFE;
        m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &m_sOutBufList.pBufArr[nIndex];

        m_sOutBufList.nAllocSize++;
        if (m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        	pPortDef->bPopulated = OMX_TRUE;

    	pthread_mutex_unlock(&m_outBufMutex);
    }

    return OMX_ErrorNone;
}



OMX_ERRORTYPE omx_venc::free_buffer(OMX_IN  OMX_HANDLETYPE        hComponent,
        							OMX_IN  OMX_U32               nPortIndex,
        							OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_S32                       nIndex;

	logv(" COMPONENT_FREE_BUFFER, nPortIndex = %d, pBufferHdr = %p", (int)nPortIndex, pBufferHdr);

    if(hComponent == NULL || pBufferHdr == NULL)
    	return OMX_ErrorBadParameter;

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == m_sInPortDef.nPortIndex)
    {
        pPortDef = &m_sInPortDef;

    	pthread_mutex_lock(&m_inBufMutex);
        logv("vencInPort: free_buffer");
    
    	for(nIndex = 0; nIndex < m_sInBufList.nBufArrSize; nIndex++)
    	{
    		if(pBufferHdr == &m_sInBufList.pBufArr[nIndex])
    			break;
    	}

    	if(nIndex == m_sInBufList.nBufArrSize)
    	{
    		pthread_mutex_unlock(&m_inBufMutex);
    		return OMX_ErrorBadParameter;
    	}

    	if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
    	{
    		free(m_sInBufList.pBufArr[nIndex].pBuffer);
    		m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
    		m_sInBufList.nAllocBySelfFlags &= ~(1<<nIndex);
    	}

    	m_sInBufList.nAllocSize--;
    	if(m_sInBufList.nAllocSize == 0)
    		pPortDef->bPopulated = OMX_FALSE;

    	pthread_mutex_unlock(&m_inBufMutex);
    }
    else if (nPortIndex == m_sOutPortDef.nPortIndex)
    {
	    pPortDef = &m_sOutPortDef;

    	pthread_mutex_lock(&m_outBufMutex);
        logv("vencOutPort: free_buffer");
    
    	for(nIndex = 0; nIndex < m_sOutBufList.nBufArrSize; nIndex++)
    	{
    		logv("pBufferHdr = %p, &m_sOutBufList.pBufArr[%d] = %p", pBufferHdr, (int)nIndex, &m_sOutBufList.pBufArr[nIndex]);
    		if(pBufferHdr == &m_sOutBufList.pBufArr[nIndex])
    			break;
    	}

    	logv("index = %d", (int)nIndex);

    	if(nIndex == m_sOutBufList.nBufArrSize)
    	{
    		pthread_mutex_unlock(&m_outBufMutex);
    		return OMX_ErrorBadParameter;
    	}

    	if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
    	{
    		free(m_sOutBufList.pBufArr[nIndex].pBuffer);
    		m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
    		m_sOutBufList.nAllocBySelfFlags &= ~(1<<nIndex);
    	}

    	m_sOutBufList.nAllocSize--;
    	if(m_sOutBufList.nAllocSize == 0)
    		pPortDef->bPopulated = OMX_FALSE;

    	pthread_mutex_unlock(&m_outBufMutex);
    }
    else
        return OMX_ErrorBadParameter;

    return OMX_ErrorNone;
}



OMX_ERRORTYPE  omx_venc::empty_this_buffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    ThrCmdType eCmd   = EmptyBuf;

    logv(" COMPONENT_EMPTY_THIS_BUFFER");
    
    if(hComponent == NULL || pBufferHdr == NULL)
    	return OMX_ErrorBadParameter;

    if (!m_sInPortDef.bEnabled)
    	return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;
	

    //fwrite(pBufferHdr->pBuffer, 1, pBufferHdr->nFilledLen, ph264File);
    //logv("BHF[0x%x],len[%d]", pBufferHdr->nFlags, pBufferHdr->nFilledLen);
    // Put the command and data in the pipe
    post_event(eCmd, 0, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}



OMX_ERRORTYPE  omx_venc::fill_this_buffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    ThrCmdType eCmd = FillBuf;

//	logv(" COMPONENT_FILL_THIS_BUFFER");
    logv("vencOutPort: fill_this_buffer");

    if(hComponent == NULL || pBufferHdr == NULL)
    	return OMX_ErrorBadParameter;

    if (!m_sOutPortDef.bEnabled)
    	return OMX_ErrorIncorrectStateOperation;

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
        return OMX_ErrorBadPortIndex;

    if (m_state != OMX_StateExecuting && m_state != OMX_StatePause)
        return OMX_ErrorIncorrectStateOperation;

    // Put the command and data in the pipe
	post_event(eCmd, 0, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}



OMX_ERRORTYPE  omx_venc::set_callbacks(OMX_IN OMX_HANDLETYPE        hComp,
                                           OMX_IN OMX_CALLBACKTYPE* callbacks,
                                           OMX_IN OMX_PTR           appData)
{
	logv(" COMPONENT_SET_CALLBACKS");

    if(hComp == NULL || callbacks == NULL || appData == NULL)
    	return OMX_ErrorBadParameter;
    memcpy(&m_Callbacks, callbacks, sizeof(OMX_CALLBACKTYPE));
    m_pAppData = appData;

    return OMX_ErrorNone;
}


OMX_ERRORTYPE  omx_venc::component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
	logv(" COMPONENT_DEINIT");

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType    eCmd   = Stop;
    OMX_S32       nIndex = 0;

	logd("component_deinit");

	CEDARX_UNUSE(hComp);
	
    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    if (m_sInBufList.nAllocSize > 0)
    {
    	for(nIndex=0; nIndex<m_sInBufList.nBufArrSize; nIndex++)
    	{
    		if(m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
    		{
    			if(m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
    			{
    				free(m_sInBufList.pBufArr[nIndex].pBuffer);
    				m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
    			}
    		}
    	}

        if (m_sInBufList.pBufArr != NULL)
        	free(m_sInBufList.pBufArr);

        if (m_sInBufList.pBufHdrList != NULL)
        	free(m_sInBufList.pBufHdrList);

    	memset(&m_sInBufList, 0, sizeof(struct _BufferList));
    	m_sInBufList.nBufArrSize = m_sInPortDef.nBufferCountActual;
    }

    if (m_sOutBufList.nAllocSize > 0)
    {
    	for(nIndex=0; nIndex<m_sOutBufList.nBufArrSize; nIndex++)
    	{
    		if(m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
    		{
    			if(m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
    			{
    				free(m_sOutBufList.pBufArr[nIndex].pBuffer);
    				m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
    			}
    		}
    	}

        if (m_sOutBufList.pBufArr != NULL)
        	free(m_sOutBufList.pBufArr);

        if (m_sOutBufList.pBufHdrList != NULL)
        	free(m_sOutBufList.pBufHdrList);

    	memset(&m_sOutBufList, 0, sizeof(struct _BufferList));
    	m_sOutBufList.nBufArrSize = m_sOutPortDef.nBufferCountActual;
    }

    // Put the command and data in the pipe
    //write(m_cmdpipe[1], &eCmd, sizeof(eCmd));
    //write(m_cmddatapipe[1], &eCmd, sizeof(eCmd));
	post_event(eCmd, eCmd, NULL);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(m_thread_id, (void**)&eError);
	pthread_join(m_venc_thread_id, (void**)&eError);

    // close the pipe handles
    close(m_cmdpipe[0]);
    close(m_cmdpipe[1]);
    close(m_cmddatapipe[0]);
    close(m_cmddatapipe[1]);
    close(m_venc_cmdpipe[0]);
    close(m_venc_cmdpipe[1]);

#if SAVE_BITSTREAM
	if(OutFile)
	{
		fclose(OutFile);
		OutFile = NULL;
	}
#endif

    return eError;
}


OMX_ERRORTYPE  omx_venc::use_EGL_image(OMX_IN OMX_HANDLETYPE               hComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
                                          OMX_IN OMX_U32                   port,
                                          OMX_IN OMX_PTR                   appData,
                                          OMX_IN void*                     eglImage)
{
	logv("Error : use_EGL_image:  Not Implemented \n");

	CEDARX_UNUSE(hComp);
	CEDARX_UNUSE(bufferHdr);
	CEDARX_UNUSE(port);
	CEDARX_UNUSE(appData);
	CEDARX_UNUSE(eglImage);
	
    return OMX_ErrorNotImplemented;
}


OMX_ERRORTYPE  omx_venc::component_role_enum(OMX_IN  OMX_HANDLETYPE hComp,
                                             OMX_OUT OMX_U8*        role,
                                             OMX_IN  OMX_U32        index)
{
	logv(" COMPONENT_ROLE_ENUM");

	OMX_ERRORTYPE eRet = OMX_ErrorNone;

	CEDARX_UNUSE(hComp);

	if(!strncmp((char*)m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
	{
		if((0 == index) && role)
		{
			strncpy((char *)role, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
			logv("component_role_enum: role %s\n", role);
		}
		else
		{
			eRet = OMX_ErrorNoMore;
		}
	}
	else
	{
		logv("\nERROR:Querying Role on Unknown Component\n");
		eRet = OMX_ErrorInvalidComponentName;
	}

	return eRet;
}


OMX_ERRORTYPE omx_venc::post_event(OMX_IN ThrCmdType eCmd,
                                   OMX_IN OMX_U32         param1,
                                   OMX_IN OMX_PTR         cmdData)
{
    pthread_mutex_lock(&m_pipeMutex);
    int ret = write(m_cmdpipe[1], &eCmd, sizeof(eCmd));

    // In case of MarkBuf, the pCmdData parameter is used to carry the data.
    // In other cases, the nParam1 parameter carries the data.
    if(eCmd == MarkBuf || eCmd == EmptyBuf || eCmd == FillBuf)
        ret = write(m_cmddatapipe[1], &cmdData, sizeof(OMX_PTR));
    else
        ret = write(m_cmddatapipe[1], &param1, sizeof(param1));
    
    pthread_mutex_unlock(&m_pipeMutex);

    return OMX_ErrorNone;
}



int waitPipeDataToRead(int nPipeFd, int nTimeUs)
{
    int                     i;
    fd_set                  rfds;
    struct timeval          timeout;
    FD_ZERO(&rfds);
    FD_SET(nPipeFd, &rfds);

    // Check for new command
    timeout.tv_sec  = 0;
    timeout.tv_usec = nTimeUs;

    i = select(nPipeFd+1, &rfds, NULL, NULL, &timeout);

    if (FD_ISSET(nPipeFd, &rfds))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


int openVencDriver(omx_venc* pVenc)
{
	VencBaseConfig sBaseConfig;
	int result;
	memset(&sBaseConfig, 0, sizeof(VencBaseConfig));
	int size_y;
	int size_c;
	
	pVenc->m_encoder = VideoEncCreate(pVenc->m_vencCodecType);
	if(pVenc->m_encoder == NULL)
	{			
    	pVenc->m_Callbacks.EventHandler(&pVenc->m_cmp, pVenc->m_pAppData, OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        logw("VideoEncCreate fail");
		return -1;
	}

	if(pVenc->m_vencCodecType == VENC_CODEC_H264)
	{
		//* h264 param
		pVenc->m_vencH264Param.bEntropyCodingCABAC = 1;
		pVenc->m_vencH264Param.nBitrate = pVenc->m_sOutPutBitRateType.nTargetBitrate; /* bps */
		pVenc->m_vencH264Param.nFramerate = pVenc->m_framerate; /* fps */
		pVenc->m_vencH264Param.nCodingMode = VENC_FRAME_CODING;
		pVenc->m_vencH264Param.nMaxKeyInterval = 30;

		//set profile
		switch(pVenc->m_sH264Type.eProfile)
		{
			case 1:
				pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
				break;
			case 2:
				pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
				break;
			case 8:
				pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
				break;
				
			default:
				pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
				break;
		}

		if(pVenc->mIsFromVideoeditor){
			logd ("Called from Videoeditor,set VENC_H264ProfileBaseline");
			pVenc->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
		}

		logd("ycy profile-venc=%d, profile-omx=%d",pVenc->m_vencH264Param.sProfileLevel.nProfile,pVenc->m_sH264Type.eProfile);
		//set level
		switch(pVenc->m_sH264Type.eLevel)
		{
			case 0x100:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level3;
				break;
			case 0x200:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level31;
				break;
			case 0x400:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level32;
				break;
			case 0x800:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level4;
                break;
			case 0x1000:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
				break;
			case 0x2000:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level42;
                break;
			case 0x4000:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level5;
				break;
			case 0x8000:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level51;
				break;
			default:
				pVenc->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
				break;
		}
		
		pVenc->m_vencH264Param.sQPRange.nMinqp = 10;
		pVenc->m_vencH264Param.sQPRange.nMaxqp = 40;
		VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamH264Param, &pVenc->m_vencH264Param);

#if 0 //* do not use this function right now
		if(pVenc->m_vencCyclicIntraRefresh.bEnable)
		{
			VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamH264CyclicIntraRefresh, &pVenc->m_vencCyclicIntraRefresh);
		}
#endif

	}

	logd("openVencDriver: eColorFormat = %08x...\n", pVenc->m_sInPortFormat.eColorFormat);
	switch(pVenc->m_sInPortFormat.eColorFormat)
	{
		case OMX_COLOR_FormatYUV420SemiPlanar:
			pVenc->m_vencColorFormat = VENC_PIXEL_YVU420SP;
			if(pVenc->mIsFromCts)
				pVenc->m_vencColorFormat = VENC_PIXEL_YUV420SP;
			break;
		case OMX_COLOR_FormatAndroidOpaque:
			pVenc->m_vencColorFormat = VENC_PIXEL_ARGB; //maybe change this later;
			break;
		default:
			break;			
	}

	
	if(!pVenc->m_useMetaDataInBuffers)
	{
	
		pVenc->m_useAllocInputBuffer = OMX_TRUE;
	}
	else
	{
		if(pVenc->m_useAndroidBuffer && pVenc->mIsFromCts)
		{
		    VENC_COLOR_SPACE colorSpace = VENC_BT601;
			if(VideoEncSetParameter(pVenc->m_encoder, VENC_IndexParamRgb2Yuv, &colorSpace) != 0)
			{
				pVenc->m_useAllocInputBuffer = OMX_TRUE;
				pVenc->m_vencColorFormat = VENC_PIXEL_YUV420SP; //use ImgRGBA2YUV420SP_neon convert argb to yuv420sp
			}
			else
			{
				logd("use bt601 ok");
				pVenc->m_useAllocInputBuffer = OMX_FALSE;
			}
		}
		else
		{
			pVenc->m_useAllocInputBuffer = OMX_FALSE;
		}
	}

	logd("omx_venc base_config info: src_width: %d, src_height:%d, dis_width:%d, dis_height:%d", (int)pVenc->m_sInPortDef.format.video.nFrameWidth, (int)pVenc->m_sInPortDef.format.video.nFrameHeight,(int)pVenc->m_sOutPortDef.format.video.nFrameWidth,(int)pVenc->m_sOutPortDef.format.video.nFrameHeight);
	sBaseConfig.nInputWidth= pVenc->m_sInPortDef.format.video.nFrameWidth;
	sBaseConfig.nInputHeight = pVenc->m_sInPortDef.format.video.nFrameHeight;
	sBaseConfig.nStride = pVenc->m_sInPortDef.format.video.nStride;
	sBaseConfig.nDstWidth = pVenc->m_sOutPortDef.format.video.nFrameWidth;
	sBaseConfig.nDstHeight = pVenc->m_sOutPortDef.format.video.nFrameHeight;
	sBaseConfig.eInputFormat = pVenc->m_vencColorFormat;
	if(pVenc->m_useAndroidBuffer && (pVenc->mIsFromCts == false) && pVenc->m_vencColorFormat == VENC_PIXEL_ARGB)
		sBaseConfig.nStride = (sBaseConfig.nStride + 31)&(~31);
	result = VideoEncInit(pVenc->m_encoder, &sBaseConfig);

	if(result != 0)
	{	
		VideoEncDestroy(pVenc->m_encoder);
		pVenc->m_encoder = NULL;
		
		pVenc->m_Callbacks.EventHandler(&pVenc->m_cmp, pVenc->m_pAppData, OMX_EventError, OMX_ErrorHardware, 0 , NULL);
		logw("VideoEncInit, failed, result: %d\n", result);

		return -1;
	}

	if(pVenc->m_vencCodecType == VENC_CODEC_H264)
	{
		VideoEncGetParameter(pVenc->m_encoder, VENC_IndexParamH264SPSPPS, &pVenc->m_headdata);
	}


	size_y = sBaseConfig.nStride*sBaseConfig.nInputHeight;
	size_c = size_y>>1;

	pVenc->m_vencAllocBufferParam.nBufferNum = pVenc->m_sOutPortDef.nBufferCountActual;
	pVenc->m_vencAllocBufferParam.nSizeY = size_y;
	pVenc->m_vencAllocBufferParam.nSizeC = size_c;

	pVenc->m_firstFrameFlag = OMX_TRUE;
	pVenc->mFirstInputFrame = OMX_TRUE;

	if(pVenc->m_useAllocInputBuffer)
	{
		result = AllocInputBuffer(pVenc->m_encoder, &pVenc->m_vencAllocBufferParam);
		if(result !=0 )
		{
			VideoEncUnInit(pVenc->m_encoder);
			VideoEncDestroy(pVenc->m_encoder);
			loge("AllocInputBuffer,error");
			pVenc->m_Callbacks.EventHandler(&pVenc->m_cmp, pVenc->m_pAppData, OMX_EventError, OMX_ErrorHardware, 0 , NULL);
			return -1;
		}
	}

	return 0;
}


void closeVencDriver(omx_venc* pVenc)
{
	if(pVenc->m_useAllocInputBuffer)
	{
		// ReleaseAllocInputBuffer(pVenc->m_encoder);
		pVenc->m_useAllocInputBuffer = OMX_FALSE;
	}

	VideoEncUnInit(pVenc->m_encoder);
	VideoEncDestroy(pVenc->m_encoder);
	pVenc->m_encoder = NULL;	
}


/*
 *  Component Thread
 *    The ComponentThread function is exeuted in a separate pThread and
 *    is used to implement the actual component functions.
 */
 /*****************************************************************************/
static void* ComponentThread(void* pThreadData)
{
	int                     result;

    int                     i;
    int                     fd1;
    fd_set                  rfds;
    OMX_U32                 cmddata;
    ThrCmdType              cmd;

    // Variables related to encoder buffer handling
    OMX_BUFFERHEADERTYPE*   pInBufHdr   = NULL;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr  = NULL;
    OMX_MARKTYPE*           pMarkBuf    = NULL;
    OMX_U8*                 pInBuf      = NULL;
    OMX_U32                 nInBufSize;
	OMX_BOOL                bNoNeedSleep;
	int                     nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;

    // Variables related to encoder timeouts
    OMX_U32                 nTimeout;
    OMX_BOOL                port_setting_match;
    OMX_BOOL                nInBufEos;
	OMX_BOOL				nVencNotifyEosFlag;

    OMX_PTR                 pMarkData;
    OMX_HANDLETYPE          hMarkTargetComponent;

    struct timeval          timeout;

    port_setting_match   = OMX_TRUE;
    nInBufEos            = OMX_FALSE;
	nVencNotifyEosFlag   = OMX_FALSE;
    pMarkData            = NULL;
    hMarkTargetComponent = NULL;

	VencInputBuffer       sInputBuffer;
	VencInputBuffer       sInputBuffer_return;
	VencOutputBuffer      sOutputBuffer;

    // Recover the pointer to my component specific data
    omx_venc* pSelf = (omx_venc*)pThreadData;


    while (1)
    {
        fd1 = pSelf->m_cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);

        // Check for new command
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;

        i = select(pSelf->m_cmdpipe[0]+1, &rfds, NULL, NULL, &timeout);

        if (FD_ISSET(pSelf->m_cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            int ret = read(pSelf->m_cmdpipe[0], &cmd, sizeof(cmd));
	        ret = read(pSelf->m_cmddatapipe[0], &cmddata, sizeof(cmddata));

            // State transition command
	        if (cmd == SetState)
	        {
	        	logd("x set state command, cmd = %d, cmddata = %d.", (int)cmd, (int)cmddata);
                // If the parameter states a transition to the same state
                // raise a same state transition error.
                if (pSelf->m_state == (OMX_STATETYPE)(cmddata))
                {
                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorSameState, 0 , NULL);
                }
                else
                {
	                // transitions/callbacks made based on state transition table
                    // cmddata contains the target state
	                switch ((OMX_STATETYPE)(cmddata))
	                {
             	        case OMX_StateInvalid:
             	        	pSelf->m_state = OMX_StateInvalid;
             	        	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorInvalidState, 0 , NULL);
             	        	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);
			                break;

		                case OMX_StateLoaded:
                            if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateWaitForResources)
                            {
			                    nTimeout = 0x0;
			                    while (1)
			                    {
                                    // Transition happens only when the ports are unpopulated
			                        if (!pSelf->m_sInPortDef.bPopulated && !pSelf->m_sOutPortDef.bPopulated)
			                        {
			                        	pSelf->m_state = OMX_StateLoaded;
			                        	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);

				                        break;
 	                                }
				                    else if (nTimeout++ > OMX_MAX_TIMEOUTS)
				                    {
				                    	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorInsufficientResources, 0 , NULL);
 	                                    logw("Transition to loaded failed\n");
				                        break;
				                    }

			                        usleep(OMX_TIMEOUT*1000);
			                    }

		                    }
			                else
			                {
			                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
			                }

			                break;

                        case OMX_StateIdle:
		                    if (pSelf->m_state == OMX_StateInvalid)
		                    	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
		                    else
		                    {
			                    // Return buffers if currently in pause and executing
			                    if (pSelf->m_state == OMX_StatePause || pSelf->m_state == OMX_StateExecuting)
			                    {

									// venc should in idle state before stop
									post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Enc_Idle);

									if(pSelf->m_useAllocInputBuffer)
									{
										if(nInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
										{
											pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
										}
									}
									else
									{
										if(nInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
										{
											pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
										}

										//* check used buffer
										while(0==AlreadyUsedInputBuffer(pSelf->m_encoder, &sInputBuffer_return))
										{
											pInBufHdr = (OMX_BUFFERHEADERTYPE*)sInputBuffer_return.nID;
											pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);

										}
									}

			                    	pthread_mutex_lock(&pSelf->m_inBufMutex);

                                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                                    {
                                    	pSelf->m_sInBufList.nSizeOfList--;
                                    	pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp,
                                    			                           pSelf->m_pAppData,
                                    			                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                                    	if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                                    		pSelf->m_sInBufList.nReadPos = 0;
                                    }

			                    	pthread_mutex_unlock(&pSelf->m_inBufMutex);


			                    	pthread_mutex_lock(&pSelf->m_outBufMutex);

                                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                                    {
                                    	pSelf->m_sOutBufList.nSizeOfList--;
                                    	pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp,
                                    			                           pSelf->m_pAppData,
                                    			                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                                    	if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                                    		pSelf->m_sOutBufList.nReadPos = 0;
                                    }

			                    	pthread_mutex_unlock(&pSelf->m_outBufMutex);


									post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Close);
			                    }
			                    else
			                    {
									
			                    	//* init encoder 
									//post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Open);

			                    }

			                    nTimeout = 0x0;
			                    while (1)
			                    {
			                    	logv("pSelf->m_sInPortDef.bPopulated:%d, pSelf->m_sOutPortDef.bPopulated: %d", pSelf->m_sInPortDef.bPopulated, pSelf->m_sOutPortDef.bPopulated);
			                        // Ports have to be populated before transition completes
			                        if ((!pSelf->m_sInPortDef.bEnabled && !pSelf->m_sOutPortDef.bEnabled)   ||
                                        (pSelf->m_sInPortDef.bPopulated && pSelf->m_sOutPortDef.bPopulated))
                                    {
			                        	pSelf->m_state = OMX_StateIdle;
			                        	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);

 		                                break;
			                        }
			                        else if (nTimeout++ > OMX_MAX_TIMEOUTS)
			                        {
			                        	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorInsufficientResources, 0 , NULL);
                		                logw("Idle transition failed\n");
	                                    break;
				                    }

			                        usleep(OMX_TIMEOUT*1000);
			                    }
		                    }

		                    break;

		                case OMX_StateExecuting:


                            // Transition can only happen from pause or idle state
                            if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StatePause)
                            {
                                // Return buffers if currently in pause
			                    if (pSelf->m_state == OMX_StatePause)
			                    {
			                    	loge("encode component do not support OMX_StatePause");
									
			                    	pthread_mutex_lock(&pSelf->m_inBufMutex);

                                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                                    {
                                    	pSelf->m_sInBufList.nSizeOfList--;
                                    	pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp,
                                    			                           pSelf->m_pAppData,
                                    			                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                                    	if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                                    		pSelf->m_sInBufList.nReadPos = 0;
                                    }

			                    	pthread_mutex_unlock(&pSelf->m_inBufMutex);

			                    	pthread_mutex_lock(&pSelf->m_outBufMutex);

                                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                                    {
                                    	pSelf->m_sOutBufList.nSizeOfList--;
                                    	pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp,
                                    			                           pSelf->m_pAppData,
                                    			                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                                    	if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                                    		pSelf->m_sOutBufList.nReadPos = 0;
                                    }

			                    	pthread_mutex_unlock(&pSelf->m_outBufMutex);
			                    }

			                    pSelf->m_state = OMX_StateExecuting;
			                    pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);

								post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Open);
			                }
			                else
			                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);

                            nInBufEos            = OMX_FALSE; //if need it
                            pMarkData            = NULL;
                            hMarkTargetComponent = NULL;

			                break;

                        case OMX_StatePause:

							loge("encode component do not support OMX_StatePause");
                            // Transition can only happen from idle or executing state
		                    if (pSelf->m_state == OMX_StateIdle || pSelf->m_state == OMX_StateExecuting)
		                    {
		                    	pSelf->m_state = OMX_StatePause;
		                    	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);
		                    }
			                else
			                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);

		                    break;

                        case OMX_StateWaitForResources:
		                    if (pSelf->m_state == OMX_StateLoaded)
		                    {
		                    	pSelf->m_state = OMX_StateWaitForResources;
		                    	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, pSelf->m_state, NULL);
			                }
			                else
			                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);

		                    break;

                        default:

							loge("unknowed OMX_State");
		                	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorIncorrectStateTransition, 0 , NULL);
		                	break;

		            }
                }
	        }
	        else if (cmd == StopPort)
	        {
	        	logd("x stop port command, cmddata = %d.", (int)cmddata);
	            // Stop Port(s)
	            // cmddata contains the port index to be stopped.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be stopped.
	            if (cmddata == 0x0 || (int)cmddata == -1)
	            {
	                // Return all input buffers
                	pthread_mutex_lock(&pSelf->m_inBufMutex);

                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                    {
                    	pSelf->m_sInBufList.nSizeOfList--;
                    	pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp,
                    			                           pSelf->m_pAppData,
                    			                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                    	if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                    		pSelf->m_sInBufList.nReadPos = 0;
                    }

                	pthread_mutex_unlock(&pSelf->m_inBufMutex);

 		            // Disable port
					pSelf->m_sInPortDef.bEnabled = OMX_FALSE;
		        }

	            if (cmddata == 0x1 || (int)cmddata == -1)
	            {
		            // Return all output buffers
                	pthread_mutex_lock(&pSelf->m_outBufMutex);

                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                    {
                    	pSelf->m_sOutBufList.nSizeOfList--;
                    	pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp,
                    			                           pSelf->m_pAppData,
                    			                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                    	if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                    		pSelf->m_sOutBufList.nReadPos = 0;
                    }

                	pthread_mutex_unlock(&pSelf->m_outBufMutex);

       	            // Disable port
					pSelf->m_sOutPortDef.bEnabled = OMX_FALSE;
		        }

		        // Wait for all buffers to be freed
		        nTimeout = 0x0;
		        while (1)
		        {
		            if (cmddata == 0x0 && !pSelf->m_sInPortDef.bPopulated)
		            {
		                // Return cmdcomplete event if input unpopulated
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortDisable, 0x0, NULL);
			            break;
		            }

		            if (cmddata == 0x1 && !pSelf->m_sOutPortDef.bPopulated)
		            {
		                // Return cmdcomplete event if output unpopulated
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortDisable, 0x1, NULL);
			            break;
		            }

		            if ((int)cmddata == -1 &&  !pSelf->m_sInPortDef.bPopulated && !pSelf->m_sOutPortDef.bPopulated)
		            {
            		    // Return cmdcomplete event if inout & output unpopulated
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortDisable, 0x0, NULL);
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortDisable, 0x1, NULL);
			            break;
		            }

		            if (nTimeout++ > OMX_MAX_TIMEOUTS)
		            {
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorPortUnresponsiveDuringDeallocation, 0 , NULL);
       		            break;
		            }

                    usleep(OMX_TIMEOUT*1000);
		        }
	        }
	        else if (cmd == RestartPort)
	        {
	        	logd("x restart port command.");
                // Restart Port(s)
                // cmddata contains the port index to be restarted.
                // It is assumed that 0 is input and 1 is output port for this component.
                // The cmddata value -1 means both input and output ports will be restarted.
                if (cmddata == 0x0 || (int)cmddata == -1)
                	pSelf->m_sInPortDef.bEnabled = OMX_TRUE;

	            if (cmddata == 0x1 || (int)cmddata == -1)
	            	pSelf->m_sOutPortDef.bEnabled = OMX_TRUE;

 	            // Wait for port to be populated
		        nTimeout = 0x0;
		        while (1)
		        {
                    // Return cmdcomplete event if input port populated
		            if (cmddata == 0x0 && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sInPortDef.bPopulated))
		            {
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortEnable, 0x0, NULL);
			            break;
		            }
                    // Return cmdcomplete event if output port populated
		            else if (cmddata == 0x1 && (pSelf->m_state == OMX_StateLoaded || pSelf->m_sOutPortDef.bPopulated))
		            {
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortEnable, 0x1, NULL);
                        break;
		            }
                    // Return cmdcomplete event if input and output ports populated
		            else if ((int)cmddata == -1 && (pSelf->m_state == OMX_StateLoaded || (pSelf->m_sInPortDef.bPopulated && pSelf->m_sOutPortDef.bPopulated)))
		            {
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortEnable, 0x0, NULL);
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandPortEnable, 0x1, NULL);
			            break;
		            }
		            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
		            {
		            	pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorPortUnresponsiveDuringAllocation, 0, NULL);
                        break;
	                }

                    usleep(OMX_TIMEOUT*1000);
		        }

		        if(port_setting_match == OMX_FALSE)
		        	port_setting_match = OMX_TRUE;
	        }
	        else if (cmd == Flush)
	        {
	        	logd("x flush command.");
	            // Flush port(s)
                // cmddata contains the port index to be flushed.
                // It is assumed that 0 is input and 1 is output port for this component
                // The cmddata value -1 means that both input and output ports will be flushed.
	            if (cmddata == 0x0 || (int)cmddata == -1)
	            {
	                // Return all input buffers and send cmdcomplete
                	pthread_mutex_lock(&pSelf->m_inBufMutex);

                    while (pSelf->m_sInBufList.nSizeOfList > 0)
                    {
                    	pSelf->m_sInBufList.nSizeOfList--;
                    	pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp,
                    			                           pSelf->m_pAppData,
                    			                           pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++]);

                    	if (pSelf->m_sInBufList.nReadPos >= pSelf->m_sInBufList.nBufArrSize)
                    		pSelf->m_sInBufList.nReadPos = 0;
                    }

                	pthread_mutex_unlock(&pSelf->m_inBufMutex);

					pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandFlush, 0x0, NULL);
		        }

	            if (cmddata == 0x1 || (int)cmddata == -1)
	            {
	                // Return all output buffers and send cmdcomplete
                	pthread_mutex_lock(&pSelf->m_outBufMutex);

                    while (pSelf->m_sOutBufList.nSizeOfList > 0)
                    {
                    	pSelf->m_sOutBufList.nSizeOfList--;
                    	pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp,
                    			                           pSelf->m_pAppData,
                    			                           pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++]);

                    	if (pSelf->m_sOutBufList.nReadPos >= pSelf->m_sOutBufList.nBufArrSize)
                    		pSelf->m_sOutBufList.nReadPos = 0;
                    }

                	pthread_mutex_unlock(&pSelf->m_outBufMutex);

					pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventCmdComplete, OMX_CommandFlush, 0x1, NULL);
		        }
	        }
	        else if (cmd == Stop)
	        {
	        	logd("x stop command.");

				post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Stop);
				
  		        // Kill thread
 	            goto EXIT;
	        }
	        else if (cmd == FillBuf)
	        {
	        	logv("LINE %d", __LINE__);
	        	OMX_BUFFERHEADERTYPE*   OutBufHdr  = NULL;
	            // Fill buffer
	        	pthread_mutex_lock(&pSelf->m_outBufMutex);
	        	if (pSelf->m_sOutBufList.nSizeOfList <= pSelf->m_sOutBufList.nAllocSize)
	        	{
	        		pSelf->m_sOutBufList.nSizeOfList++;
	        		OutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nWritePos++] = ((OMX_BUFFERHEADERTYPE*) cmddata);

	                if (pSelf->m_sOutBufList.nWritePos >= (int)pSelf->m_sOutBufList.nAllocSize)
	                	pSelf->m_sOutBufList.nWritePos = 0;
	        	}
	        	//logd("###[FillBuf ] OutBufHdr=%p", OutBufHdr);

	        	pthread_mutex_unlock(&pSelf->m_outBufMutex);
            }
	        else if (cmd == EmptyBuf)
	        {
	        	logv("LINE %d", __LINE__);
	        	OMX_BUFFERHEADERTYPE*   inBufHdr  = NULL;

	            // Empty buffer
	    	    pthread_mutex_lock(&pSelf->m_inBufMutex);

	        	if (pSelf->m_sInBufList.nSizeOfList <= pSelf->m_sInBufList.nAllocSize)
	        	{
	        		pSelf->m_sInBufList.nSizeOfList++;
	        		inBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nWritePos++] = ((OMX_BUFFERHEADERTYPE*) cmddata);

	                if (pSelf->m_sInBufList.nWritePos >= (int)pSelf->m_sInBufList.nAllocSize)
	                	pSelf->m_sInBufList.nWritePos = 0;
	        	}
	    	    pthread_mutex_unlock(&pSelf->m_inBufMutex);
				
				
        	    // Mark current buffer if there is outstanding command
		        if (pMarkBuf)
		        {
		            ((OMX_BUFFERHEADERTYPE *)(cmddata))->hMarkTargetComponent = &pSelf->m_cmp;
		            ((OMX_BUFFERHEADERTYPE *)(cmddata))->pMarkData = pMarkBuf->pMarkData;
		            pMarkBuf = NULL;
		        }
		    }
	        else if (cmd == MarkBuf)
	        {
	            if (!pMarkBuf)
	                pMarkBuf = (OMX_MARKTYPE *)(cmddata);
	        }
	    }


        // Buffer processing
        // Only happens when the component is in executing state.
        if (pSelf->m_state == OMX_StateExecuting  &&
        	pSelf->m_sInPortDef.bEnabled          &&
        	pSelf->m_sOutPortDef.bEnabled         &&
        	port_setting_match)
        {

			//* check input buffer.
			bNoNeedSleep = OMX_FALSE;

			
			if (nInBufEos && (nInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER)) 
			{
				post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_Enc_Idle);
				
				if(ValidBitstreamFrameNum(pSelf->m_encoder) <= 0)
				{
					pthread_mutex_lock(&pSelf->m_outBufMutex);

					if(pSelf->m_sOutBufList.nSizeOfList > 0)
					{
						pSelf->m_sOutBufList.nSizeOfList--;
						pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
						if (pSelf->m_sOutBufList.nReadPos >= (int)pSelf->m_sOutBufList.nAllocSize)
							pSelf->m_sOutBufList.nReadPos = 0;
					}
					else
					{
						pOutBufHdr = NULL;
					}

					pthread_mutex_unlock(&pSelf->m_outBufMutex);


					//* if no output buffer, wait for some time.
					if(pOutBufHdr == NULL)
					{
					
					}
					else
					{
						pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
						pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

						pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
						pOutBufHdr->nFilledLen = 0;

						pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pOutBufHdr);
						pOutBufHdr = NULL;	
						nInBufEos = OMX_FALSE;
					}
				}
			}
			
			if(nInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER)
			{
				pthread_mutex_lock(&pSelf->m_inBufMutex);
				
				if(pSelf->m_sInBufList.nSizeOfList > 0)
				{
					pSelf->m_sInBufList.nSizeOfList--;
					pInBufHdr = pSelf->m_sInBufList.pBufHdrList[pSelf->m_sInBufList.nReadPos++];
					if (pSelf->m_sInBufList.nReadPos >= (int)pSelf->m_sInBufList.nAllocSize)
						pSelf->m_sInBufList.nReadPos = 0;
				
				}
				else
				{
					pInBufHdr = NULL;
				}


				pthread_mutex_unlock(&pSelf->m_inBufMutex);


				if(pInBufHdr)
				{
					bNoNeedSleep = OMX_TRUE;
				
					if (pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
					{
						// Copy flag to output buffer header
						nInBufEos = OMX_TRUE;
						logd(" set up nInBufEos flag.: %p", pInBufHdr);
				
						// Trigger event handler
						pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventBufferFlag, 0x1, pInBufHdr->nFlags, NULL);
				
						// Clear flag
						pInBufHdr->nFlags = 0;
					}
					
					// Check for mark buffers
					if (pInBufHdr->pMarkData)
					{
						// Copy mark to output buffer header
						if (pOutBufHdr)
						{
							pMarkData = pInBufHdr->pMarkData;
							// Copy handle to output buffer header
							hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;
						}
					}
				
					// Trigger event handler
					if (pInBufHdr->hMarkTargetComponent == &pSelf->m_cmp && pInBufHdr->pMarkData)
						pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventMark, 0, 0, pInBufHdr->pMarkData);


				
					if(!pSelf->m_useAllocInputBuffer)
					{	

						if(pInBufHdr->nFilledLen <= 0)
						{
							logw("skip this input buffer, pInBufHdr->nTimeStamp %lld", pInBufHdr->nTimeStamp);
							pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
							pInBufHdr = NULL;
						}
						else
						{
#if CONFIG_OS == OPTION_OS_ANDROID

							int buffer_type =  *(int*)(pInBufHdr->pBuffer+pInBufHdr->nOffset);
							
							if(pSelf->m_useAndroidBuffer || buffer_type == kMetadataBufferTypeGrallocSource)
							{
								unsigned int phyaddress = 0;
								buffer_handle_t bufferHanle;
		
								if(pSelf->m_sInPortFormat.eColorFormat != OMX_COLOR_FormatAndroidOpaque) {
									logw("do not support this format: %d", pSelf->m_sInPortFormat.eColorFormat);
								}
								
								bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer+pInBufHdr->nOffset + 4);

#if (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_2)

								const Rect rect(pSelf->m_sInPortDef.format.video.nFrameWidth, pSelf->m_sInPortDef.format.video.nFrameHeight);
								unsigned char *img;
								void *PhyAddr;
								int res;
								res = GraphicBufferMapper::get().lock(bufferHanle,
										GRALLOC_USAGE_SW_WRITE_OFTEN,
										rect, (void**)&img);
								if (res != OK) {
									loge("%s: Unable to lock image buffer %p for access", __FUNCTION__, bufferHanle);
									GraphicBufferMapper::get().unlock(bufferHanle);
								}

								GraphicBufferMapper::get().get_phy_addess(bufferHanle, &PhyAddr);
								phyaddress = (unsigned int)PhyAddr;

#else

		                        if(bufferHanle)
		                        {
									int colorFormat;

									//for mali GPU
#if (CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667)
									private_handle_t* hnd = (private_handle_t *)(bufferHanle);
									colorFormat = hnd->format;
#else 
									IMG_native_handle_t* hnd = (IMG_native_handle_t*)(bufferHanle);
									colorFormat = hnd->iFormat;
#endif

								    switch(colorFormat)
								    {
									    case HAL_PIXEL_FORMAT_RGBA_8888:
											if(pSelf->mFirstInputFrame)
											{
												pSelf->m_vencColorFormat = VENC_PIXEL_ABGR;
											    post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_ChangeColorFormat);
												pSelf->mFirstInputFrame = OMX_FALSE;
											    logd("set color format to ABGR");
											}
											break;
									    case HAL_PIXEL_FORMAT_BGRA_8888:
											logd("do nothing, defalt is ARGB");
											break;
									    default:
											logw("do not support this format: %d", colorFormat);
											break;
								    }

		                            int fd = ion_open();

#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
	                                ion_user_handle_t handle_ion;
#else
	                                struct ion_handle *handle_ion;
#endif
		                            
		                            if(fd != -1)
		                            {

#if (CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667) 
										ion_import(fd, hnd->share_fd, &handle_ion);
#else
										ion_import(fd, hnd->fd[0], &handle_ion);
#endif
		                                phyaddress= ion_getphyadr(fd, handle_ion);
		                                ion_close(fd);
		                            }
		                            else
		                            {
		                                loge("ion_open fail");
		                            }
		                        }
		                        else
		                        {
		                            loge("bufferHanle is null");
		                        }
#endif

								logv("phyaddress: %x", phyaddress);
								sInputBuffer.pAddrPhyY= (unsigned char *)phyaddress; // only support ARGB now
								sInputBuffer.pAddrPhyC = 0;

							}
							else
							{
							#if 0
								if(pSelf->mFirstInputFrame)
								{
									pSelf->m_vencColorFormat = VENC_PIXEL_YVU420SP;
								    post_message_to_venc_and_wait(pSelf, OMX_Venc_Cmd_ChangeColorFormat);
									pSelf->mFirstInputFrame = OMX_FALSE;
								}
							#endif
								
								if(buffer_type != kMetadataBufferTypeCameraSource)
								{
									logw("skip this input buffer, error buffer type: %d", buffer_type);
									pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
									pInBufHdr = NULL;
								}
								else
								{
									memcpy(&sInputBuffer, (pInBufHdr->pBuffer+pInBufHdr->nOffset + 4), sizeof(VencInputBuffer));
									sInputBuffer.pAddrPhyC= sInputBuffer.pAddrPhyY + pSelf->m_sInPortDef.format.video.nStride*pSelf->m_sInPortDef.format.video.nFrameHeight;
								}
							}

							//* clear flag
							sInputBuffer.nFlag = 0;
							sInputBuffer.nPts = pInBufHdr->nTimeStamp;
							sInputBuffer.nID = (int)pInBufHdr;
							
							if(pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
							{
								sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
							}
							

							result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
							if(result!=0)
							{
								nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
							}
							else
							{
								nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
							}
#else
							loge("do not support metadata input buffer");
#endif							
						}



					}
					else
					{	
						int size_y;
						int size_c;
						int buffer_type =  *(int*)(pInBufHdr->pBuffer+pInBufHdr->nOffset);


						//if( pSelf->m_useMetaDataInBuffers && buffer_type != kMetadataBufferTypeGrallocSource)
						if(pInBufHdr->nFilledLen <= 0)
						{
							logw("skip this input buffer, pInBufHd:%p, buffer_type=%08x, buf_size=%d", pInBufHdr,buffer_type,(int)pInBufHdr->nFilledLen);
							pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
							pInBufHdr = NULL;
						}
						else
						{
							result = GetOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);

							if(result !=0)
							{
								nInputBufferStep = OMX_VENC_STEP_GET_ALLOCBUFFER;
							}
							else
							{
								
								switch(pSelf->m_sInPortFormat.eColorFormat)
								{
								
									case OMX_COLOR_FormatYUV420SemiPlanar:
										size_y = pSelf->m_sInPortDef.format.video.nStride*pSelf->m_sInPortDef.format.video.nFrameHeight;
										size_c = size_y>>1;
										break;
									default:
										size_y = pSelf->m_sInPortDef.format.video.nStride*pSelf->m_sInPortDef.format.video.nFrameHeight;
										size_c = size_y>>1;
										break;
								}


								//* clear flag
								sInputBuffer.nFlag = 0;
								if(pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
								{
									sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
								}

								sInputBuffer.nPts = pInBufHdr->nTimeStamp;
								sInputBuffer.bEnableCorp = 0;

								if(pSelf->m_sInPortFormat.eColorFormat == OMX_COLOR_FormatAndroidOpaque)
								{

#if CONFIG_OS == OPTION_OS_ANDROID
								    void* bufAddr;
									buffer_handle_t bufferHanle;
		
									int width;
									int height;

									//* get the argb buffer.
									bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer+pInBufHdr->nOffset + 4);
			                    	android::Rect rect(pSelf->m_sInPortDef.format.video.nStride, pSelf->m_sInPortDef.format.video.nFrameHeight);
									GraphicBufferMapper::get().lock(bufferHanle, GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &bufAddr);

									width = pSelf->m_sInPortDef.format.video.nStride;
									height = pSelf->m_sInPortDef.format.video.nFrameHeight;

									if(width % 32 == 0)
									{
										int widthandstride[2];
										unsigned char* addr[2];
										
										widthandstride[0] = width;
										widthandstride[1] = width;

										addr[0] = sInputBuffer.pAddrVirY;
										addr[1] = sInputBuffer.pAddrVirC;
										
										ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr, widthandstride, height);

									}
									else
									{
										int widthandstride[2];
										unsigned char* addr[2];
										
										widthandstride[0] = width;
										widthandstride[1] = (width + 31) & (~31);

										addr[0] = sInputBuffer.pAddrVirY;
										addr[1] = sInputBuffer.pAddrVirC;
										
										ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr, widthandstride, height);

										
									}
									

									MemAdapterFlushCache(sInputBuffer.pAddrVirY, width*height);
									MemAdapterFlushCache(sInputBuffer.pAddrVirC, width*height/2);

									GraphicBufferMapper::get().unlock(bufferHanle);
#endif								
								}
								else
								{
									memcpy(sInputBuffer.pAddrVirY, pInBufHdr->pBuffer + pInBufHdr->nOffset, size_y);
									memcpy(sInputBuffer.pAddrVirC, pInBufHdr->pBuffer + pInBufHdr->nOffset + size_y, size_c);

									
								}
									

								pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);

								FlushCacheAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);
								
								result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
								if(result!=0)
								{
									nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
								}
								else
								{
									nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
								}
							}
						}
					}
				}
				else
				{
					//* do nothing
				}
			}
			else if(nInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
			{
				int size_y;
				int size_c;
			
				result = GetOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer);
			
				if(result !=0)
				{
					nInputBufferStep = OMX_VENC_STEP_GET_ALLOCBUFFER;
				}
				else
				{
					
					switch(pSelf->m_sInPortFormat.eColorFormat)
					{
					
						case OMX_COLOR_FormatYUV420SemiPlanar:
							size_y = pSelf->m_sInPortDef.format.video.nStride*pSelf->m_sInPortDef.format.video.nFrameHeight;
							size_c = size_y>>1;
							break;
						default:
							size_y = pSelf->m_sInPortDef.format.video.nStride*pSelf->m_sInPortDef.format.video.nFrameHeight;
							size_c = size_y>>1;
							break;
					}
			
			
					//* clear flag
					sInputBuffer.nFlag = 0;
					if(pInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
					{
						sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
					}
			
					sInputBuffer.nPts = pInBufHdr->nTimeStamp;
					sInputBuffer.bEnableCorp = 0;

					if(pSelf->m_sInPortFormat.eColorFormat == OMX_COLOR_FormatAndroidOpaque)
					{

#if CONFIG_OS == OPTION_OS_ANDROID
						void* bufAddr;
						buffer_handle_t bufferHanle;
		
						int width;
						int height;

						//* get the argb buffer.
						bufferHanle = *(buffer_handle_t*)(pInBufHdr->pBuffer+pInBufHdr->nOffset + 4);
			            android::Rect rect(pSelf->m_sInPortDef.format.video.nStride, pSelf->m_sInPortDef.format.video.nFrameHeight);
						GraphicBufferMapper::get().lock(bufferHanle, GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &bufAddr);

						width = pSelf->m_sInPortDef.format.video.nStride;
						height = pSelf->m_sInPortDef.format.video.nFrameHeight;

						if(width % 32 == 0)
						{
							int widthandstride[2];
							unsigned char* addr[2];
							
							widthandstride[0] = width;
							widthandstride[1] = width;

							addr[0] = sInputBuffer.pAddrVirY;
							addr[1] = sInputBuffer.pAddrVirC;
							
							ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr, widthandstride, height);

						}
						else
						{
							int widthandstride[2];
							unsigned char* addr[2];
							
							widthandstride[0] = width;
							widthandstride[1] = (width + 31) & (~31);

							addr[0] = sInputBuffer.pAddrVirY;
							addr[1] = sInputBuffer.pAddrVirC;
							
							ImgRGBA2YUV420SP_neon((unsigned char *)bufAddr, addr, widthandstride, height);

							
						}
						

						MemAdapterFlushCache(sInputBuffer.pAddrVirY, width*height);
						MemAdapterFlushCache(sInputBuffer.pAddrVirC, width*height/2);

						GraphicBufferMapper::get().unlock(bufferHanle);
#endif						
					}
					else
					{
						memcpy(sInputBuffer.pAddrVirY, pInBufHdr->pBuffer + pInBufHdr->nOffset, size_y);
						memcpy(sInputBuffer.pAddrVirC, pInBufHdr->pBuffer + pInBufHdr->nOffset + size_y, size_c);
					}

					pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
					
					result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
					if(result!=0)
					{
						nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
					}
					else
					{
						nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
					}
				}
		
				bNoNeedSleep = OMX_TRUE;
			}
			else if(nInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
			{
				result = AddOneInputBuffer(pSelf->m_encoder, &sInputBuffer);
				if(result!=0)
				{
					nInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;

				}
				else
				{
					nInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
					bNoNeedSleep = OMX_TRUE;
				}
			}


			//* check used buffer
			if(0==AlreadyUsedInputBuffer(pSelf->m_encoder, &sInputBuffer_return))
			{
				bNoNeedSleep = OMX_TRUE;

				if(pSelf->m_useAllocInputBuffer)
				{
					ReturnOneAllocInputBuffer(pSelf->m_encoder, &sInputBuffer_return);
				}
				else
				{
					pInBufHdr = (OMX_BUFFERHEADERTYPE*)sInputBuffer_return.nID;
					pSelf->m_Callbacks.EmptyBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pInBufHdr);
				}
			}

			if(ValidBitstreamFrameNum(pSelf->m_encoder) > 0)
			{
				//* check output buffer
				pthread_mutex_lock(&pSelf->m_outBufMutex);

	            if(pSelf->m_sOutBufList.nSizeOfList > 0)
	            {
	            	pSelf->m_sOutBufList.nSizeOfList--;
	            	pOutBufHdr = pSelf->m_sOutBufList.pBufHdrList[pSelf->m_sOutBufList.nReadPos++];
	            	if (pSelf->m_sOutBufList.nReadPos >= (int)pSelf->m_sOutBufList.nAllocSize)
	            		pSelf->m_sOutBufList.nReadPos = 0;
	            }
	            else
	            {
	            	pOutBufHdr = NULL;
	            }

	            pthread_mutex_unlock(&pSelf->m_outBufMutex);

	        	if(pOutBufHdr)
				{
					pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
					pOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

					if(pSelf->m_firstFrameFlag && pSelf->m_vencCodecType == VENC_CODEC_H264)
					{
						pOutBufHdr->nTimeStamp = 0; //fixed it later;
						pOutBufHdr->nFilledLen = pSelf->m_headdata.nLength;
						pOutBufHdr->nOffset = 0;

						memcpy(pOutBufHdr->pBuffer, pSelf->m_headdata.pBuffer, pOutBufHdr->nFilledLen);
	                    pSelf->m_firstFrameFlag = OMX_FALSE;
						pOutBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;

#if SAVE_BITSTREAM
						if(OutFile)
						{
							fwrite(pOutBufHdr->pBuffer, 1, pOutBufHdr->nFilledLen, OutFile);
						}
						else
						{
							logw("open outfile failed");
						}
#endif

					    pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pOutBufHdr);
		                pOutBufHdr = NULL;	
					}
					else
					{
						GetOneBitstreamFrame(pSelf->m_encoder, &sOutputBuffer);
		            	
		            	pOutBufHdr->nTimeStamp = sOutputBuffer.nPts;
		            	pOutBufHdr->nFilledLen = sOutputBuffer.nSize0 + sOutputBuffer.nSize1;
					    pOutBufHdr->nOffset = 0;

						pOutBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

						if (sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME)
						{
							pOutBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
						}

						if (sOutputBuffer.nFlag & VENC_BUFFERFLAG_EOS)
						{
							pOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
						}

						if(pSelf->m_prependSPSPPSToIDRFrames == OMX_TRUE && (sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME))
						{
							memcpy(pOutBufHdr->pBuffer, pSelf->m_headdata.pBuffer, pSelf->m_headdata.nLength);
							memcpy(pOutBufHdr->pBuffer + pSelf->m_headdata.nLength, sOutputBuffer.pData0, sOutputBuffer.nSize0);
							if(sOutputBuffer.nSize1)
							{
								memcpy(pOutBufHdr->pBuffer + pSelf->m_headdata.nLength + sOutputBuffer.nSize0, sOutputBuffer.pData1, sOutputBuffer.nSize1);
							}

							pOutBufHdr->nFilledLen += pSelf->m_headdata.nLength;
						}
						else
						{
							memcpy(pOutBufHdr->pBuffer, sOutputBuffer.pData0, sOutputBuffer.nSize0);
							if(sOutputBuffer.nSize1){
								memcpy(pOutBufHdr->pBuffer + sOutputBuffer.nSize0, sOutputBuffer.pData1, sOutputBuffer.nSize1);
							}					
						}

#if SAVE_BITSTREAM
						if(OutFile)
						{
							fwrite(pOutBufHdr->pBuffer, 1, pOutBufHdr->nFilledLen, OutFile);
						}
						else
						{
							logw("open outfile failed");
						}
#endif


		                FreeOneBitStreamFrame(pSelf->m_encoder, &sOutputBuffer);

			            // Check for mark buffers
			            if (pMarkData != NULL && hMarkTargetComponent != NULL)
			            {
		                	if(ValidBitstreamFrameNum(pSelf->m_encoder) == 0)
		                	{
		                		// Copy mark to output buffer header
		                		pOutBufHdr->pMarkData = pInBufHdr->pMarkData;
		                		// Copy handle to output buffer header
		                		pOutBufHdr->hMarkTargetComponent = pInBufHdr->hMarkTargetComponent;

		                		pMarkData = NULL;
		                		hMarkTargetComponent = NULL;
		                	}
		 		        }

		                pSelf->m_Callbacks.FillBufferDone(&pSelf->m_cmp, pSelf->m_pAppData, pOutBufHdr);
		                pOutBufHdr = NULL;	
					}
				}
				else
				{
					//* do nothing
				}			
			}

			if(!bNoNeedSleep)
			{
				logv("need sleep");
				usleep(20*1000);
			}
			else
			{
				logv("no need sleep");
			}
        }
    }

EXIT:

	return (void*)OMX_ErrorNone;
}


static void* ComponentVencThread(void* pThreadData)
{
	int                     result = 0;
    int                     i;
    int                     fd1;
    fd_set                  rfds;
    OMX_VENC_COMMANDTYPE    cmd;
    OMX_BOOL                nEosFlag = OMX_FALSE;

    struct timeval          timeout;

    int nSemVal;
    int nRetSemGetValue;
    int nStopFlag = 0;
	int nWaitIdle = 0;
	
    // Recover the pointer to my component specific data
    omx_venc* pSelf = (omx_venc*)pThreadData;

    while (1)
    {
        fd1 = pSelf->m_venc_cmdpipe[0];
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);
        // Check for new command
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;

        i = select(pSelf->m_venc_cmdpipe[0]+1, &rfds, NULL, NULL, &timeout);

        if (FD_ISSET(pSelf->m_venc_cmdpipe[0], &rfds))
        {
            // retrieve command and data from pipe
            int ret = read(pSelf->m_venc_cmdpipe[0], &cmd, sizeof(cmd));
            logv("(f:%s, l:%d) vdrvThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
            // State transition command

			switch(cmd)
			{
				case OMX_Venc_Cmd_Open:
					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					if(!pSelf->m_encoder)
					{
						openVencDriver(pSelf);
					}
					omx_sem_up(&pSelf->m_msg_sem);
					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					
					break;

				case OMX_Venc_Cmd_Close:

					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					if(pSelf->m_encoder)
					{
						closeVencDriver(pSelf);
					}
					
					omx_sem_up(&pSelf->m_msg_sem);
					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					break;

				case OMX_Venc_Cmd_Stop:

					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);	
					nStopFlag = 1;
					omx_sem_up(&pSelf->m_msg_sem);
					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					break;

				case OMX_Venc_Cmd_Enc_Idle:

					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);	
					nWaitIdle = 1;
					logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
					break;

				case OMX_Venc_Cmd_ChangeBitrate:
					logd("pSelf->m_sOutPortDef.format.video.nBitrate: %d", (int)pSelf->m_sOutPortDef.format.video.nBitrate);
					VideoEncSetParameter(pSelf->m_encoder, VENC_IndexParamBitrate, &pSelf->m_sOutPortDef.format.video.nBitrate);
					break;

				case OMX_Venc_Cmd_ChangeColorFormat:
					VideoEncSetParameter(pSelf->m_encoder, VENC_IndexParamColorFormat, &pSelf->m_vencColorFormat);
					omx_sem_up(&pSelf->m_msg_sem);
					break;

				case OMX_Venc_Cmd_RequestIDRFrame:
				{
					int value = 1;
					VideoEncSetParameter(pSelf->m_encoder,VENC_IndexParamForceKeyFrame, &value);
					logd("(f:%s, l:%d) OMX_Venc_Cmd_RequestIDRFrame[0x%x]", __FUNCTION__, __LINE__, cmd);
					break;
				}
				default:
					logw("unknown cmd: %d", cmd);
					break;
			}
	    }

        if(nStopFlag)
        {
            logd("vencThread detect nStopFlag[%d], exit!", nStopFlag);
            goto EXIT;
		}


        if (pSelf->m_state == OMX_StateExecuting && pSelf->m_encoder)
        {
			result = VideoEncodeOneFrame(pSelf->m_encoder);

			if(result == VENC_RESULT_ERROR)
			{
				pSelf->m_Callbacks.EventHandler(&pSelf->m_cmp, pSelf->m_pAppData, OMX_EventError, OMX_ErrorHardware, 0 , NULL);
				loge("VideoEncodeOneFrame, failed, result: %d\n", result);
			}

			if(nWaitIdle && result == VENC_RESULT_NO_FRAME_BUFFER)
			{
				logv("input buffer idle \n");
				omx_sem_up(&pSelf->m_msg_sem);
				nWaitIdle = 0;
			}

			if(result != VENC_RESULT_OK)
			{
				waitPipeDataToRead(pSelf->m_venc_cmdpipe[0], 20*1000);
			}
        }
        else
        {
            waitPipeDataToRead(pSelf->m_venc_cmdpipe[0], 20*1000);
        }
    }

EXIT:
    return (void*)OMX_ErrorNone;
}


