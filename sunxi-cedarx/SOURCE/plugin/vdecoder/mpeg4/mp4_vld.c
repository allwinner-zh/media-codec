/*
* Cedarx framework.
* Copyright (c) 2008-2015 Allwinner Technology Co. Ltd.
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


#include "mpeg4.h"
extern tab_type tableB16_1[112];
extern tab_type tableB16_2[96];
extern tab_type tableB16_3[120];
extern tab_type tableB17_1[112];
extern tab_type tableB17_2[96];
extern tab_type tableB17_3[120];
extern tab_type tableI2_1[112];
extern tab_type tableI2_2[96];
extern tab_type tableI2_3[120];
extern tab_type MVtab0[14];
extern tab_type MVtab1[96] ;
extern tab_type MVtab2[124];

extern void mp4_flushbits (MP4_STREAM * ld, int32_t n);



/* Table B-19 -- ESCL(a), LMAX values of intra macroblocks */
static __inline int vldTableB19(int last, int run) {
	if (!last){ /* LAST == 0 */
		if        (run ==  0) {
			return 27;
		} else if (run ==  1) {
			return 10;
		} else if (run ==  2) {
			return  5;
		} else if (run ==  3) {
			return  4;
		} else if (run <=  7) {
			return  3;
		} else if (run <=  9) {
			return  2;
		} else if (run <= 14) {
			return  1;
		} else { /* illegal? */
			return  0; 
		}
	} else {    /* LAST == 1 */
		if        (run ==  0) {
			return  8;
		} else if (run ==  1) {
			return  3;
		} else if (run <=  6) {
			return  2;
		} else if (run <= 20) {
			return  1;
		} else { /* illegal? */
			return  0; 
		}		
	}
}

/* Table B-20 -- ESCL(b), LMAX values of inter macroblocks */
static __inline int vldTableB20(int last, int run) {
	if (!last){ /* LAST == 0 */
		if        (run ==  0) {
			return 12;
		} else if (run ==  1) {
			return  6;
		} else if (run ==  2) {
			return  4;
		} else if (run <=  6) {
			return  3;
		} else if (run <= 10) {
			return  2;
		} else if (run <= 26) {
			return  1;
		} else { /* illegal? */
			return  0; 
		}
	} else {    /* LAST == 1 */
		if        (run ==  0) {
			return  3;
		} else if (run ==  1) {
			return  2;
		} else if (run <= 40) {
			return  1;
		} else { /* illegal? */
			return  0; 
		}		
	}
}

/* Table B-21 -- ESCR(a), RMAX values of intra macroblocks */
static __inline int vldTableB21(int last, int level) {
	if (!last){ /* LAST == 0 */
		if        (level ==  1) {
			return 14;
		} else if (level ==  2) {
			return  9;
		} else if (level ==  3) {
			return  7;
		} else if (level ==  4) {
			return  3;
		} else if (level ==  5) {
			return  2;
		} else if (level <= 10) {
			return  1;
		} else if (level <= 27) {
			return  0;
		} else { /* illegal? */
			return  0; 
		}
	} else {    /* LAST == 1 */
		if        (level ==  1) {
			return  20;
		} else if (level ==  2) {
			return  6;
		} else if (level ==  3) {
			return  1;
		} else if (level <=  8) {
			return  0;
		} else { /* illegal? */
			return  0; 
		}		
	}
}

/* Table B-22 -- ESCR(b), RMAX values of inter macroblocks */
static __inline int vldTableB22(int last, int level) {
	if (!last){ /* LAST == 0 */
		if        (level ==  1) {
			return 26;
		} else if (level ==  2) {
			return 10;
		} else if (level ==  3) {
			return  6;
		} else if (level ==  4) {
			return  2;
		} else if (level <=  6) {
			return  1;
		} else if (level <= 12) {
			return  0;
		} else { /* illegal? */
			return  0; 
		}
	} else {    /* LAST == 1 */
		if        (level ==  1) {
			return  40;
		} else if (level ==  2) {
			return  1;
		} else if (level ==  3) {
			return  0;
		} else { /* illegal? */
			return  0; 
		}		
	}
}

/***/

static __inline tab_type *vldTableB16(MP4_STREAM * _ld, int code) 
{
	MP4_STREAM * ld = _ld;

	tab_type *tab;

	if (code >= 512) {
		tab = &(tableB16_1[(code >> 5) - 16]);
	} else if (code >= 128) {
		tab = &(tableB16_2[(code >> 2) - 32]);
	} else if (code >= 8) {
		tab = &(tableB16_3[(code >> 0) - 8]);
	} else {
		/* invalid Huffman code */
		return (tab_type *) NULL;
	}
	mp4_flushbits(ld, tab->len);
	return tab;
}

