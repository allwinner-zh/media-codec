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
#ifndef CDX_VERSION_H
#define CDX_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAJOR_VERSION "2.1"
#define LIBRARY_SVN_REPOSITORY ""
#define LIBRARY_SVN_VERSION ""
#define LIBRARY_SVN_DATE ""
#define LIBRARY_RELEASE_AUTHOR "xxx"

static inline void LogVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> CedarX 2.0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" 
         "version: %s.%s\n"
         "svn repo:'%s'\n"
         "last change date: %s\n"
         "author: %s\n"
         "----------------------------------------------------------------------\n",
         MAJOR_VERSION, LIBRARY_SVN_VERSION,
         LIBRARY_SVN_REPOSITORY,
         LIBRARY_SVN_DATE,
         LIBRARY_RELEASE_AUTHOR);
}

/* usage: TagVersionInfo(myLibTag) */
#define TagVersionInfo(tag) \
    static void VersionInfo_##tag(void) __attribute__((constructor));\
    void VersionInfo_##tag(void) \
    { \
        logd("-------library tag: %s-------", #tag);\
        LogVersionInfo(); \
    }


#ifdef __cplusplus
}
#endif

#endif

