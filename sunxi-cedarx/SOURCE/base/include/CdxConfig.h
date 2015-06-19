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
#ifndef CDX_CONFIG_H
#define CDX_CONFIG_H

// option of conpile tool chain for linux makefile.
// arm-linux-gnueabihf- or arm-none-linux-gnueabi- tool chain
#define OPTION_CC_GNUEABIHF 1
#define OPTION_CC_GNUEABI   2

// option for os config.
#define OPTION_OS_ANDROID   1
#define OPTION_OS_LINUX     2

// option for momory driver config.
#define OPTION_MEMORY_DRIVER_SUNXIMEM   1
#define OPTION_MEMORY_DRIVER_ION        2
#define OPTION_MEMORY_DRIVER_ION_LINUX_3_10   3
#define OPTION_MEMORY_DRIVER_VE         4

// option for product config.
#define OPTION_PRODUCT_PAD      1
#define OPTION_PRODUCT_TVBOX    2
#define OPTION_PRODUCT_OTT_CMCC 3
#define OPTION_PRODUCT_IPTV     4
#define OPTION_PRODUCT_DVB      5

// option for dram interface.
#define OPTION_DRAM_INTERFACE_DDR1_16BITS   1
#define OPTION_DRAM_INTERFACE_DDR1_32BITS   2
#define OPTION_DRAM_INTERFACE_DDR2_16BITS   3
#define OPTION_DRAM_INTERFACE_DDR2_32BITS   4
#define OPTION_DRAM_INTERFACE_DDR3_16BITS   5
#define OPTION_DRAM_INTERFACE_DDR3_32BITS   6
#define OPTION_DRAM_INTERFACE_DDR3_64BITS   7

// option for debug level.
#define OPTION_LOG_LEVEL_CLOSE      0
#define OPTION_LOG_LEVEL_ERROR      1
#define OPTION_LOG_LEVEL_WARNING    2
#define OPTION_LOG_LEVEL_DEFAULT    3
#define OPTION_LOG_LEVEL_DETAIL     4

// configuration.
#define CONFIG_CC    OPTION_CC_GNUEABIHF
#define CONFIG_OS    OPTION_OS_LINUX
#define CONFIG_MEMORY_DRIVER    OPTION_MEMORY_DRIVER_VE
#define CONFIG_DRAM_INTERFACE    OPTION_DRAM_INTERFACE_DDR3_32BITS

#ifdef CONFIG_LOG_LEVEL
#undef CONFIG_LOG_LEVEL
#endif
#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_WARNING
//#endif


#endif
