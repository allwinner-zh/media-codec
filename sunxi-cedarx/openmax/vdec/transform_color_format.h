
#ifndef TRANSFORM_COLOR_FORMAT_H
#define TRANSFORM_COLOR_FORMAT_H

//#include "libcedarv.h"

#include <vdecoder.h>

#ifdef __cplusplus
extern "C" {
#endif

void TransformMB32ToYV12(VideoPicture* pict, void* ybuf);
void TransformYV12ToYUV420(VideoPicture* pict, void* ybuf);
void TransformYV12ToYUV420Soft(VideoPicture* pict, void* ybuf);
void TransformYV12ToYV12Hw(VideoPicture* pict, void* ybuf);
void TransformYV12ToYV12Soft(VideoPicture* pict, void* ybuf);

#ifdef __cplusplus
}
#endif

#endif

