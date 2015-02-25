
#ifndef LOG_H
#define LOG_H

#include "config.h"

#ifndef LOG_TAG
#define LOG_TAG "awplayer"
#endif

#if CONFIG_OS == OPTION_OS_ANDROID
    #include <cutils/log.h>

    #define LOG_LEVEL_ERROR     ANDROID_LOG_ERROR
    #define LOG_LEVEL_WARNING   ANDROID_LOG_WARN
    #define LOG_LEVEL_INFO      ANDROID_LOG_INFO
    #define LOG_LEVEL_VERBOSE   ANDROID_LOG_VERBOSE
    #define LOG_LEVEL_DEBUG     ANDROID_LOG_DEBUG
    
    #define AWLOG(level, fmt, arg...)  \
        LOG_PRI(level, LOG_TAG, "<%s:%u>: "fmt, strrchr(__FILE__, '/')+1, __LINE__, ##arg)
        
#elif CONFIG_OS == OPTION_OS_LINUX
    #include <stdio.h>
    #include <string.h>
    
    #define LOG_LEVEL_ERROR     "error  "
    #define LOG_LEVEL_WARNING   "warning"
    #define LOG_LEVEL_INFO      "info   "
    #define LOG_LEVEL_VERBOSE   "verbose"
    #define LOG_LEVEL_DEBUG     "debug  "
    
    #define AWLOG(level, fmt, arg...)  \
        printf("%s: %s <%s:%u>: "fmt"\n", level, LOG_TAG, __FILE__, __LINE__, ##arg)
#else
    #error "invalid configuration of os."
#endif

#define logd(fmt, arg...) AWLOG(LOG_LEVEL_DEBUG, fmt, ##arg)

#if CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_CLOSE

#define loge(fmt, arg...)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_ERROR

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m"fmt"\033[0m", ##arg)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_WARNING

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m"fmt"\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DEFAULT

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m"fmt"\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DETAIL

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m"fmt"\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logv(fmt, arg...) AWLOG(LOG_LEVEL_VERBOSE, fmt, ##arg)

#else
    #error "invalid configuration of debug level."
#endif

#define CEDARX_UNUSE(param) (void)param

#endif

