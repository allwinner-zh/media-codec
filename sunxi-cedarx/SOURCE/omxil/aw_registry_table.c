/*
* Cedarx framework.
* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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
/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

  This module contains the registry table for the QCOM's OpenMAX core.

*//*========================================================================*/


#include "aw_omx_core.h"

omx_core_cb_type core[] =
{
    //*1. mjpeg decoder
	{
		"OMX.allwinner.video.decoder.mjpeg",
		NULL,// Create instance function
		// Unique instance handle
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL, // Shared object library handle
		"libOmxVdec.so",
		{
			"video_decoder.mjpeg"
		}
	},
	
  

    //*2. mpeg1 decoder
    {
		"OMX.allwinner.video.decoder.mpeg1",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.mpeg1"
		}
	},

    //*3. mpeg2 decoder
    {
		"OMX.allwinner.video.decoder.mpeg2",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.mpeg2"
		}
	},

    //*4. mpeg4 decoder
	{
		"OMX.allwinner.video.decoder.mpeg4",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.mpeg4"
		}
	},

    //*5. msmpeg4v1 decoder
	{
		"OMX.allwinner.video.decoder.msmpeg4v1",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.msmpeg4v1"
		}
	},

    //*6. msmpeg4v2 decoder
	{
		"OMX.allwinner.video.decoder.msmpeg4v2",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.msmpeg4v2"
		}
	},

    //*7. divx decoder
	{
		"OMX.allwinner.video.decoder.divx",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.divx"
		}
	},

    //*8. xvid decoder
	{
		"OMX.allwinner.video.decoder.xvid",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.xvid"
		}
	},

    //*9. h263 decoder
    {
		"OMX.allwinner.video.decoder.h263",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.h263"
		}
	},

    //*10. h263 decoder
    {
		"OMX.allwinner.video.decoder.s263",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.s263"
		}
	},

    //*11. rxg2 decoder
    {
		"OMX.allwinner.video.decoder.rxg2",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.rxg2"
		}
	},

    //*12. wmv1 decoder
	{
		"OMX.allwinner.video.decoder.wmv1",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.wmv1"
		}
	},

    //*13. wmv2 decoder
	{
		"OMX.allwinner.video.decoder.wmv2",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.wmv2"
		}
	},

    //*14. vc1 decoder
	{
		"OMX.allwinner.video.decoder.vc1",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.vc1"
		}
	},
	
   

    //*15. vp6 decoder
	{
		"OMX.allwinner.video.decoder.vp6",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.vp6"
		}
	},
    
    //*16. vp8 decoder
    {
		"OMX.allwinner.video.decoder.vp8",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.vp8"
		}
	},

    //*17. vp9 decoder
    {
		"OMX.allwinner.video.decoder.vp9",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.vp9"
		}
	},

    //*18. avc decoder
	{
		"OMX.allwinner.video.decoder.avc",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.avc"
		}
	},
	
    //*19. avc decoder (secure)
    {
		"OMX.allwinner.video.decoder.avc.secure",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.avc.secure"
		}
	},

    //*20. h265 decoder
    {
		"OMX.allwinner.video.decoder.hevc",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVdec.so",
		{
			"video_decoder.hevc"
		}
	},
	//*20. divx3 decoder
	{
        "OMX.allwinner.video.decoder.divx3",
        NULL,
        {
            NULL,
            NULL,
            NULL,
            NULL
        },
        NULL,
        "libOmxVdec.so",
        {
            "video_decoder.divx3"
        }
    },
#if 0
    //*1. avc encoder
	{
		"OMX.allwinner.video.encoder.avc",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVenc.so",
		{
			"video_encode.avc"
		}
	},

    //*2. h263 encoder
	{
		"OMX.allwinner.video.encoder.h263",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVenc.so",
		{
			"video_encode.h263"
		}
	},

    //*3. mpeg4 encoder
	{
		"OMX.allwinner.video.encoder.mpeg4",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxVenc.so",
		{
			"video_encode.mpeg4"
		}
	},
	//audio dec
	  //*3. wma decoder
	{
		"OMX.allwinner.audio.decoder.wma",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.wma"
		}
	},
	 //*3. aac decoder
	{
		"OMX.allwinner.audio.decoder.aac",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.aac"
		}
	},
	 //*3. mp3 decoder
	{
		"OMX.allwinner.audio.decoder.mp3",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.mp3"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.amrnb",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.amrnb"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.amrwb",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.amrwb"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.g711.alaw",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.g711alaw"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.g711.mlaw",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.g711mlaw"
		}
	},
		{
		"OMX.allwinner.audio.decoder.adpcm",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.adpcm"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.vorbis",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.ogg"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.ape",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.ape"
		}
	},
	{
		"OMX.allwinner.audio.decoder.ac3",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.ac3"
		}
	},
	{
		"OMX.allwinner.audio.decoder.dts",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.dts"
		}
	},
	
	 //
	{
		"OMX.allwinner.audio.decoder.flac",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.flac"
		}
	},
	 //
	{
		"OMX.allwinner.audio.decoder.raw",
		NULL,
		{
			NULL,
			NULL,
			NULL,
			NULL
		},
		NULL,
		"libOmxAdec.so",
		{
			"audio_decoder.raw"
		}
	},
#endif	
	
};

const unsigned int SIZE_OF_CORE = sizeof(core) / sizeof(omx_core_cb_type);


