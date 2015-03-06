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
#ifndef __OMX_VDEC_H__
#define __OMX_VDEC_H__
/*============================================================================
                            O p e n M A X   Component
                                Video Decoder

*//** @file omx_vdec.h
  This module contains the class definition for openMAX decoder component.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <semaphore.h>

#include "OMX_Core.h"
#include "aw_omx_component.h"

#include "vdecoder.h"

#include <hardware/hal_public.h>
#include <linux/ion.h>
#include <ion/ion.h>

extern "C"
{
	OMX_API void* get_omx_component_factory_fn(void);
}

/*
 * Enumeration for the commands processed by the component
 */

typedef enum ThrCmdType
{
    SetState,
    Flush,
    StopPort,
    RestartPort,
    MarkBuf,
    Stop,
    FillBuf,
    EmptyBuf,
    VideoSizeInfo,

    VdrvNotifyEos = 0x100,
    VdrvResolutionChange,
    VdrvNotify_,
} ThrCmdType;

//////////////////////////////////////////////////////////////////////////////
//                       Module specific globals
//////////////////////////////////////////////////////////////////////////////
#define OMX_SPEC_VERSION  0x00000101

/*
 *  D E C L A R A T I O N S
 */
#define OMX_NOPORT                      0xFFFFFFFE
#define NUM_IN_BUFFERS                  2        	  // Input Buffers
#define NUM_IN_BUFFERS_SECURE           1        	  // Input Buffers(secure)
#define NUM_OUT_BUFFERS                 10       	  // Output Buffers
#define OMX_TIMEOUT                     10            // Timeout value in milisecond
#define OMX_MAX_TIMEOUTS                160   		  // Count of Maximum number of times the component can time out
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE (2*1024*1024)     // 2 MB
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE_SECURE (256*1024) // 256 kB

#define AWOMX_PTS_JUMP_THRESH             (2000000)   //pts jump threshold to judge, unit:us

#define OMX_MAX_VIDEO_BUFFER_NUM 32
#define CODEC_SPECIFIC_DATA_LENGTH 2*1024
/*
 *     D E F I N I T I O N S
 */

typedef struct _BufferList BufferList;


/*
 * The main structure for buffer management.
 *
 *   pBufHdr     - An array of pointers to buffer headers.
 *                 The size of the array is set dynamically using the nBufferCountActual value
 *                   send by the client.
 *   nListEnd    - Marker to the boundary of the array. This points to the last index of the
 *                   pBufHdr array.
 *   nSizeOfList - Count of valid data in the list.
 *   nAllocSize  - Size of the allocated list. This is equal to (nListEnd + 1) in most of
 *                   the times. When the list is freed this is decremented and at that
 *                   time the value is not equal to (nListEnd + 1). This is because
 *                   the list is not freed from the end and hence we cannot decrement
 *                   nListEnd each time we free an element in the list. When nAllocSize is zero,
 *                   the list is completely freed and the other paramaters of the list are
 *                   initialized.
 *                 If the client crashes before freeing up the buffers, this parameter is
 *                   checked (for nonzero value) to see if there are still elements on the list.
 *                   If yes, then the remaining elements are freed.
 *    nWritePos  - The position where the next buffer would be written. The value is incremented
 *                   after the write. It is wrapped around when it is greater than nListEnd.
 *    nReadPos   - The position from where the next buffer would be read. The value is incremented
 *                   after the read. It is wrapped around when it is greater than nListEnd.
 *    eDir       - Type of BufferList.
 *                            OMX_DirInput  =  Input  Buffer List
 *                           OMX_DirOutput  =  Output Buffer List
 */
 
struct _BufferList
{
   OMX_BUFFERHEADERTYPE**   pBufHdrList;
   OMX_U32                  nSizeOfList;
   OMX_S32                  nWritePos;
   OMX_S32                  nReadPos;

