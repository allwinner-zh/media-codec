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
#ifndef CDX_LOG_H
#define CDX_LOG_H

#include "CdxConfig.h"
#include "log.h"

#include <CdxTypes.h>

#define CDX_LOGV(fmt, arg...) logv(fmt, ##arg)
#define CDX_LOGD(fmt, arg...) logd(fmt, ##arg)
#define CDX_LOGI(fmt, arg...) logi(fmt, ##arg)
#define CDX_LOGW(fmt, arg...) logw(fmt, ##arg)
#define CDX_LOGE(fmt, arg...) loge(fmt, ##arg)

#define CDX_TRACE() \
    CDX_LOGI("<%s:%u> tid(%d)", strrchr(__FILE__, '/') + 1, __LINE__, gettid())

#if CONFIG_OS == OPTION_OS_ANDROID 
/*check when realease version*/
#define CDX_ASSERT(e) \
        LOG_ALWAYS_FATAL_IF(                        \
                !(e),                               \
                "<%s:%d> assert (%s) failed.",     \
                strrchr(__FILE__, '/') + 1, __LINE__, #e)      \

#define CDX_LOG_ASSERT(e, fmt, arg...)                           \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>check (%s) failed:"fmt,                     \
            strrchr(__FILE__, '/') + 1, __LINE__, #e, ##arg)    \
 
#elif CONFIG_OS == OPTION_OS_LINUX

#include <assert.h>

#define CDX_LOG_ASSERT(e, fmt, arg...)                           \
    do {                                                        \
        if (!(e))                                                 \
        {                                                       \
            CDX_LOGE("check (%s) failed:"fmt, #e, ##arg);       \
            assert(0);                                          \
        }                                                       \
    } while (0)


#define CDX_ASSERT(e)                                            \
    do {                                                        \
        if (!(e))                                                 \
        {                                                       \
            CDX_LOGE("check (%s) failed.", #e);                 \
            assert(0);                                          \
        }                                                       \
    } while (0)

#else
    #error "invalid configuration of os."
#endif


#define CDX_BUF_DUMP(buf, len) \
    do { \
        char *_buf = (char *)buf;\
        char str[1024] = {0};\
        unsigned int index = 0, _len;\
        _len = (unsigned int)len;\
        snprintf(str, 1024, ":%d:[", _len);\
        for (index = 0; index < _len; index++)\
        {\
            snprintf(str + strlen(str), 1024 - strlen(str), "%02hhx ", _buf[index]);\
        }\
        str[strlen(str) - 1] = ']';\
        CDX_LOGD("%s", str);\
    }while (0)
	
#define CDX_ITF_CHECK(base, itf)    \
    CDX_ASSERT(base);                \
    CDX_ASSERT(base->ops);           \
    CDX_ASSERT(base->ops->itf)

#define CDX_UNUSE(param) (void)param
#endif
