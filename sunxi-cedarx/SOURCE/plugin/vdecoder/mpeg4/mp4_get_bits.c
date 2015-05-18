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

#define _SWAP(a,b) (b=((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]))

/* initialize buffer, call once before first mp4_getbits or mp4_showbits */
void mp4_initbits (MP4_STREAM * _ld, uint8_t *stream, int32_t length,uint8_t *bufstartptr,uint8_t *bufendptr)
{
	int32_t i;
	MP4_STREAM * ld = _ld;

	ld->bitcnt = 0;
	ld->count = 0;

	ld->startptr = ld->rdptr = stream;
	ld->length = length;
	ld->buf_end_ptr = bufendptr;
	ld->buf_start_ptr = bufstartptr;

	//check 32-bits aligned
	if(((uint32_t)ld->rdptr)&3)
	{
		ld->bit_a = 0;

		if(ld->rdptr > ld->buf_end_ptr)
			ld->rdptr = ld->buf_start_ptr;	//loop back
		ld->bit_a <<= 8;
		ld->bit_a |= ld->rdptr[0];
		ld->rdptr ++;
		ld->count ++;
		ld->bitcnt = 24;

		if(((uint32_t)ld->rdptr)&3)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr;	//loop back
			ld->bit_a <<= 8;
			ld->bit_a |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
			ld->bitcnt = 16;

			if(((uint32_t)ld->rdptr)&3)
			{
				if(ld->rdptr > ld->buf_end_ptr)
					ld->rdptr = ld->buf_start_ptr;	//loop back
				ld->bit_a <<= 8;
				ld->bit_a |= ld->rdptr[0];
				ld->rdptr ++;
				ld->count ++;
				ld->bitcnt = 8;
			}
		}
	}
	else if(ld->rdptr+4 <= ld->buf_end_ptr)
	{
		_SWAP(ld->rdptr, ld->bit_a);
		ld->rdptr += 4;
		ld->count += 4;
	}
	else
	{//will loop back
		ld->bit_a = 0;
		for (i=0;i<4;i++)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr; //loop back
			ld->bit_a <<= 8;
			ld->bit_a |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
		}
	}

	if(ld->rdptr+4 <= ld->buf_end_ptr)
	{
		_SWAP(ld->rdptr, ld->bit_b);
		ld->rdptr += 4;
		ld->count += 4;
	}
	else
	{
		ld->bit_b = 0;
		for (i=0;i<4;i++)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr; //loop back
			ld->bit_b <<= 8;
			ld->bit_b |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
		}

	}
}


/* advance by n bits */
void mp4_flushbits (MP4_STREAM * ld, int32_t n)
{
	int32_t i;

	ld->bitcnt += n;
	if (ld->bitcnt >= 32)
	{
		ld->bit_a = ld->bit_b;

		if(ld->rdptr+4 <= ld->buf_end_ptr)
		{
			_SWAP(ld->rdptr, ld->bit_b);
			ld->rdptr += 4;
			ld->count += 4;
		}
		else
		{
			ld->bit_b = 0;
			for (i=0;i<4;i++)
			{
				if(ld->rdptr > ld->buf_end_ptr)
					ld->rdptr = ld->buf_start_ptr;	//loop back
				ld->bit_b <<= 8;
				ld->bit_b |= ld->rdptr[0];
				ld->rdptr ++;
				ld->count ++;
			}

		}
		ld->bitcnt -= 32;
	}
}

/* read n bits */
uint32_t mp4_showbits (MP4_STREAM * ld, int32_t n)
{
    int32_t nbit = (n + ld->bitcnt) - 32;

    if (nbit > 0)
    {
		// The bits are on both ints
		return (((ld->bit_a & (0xFFFFFFFF >> (ld->bitcnt))) << nbit) |
			(ld->bit_b >> (32 - nbit)));

    }
    else
    {
		int32_t rbit = 32 - ld->bitcnt;
		return (ld->bit_a & (0xFFFFFFFF >> (ld->bitcnt))) >> (rbit-n);
    }
}

int32_t mp4_showbits1 (MP4_STREAM * ld)
{
	if(ld->bit_a & (0x80000000 >> ld->bitcnt))
		return 1;
	else
		return 0;
}

