#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _VENCODER_H_
#define _VENCODER_H_

#define  DATA_TIME_LENGTH 24
#define  INFO_LENGTH 64
#define  GPS_PROCESS_METHOD_LENGTH 100


#define VENC_BUFFERFLAG_KEYFRAME 0x00000001
#define VENC_BUFFERFLAG_EOS 0x00000002 


typedef struct rational_t
{
	unsigned int num;
	unsigned int den;
}rational_t;

typedef struct srational_t
{
	int num;
	int den;
}srational_t;

typedef enum ExifMeteringModeType{
    EXIF_METERING_UNKNOWN,
    EXIF_METERING_AVERAGE,
    EXIF_METERING_CENTER,
    EXIF_METERING_SPOT,
    EXIF_METERING_MULTISPOT,
    EXIF_METERING_PATTERN,
    EXIF_METERING_PARTIAL,
    EXIF_METERING_OTHER     = 255,
} ExifMeteringModeType;

typedef enum ExifExposureModeType
{
    EXIF_EXPOSURE_AUTO,
    EXIF_EXPOSURE_MANUAL,
    EXIF_EXPOSURE_AUTO_BRACKET,
}ExifExposureModeType;

typedef struct EXIFInfo
{
    unsigned char  CameraMake[INFO_LENGTH];
    unsigned char  CameraModel[INFO_LENGTH];
    unsigned char  DateTime[DATA_TIME_LENGTH];

	unsigned int   ThumbWidth;
	unsigned int   ThumbHeight;

    int   		   Orientation;  //value can be 0,90,180,270 degree
	rational_t	   ExposureTime; //tag 0x829A
	rational_t	   FNumber; //tag 0x829D
	short		   ISOSpeed;//tag 0x8827

	srational_t    ShutterSpeedValue; //tag 0x9201
	//srational_t    BrightnessValue;   //tag 0x9203
	srational_t    ExposureBiasValue; //tag 0x9204

	short		   MeteringMode; //tag 0x9207
	short		   FlashUsed; 	//tag 0x9209
	rational_t	   FocalLength; //tag 0x920A

	rational_t	   DigitalZoomRatio; // tag A404
	short		   WhiteBalance; //tag 0xA403
	short		   ExposureMode; //tag 0xA402

	// gps info
	int            enableGpsInfo;
	double         gps_latitude;
	double		   gps_longitude;
	double         gps_altitude;  
	long           gps_timestamp;
	unsigned char  gpsProcessingMethod[GPS_PROCESS_METHOD_LENGTH];
}EXIFInfo;

typedef struct VencRect
{
	int nLeft;
	int nTop;
	int nWidth;
	int nHeight;
}VencRect;


typedef enum VENC_COLOR_SPACE
{
    VENC_BT601,
	VENC_BT709,
	VENC_YCC,
}VENC_COLOR_SPACE;

typedef enum VENC_YUV2YUV
{
    VENC_YCCToBT601,
	VENC_BT601ToYCC,
}VENC_YUV2YUV;


typedef enum VENC_CODING_MODE
{
  VENC_FRAME_CODING         = 0,
  VENC_FIELD_CODING         = 1,
}VENC_CODING_MODE;


typedef enum VENC_CODEC_TYPE
{
	VENC_CODEC_H264,
	VENC_CODEC_JPEG,
	VENC_CODEC_VP8,
}VENC_CODEC_TYPE;


typedef enum VENC_PIXEL_FMT
{
	VENC_PIXEL_YUV420SP,
    VENC_PIXEL_YVU420SP,
    VENC_PIXEL_YUV420P,
    VENC_PIXEL_YVU420P,
	VENC_PIXEL_YUV422SP,
    VENC_PIXEL_YVU422SP,
    VENC_PIXEL_YUV422P,
    VENC_PIXEL_YVU422P,
	VENC_PIXEL_YUYV422,
	VENC_PIXEL_UYVY422,
	VENC_PIXEL_YVYU422,
	VENC_PIXEL_VYUY422,
	VENC_PIXEL_ARGB,
    VENC_PIXEL_RGBA,
    VENC_PIXEL_ABGR,
    VENC_PIXEL_BGRA,
    VENC_PIXEL_TILE_32X32,
    VENC_PIXEL_TILE_128X32,
}VENC_PIXEL_FMT;


typedef struct VencBaseConfig
{
	unsigned int		nInputWidth;
	unsigned int		nInputHeight;
	unsigned int		nDstWidth;
	unsigned int		nDstHeight;
	unsigned int        nStride;
	VENC_PIXEL_FMT  	eInputFormat;
}VencBaseConfig;


/** 
 * H264 profile types
 */