   OMX_BUFFERHEADERTYPE*    pBufArr;
   OMX_S32                  nAllocBySelfFlags;
   OMX_S32                  nBufArrSize;
   OMX_U32                  nAllocSize;
   OMX_DIRTYPE              eDir;
};



typedef struct VIDDEC_CUSTOM_PARAM
{
    unsigned char cCustomParamName[128];
    OMX_INDEXTYPE nCustomParamIndex;
} VIDDEC_CUSTOM_PARAM;

typedef struct VIDEO_PROFILE_LEVEL
{
    OMX_S32  nProfile;
    OMX_S32  nLevel;
} VIDEO_PROFILE_LEVEL_TYPE;

typedef enum OMX_VDRV_FEEDBACK_MSGTYPE
{
    OMX_VdrvFeedbackMsg_NotifyEos        = OMX_CommandVendorStartUnused,   //vdeclib has drained all input data.
    OMX_VdrvFeedbackMsg_ResolutionChange,
    OMX_VdrvFeedbackMsg_Max              = 0X7FFFFFFF,
} OMX_VDRV_FEEDBACK_MSGTYPE;

//* Out Buffer can be owned by twos at the same time 
enum OutBufferStatus {
    OWNED_BY_UPSTREAM   = 1,  //*means owned by ACodec.cpp or OMXCodec
    OWNED_BY_US         = 2,  //*means owned by omx_vdec.cpp
    OWNED_BY_DOWNSTREAM = 4,  //*means owned by decoder.cpp
};

typedef struct OMXGpuBufferInfoS
{
    ANativeWindowBuffer*  pWindowBuf;
	
#if (CONFIG_OS_VERSION >= OPTION_OS_VERSION_ANDROID_5_0)
	ion_user_handle_t handle_ion;
#else
	struct ion_handle *handle_ion;
#endif

    OMX_BUFFERHEADERTYPE* pHeadBufInfo; 
    char*                 pBufPhyAddr;
    char*                 pBufVirAddr;
}OMXGpuBufferInfoT;

//* describe the output buffer (malloc by gpu), share by gpu and decoder
typedef struct OMXOutputBufferInfoS
{
    VideoPicture     mRelateVideoPicInfo;
    OMXGpuBufferInfoT mRelateGpuInfo;
    OMX_U32           mStatus;
}OMXOutputBufferInfoT;

typedef struct OMXDisplayBufferNodeS OMXDisplayBufferNodeT; 

struct OMXDisplayBufferNodeS
{
    OMXOutputBufferInfoT*   pOutputBufferInfo;
    OMXDisplayBufferNodeT*  pNext;
    int                     bUseFlag;
    int                     nPrivate;
};

// OMX video decoder class
class omx_vdec: public aw_omx_component
{
public:
    omx_vdec();           // constructor
    virtual ~omx_vdec();  // destructor

    OMX_ERRORTYPE allocate_buffer(OMX_HANDLETYPE         hComp,
                                  OMX_BUFFERHEADERTYPE** bufferHdr,
                                  OMX_U32                port,
                                  OMX_PTR                appData,
                                  OMX_U32                bytes
                                  );


    OMX_ERRORTYPE component_deinit(OMX_HANDLETYPE hComp);

    OMX_ERRORTYPE component_init(OMX_STRING role);

    OMX_ERRORTYPE component_role_enum(OMX_HANDLETYPE hComp,
                                      OMX_U8*        role,
                                      OMX_U32        index
                                      );

    OMX_ERRORTYPE component_tunnel_request(OMX_HANDLETYPE       hComp,
                                           OMX_U32              port,
                                           OMX_HANDLETYPE       peerComponent,
                                           OMX_U32              peerPort,
                                           OMX_TUNNELSETUPTYPE* tunnelSetup
                                           );

    OMX_ERRORTYPE empty_this_buffer(OMX_HANDLETYPE        hComp,
                                    OMX_BUFFERHEADERTYPE* buffer
                                    );



