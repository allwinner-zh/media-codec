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

#ifndef _AWOMX_VIDEO_INDEX_EXTENSION_H_
#define _AWOMX_VIDEO_INDEX_EXTENSION_H_

/*========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <OMX_Core.h>

/*========================================================================

                      DEFINITIONS AND DECLARATIONS

========================================================================== */

#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */

/**
 * Enumeration used to define Allwinner's vendor extensions for
 * video. The video extensions occupy a range of
 * 0x7F100000-0x7F1FFFFF, inclusive.
 */

typedef enum AW_VIDEO_EXTENSIONS_INDEXTYPE
{
	AWOMX_IndexParamVideoEnableAndroidNativeBuffers    = 0x7F100000,	/* OMX.google.android.index.enableAndroidNativeBuffers */
	AWOMX_IndexParamVideoGetAndroidNativeBufferUsage   = 0x7F100001,    /* OMX.google.android.index.getAndroidNativeBufferUsage */
	AWOMX_IndexParamVideoUseAndroidNativeBuffer2       = 0x7F100002,    /* OMX.google.android.index.useAndroidNativeBuffer2 */
    AWOMX_IndexParamVideoUseStoreMetaDataInBuffer      = 0x7F100003,
    AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback = 0x7F100004,
    AWOMX_IndexParamVideoUnused                        = 0x7F2FFFFF
} AW_VIDEO_EXTENSIONS_INDEXTYPE;

#define VIDDEC_CUSTOMPARAM_ENABLE_ANDROID_NATIVE_BUFFER "OMX.google.android.index.enableAndroidNativeBuffers"
#define VIDDEC_CUSTOMPARAM_GET_ANDROID_NATIVE_BUFFER_USAGE "OMX.google.android.index.getAndroidNativeBufferUsage"
#define VIDDEC_CUSTOMPARAM_USE_ANDROID_NATIVE_BUFFER2 "OMX.google.android.index.useAndroidNativeBuffer2"
#define VIDDEC_CUSTOMPARAM_STORE_META_DATA_IN_BUFFER  "OMX.google.android.index.storeMetaDataInBuffers"
#define VIDDEC_CUSTOMPARAM_PREPARE_FOR_ADAPTIVE_PLAYBACK  "OMX.google.android.index.prepareForAdaptivePlayback"
#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro _AWOMX_VIDEO_INDEX_EXTENSION_H_ */
