/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <log.h>
#include <II420ColorConverter.h>
#include <OMX_IVCommon.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int getDecoderOutputFormat() {
    return OMX_COLOR_FormatYUV420Planar;
}

static int convertDecoderOutputToI420(
    void* srcBits, int srcWidth, int srcHeight, ARect srcRect, void* dstBits) {

	CEDARX_UNUSE(srcBits);
	CEDARX_UNUSE(srcWidth);
	CEDARX_UNUSE(srcHeight);
	CEDARX_UNUSE(srcRect);
	CEDARX_UNUSE(dstBits);
    return 0;
}

static int getEncoderInputFormat() {
    return OMX_COLOR_FormatYUV420SemiPlanar;
}

static int convertI420ToEncoderInput(
    void* srcBits, int srcWidth, int srcHeight,
    int dstWidth, int dstHeight, ARect dstRect,
    void* dstBits) {

	CEDARX_UNUSE(dstRect);
	
    uint8_t *pSrc_y = (uint8_t*) srcBits;
    uint8_t *pDst_y = (uint8_t*) dstBits;
    for(int i=0; i < srcHeight; i++) {
        memcpy(pDst_y, pSrc_y, srcWidth);
        pSrc_y += srcWidth;
        pDst_y += dstWidth;
    }
    uint8_t* pSrc_u = (uint8_t*)srcBits + (srcWidth * srcHeight);
    uint8_t* pSrc_v = (uint8_t*)pSrc_u + (srcWidth / 2) * (srcHeight / 2);
    uint8_t* pDst_uv  = (uint8_t*)dstBits + dstWidth * dstHeight;
	
    for(int i=0; i < srcHeight / 2; i++) {
        for(int j=0, k=0; j < srcWidth / 2; j++, k+=2) {
            pDst_uv[k] = pSrc_v[j];
			pDst_uv[k+1] = pSrc_u[j]; //convert I420 to NV21
        }
        pDst_uv += dstWidth;
        pSrc_u += srcWidth / 2;
        pSrc_v += srcWidth / 2;
    }

    return 0;
}

static int getEncoderInputBufferInfo(
    int actualWidth, int actualHeight,
    int* encoderWidth, int* encoderHeight,
    ARect* encoderRect, int* encoderBufferSize) {

    *encoderWidth = actualWidth;
    *encoderHeight = actualHeight;
    encoderRect->left = 0;
    encoderRect->top = 0;
    encoderRect->right = actualWidth - 1;
    encoderRect->bottom = actualHeight - 1;
    *encoderBufferSize = (actualWidth * actualHeight * 3 / 2);

    return 0;
}

extern "C" void getI420ColorConverter(II420ColorConverter *converter) {
    converter->getDecoderOutputFormat = getDecoderOutputFormat;
    converter->convertDecoderOutputToI420 = convertDecoderOutputToI420;
    converter->getEncoderInputFormat = getEncoderInputFormat;
    converter->convertI420ToEncoderInput = convertI420ToEncoderInput;
    converter->getEncoderInputBufferInfo = getEncoderInputBufferInfo;
}