static __inline tab_type *vldTableI2(MP4_STREAM * _ld, int code) 
{
	MP4_STREAM * ld = _ld;

	tab_type *tab;

	if (code >= 512) {
		tab = &(tableI2_1[(code >> 5) - 16]);
	} else if (code >= 128) {
		tab = &(tableI2_2[(code >> 2) - 32]);
	} else if (code >= 8) {
		tab = &(tableI2_3[(code >> 0) - 8]);
	} else {
		/* invalid Huffman code */
		return (tab_type *) NULL;
	}
	mp4_flushbits(ld, tab->len);
	return tab;
}

/***/

static __inline tab_type *vldTableB17(MP4_STREAM * _ld, int code) 
{
	MP4_STREAM * ld = _ld;

	tab_type *tab;

	if (code >= 512) {
		tab = &(tableB17_1[(code >> 5) - 16]);
	} else if (code >= 128) {
		tab = &(tableB17_2[(code >> 2) - 32]);
	} else if (code >= 8) {
		tab = &(tableB17_3[(code >> 0) - 8]);
	} else {
		/* invalid Huffman code */
		return (tab_type *) NULL;
	}
	mp4_flushbits(ld, tab->len);
	return tab;
}

event_t mp4_vld_intra_dct(MP4_STREAM * _ld) 
{
	MP4_STREAM * ld = _ld;

	event_t event;
	tab_type *tab = (tab_type *) NULL;
	int lmax, rmax;
    #define ESCAPE 7167
    
	tab = vldTableB16(ld, mp4_showbits(ld, 12));
	if (!tab) { /* bad code */
		event.run   = 
		event.level = 
		event.last  = -1;
		return event;
	} 

	if (tab->val != ESCAPE) {
		event.run   = (tab->val >>  6) & 63;
		event.level =  tab->val        & 63;
		event.last  = (tab->val >> 12) &  1;
		event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
	} else {
		/* this value is escaped - see para 7.4.1.3 */
		/* assuming short_video_header == 0 */
		switch (mp4_showbits(ld, 2)) {
			case 0x0 :  /* Type 1 */
			case 0x1 :  /* Type 1 */
			default:
				mp4_flushbits(ld, 1);
				tab = vldTableB16(ld, mp4_showbits(ld, 12));  /* use table B-16 */
				if (!tab) { /* bad code */
					event.run   = 
					event.level = 
					event.last  = -1;
					return event;
				}
				event.run   = (tab->val >>  6) & 63;
				event.level =  tab->val        & 63;
				event.last  = (tab->val >> 12) &  1;
				lmax = vldTableB19(event.last, event.run);  /* use table B-19 */
				event.level += lmax;
				event.level =  mp4_getbits(ld, 1) ? -event.level : event.level;
				break;
			case 0x2 :  /* Type 2 */
				mp4_flushbits(ld, 2);
				tab = vldTableB16(ld, mp4_showbits(ld, 12));  /* use table B-16 */
				if (!tab) { /* bad code */
					event.run   = 
					event.level = 
					event.last  = -1;
					break;
				}
				event.run   = (tab->val >>  6) & 63;
				event.level =  tab->val        & 63;
				event.last  = (tab->val >> 12) &  1;
				rmax = vldTableB21(event.last, event.level);  /* use table B-21 */
				event.run = event.run + rmax + 1;
				event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
				break;
			case 0x3 :  /* Type 3  - fixed length codes */
				mp4_flushbits(ld, 2);
				event.last  = mp4_getbits(ld, 1);
				event.run   = mp4_getbits(ld, 6);  /* table B-18 */ 
				mp4_getbits(ld, 1); /* marker bit */
				event.level = mp4_getbits(ld, 12); /* table B-18 */
				/* sign extend level... */
				event.level = (event.level & 0x800) ? (event.level | (-1 ^ 0xfff)) : event.level;
				mp4_getbits(ld, 1); /* marker bit */
				break;
		}
	}

	return event;
}

event_t mp4_vld_intra_aic_dct(MP4_STREAM * _ld) 
{
	MP4_STREAM * ld = _ld;

	event_t event;
	tab_type *tab = (tab_type *) NULL;
//	int lmax, rmax;

	tab = vldTableI2(ld, mp4_showbits(ld, 12));
	if (!tab) { /* bad code */
		event.run   = 
		event.level = 
		event.last  = -1;
		return event;
	} 

	if (tab->val != ESCAPE) {
		event.run   = (tab->val >>  6) & 63;
		event.level =  tab->val        & 63;
		event.last  = (tab->val >> 12) &  1;
		event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
	} else {
		/* this value is escaped*/
		event.last = mp4_getbits(ld, 1);
		event.run = mp4_getbits(ld, 6);
		event.level = mp4_getbits(ld, 8);
		if (event.level >= 128)
				event.level = event.level - 256;

		if(event.level== -128)
		{
			int t;
			t = mp4_getbits(ld, 5);
			event.level = mp4_getbits(ld, 6);
			event.level <<= 26;
			event.level >>= 21;
			event.level |= t & 0x1f;
		}

		//assert(event.level != 0);
		//assert(event.level != 128);
	}

	return event;
}


