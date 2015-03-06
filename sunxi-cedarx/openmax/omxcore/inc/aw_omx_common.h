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

*//** @file aw_omx_common.h
  This module contains the definitions of the OpenMAX core.

*//*========================================================================*/

#ifndef AW_OMX_COMMON_H
#define AW_OMX_COMMON_H

#include <stdio.h>           // Standard IO
#include "OMX_Core.h"        // OMX API

#define OMX_CORE_MAX_CMP                1 // MAX Components supported
#define OMX_CORE_MAX_CMP_ROLES          1 // MAX Roles per component
#define OMX_SPEC_VERSION       0x00000101 // OMX Version

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*create_aw_omx_component)(void);


#ifdef __cplusplus
}
#endif

#endif

