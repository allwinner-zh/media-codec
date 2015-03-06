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
#ifndef CONFIG_H
#define CONFIG_H

// option of conpile tool chain for linux makefile.
// arm-linux-gnueabihf- or arm-none-linux-gnueabi- tool chain
#define OPTION_CC_GNUEABIHF 1
#define OPTION_CC_GNUEABI   2

// option for os config.
#define OPTION_OS_ANDROID   1
#define OPTION_OS_LINUX     2

// option for os version config.
#define OPTION_OS_VERSION_ANDROID_4_2   1
#define OPTION_OS_VERSION_ANDROID_4_4   2
#define OPTION_OS_VERSION_ANDROID_5_0   3

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

// option for chip config.
#define OPTION_CHIP_1623        1
#define OPTION_CHIP_1625        2
#define OPTION_CHIP_1633        3
#define OPTION_CHIP_1651        4
#define OPTION_CHIP_1650        5
#define OPTION_CHIP_1661        6
#define OPTION_CHIP_1667        7
#define OPTION_CHIP_1639        8
#define OPTION_CHIP_1673        9
#define OPTION_CHIP_1680        10
#define OPTION_CHIP_1681        11
#define OPTION_CHIP_1689        12


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
#define CONFIG_OS_VERSION    OPTION_OS_VERSION_ANDROID_4_4
#define CONFIG_MEMORY_DRIVER    OPTION_MEMORY_DRIVER_VE
#define CONFIG_PRODUCT    OPTION_PRODUCT_TVBOX
#define CONFIG_CHIP    OPTION_CHIP_1651
#define CONFIG_DRAM_INTERFACE    OPTION_DRAM_INTERFACE_DDR3_32BITS
#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_WARNING
#endif

#define CONFIG_ENABLE_DEMUX_ASF    1
#define CONFIG_ENABLE_DEMUX_AVI    1
#define CONFIG_ENABLE_DEMUX_BLUERAYDISK    1
#define CONFIG_ENABLE_DEMUX_MPEGDASH    1
#define CONFIG_ENABLE_DEMUX_FLV    1
#define CONFIG_ENABLE_DEMUX_HLS    1
#define CONFIG_ENABLE_DEMUX_MKV    1
#define CONFIG_ENABLE_DEMUX_MMS    1
#define CONFIG_ENABLE_DEMUX_MOV    1
#define CONFIG_ENABLE_DEMUX_MPG    1
#define CONFIG_ENABLE_DEMUX_PMP    1
#define CONFIG_ENABLE_DEMUX_OGG    1
#define CONFIG_ENABLE_DEMUX_RX    1
#define CONFIG_ENABLE_DEMUX_TS    1

//* other global define
#if(CONFIG_CHIP == OPTION_CHIP_1667)
    #define USE_NEW_DISPLAY 1
#else
    #define USE_NEW_DISPLAY 0
#endif

#if(CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667)
    #define GPU_TYPE_MALI 1
#else
    #define GPU_TYPE_MALI 0
#endif

#if(USE_NEW_DISPLAY == 1) && (CONFIG_PRODUCT == OPTION_PRODUCT_PAD)
    #define DROP_3D_SECOND_VIDEO_STREAM 1
#else
    #define DROP_3D_SECOND_VIDEO_STREAM 0
#endif

#endif