    OMX_ERRORTYPE fill_this_buffer(OMX_HANDLETYPE        hComp,
                                   OMX_BUFFERHEADERTYPE* buffer
                                   );


    OMX_ERRORTYPE free_buffer(OMX_HANDLETYPE        hComp,
                              OMX_U32               port,
                              OMX_BUFFERHEADERTYPE* buffer
                              );

    OMX_ERRORTYPE get_component_version(OMX_HANDLETYPE   hComp,
                                        OMX_STRING       componentName,
                                        OMX_VERSIONTYPE* componentVersion,
                                        OMX_VERSIONTYPE* specVersion,
                                        OMX_UUIDTYPE*    componentUUID
                                        );

    OMX_ERRORTYPE get_config(OMX_HANDLETYPE hComp,
                             OMX_INDEXTYPE  configIndex,
                             OMX_PTR        configData
                             );

    OMX_ERRORTYPE get_extension_index(OMX_HANDLETYPE hComp,
                                      OMX_STRING     paramName,
                                      OMX_INDEXTYPE* indexType
                                      );

    OMX_ERRORTYPE get_parameter(OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE  paramIndex,
                                OMX_PTR        paramData);

    OMX_ERRORTYPE get_state(OMX_HANDLETYPE hComp,
                            OMX_STATETYPE *state);


    OMX_ERRORTYPE send_command(OMX_HANDLETYPE  hComp,
                               OMX_COMMANDTYPE cmd,
                               OMX_U32         param1,
                               OMX_PTR         cmdData);


    OMX_ERRORTYPE set_callbacks(OMX_HANDLETYPE    hComp,
                                OMX_CALLBACKTYPE* callbacks,
                                OMX_PTR           appData);

    OMX_ERRORTYPE set_config(OMX_HANDLETYPE hComp,
                             OMX_INDEXTYPE  configIndex,
                             OMX_PTR        configData);

    OMX_ERRORTYPE set_parameter(OMX_HANDLETYPE hComp,
                                OMX_INDEXTYPE  paramIndex,
                                OMX_PTR        paramData);

    OMX_ERRORTYPE use_buffer(OMX_HANDLETYPE         hComp,
                             OMX_BUFFERHEADERTYPE** bufferHdr,
                             OMX_U32                port,
                             OMX_PTR                appData,
                             OMX_U32                bytes,
                             OMX_U8*                buffer);


    OMX_ERRORTYPE use_EGL_image(OMX_HANDLETYPE         hComp,
                                OMX_BUFFERHEADERTYPE** bufferHdr,
                                OMX_U32                port,
                                OMX_PTR                appData,
                                void *                 eglImage);

    OMX_ERRORTYPE send_vdrv_feedback_msg(OMX_VDRV_FEEDBACK_MSGTYPE nMsg,
                                           OMX_U32         param1,
                                           OMX_PTR         cmdData);
public:
    OMX_ERRORTYPE post_event(ThrCmdType eCmd,
                           OMX_U32         param1,
                           OMX_PTR         cmdData);
public:
    OMX_STATETYPE                   m_state;
    OMX_U8                       	m_cRole[OMX_MAX_STRINGNAME_SIZE];
    OMX_U8                       	m_cName[OMX_MAX_STRINGNAME_SIZE];
    OMX_VIDEO_CODINGTYPE         	m_eCompressionFormat;
    OMX_CALLBACKTYPE                m_Callbacks;
    OMX_PTR                         m_pAppData;
    OMX_PORT_PARAM_TYPE             m_sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sInPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE    m_sOutPortDef;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sInPortFormat;
    OMX_VIDEO_PARAM_PORTFORMATTYPE  m_sOutPortFormat;
    OMX_PRIORITYMGMTTYPE            m_sPriorityMgmt;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sInBufSupplier;
    OMX_PARAM_BUFFERSUPPLIERTYPE    m_sOutBufSupplier;
    pthread_t                       m_thread_id;
    pthread_t                       m_vdrv_thread_id;
    int                             m_cmdpipe[2];
    int                             m_cmddatapipe[2];
    int                             m_vdrv_cmdpipe[2];     //pipe connect main_thread and vdrv_thread
    BufferList                      m_sInBufList;
    BufferList                      m_sOutBufList;
    pthread_mutex_t					m_inBufMutex;
    pthread_mutex_t                 m_outBufMutex;
    pthread_mutex_t                 m_BufferInfoMutex;