typedef enum VENC_H264PROFILETYPE
{
    VENC_H264ProfileBaseline  = 66,         /**< Baseline profile */
    VENC_H264ProfileMain      = 77,         /**< Main profile */
    VENC_H264ProfileHigh      = 100,   		/**< High profile */
}VENC_H264PROFILETYPE;


/** 
 * H264 level types
 */
typedef enum VENC_H264LEVELTYPE
{
    VENC_H264Level1   = 10,     /**< Level 1 */
    VENC_H264Level11  = 11,     /**< Level 1.1 */
    VENC_H264Level12  = 12,     /**< Level 1.2 */
    VENC_H264Level13  = 13,     /**< Level 1.3 */
    VENC_H264Level2   = 12,     /**< Level 2 */
    VENC_H264Level21  = 21,     /**< Level 2.1 */
    VENC_H264Level22  = 22,     /**< Level 2.2 */
    VENC_H264Level3   = 30,     /**< Level 3 */
    VENC_H264Level31  = 31,     /**< Level 3.1 */
    VENC_H264Level32  = 32,     /**< Level 3.2 */
    VENC_H264Level4   = 40,     /**< Level 4 */
    VENC_H264Level41  = 41,     /**< Level 4.1 */
    VENC_H264Level42  = 42,     /**< Level 4.2 */
    VENC_H264Level5   = 50,     /**< Level 5 */
    VENC_H264Level51  = 51,     /**< Level 5.1 */
}VENC_H264LEVELTYPE;

typedef struct VencH264ProfileLevel
{
	VENC_H264PROFILETYPE	nProfile;
	VENC_H264LEVELTYPE		nLevel;
}VencH264ProfileLevel;

typedef struct VencQPRange
{
	int	nMaxqp;
	int	nMinqp;
}VencQPRange;

typedef struct MotionParam
{
	int nMotionDetectEnable;
	int nMotionDetectRatio; /* 0~12, 0 is the best sensitive */
}MotionParam;

typedef struct VencHeaderData
{
	unsigned char*  pBuffer;
	unsigned int	nLength;
}VencHeaderData;

typedef struct VencInputBuffer
{
	int			   nID;
	long long  	   nPts;
	unsigned int   nFlag;
	unsigned char* pAddrPhyY;
	unsigned char* pAddrPhyC;
	unsigned char* pAddrVirY;
	unsigned char* pAddrVirC;
	int 		   bEnableCorp;
	VencRect       sCropInfo;
}VencInputBuffer;

typedef struct VencOutputBuffer
{
	int			   nID;
	long long  	   nPts;
	unsigned int   nFlag;
	unsigned int   nSize0;
	unsigned int   nSize1;
	unsigned char* pData0;
	unsigned char* pData1;
}VencOutputBuffer;

typedef struct VencAllocateBufferParam
{
    unsigned int   nBufferNum;
	unsigned int   nSizeY;
	unsigned int   nSizeC;
}VencAllocateBufferParam;


typedef struct VencH264FixQP
{
	int                     bEnable;
	int                     nIQp;
	int 					nPQp;
}VencH264FixQP;

typedef struct VencCyclicIntraRefresh
{
	int                     bEnable;
	int                     nBlockNumber;
}VencCyclicIntraRefresh;


typedef struct VencSize
{
	int                     nWidth;
	int                     nHeight;
}VencSize;


typedef struct VencH264Param
{
	VencH264ProfileLevel	sProfileLevel;
	int                 	bEntropyCodingCABAC; /* 0:CAVLC 1:CABAC*/
	VencQPRange   			sQPRange;
	int                     nFramerate; /* fps*/
	int                     nBitrate;   /* bps*/
	int                     nMaxKeyInterval;
	VENC_CODING_MODE        nCodingMode;
}VencH264Param;


/* support 4 ROI region */
typedef struct VencROIConfig
{
	int                     bEnable;
	int                		index; /* (0~3) */
	int                     nQPoffset;
	VencRect                sRect;
}VencROIConfig;


typedef struct VencCheckColorFormat
{
	int                		index;
	VENC_PIXEL_FMT          eColorFormat;
}VencCheckColorFormat;


typedef struct VencVP8Param
{
	int                     nFramerate; /* fps*/
	int                     nBitrate;   /* bps*/
	int                     nMaxKeyInterval;
}VencVP8Param;