// returns absolute bis position inside the stream
uint32_t mp4_bitpos(MP4_STREAM * ld)
{
    return 8*ld->count + ld->bitcnt - 64;
}

uint32_t mp4_getbits (MP4_STREAM * ld, int32_t n)
{
	uint32_t l = mp4_showbits (ld, n);
	mp4_flushbits (ld, n);
	return l;
}

uint32_t mp4_getbits1(MP4_STREAM * ld)
{
	uint32_t l = mp4_showbits1 (ld);
	mp4_flushbits (ld, 1);
	return l;
}


int32_t mp4_nextbits(MP4_STREAM * _ld, int32_t nbit)
{
	MP4_STREAM * ld = _ld;

	return mp4_showbits(ld, nbit);
}

/***/

// Purpose: look nbit forward for an alignement
int16_t mp4_bytealigned(MP4_STREAM * _ld, int32_t nbit)
{
	MP4_STREAM * ld = _ld;

	return (((ld->bitcnt + nbit) % 8) == 0);
}

/***/

int16_t mp4_bytealign(MP4_STREAM * _ld)
{
	MP4_STREAM * ld = _ld;
	int16_t skipcnt = 0;

	while (! mp4_bytealigned(ld, skipcnt))
		skipcnt += 1;
		/*
		// verify stuffing bits here
		if (! mp4_check_stuffingcode(ld, skipcnt))
		return;
	*/
	mp4_flushbits(ld, skipcnt);
	return skipcnt;
}

uint32_t mp4_getLeftData(MP4_STREAM * _ld)
{
	MP4_STREAM * ld = _ld;
	uint32_t cur_bit_pos;
	uint32_t r;

	cur_bit_pos = mp4_bitpos(ld);
	if(cur_bit_pos&7)
		r = ld->length - (cur_bit_pos>>3) - 1;
	else
		r = ld->length - (cur_bit_pos>>3);

	return r;
}

void mp4_setld_offset(MP4_STREAM * _ld)
{
	int32_t i;
	MP4_STREAM * ld = _ld;

	ld->bitcnt  = 0;
	//check 32-bits aligned
	if(((uint32_t)ld->rdptr)&3)
	{
		ld->bit_a = 0;

		if(ld->rdptr > ld->buf_end_ptr)
			ld->rdptr = ld->buf_start_ptr;	//loop back
		ld->bit_a <<= 8;
		ld->bit_a |= ld->rdptr[0];
		ld->rdptr ++;
		ld->count ++;
		ld->bitcnt = 24;

		if(((uint32_t)ld->rdptr)&3)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr;	//loop back
			ld->bit_a <<= 8;
			ld->bit_a |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
			ld->bitcnt = 16;

			if(((uint32_t)ld->rdptr)&3)
			{
				if(ld->rdptr > ld->buf_end_ptr)
					ld->rdptr = ld->buf_start_ptr;	//loop back
				ld->bit_a <<= 8;
				ld->bit_a |= ld->rdptr[0];
				ld->rdptr ++;
				ld->count ++;
				ld->bitcnt = 8;
			}
		}
	}
	else if(ld->rdptr+4 <= ld->buf_end_ptr)
	{
		_SWAP(ld->rdptr, ld->bit_a);
		ld->rdptr += 4;
		ld->count += 4;
	}
	else
	{//will loop back
		ld->bit_a = 0;
		for (i=0;i<4;i++)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr; //loop back
			ld->bit_a <<= 8;
			ld->bit_a |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
		}
	}

	if(ld->rdptr+4 <= ld->buf_end_ptr)
	{
		_SWAP(ld->rdptr, ld->bit_b);
		ld->rdptr += 4;
		ld->count += 4;
	}
	else
	{
		ld->bit_b = 0;
		for (i=0;i<4;i++)
		{
			if(ld->rdptr > ld->buf_end_ptr)
				ld->rdptr = ld->buf_start_ptr; //loop back
			ld->bit_b <<= 8;
			ld->bit_b |= ld->rdptr[0];
			ld->rdptr ++;
			ld->count ++;
		}

	}
}
