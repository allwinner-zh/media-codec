/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd. 
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 * 
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from 
 * Softwinner Products. 
 *
 * The entire notice above must be reproduced on all copies 
 * and should not be removed. 
 */
 

#include <stdlib.h>
#include <string.h>
#include "venc_device.h"

#include <CdxList.h>
#include <CdxLog.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

struct VEncoderNodeS
{
    CdxListNodeT node;
    VENC_DEVICE *device;
	VENC_CODEC_TYPE type;
    char desc[64]; /* specific by mime type */
    
};

struct VEncoderListS
{
    CdxListT list;
    int size;
    pthread_mutex_t mutex;
};

static struct VEncoderListS gVEncoderList = {{NULL, NULL}, 0, PTHREAD_MUTEX_INITIALIZER};

int VEncoderRegister(VENC_CODEC_TYPE type, char *desc, VENC_DEVICE *device)
{
    struct VEncoderNodeS *newVEncNode = NULL, *posVEncNode = NULL;
    
    CDX_ASSERT(desc);
    if (strlen(desc) > 63)
    {
        loge("type name '%s' too long", desc);
        return -1;
    }
    
    if (gVEncoderList.size == 0)
    {
        CdxListInit(&gVEncoderList.list);
        pthread_mutex_init(&gVEncoderList.mutex, NULL);
    }

    pthread_mutex_lock(&gVEncoderList.mutex);

    /* check if conflict */
    CdxListForEachEntry(newVEncNode, &gVEncoderList.list, node)
    {
        if (newVEncNode->type == type)
        {
            loge("Add '%x:%s' fail! '%x:%s' already register!", type, desc, type, posVEncNode->desc);
            return -1;
        }
    }

    newVEncNode = malloc(sizeof(*newVEncNode));
    newVEncNode->device = device;
    strncpy(newVEncNode->desc, desc, 63);
    newVEncNode->type = type;

    CdxListAdd(&newVEncNode->node, &gVEncoderList.list);
    gVEncoderList.size++;
    
    pthread_mutex_unlock(&gVEncoderList.mutex);
    logw("register encoder: '%x:%s' success.", type, desc);
    return 0;
}


VENC_DEVICE *VencoderDeviceCreate(VENC_CODEC_TYPE type)
{
	VENC_DEVICE *vencoder_device_handle = NULL;

    struct VEncoderNodeS *posVEncNode = NULL;

    CdxListForEachEntry(posVEncNode, &gVEncoderList.list, node)
    {
        if (posVEncNode->type == type)
        {
            logi("Create encoder '%x:%s'", posVEncNode->type, posVEncNode->desc);
            vencoder_device_handle = posVEncNode->device;
            return vencoder_device_handle;
        }
    }

    loge("format '%x' support!", type);

	return vencoder_device_handle;	
}

void VencoderDeviceDestroy(void *handle)
{
    (void)handle;
	return;
}

typedef void VEPluginEntry(void);

void AddVEncPluginSingle(char *lib)
{
    void *libFd = NULL;
    CDX_ASSERT(lib);
    libFd = dlopen(lib, RTLD_NOW);

    VEPluginEntry *PluginInit = NULL;
    
    if (libFd == NULL)
    {
        loge("dlopen '%s' fail: %s", lib, dlerror());
        return ;
    }

    PluginInit = dlsym(libFd, "CedarPluginVEncInit");
    if (PluginInit == NULL)
    {
        logw("Invalid plugin, CedarPluginVEInit not found.");
        return;
    }

    PluginInit(); /* init plugin */
    return ;
}


static int GetLocalPathFromProcessMaps(char *localPath, int len)
{
#define LOCAL_LIB "libcedar_vencoder.so"
    char path[512] = {0};
    char line[1024] = {0};
    FILE *file = NULL;
    char *strLibPos = NULL;
    int ret = -1;
    
    memset(localPath, 0x00, len);
    
    sprintf(path, "/proc/%d/maps", getpid());
    file = fopen(path, "r");
    if (file == NULL)
    {
        loge("fopen failure, errno(%d)", errno);
        ret = -1;
        goto out;
    }
    
    while (fgets(line, 1023, file) != NULL)
    {
        if ((strLibPos = strstr(line, LOCAL_LIB)) != NULL)
        {
            char *rootPathPos = NULL;
            int localPathLen = 0;
            rootPathPos = strchr(line, '/');
            if (rootPathPos == NULL)
            {
                loge("some thing error, cur line '%s'", line);
                ret = -1;
                goto out;
            }

            localPathLen = strLibPos - rootPathPos - 1;
            if (localPathLen > len -1)
            {
                loge("localPath too long :%s ", localPath);
                ret = -1;
                goto out;
            }
            
            memcpy(localPath, rootPathPos, localPathLen);
            ret = 0;
            goto out;
        }
    }
    loge("Are you kidding? not found?");

out:
    if (file)
    {
        fclose(file);
    }
    return ret;
}

/* executive when load */
static void AddVEncPlugin(void) __attribute__((constructor));
void AddVEncPlugin(void)
{
    char localPath[512] = {0};
    int ret;

//scan_local_path:
    ret = GetLocalPathFromProcessMaps(localPath, 512);
    if (ret != 0)
    {
        logw("get local path failure, scan /system/lib ");
        goto scan_system_lib;
    }

    struct dirent **namelist = NULL;
    int num = 0, index = 0;
    num = scandir(localPath, &namelist, NULL, NULL);
    if (num <= 0)
    {
        logw("scandir failure, errno(%d), scan /system/lib ", errno);
        goto scan_system_lib;
    }
    
    for (index = 0; index < num; index++)
    {
        if ((strstr(namelist[index]->d_name, "libcedar_plugin_venc") != NULL)
            && (strstr(namelist[index]->d_name, ".so") != NULL))
        {
            AddVEncPluginSingle(namelist[index]->d_name);
        }
        free(namelist[index]);
        namelist[index] = NULL;
    }

scan_system_lib:
    // TODO: scan /system/lib 
    
    return;
}