typedef enum VENC_INDEXTYPE
{
	VENC_IndexParamBitrate				= 0x0,		/**< reference type: int */
	VENC_IndexParamFramerate,                		/**< reference type: int */
	VENC_IndexParamMaxKeyInterval,           		/**< reference type: int */
	VENC_IndexParamIfilter,                  		/**< reference type: int */            
	VENC_IndexParamRotation,                 		/**< reference type: int */
	VENC_IndexParamSliceHeight,              		/**< reference type: int */
	VENC_IndexParamForceKeyFrame,            		/**< reference type: int (write only)*/
	VENC_IndexParamMotionDetectEnable,             	/**< reference type: MotionParam(write only) */
	VENC_IndexParamMotionDetectStatus,             	/**< reference type: int(read only) */
	VENC_IndexParamRgb2Yuv,             			/**< reference type: VENC_COLOR_SPACE */
	VENC_IndexParamYuv2Yuv,             			/**< reference type: VENC_YUV2YUV */
	VENC_IndexParamROIConfig,						/**< reference type: VencROIConfig */
	VENC_IndexParamStride,              		    /**< reference type: int */
	VENC_IndexParamColorFormat,                     /**< reference type: VENC_PIXEL_FMT */
	VENC_IndexParamSize,                     		/**< reference type: VencSize(read only) */

	/* check capabiliy */
	VENC_IndexParamMAXSupportSize,					/**< reference type: VencSize(read only) */
	VENC_IndexParamCheckColorFormat,				/**< reference type: VencCheckFormat(read only) */

	/* H264 param */
	VENC_IndexParamH264Param,						/**< reference type: VencH264Param */
	VENC_IndexParamH264SPSPPS,                    	/**< reference type: VencHeaderData (read only)*/
	VENC_IndexParamH264QPRange			= 0x100,	/**< reference type: VencQPRange */
	VENC_IndexParamH264ProfileLevel,              	/**< reference type: VencProfileLevel */
	VENC_IndexParamH264EntropyCodingCABAC,			/**< reference type: int(0:CAVLC 1:CABAC) */
	VENC_IndexParamH264CyclicIntraRefresh,			/**< reference type: VencCyclicIntraRefresh */
	VENC_IndexParamH264FixQP,						/**< reference type: VencH264FixQP */
	

	/* jpeg param */
	VENC_IndexParamJpegQuality			= 0x200,	/**< reference type: int (1~100) */
	VENC_IndexParamJpegExifInfo,                  	/**< reference type: EXIFInfo */

	/* VP8 param */
	VENC_IndexParamVP8Param,	

}VENC_INDEXTYPE;

typedef enum VENC_RESULT_TYPE
{
    VENC_RESULT_ERROR             = -1,
    VENC_RESULT_OK             	  = 0,
    VENC_RESULT_NO_FRAME_BUFFER   = 1,
    VENC_RESULT_BITSTREAM_IS_FULL = 2,
}VENC_RESULT_TYPE;


typedef struct JpegEncInfo
{
	VencBaseConfig  sBaseInfo;
	int             bNoUseAddrPhy;
	unsigned char*  pAddrPhyY;
	unsigned char*  pAddrPhyC;
	unsigned char*  pAddrVirY;
	unsigned char*  pAddrVirC;
	int 		    bEnableCorp;
	VencRect        sCropInfo;
	int				quality;
}JpegEncInfo;

int AWJpecEnc(JpegEncInfo* pJpegInfo, EXIFInfo* pExifInfo, void* pOutBuffer, int* pOutBufferSize);


typedef void* VideoEncoder;

VideoEncoder* VideoEncCreate(VENC_CODEC_TYPE eCodecType);
void VideoEncDestroy(VideoEncoder* pEncoder);
int VideoEncInit(VideoEncoder* pEncoder, VencBaseConfig* pConfig);
int VideoEncUnInit(VideoEncoder* pEncoder);

int AllocInputBuffer(VideoEncoder* pEncoder, VencAllocateBufferParam *pBufferParam);
int GetOneAllocInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
int FlushCacheAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer);
int ReturnOneAllocInputBuffer(VideoEncoder* pEncoder,  VencInputBuffer *pInputbuffer);
int ReleaseAllocInputBuffer(VideoEncoder* pEncoder);


int AddOneInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pInputbuffer);
int VideoEncodeOneFrame(VideoEncoder* pEncoder);
int AlreadyUsedInputBuffer(VideoEncoder* pEncoder, VencInputBuffer* pBuffer);

int ValidBitstreamFrameNum(VideoEncoder* pEncoder);
int GetOneBitstreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);
int FreeOneBitStreamFrame(VideoEncoder* pEncoder, VencOutputBuffer* pBuffer);

int VideoEncGetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);
int VideoEncSetParameter(VideoEncoder* pEncoder, VENC_INDEXTYPE indexType, void* paramData);

#endif	//_VENCODER_H_

#ifdef __cplusplus
}
#endif /* __cplusplus */