event_t mp4_vld_inter_dct(MP4_STREAM * _ld) 
{
	MP4_STREAM * ld = _ld;

	event_t event;
	tab_type *tab = (tab_type *) NULL;
	int lmax, rmax;

	tab = vldTableB17(ld, mp4_showbits(ld, 12));
	if (!tab) { /* bad code */
		event.run   = 
		event.level = 
		event.last  = -1;
		return event;
	} 
	if (tab->val != ESCAPE) {
		event.run   = (tab->val >>  4) & 255;
		event.level =  tab->val        & 15;
		event.last  = (tab->val >> 12) &  1;
		event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
	} else {
		/* this value is escaped - see para 7.4.1.3 */
		/* assuming short_video_header == 0 */
		int mode = mp4_showbits(ld, 2);
		switch (mode) {
			case 0x0 :  /* Type 1 */
			case 0x1 :  /* Type 1 */
			default:
				mp4_flushbits(ld, 1);
				tab = vldTableB17(ld, mp4_showbits(ld, 12));  /* use table B-17 */
				if (!tab) { /* bad code */
					event.run   = 
					event.level = 
					event.last  = -1;
					return event;
				}
				event.run   = (tab->val >>  4) & 255;
				event.level =  tab->val        & 15;
				event.last  = (tab->val >> 12) &  1;
				lmax = vldTableB20(event.last, event.run);  /* use table B-20 */
				event.level += lmax;
				event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
				break;
			case 0x2 :  /* Type 2 */
				mp4_flushbits(ld, 2);
				tab = vldTableB17(ld, mp4_showbits(ld, 12));  /* use table B-17 */
				if (!tab) { /* bad code */
					event.run   = 
					event.level = 
					event.last  = -1;
					break;
				}
				event.run   = (tab->val >>  4) & 255;
				event.level =  tab->val        & 15;
				event.last  = (tab->val >> 12) &  1;
				rmax = vldTableB22(event.last, event.level);  /* use table B-22 */
				event.run = event.run + rmax + 1;
				event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
				break;
			case 0x3 :  /* Type 3  - fixed length codes */
				mp4_flushbits(ld, 2);
				event.last  = mp4_getbits(ld, 1);
				event.run   = mp4_getbits(ld, 6);  /* table B-18 */ 
				mp4_getbits(ld, 1); /* marker bit */
				event.level = mp4_getbits(ld, 12); /* table B-18 */
				/* sign extend level... */
				event.level = (event.level & 0x800) ? (event.level | (-1 ^ 0xfff)) : event.level;
				mp4_getbits(ld, 1); /* marker bit */
				break;
		}
	}

	return event;
}

event_t mp4_vld_inter_mq_dct(MP4_STREAM * _ld) 
{
	MP4_STREAM * ld = _ld;

	event_t event;
	tab_type *tab = (tab_type *) NULL;
//	int lmax, rmax;

	tab = vldTableB17(ld, mp4_showbits(ld, 12));
	if (!tab) { /* bad code */
		event.run   = 
		event.level = 
		event.last  = -1;
		return event;
	} 
	if (tab->val != ESCAPE) {
		event.run   = (tab->val >>  4) & 255;
		event.level =  tab->val        & 15;
		event.last  = (tab->val >> 12) &  1;
		event.level = mp4_getbits(ld, 1) ? -event.level : event.level;
	} else {
		/* this value is escaped */
		event.last = mp4_getbits(ld, 1);
		event.run = mp4_getbits(ld, 6);
		event.level = mp4_getbits(ld, 8);
		if (event.level >= 128)
				event.level = event.level - 256;

		if(event.level== -128)
		{
			int t;
			t = mp4_getbits(ld, 5);
			event.level = mp4_getbits(ld, 6);
			event.level <<= 26;
			event.level >>= 21;
			event.level |= t & 0x1f;
		}
	}

	return event;
}


int mp4_getMVdata(MP4_STREAM * _ld)
{
	MP4_STREAM * ld = _ld;
	
	int code;
	
	if (mp4_getbits(ld, 1)) {
		return 0; // hor_mv_data == 0
	}
	
	code = mp4_showbits(ld, 12);
	
	if (code >= 512)
	{
		code = (code >> 8) - 2;
		mp4_flushbits(ld, MVtab0[code].len);
		return MVtab0[code].val;
	}
	
	if (code >= 128)
	{
		code = (code >> 2) - 32;
		mp4_flushbits(ld, MVtab1[code].len);
		return MVtab1[code].val;
	}
	
	code -= 4; 
	
	//	assert(code >= 0);
	
	mp4_flushbits(ld, MVtab2[code].len);
	return MVtab2[code].val;
}