    pthread_mutex_t                 m_pipeMutex;
    sem_t                           m_vdrv_cmd_lock;       //for synchronise cmd.
    //* for cedarv decoder.
    VideoDecoder*                   m_decoder;
    VideoStreamInfo                 m_streamInfo;

    OMX_BOOL                        m_useAndroidBuffer;
    
    //* for detect pts jump
    OMX_BOOL                        m_storeOutputMetaDataFlag;
    OMX_S32                         m_maxWidth;
    OMX_S32                         m_maxHeight;
    OMX_BOOL                        m_firstPrepareDecoderFlag;

	OMX_S32							m_InputNum;
	OMX_S32							m_OutputNum;
    //for statistics
    int64_t     mDecodeFrameTotalDuration;
    int64_t     mDecodeOKTotalDuration;
    int64_t     mDecodeNoFrameTotalDuration;
    int64_t     mDecodeNoBitstreamTotalDuration;
    int64_t     mDecodeOtherTotalDuration;
    int64_t     mDecodeFrameTotalCount;
    int64_t     mDecodeOKTotalCount;
    int64_t     mDecodeNoFrameTotalCount;
    int64_t     mDecodeNoBitstreamTotalCount;
    int64_t     mDecodeOtherTotalCount;
    int64_t     mDecodeFrameSmallAverageDuration;
    int64_t     mDecodeFrameBigAverageDuration;
    int64_t     mDecodeNoFrameAverageDuration;
    int64_t     mDecodeNoBitstreamAverageDuration;
    int64_t     mConvertTotalDuration;
    int64_t     mConvertTotalCount;
    int64_t     mConvertAverageDuration;
	char        mCallingProcess[256];  //add by fuqiang for cts
	bool        mIsFromCts;            //add by fuqiang for cts
    OMX_BOOL    mVp9orH265SoftDecodeFlag;
    OMX_BOOL    mResolutionChangeFlag;
	
	//* for new display
    OMX_PTR                pMarkData;
    OMX_HANDLETYPE         hMarkTargetComponent;
    OMX_BOOL               bPortSettingMatchFlag;
    OMXOutputBufferInfoT   mOutputBufferInfo[OMX_MAX_VIDEO_BUFFER_NUM];
    OMXDisplayBufferNodeT* pDisplayerBufferListHead;
    OMXDisplayBufferNodeT  mDisplayBufferNode[OMX_MAX_VIDEO_BUFFER_NUM];
    OMXDisplayBufferNodeT  mFreeBufferNode[OMX_MAX_VIDEO_BUFFER_NUM];
    OMX_S32                mIonFd;
    OMX_BOOL               mIs4KAlignFlag;
    
    OMX_BOOL               mHadInitDecoderFlag;
    OMX_BOOL               mVideoSizeInfoValidFlag;
    FbmBufInfo             mVideoSizeInfo;
    
    OMX_CONFIG_RECTTYPE    mVideoRect;    //* for display crop
    OMX_S32                mPicNum; //* just for debug

    OMX_BOOL               mIsSecureVideoFlag;
    OMX_S32                mGpuAlignStride;

    OMX_U8                 mCodecSpecificData[CODEC_SPECIFIC_DATA_LENGTH];
    OMX_U32                mCodecSpecificDataLen;

    OMX_BOOL               mIsFlushingFlag;
    OMX_BOOL               mIsSoftwareDecoderFlag;

    OMX_BOOL               mInputEosFlag;
    
};

#endif // __OMX_VDEC_H__
