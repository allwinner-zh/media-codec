
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

