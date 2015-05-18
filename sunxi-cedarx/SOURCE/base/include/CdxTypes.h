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
#ifndef CDX_TYPES_H
#define CDX_TYPES_H

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define CDX_SUCCESS 0
#define CDX_FAILURE (-1)

typedef long long cdx_int64;
typedef unsigned long long cdx_uint64;

typedef int cdx_int32;
typedef unsigned int cdx_uint32;

typedef short cdx_int16;
typedef unsigned short cdx_uint16;

typedef unsigned char cdx_uint8;
typedef char cdx_int8;

typedef unsigned long cdx_ulong;
typedef long cdx_long;

typedef char cdx_char;
typedef int cdx_bool;
typedef void cdx_void;
typedef unsigned long cdx_size;
typedef signed long cdx_ssize;

typedef float cdx_float;

typedef struct CdxListNodeS CdxListNodeT;
typedef struct CdxListS CdxListT;

#ifdef AWP_DEBUG
#define CDX_INTERFACE
#else
#define CDX_INTERFACE static inline
#endif

typedef cdx_int32 cdx_err;

#define CDX_TRUE 1
#define CDX_FALSE 0

#define CdxOffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER) 

#define CdxContainerOf(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - CdxOffsetof(type,member) ); })


enum CdxMediaTypeE
{
    CDX_MEDIA_UNKNOWN = -1,
    CDX_MEDIA_VIDEO = 0,
    CDX_MEDIA_AUDIO,
    CDX_MEDIA_SUBTITLE,
    CDX_MEDIA_DATA,
};

#endif
