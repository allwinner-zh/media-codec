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
#include "mp4_decfuncs.h"


void mp4_reset_sw_bits_status(MP4_STREAM *ld, uint32_t end_bit_pos);
void mp4_clear_status_reg(void);

#define MCBPC_CBPC(d)   (((d)>>8)  & 0x3)

#define CBPY_INTRA(d)   (((d)>>12) & 0xf )
#define CBPY_INTER(d)   (((d)>>8)  & 0xf )
#define CBPY_BITS(d)    ( (d)      & 0xff)

#define MBTYPE_MVDFW(x)   (((x) & 8) ? 1 : 0)
#define MBTYPE_MVDBW(x)   (((x) & 4) ? 1 : 0)
#define MBTYPE_CBPC(x)    (((x) & 2) ? 1 : 0)
#define MBTYPE_CBPY(x)    (((x) & 2) ? 1 : 0)
#define MBTYPE_DQUANT(x)  (((x) & 1) ? 1 : 0)

/* MBTYPE table for B pictures
 *     PredictionType   bits
 *                7-4    3-0
 */
#define    B_DIRECT_SKIPPED                     1
#define    B_DIRECT                             2
#define    B_DIRECT_Q                           3
#define    B_FORWARD_SKIPPED                    4
#define    B_FORWARD                            5
#define    B_FORWARD_Q                          6
#define    B_BACKWARD_SKIPPED                   7
#define    B_BACKWARD                           8
#define    B_BACKWARD_Q                         9
#define    B_BIDIR_SKIPPED                     10
#define    B_BIDIR                             11
#define    B_BIDIR_Q                           12
#define    B_INTRA                             13
#define    B_INTRA_Q                           14

#define MBTYPE_ENTRY(p,b)      ( ((p & 0xF) << 4) | (b & 0xF) )

const uint8_t gNewTAB_MBTYPE_B[128] = 
{

    // 0000000 - 0
    MBTYPE_ENTRY(0,7),

    // 0000001 - 1
    MBTYPE_ENTRY(B_INTRA_Q,7),

    // 000001x - 2 to 3
    MBTYPE_ENTRY(B_INTRA,6),    MBTYPE_ENTRY(B_INTRA,6),

    // 00001xx - 4 to 7
    MBTYPE_ENTRY(B_BIDIR_Q,5),    MBTYPE_ENTRY(B_BIDIR_Q,5),
    MBTYPE_ENTRY(B_BIDIR_Q,5),    MBTYPE_ENTRY(B_BIDIR_Q,5),

    // 0001xxx - 8 to 15
    MBTYPE_ENTRY(B_DIRECT_Q,4),    MBTYPE_ENTRY(B_DIRECT_Q,4),
    MBTYPE_ENTRY(B_DIRECT_Q,4),    MBTYPE_ENTRY(B_DIRECT_Q,4),
    MBTYPE_ENTRY(B_DIRECT_Q,4),    MBTYPE_ENTRY(B_DIRECT_Q,4),
    MBTYPE_ENTRY(B_DIRECT_Q,4),    MBTYPE_ENTRY(B_DIRECT_Q,4),

    // 00100xx - 16 to 19
    MBTYPE_ENTRY(B_BIDIR_SKIPPED,5),    MBTYPE_ENTRY(B_BIDIR_SKIPPED,5),
    MBTYPE_ENTRY(B_BIDIR_SKIPPED,5),    MBTYPE_ENTRY(B_BIDIR_SKIPPED,5),

    // 00101xx - 20 to 23
    MBTYPE_ENTRY(B_BIDIR,5),    MBTYPE_ENTRY(B_BIDIR,5),
    MBTYPE_ENTRY(B_BIDIR,5),    MBTYPE_ENTRY(B_BIDIR,5),

    // 00110xx - 24 to 27
    MBTYPE_ENTRY(B_FORWARD_Q,5),    MBTYPE_ENTRY(B_FORWARD_Q,5),
    MBTYPE_ENTRY(B_FORWARD_Q,5),    MBTYPE_ENTRY(B_FORWARD_Q,5),

    // 00111xx - 28 to 31
    MBTYPE_ENTRY(B_BACKWARD_Q,5),    MBTYPE_ENTRY(B_BACKWARD_Q,5),
    MBTYPE_ENTRY(B_BACKWARD_Q,5),    MBTYPE_ENTRY(B_BACKWARD_Q,5),

    // 010xxxx - 32 to 47
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),    MBTYPE_ENTRY(B_BACKWARD_SKIPPED,3),

    // 011xxxx - 48 to 63
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),
    MBTYPE_ENTRY(B_BACKWARD,3),    MBTYPE_ENTRY(B_BACKWARD,3),

    // 100xxxx - 64 to 79
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),
    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),    MBTYPE_ENTRY(B_FORWARD_SKIPPED,3),

    // 101xxxx - 80 to 95
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),
    MBTYPE_ENTRY(B_FORWARD,3),    MBTYPE_ENTRY(B_FORWARD,3),

    // 11xxxxx - 96 to 127
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),
    MBTYPE_ENTRY(B_DIRECT,2),    MBTYPE_ENTRY(B_DIRECT,2),

};
// Macroblock type defines
#define D_MBTYPE_INTRA        		0
#define D_MBTYPE_INTRA_Q            1
#define D_MBTYPE_FORWARD_SKIPPED    2
#define D_MBTYPE_FORWARD            3
#define D_MBTYPE_FORWARD_Q          4
#define D_MBTYPE_FORWARD_4V         5    // AP mode
#define D_MBTYPE_FORWARD_Q_4V       6    // AP mode with quant
#define D_MBTYPE_UPWARD_SKIPPED     7    // EI or EP pictures
#define D_MBTYPE_UPWARD             8    // EI or EP pictures
#define D_MBTYPE_UPWARD_Q           9    // EI or EP pictures
#define D_MBTYPE_BACKWARD_SKIPPED   10    // true B pictures
#define D_MBTYPE_BACKWARD           11    // true B pictures
#define D_MBTYPE_BACKWARD_Q         12    // true B pictures
#define D_MBTYPE_BIDIR_SKIPPED      13    // EP or true B pictures
#define D_MBTYPE_BIDIR              14    // EP or true B pictures
#define D_MBTYPE_BIDIR_Q            15    // EP or true B pictures
#define D_MBTYPE_DIRECT_SKIPPED     16    // true B pictures
#define D_MBTYPE_DIRECT             17    // true B pictures
#define D_MBTYPE_DIRECT_Q           18    // true B pictures


static const uint32_t MapMBType[] = {
    INTRA_MODE, // D_MBTYPE_INTRA
    INTRA_MODE, // D_MBTYPE_INTRA_Q
    FORWARD_MODE, // D_MBTYPE_FORWARD_SKIPPED
    FORWARD_MODE, // D_MBTYPE_FORWARD
    FORWARD_MODE, // D_MBTYPE_FORWARD_Q
    FORWARD_MODE, // D_MBTYPE_FORWARD_4V
    FORWARD_MODE, // D_MBTYPE_FORWARD_Q_4V
    FORWARD_MODE, // D_MBTYPE_UPWARD_SKIPPED
    FORWARD_MODE, // D_MBTYPE_UPWARD
    FORWARD_MODE, // D_MBTYPE_UPWARD_Q
    BACKWARD_MODE, // D_MBTYPE_BACKWARD_SKIPPED
    BACKWARD_MODE, // D_MBTYPE_BACKWARD
    BACKWARD_MODE, // D_MBTYPE_BACKWARD_Q
    BIDIR_MODE, // D_BTYPE_BIDIR_SKIPPED
    BIDIR_MODE, // D_MBTYPE_BIDIR
    BIDIR_MODE, // D_MBTYPE_BIDIR_Q
    DIRECT_MODE, // D_MBTYPE_DIRECT_SKIPPED
    DIRECT_MODE, // D_BTYPE_DIRECT
    DIRECT_MODE, // D_MBTYPE_DIRECT_Q
};

#define B_PREDICTION_TYPE(t)    ((uint32_t)(BPredictMap[t][1]))
#define B_FIELD_FLAGS(t)        ((uint32_t)(BPredictMap[t][0]))

static const uint8_t BPredictMap[15][2] = {
    {0x0, D_MBTYPE_DIRECT_SKIPPED},
    {0x0, D_MBTYPE_DIRECT_SKIPPED},  // B_DIRECT_SKIPPED    1
    {0x2, D_MBTYPE_DIRECT},          // B_DIRECT            2
    {0x3, D_MBTYPE_DIRECT_Q},        // B_DIRECT_Q          3
    {0x8, D_MBTYPE_FORWARD_SKIPPED}, // B_FORWARD_SKIPPED   4
    {0xA, D_MBTYPE_FORWARD},         // B_FORWARD           5
    {0xB, D_MBTYPE_FORWARD_Q},       // B_FORWARD_Q         6
    {0x4, D_MBTYPE_BACKWARD_SKIPPED},// B_BACKWARD_SKIPPED  7
    {0x6, D_MBTYPE_BACKWARD},        // B_BACKWARD          8
    {0x7, D_MBTYPE_BACKWARD_Q},      // B_BACKWARD_Q        9
    {0xC, D_MBTYPE_BIDIR_SKIPPED},   // B_BIDIR_SKIPPED    10
    {0xE, D_MBTYPE_BIDIR},           // B_BIDIR            11
    {0xF, D_MBTYPE_BIDIR_Q},         // B_BIDIR_Q          12
    {0x2, D_MBTYPE_INTRA},           // B_INTRA            13
    {0x3, D_MBTYPE_INTRA_Q}         // B_INTRA_Q          14
};

int32_t mp4_macroblock_h263_bvop(MP4_STATE *mp4_state);

/*******************************************************************************
Function name: update_framerate
Description:
    1.¸ü¸ÄµÄ³ÉÔ±±äÁ¿ÓÐ:
        pDev->vFormat.frame_rate
        pDev->vFormat.mic_sec_per_frm
        pDev->frm_show_low_threshold
        pDev->frm_show_high_threshold
Parameters:
    1.uFrmRate :£
Return:

Time: 2010/1/23
*******************************************************************************/
int32_t mp4_update_framerate(mp4_dec_ctx_t* ctx, uint32_t uFrmRate)
{
	ctx->videoStreamInfo.nFrameRate = uFrmRate;
	ctx->videoStreamInfo.nFrameDuration = 1000*1000*1000/uFrmRate;
    return 0;
}



int32_t GetSSC(MP4_STATE *mp4_state)
{
    MP4_STREAM *ld = &mp4_state->bits;
	if(1)
    {
        uint32_t w,w0;
        int32_t n;
        uint32_t msb = (1 << 17) - 1;

        w0 = mp4_showbits(ld,24);
        w = w0 >> 7;

        for (n = 0; w != 1 && n < 7; n ++)
        {
            if (w)
                break;
            w = w0 >>(6-n);
            w &= msb;
        }

        if (w == 1)
        {
            mp4_flushbits(ld,17+n);
            return 1;
        }
        else
            return 0;
    }

    return 0;
}


/********************************************************
* Get the start mb address of next slice                *
*********************************************************/
//If the data format is changed, here should be changed also.
int32_t get_next_next_sliceMba(mp4_dec_ctx_t* ctx, int32_t nbits)
{
	MP4_STATE*   mp4_state;
    MP4_STREAM * ld;
    int32_t byte_offset1;
    uint8_t t1,t2,t3,t4;
    uint32_t value;
    uint8_t *tmpPtr;
    
    mp4_state = &ctx->s;
    ld        = &mp4_state->bits;

    if(mp4_state->hdr.gob_number != (mp4_state->packet_num-1))
    {
        byte_offset1 = mp4_state->data_offset[mp4_state->hdr.gob_number+1];
        if(!byte_offset1)
        {
            value = mp4_state->hdr.mb_ysize*mp4_state->hdr.mb_xsize;
            return value;
        }
    }
    else
    {
        value = mp4_state->hdr.mb_ysize*mp4_state->hdr.mb_xsize;
        return value;
    }

    tmpPtr = ld->startptr+byte_offset1+2;
    if (tmpPtr > ld->buf_end_ptr)
        tmpPtr -= (ld->buf_end_ptr - ld->buf_start_ptr + 1);
    t1 = *tmpPtr;

    tmpPtr = ld->startptr+byte_offset1+3;
    if (tmpPtr > ld->buf_end_ptr)
        tmpPtr -= (ld->buf_end_ptr - ld->buf_start_ptr + 1);
    t2 = *tmpPtr;

    tmpPtr = ld->startptr+byte_offset1+4;
    if (tmpPtr > ld->buf_end_ptr)
        tmpPtr -= (ld->buf_end_ptr - ld->buf_start_ptr + 1);
    t3 = *tmpPtr;

    tmpPtr = ld->startptr+byte_offset1+5;
    if (tmpPtr > ld->buf_end_ptr)
        tmpPtr -= (ld->buf_end_ptr - ld->buf_start_ptr + 1);
    t4 = *tmpPtr;
    value = (t1<<24) | (t2<<16) | (t3<<8) | t4;
	
    if(mp4_state->m_pctszSize > 0)
        value <<= mp4_state->m_pctszSize;
    value >>= (32-nbits);

    return value;
}

int32_t mp4_getgobhdr(mp4_dec_ctx_t* ctx, int32_t gob_index)
{
    MP4_STATE*  mp4_state;
    MP4_STREAM *ld;
    uint32_t start_bit_pos,end_bit_pos,bit_length;
    int32_t m_mbaSize,next_sliceMba,display_time=0,sliceMba=0;
    int32_t i,j;
    
    mp4_state = &ctx->s;

    ld = &mp4_state->bits;
    mp4_state->hdr.macroblok_bounday = 0;

    if(mp4_state->hdr.bSliceStructured)
    {
        mp4_state->hdr.GobEdge.iAbove = mp4_state->hdr.mb_xsize;
        mp4_state->hdr.GobEdge.iLeft = 1;
    }

    if (mp4_state->hdr.gob_number != 0 || mp4_state->isH263)
    {
        if(mp4_state->isH263)
        {
            if(1)
            {
                mp4_getbits(ld, 1);
                j = mp4_state->hdr.mba_size-1;
                for (i=0;i<6&& (int32_t)MBA_NumMBs[i]<j;i++)
                    ;
                m_mbaSize = MBA_FieldWidth[i];

                sliceMba = mp4_getbits(ld,m_mbaSize);
                if(m_mbaSize>11)
                {
                    /* Must be 1 to prevent start code emulation (SEPB2) */
                    mp4_getbits(ld, 1);
                }
                if(mp4_state->hdr.gob_number)
                {
                    mp4_state->hdr.vop_quant = mp4_getbits(ld, 5);
                    mp4_state->hdr.quantizer = mp4_state->hdr.vop_quant;
                    mp4_state->hdr.quant_scale = mp4_state->hdr.vop_quant;
                }

                /* Must be 1 to prevent start code emulation (SEPB3) */
                mp4_getbits(ld, 1);

                /* Get GOB frame ID. */
                if(mp4_state->hdr.gob_number)
                    mp4_getbits(ld, 2);

                if(mp4_state->hdr.h263_aic)
                    mp4_state->vld_intra_fun = mp4_vld_intra_aic_dct;

                if (mp4_state->hdr.picture_coding_type != MP4_B_VOP && !gob_index) {

                    mp4_state->hdr.display_time_prev = mp4_state->hdr.display_time_next;
                    mp4_state->hdr.display_time_next = display_time;

                    if (display_time != mp4_state->hdr.display_time_prev)
                    {
                        mp4_state->hdr.trd = mp4_state->hdr.display_time_next - mp4_state->hdr.display_time_prev;
                        if(mp4_state->hdr.trd<0)
                            mp4_state->hdr.trd += 8192;    
                    }
                }
                else {
                    mp4_state->hdr.trb = display_time - mp4_state->hdr.display_time_prev;
                    if(mp4_state->hdr.trb<0)
                        mp4_state->hdr.trb += 8192;
                }
                //try to get next slize Mba to calcument mb numbers in current gob
                next_sliceMba =get_next_next_sliceMba(ctx,m_mbaSize);
                mp4_state->hdr.num_mb_in_gob = next_sliceMba - sliceMba;
                mp4_state->hdr.mba = sliceMba;
            }


            mp4_state->hdr.macroblok_bounday = 1;
            mp4_set_packet_info(mp4_state);
        }
        else
        {
            if (mp4_nextbits(ld, 17) == (int32_t)RESYNC_MARKER)
            {
                if (mp4_getbits(ld, 17) != (uint32_t)RESYNC_MARKER)
                {
                    return -1; 
                }
                mp4_state->hdr.gob_number = mp4_getbits(ld, 5);
                mp4_state->hdr.gob_frame_id = mp4_getbits(ld, 2);
                mp4_state->hdr.quant_scale = mp4_getbits(ld, 5);
                mp4_state->hdr.quantizer = mp4_state->hdr.quant_scale;
                mp4_state->hdr.use_intra_dc_vlc = mp4_get_use_intra_dc_vlc(mp4_state->hdr.quantizer, mp4_state->hdr.intra_dc_vlc_thr);
                mp4_state->hdr.macroblok_bounday = 1;
            }
        }
    }
    else
    {
        mp4_state->hdr.macroblok_bounday = 1;
    }

    start_bit_pos = mp4_bitpos(ld);

    if(mp4_state->isH263 && mp4_state->hdr.prediction_type == MP4_B_VOP)
    {
        i= mp4_state->hdr.mba = sliceMba;
        for ( ;i < mp4_state->hdr.mba_size; i++)
        {
            //decide slice data is over, if over, break for next gob
            if(mp4_state->hdr.bSliceStructured && GetSSC(mp4_state))
                goto decode_slice_end;

            if(mp4_macroblock_h263_bvop(mp4_state)<0)
                break;

            mp4_state->hdr.mba++;
        }
    }
    else
    {
        mp4_set_packet_info(mp4_state);
        if(mp4_state->isH263)
            mp4_set_vbv_info(mp4_state->hdr.num_mb_in_gob, mp4_state->hdr.macroblok_bounday, (mp4_state->valid_data_offset<<3)+start_bit_pos,ld->length*8-start_bit_pos, ctx->nStreamBufferSize);
        else
        {
            start_bit_pos = mp4_bitpos(ld);
            bit_length = (ld->length<<3) - start_bit_pos;
            start_bit_pos += (ld->startptr - ld->buf_start_ptr)<<3;

            mp4_set_vbv_info(mp4_state->hdr.num_mb_in_gob,mp4_state->hdr.macroblok_bounday,start_bit_pos,bit_length, ctx->nStreamBufferSize);
        }

        if(AdapterVeWaitInterrupt() == 0)
        {
        	mp4_clear_status_reg();
        }
        else
        {
    		mp4_reset_ve_core(ctx);
        	mp4_set_buffer(mp4_state);
        	mp4_set_pic_size(mp4_state);
        }

        //after finish, get mba_addr, and bit offset
        mp4_state->hdr.mba += mp4_state->hdr.num_mb_in_gob;
        mp4_state->hdr.mb_xpos = mp4_state->hdr.mba % mp4_state->hdr.mb_xsize;
        mp4_state->hdr.mb_ypos = mp4_state->hdr.mba / mp4_state->hdr.mb_xsize;
        //reset bits info
        end_bit_pos = mp4_get_bitoffset();
        if(end_bit_pos&7)
            end_bit_pos = (end_bit_pos+7)&0xfffffff8;
        mp4_reset_sw_bits_status(ld,end_bit_pos);
    }


decode_slice_end:
    if ((mp4_nextbits(ld, 17) != RESYNC_MARKER) &&
        (mp4_nextbits_bytealigned(ld, 17, 1, NULL) == RESYNC_MARKER))
    {
        mp4_bytealign(ld);
    }
    mp4_state->hdr.gob_number++;

    return 0;
}

int32_t mp4_macroblock_h263_bvop(MP4_STATE *mp4_state)
{
    int32_t j;
    int8_t is_stuffing;
    int8_t bGetDQUANT = 0;
    int8_t bGetCBPC = 0;
    int8_t bGetMVDBW = 0;
    int8_t bGetMVDFW = 0;
    uint32_t uResult;

    MP4_STREAM *ld = &mp4_state->bits;

    int32_t mb_xpos = mp4_state->hdr.mb_xpos;
    int32_t mb_ypos = mp4_state->hdr.mb_ypos;


    if (mb_xpos < (mp4_state->hdr.mb_xsize-1))
    {
        if(!mb_xpos)
            mp4_state->hdr.GobEdge.iLeft = 1;
    }

Read_not_coded:
    mp4_state->hdr.not_coded = mp4_getbits(ld, 1);
    if (! mp4_state->hdr.not_coded)
    {
//        if(mp4_state->hdr.mba == 0xbb && mp4_state->frame_ctr==4)
//            mp4_check_iqis_in_empty();
        uResult = mp4_getMB_h263B_TYPE(ld,&is_stuffing); // mb_type
        if(is_stuffing)
            goto Read_not_coded;
        bGetDQUANT = bGetCBPC = bGetMVDFW  = bGetMVDBW  = 0;

        mp4_state->hdr.mb_type = B_PREDICTION_TYPE(uResult);

        bGetDQUANT = MBTYPE_DQUANT(B_FIELD_FLAGS(uResult));
        bGetMVDFW  = MBTYPE_MVDFW(B_FIELD_FLAGS(uResult));
        bGetMVDBW  = MBTYPE_MVDBW(B_FIELD_FLAGS(uResult));
        bGetCBPC = (MBTYPE_CBPC(B_FIELD_FLAGS(uResult)) ? 1 : 0);

        mp4_state->hdr.mb_type  = MapMBType[mp4_state->hdr.mb_type];

        // INTRA_MODE: present in intra MB's when advanced intra mode used
        if ( (INTRA_MODE == mp4_state->hdr.mb_type) && (! mp4_state->hdr.short_video_header || mp4_state->hdr.h263_aic) )
        {
            mp4_state->hdr.ac_pred_flag = mp4_getbits(ld, 1);
            if(mp4_state->hdr.h263_aic && mp4_state->hdr.ac_pred_flag)
                mp4_state->hdr.h263_aic_dir = mp4_getbits(ld, 1);//1: AINTRA_Mode_horiz, 0:AINTRA_MODE_VERT
        }
        else
            mp4_state->hdr.ac_pred_flag = 0;

        // CBPC ----------------------------------------------------
        if (bGetCBPC)
        {
            mp4_state->hdr.cbpc = 0;
            if (mp4_getbits(ld, 1))
            {
                if (mp4_getbits(ld, 1))
                {
                    mp4_state->hdr.cbpc = (mp4_getbits(ld, 1) ? 2 : 3);
                }
                else
                {
                    mp4_state->hdr.cbpc = 1;
                }
            }
            mp4_state->hdr.cbpy = mp4_getCBPY(ld, INTRA_MODE == mp4_state->hdr.mb_type); // cbpy
            mp4_state->hdr.modb = MODB_00;
        }
        else
        {
            mp4_state->hdr.cbpc = 0;
            mp4_state->hdr.cbpy = 0;
            mp4_state->hdr.modb = MODB_01;
        }

        mp4_state->hdr.cbp  = mp4_state->hdr.cbpb = (mp4_state->hdr.cbpy << 2) | mp4_state->hdr.cbpc;

        if (bGetDQUANT)
        {

            if(mp4_state->hdr.iModifiedQantization)
            {

                if(mp4_getbits(ld, 1))
                {
                    mp4_state->hdr.dquant = gNewTAB_DQUANT_MQ[mp4_state->hdr.quantizer][mp4_getbits(ld, 1)];
                    mp4_state->hdr.quantizer += mp4_state->hdr.dquant;

                }
                else
                {
                    int32_t m_tmp;
                    m_tmp = mp4_state->hdr.quantizer;
                    mp4_state->hdr.quantizer = mp4_getbits(ld, 5);
                    mp4_state->hdr.dquant = mp4_state->hdr.quantizer - m_tmp;
                }
            }
            else
            {
                mp4_state->hdr.dquant = mp4_getbits(ld, 2);
                mp4_state->hdr.quantizer += DQtab[mp4_state->hdr.dquant];
                if (mp4_state->hdr.quantizer > 31)
                    mp4_state->hdr.quantizer = 31;
                else if (mp4_state->hdr.quantizer < 1)
                    mp4_state->hdr.quantizer = 1;
            }
        }
        else
            mp4_state->hdr.dquant = 0;

        if(!bGetMVDFW)
            memset(&mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0], 0, 6*sizeof(MotionVector));
        if(!bGetMVDBW)
            memset(&mp4_state->MVBack[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0], 0, 6*sizeof(MotionVector));

        if ( bGetMVDFW )
            mp4_setMV_263B(ld, mp4_state, mb_xpos, mb_ypos, FORWARD_MODE);
        if ( bGetMVDBW)
            mp4_setMV_263B(ld, mp4_state, mb_xpos, mb_ypos, BACKWARD_MODE);

        if (mp4_state->hdr.mb_type == DIRECT_MODE)
        {
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x = 0;
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y = 0;
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x = 0;
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y = 0;
        }

        if(mp4_state->hdr.mb_type == INTRA_MODE)
        {
            mp4_set_mb_info(mp4_state);

            // texture decoding add
            for (j = 0; j < 6; j++)
            {
                int32_t coded = mp4_state->hdr.cbp & (1 << (5 - j));

                mp4_blockIntra(ld, mp4_state, j, (coded != 0));
            }
        }
        else
        {
            mp4_set_mb_info(mp4_state);

            for (j = 0; j < 6; j++)
            {
                int32_t coded = mp4_state->hdr.cbpb & (1 << (5 - j));

                if (coded)
                    mp4_blockInter(ld, mp4_state);
            }
        }
    }
    else
    {
        mp4_state->hdr.modb = MODB_1;
        mp4_state->hdr.field_prediction =mp4_state->fieldpredictedmap[(mb_ypos+1) * mp4_state->fieldpredictedmap_stride + mb_xpos + 1];
        mp4_state->hdr.forward_top_field_reference=mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride];
        mp4_state->hdr.forward_bottom_field_reference=mp4_state->fieldrefmap[2*mb_xpos + mb_ypos*mp4_state->fieldrefmap_stride + 1];
        mp4_state->hdr.backward_top_field_reference=0;
        mp4_state->hdr.backward_bottom_field_reference=1;

        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x = 0;
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y = 0;
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x = 0;
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y = 0;

        memset(&mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0], 0, 4*sizeof(MotionVector));
        memset(&mp4_state->MVBack[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][0], 0, 4*sizeof(MotionVector));

        mp4_set_mb_info(mp4_state);
    }

    if(mp4_state->hdr.GobEdge.iLeft)
        mp4_state->hdr.GobEdge.iLeft -- ;
    if(mp4_state->hdr.GobEdge.iAbove)
        mp4_state->hdr.GobEdge.iAbove -- ;

    if (mp4_state->hdr.mb_xpos < (mp4_state->hdr.mb_xsize-1))
    {
        mp4_state->hdr.mb_xpos++;
    }
    else 
    {
        mp4_state->hdr.mb_ypos++;
        mp4_state->hdr.mb_xpos = 0;
    }

    return 0;
}

void mp4_reset_sw_bits_status(MP4_STREAM *ld, uint32_t end_bit_pos)
{
    int32_t i;
    uint8_t cTemp[12];

    ld->bitcnt = end_bit_pos&0x1f;
    ld->rdptr = ld->buf_start_ptr + (end_bit_pos>>3);
    ld->count = ld->rdptr - ld->startptr;
    if(ld->rdptr > ld->buf_end_ptr)
        ld->rdptr  = ld->buf_start_ptr + (ld->rdptr-ld->buf_end_ptr)-1;
    if((ld->buf_end_ptr - ld->rdptr)>6 && (ld->rdptr-ld->buf_start_ptr)>3)
    {//..............********.......
        memcpy(cTemp,ld->rdptr-4,12);
    }
    else if((ld->buf_end_ptr - ld->rdptr)>6)
    {//..*********..............
        memcpy(cTemp+4,ld->rdptr,8);
        i = ld->rdptr-ld->buf_start_ptr;
        memcpy(cTemp,ld->buf_end_ptr-3+i,4-i);
        memcpy(cTemp+4-i,ld->buf_start_ptr,i);
    }
    else
    {//..................************..
        memcpy(cTemp,ld->rdptr-4,4);
        i = ld->buf_end_ptr - ld->rdptr+1;
        memcpy(cTemp+4,ld->rdptr,i);
        memcpy(cTemp+4+i,ld->buf_start_ptr,8-i);
    }

    i = ld->bitcnt>>3;
    ld->bit_a = cTemp[4-i];
    ld->bit_a <<= 8;
    ld->bit_a |= cTemp[5-i];
    ld->bit_a <<= 8;
    ld->bit_a |= cTemp[6-i];
    ld->bit_a <<= 8;
    ld->bit_a |= cTemp[7-i];
    ld->bit_b = cTemp[8-i];
    ld->bit_b <<= 8;
    ld->bit_b |= cTemp[9-i];
    ld->bit_b <<= 8;
    ld->bit_b |= cTemp[10-i];
    ld->bit_b <<= 8;
    ld->bit_b |= cTemp[11-i];

    ld->count += 8-i;
    ld->rdptr += 8-i;
    if(ld->rdptr > ld->buf_end_ptr)
        ld->rdptr  = ld->buf_start_ptr + (ld->rdptr-ld->buf_end_ptr)-1;
}

int32_t mp4_getMB_h263B_TYPE(MP4_STREAM * ld, int8_t *stuffing)
{
    int32_t code = mp4_showbits(ld, 7);

    *stuffing = 0;
    mp4_flushbits(ld, gNewTAB_MBTYPE_B[code]&0x0f);
    if(code == 0)
    {
        code = mp4_getbits(ld, 2);
        if (1 == code)
            *stuffing = 1;
    }
    return gNewTAB_MBTYPE_B[code]>>4;
}


int32_t mp4_getCBPY(MP4_STREAM * _ld, int32_t intraFlag)
{
    MP4_STREAM * ld = _ld;

    int32_t cbpy;
    int32_t code = mp4_showbits(ld, 6);

    if (code < 2) 
    {
        return -1;
    }

    if (code >= 48) 
    {
        mp4_flushbits(ld, 2);
        cbpy = 15;
    } 
    else 
    {
        mp4_flushbits(ld, CBPYtab[code].len);
        cbpy = CBPYtab[code].val;
    }

    if (! intraFlag)
        cbpy = 15 - cbpy;

    return cbpy;
}

int32_t iBlockIndex[4][3] = {{1,2,2},{0,3,2},{3,0,1},{2,0,1}};
int32_t iMBIndex_x[4][3] = {{-1,0,1},{0,0,1},{-1,0,0},{0,0,0}};
int32_t iMBIndex_y[4][3] = {{0,-1,-1},{0,-1,-1},{0,0,0},{0,0,0}};
MotionVector mp4_h263_find_pmv (MP4_STATE * _mp4_state, int32_t x, int32_t y, int32_t block, int32_t mode)
{
    MP4_STATE * mp4_state = _mp4_state;

    MotionVector p1, p2, p3, mv;
    int32_t xin1, xin2, xin3;
    int32_t yin1, yin2, yin3;
    int32_t vec1, vec2, vec3;

    p1.x = p1.y = 0;
    p2.x = p2.y = 0;
    p3.x = p3.y = 0;
    mv.x = mv.y = 0;
    x++;
    y++;

    if((block&1) || !mp4_state->hdr.GobEdge.iLeft)
    {//not left edge
        vec1 = iBlockIndex[block][0];
        yin1 = y+iMBIndex_y[block][0];        xin1 = x+iMBIndex_x[block][0];
    }
    else
    {
        vec1 = 0;    yin1 = 0;        xin1 = 0;
    }

    if(mp4_state->hdr.GobEdge.iAbove && block<2)
    {
        vec2 = vec1; yin2 = yin1; xin2 = xin1;
        //if(x==mp4_state->mb_width)
        {
            vec3 = vec1; yin3 = yin1; xin3 = xin1;
        }
        /*
        else
        {
        vec3 = 0;    yin3 = 0;        xin3 = 0;
    }*/
    }
    else
    {
        vec2 = iBlockIndex[block][1];
        yin2 = y+iMBIndex_y[block][1];        xin2 = x+iMBIndex_x[block][1];

        if(x==mp4_state->hdr.mb_xsize && block<2)
        {
            vec3 = 0;    yin3 = 0;        xin3 = 0;
        }
        else
        {
            vec3 = iBlockIndex[block][2];
            yin3 = y+iMBIndex_y[block][2];        xin3 = x+iMBIndex_x[block][2];
        }

    }

    if(mode == FORWARD_MODE)
    {
        p1 = mp4_state->MV[yin1 * mp4_state->MV_stride + xin1][vec1];
        p2 = mp4_state->MV[yin2 * mp4_state->MV_stride + xin2][vec2];
        p3 = mp4_state->MV[yin3 * mp4_state->MV_stride + xin3][vec3];
    }
    else if(mode == BACKWARD_MODE)
    {
        p1 = mp4_state->MVBack[yin1 * mp4_state->MV_stride + xin1][vec1];
        p2 = mp4_state->MVBack[yin2 * mp4_state->MV_stride + xin2][vec2];
        p3 = mp4_state->MVBack[yin3 * mp4_state->MV_stride + xin3][vec3];
    }

    mv.x = mmin(mmax(p1.x, p2.x), mmin(mmax(p2.x, p3.x), mmax(p1.x, p3.x)));
    mv.y = mmin(mmax(p1.y, p2.y), mmin(mmax(p2.y, p3.y), mmax(p1.y, p3.y)));

    return mv;
}

int32_t mp4_setMV_263B(MP4_STREAM * _ld, MP4_STATE * _mp4_state, int32_t mb_xpos, int32_t mb_ypos, int32_t mode)
{
    MP4_STREAM * ld = _ld;
    MP4_STATE * mp4_state = _mp4_state;

    int32_t vop_fcode, scale_fac;
    int32_t hor_mv_data, ver_mv_data, hor_mv_res, ver_mv_res;
    int32_t high, low, range;

    int32_t mvd_x, mvd_y;
    MotionVector pmv={0, 0}, mv={0, 0};

    int32_t i;

    if (mode == DIRECT_MODE) 
    {
        vop_fcode = 1; // scale_fac == 1, residual will not be decoded
    }
    else if (mode == FORWARD_MODE)
    {
        vop_fcode = mp4_state->hdr.fcode_for;
    }
    else
    {
        vop_fcode = mp4_state->hdr.fcode_back;
    }

    scale_fac = 1 << (vop_fcode - 1);
    high = (32 * scale_fac) - 1;
    low = ((-32) * scale_fac);
    range = (64 * scale_fac);

    hor_mv_res = 0;
    ver_mv_res = 0;

    hor_mv_data = mp4_getMVdata(ld); // mv data

    if ((scale_fac == 1) || (hor_mv_data == 0))
        mvd_x = hor_mv_data;
    else 
    {
        hor_mv_res = mp4_getbits(ld, vop_fcode-1); // mv residual
        mvd_x = ((abs(hor_mv_data) - 1) * scale_fac) + hor_mv_res + 1;
        if (hor_mv_data < 0)
            mvd_x = - mvd_x;
    }

    ver_mv_data = mp4_getMVdata(ld);

    if ((scale_fac == 1) || (ver_mv_data == 0))
        mvd_y = ver_mv_data;
    else 
    {
        ver_mv_res = mp4_getbits(ld, vop_fcode-1);
        mvd_y = ((abs(ver_mv_data) - 1) * scale_fac) + ver_mv_res + 1;
        if (ver_mv_data < 0)
            mvd_y = - mvd_y;
    }

    if (mp4_state->hdr.quarter_pixel == 1)
    {
        // do nothing here, mvd_x and mvd_y will be
        // in quarter pixel resolution
    }

    if (mode == FORWARD_MODE)
        pmv = mp4_h263_find_pmv(mp4_state, mb_xpos, mb_ypos, 0,FORWARD_MODE);
    else if (mode == BACKWARD_MODE)
        pmv = mp4_h263_find_pmv(mp4_state, mb_xpos, mb_ypos, 0,BACKWARD_MODE);
    else if (mode == DIRECT_MODE) 
    {
        // in this case the predictors are zero and the delta vector is extracted (MVDx, MVDy)
        pmv.x = pmv.y = 0;
    }

    mv.x = pmv.x + mvd_x;

    if (mv.x < low)
        mv.x += range;
    if (mv.x > high)
        mv.x -= range;

        mv.y = pmv.y + mvd_y;

    if (mv.y < low)
        mv.y += range;
    if (mv.y > high)
        mv.y -= range;

    if (mode == FORWARD_MODE)
    {
        for (i = 0; i < 4; i++)
        {
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][i].x = mv.x;
            mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][i].y = mv.y;
        }
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].x = mv.x;
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][4].y = mv.y;
    }
    else if (mode == BACKWARD_MODE)
    {
        for (i = 0; i < 4; i++)
        {
            mp4_state->MVBack[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][i].x = mv.x;
            mp4_state->MVBack[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][i].y = mv.y;
        }
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].x = mv.x;
        mp4_state->MV[(mb_ypos+1) * mp4_state->MV_stride + mb_xpos+1][5].y = mv.y;
    }

  return 1;
}

int32_t mp4_getDCsizeLum(MP4_STREAM * _ld)
{
    MP4_STREAM * ld = _ld;

    int32_t code;

    if (mp4_showbits(ld, 11) == 1) 
    {
        mp4_flushbits(ld, 11);
        return 12;
    }
    if (mp4_showbits(ld, 10) == 1)
    {
        mp4_flushbits(ld, 10);
        return 11;
    }
    if (mp4_showbits(ld, 9) == 1)
    {
        mp4_flushbits(ld, 9);
        return 10;
    }
    if (mp4_showbits(ld, 8) == 1)
    {
        mp4_flushbits(ld, 8);
        return 9;
    }
    if (mp4_showbits(ld, 7) == 1)
    {
        mp4_flushbits(ld, 7);
        return 8;
    }
    if (mp4_showbits(ld, 6) == 1)
    {
        mp4_flushbits(ld, 6);
        return 7;
    }
    if (mp4_showbits(ld, 5) == 1)
    {
        mp4_flushbits(ld, 5);
        return 6;
    }
    if (mp4_showbits(ld, 4) == 1)
    {
        mp4_flushbits(ld, 4);
        return 5;
    }

    code = mp4_showbits(ld, 3);

    if (code == 1) 
    {
        mp4_flushbits(ld, 3);
        return 4;
    } else if (code == 2)
    {
        mp4_flushbits(ld, 3);
        return 3;
    } else if (code == 3)
    {
        mp4_flushbits(ld, 3);
        return 0;
    }

    code = mp4_showbits(ld, 2);

    if (code == 2)
    {
        mp4_flushbits(ld, 2);
        return 2;
    } else if (code == 3)
    {
        mp4_flushbits(ld, 2);
        return 1;
    }

    return 0;
}

int32_t mp4_getDCsizeChr(MP4_STREAM * _ld)
{
    MP4_STREAM * ld = _ld;

    if (mp4_showbits(ld, 12) == 1)
    {
        mp4_flushbits(ld, 12);
        return 12;
    }
    if (mp4_showbits(ld, 11) == 1)
    {
        mp4_flushbits(ld, 11);
        return 11;
    }
    if (mp4_showbits(ld, 10) == 1)
    {
        mp4_flushbits(ld, 10);
        return 10;
    }
    if (mp4_showbits(ld, 9) == 1)
    {
        mp4_flushbits(ld, 9);
        return 9;
    }
    if (mp4_showbits(ld, 8) == 1)
    {
        mp4_flushbits(ld, 8);
        return 8;
    }
    if (mp4_showbits(ld, 7) == 1)
    {
        mp4_flushbits(ld, 7);
        return 7;
    }
    if (mp4_showbits(ld, 6) == 1)
    {
        mp4_flushbits(ld, 6);
        return 6;
    }
    if (mp4_showbits(ld, 5) == 1)
    {
        mp4_flushbits(ld, 5);
        return 5;
    }
    if (mp4_showbits(ld, 4) == 1)
    {
        mp4_flushbits(ld, 4);
        return 4;
    }
    if (mp4_showbits(ld, 3) == 1)
    {
        mp4_flushbits(ld, 3);
        return 3;
    }

    return (3 - mp4_getbits(ld, 2));
}

/***/

int32_t mp4_getDCdiff(MP4_STREAM * _ld, int32_t dct_dc_size)
{
    MP4_STREAM * ld = _ld;

    int32_t code = mp4_getbits(ld, dct_dc_size);
    int32_t msb = code >> (dct_dc_size - 1);

    if (msb == 0)
    {
        return (-1 * (code^((1 << dct_dc_size) - 1)));
    }
    else
    {
        return code;
    }
}

int32_t mp4_blockIntra(MP4_STREAM *ld, MP4_STATE *mp4_state, int32_t block_num, int32_t coded)
{
    int32_t i; // first AC coefficient position
    int32_t dct_dc_size, dct_dc_diff;
    event_t event;
    uint32_t tmp;

    mp4_check_iqis_in_empty();

    // dc coeff
    if(! mp4_state->hdr.short_video_header)
    {
        if(mp4_state->hdr.use_intra_dc_vlc)
        {
            if (block_num < 4)
            {
                dct_dc_size = mp4_getDCsizeLum(ld);
                if (dct_dc_size != 0)
                    dct_dc_diff = mp4_getDCdiff(ld, dct_dc_size);
                else
                    dct_dc_diff = 0;
                if (dct_dc_size > 8)
                    mp4_getbits1(ld); // marker bit
            }
            else
            {
                dct_dc_size = mp4_getDCsizeChr(ld);
                if (dct_dc_size != 0)
                    dct_dc_diff = mp4_getDCdiff(ld, dct_dc_size);
                else
                    dct_dc_diff = 0;
                if (dct_dc_size > 8)
                    mp4_getbits1(ld); // marker bit
            }

            if(dct_dc_diff >=0)
                tmp = dct_dc_diff;
            else
            {
                tmp = -dct_dc_diff;
                tmp |= 0x800;
            }

            if(!coded)
                tmp |= 0x40000;            // the last run-level value in current block
            VERegWriteD(IQ_IDCTINPUT_REG,tmp);
        }
        else
        {
            tmp = 0;
            if(!coded)
                tmp |= 0x40000;            // the last run-level value in current block
            VERegWriteD(IQ_IDCTINPUT_REG,tmp);
        }
    }
    else if(!mp4_state->hdr.h263_aic)
    {
        dct_dc_diff = mp4_getbits(ld, 8);
        if(dct_dc_diff==128)
        {
            tmp = 0x40000;            // the last run-level value in current block
            VERegWriteD(IQ_IDCTINPUT_REG,tmp);
            return -1;
        }

        if (dct_dc_diff == 255) {
            dct_dc_diff = 128;
        }
        if(dct_dc_diff >=0)
            tmp = dct_dc_diff;
        else
        {
            tmp = -dct_dc_diff;
            tmp |= 0x800;
        }

        if(!coded)
            tmp |= 0x40000;            // the last run-level value in current block
        VERegWriteD(IQ_IDCTINPUT_REG,tmp);
    }

    if (coded)
    {
        i = (mp4_state->hdr.short_video_header ? 1 :
            mp4_state->hdr.use_intra_dc_vlc ? 1 : 0); // default value (no use intra ac vld for dc value) is 1
        if(mp4_state->hdr.h263_aic)
            i=0;
        do // event vld
        {
            event = mp4_state->vld_intra_fun(ld);
            if(event.run == -1)
            {
                tmp = 0x40000;            // the last run-level value in current block
                VERegWriteD(IQ_IDCTINPUT_REG,tmp);
                return -1;
            }

            i+= event.run;

            if(event.level < 0)
                tmp = (uint32_t)((event.run&0x3f)<<12)|(uint32_t)(((-event.level)&0x7ff)|0x800);
            else
                tmp = (uint32_t)((event.run&0x3f)<<12)|(uint32_t)(event.level&0x7ff);

            if(i>=64 || event.last)
                tmp |= 0x40000;            // the last run-level value in current block
            VERegWriteD(IQ_IDCTINPUT_REG,tmp);

            if(i>=64)
                return -1;

            i++;
        } while (! event.last);
    }

    return 0;
}

/***/

int32_t mp4_blockInter(MP4_STREAM *ld, MP4_STATE *mp4_state)
{
    event_t event;
    int32_t i;
    uint32_t tmp;

    mp4_check_iqis_in_empty();

    i = 0;
    do // event vld
    {
        event = mp4_state->vld_inter_fun(ld);
        if(event.run == -1)
        {
            tmp = 0x40000;            // the last run-level value in current block
            VERegWriteD(IQ_IDCTINPUT_REG,tmp);
            return -1;
        }

        i+=event.run;

        if(event.level < 0)
            tmp = (uint32_t)((event.run&0x3f)<<12)|(uint32_t)(((-event.level)&0x7ff)|0x800);
        else
            tmp = (uint32_t)((event.run&0x3f)<<12)|(uint32_t)(event.level&0x7ff);

        if(i>=64 || event.last)
            tmp |= 0x40000;            // the last run-level value in current block
        VERegWriteD(IQ_IDCTINPUT_REG,tmp);
        if(i>=64)
            return -1;

        i++;
    } while (! event.last);

    return 0;
}

void mp4_config_interrupt_register(void)
{
    volatile uint32_t* ve_top_reg;
    volatile uint32_t* ve_intr_ctrl_reg;
    uint32_t 		  mode;

    ve_top_reg = (uint32_t*)ve_get_reglist(REG_GROUP_VETOP);

    mode = (*ve_top_reg) & 0xf;
    
	/* estimate Which video format */
    switch (mode)
    {
        case 0: //mpeg124
            //ve_status_reg = (int *)(addrs.regs_macc + 0x100 + 0x1c);
            ve_intr_ctrl_reg = (uint32_t *)((uint32_t)ve_get_reglist(REG_GROUP_MPEG_DECODER) + 0x14);
        	*ve_intr_ctrl_reg |= 0x7c;
            break;
        default:
            break;
    }
    
    return;
}

void mp4_clear_status_reg(void)
{
    volatile uint32_t* ve_top_reg;
    volatile uint32_t* ve_status_reg;
    uint32_t 		  mode;

    ve_top_reg = (uint32_t*)ve_get_reglist(REG_GROUP_VETOP);

    mode = (*ve_top_reg) & 0xf;
    
	/* estimate Which video format */
    switch (mode)
    {
        case 0: //mpeg124
            //ve_status_reg = (int *)(addrs.regs_macc + 0x100 + 0x1c);
            ve_status_reg = (uint32_t *)((uint32_t)ve_get_reglist(REG_GROUP_MPEG_DECODER) + 0x1c);
        	*ve_status_reg |= 0xf;
            break;
        default:
            break;
    }
    
    mp4_config_interrupt_register();
    
    return;
}

void mp4_ve_mode_select(uint8_t mode)
{
    volatile uint32_t* ve_top_reg;
    uint32_t           tmp;

    ve_top_reg = (uint32_t*)ve_get_reglist(REG_GROUP_VETOP);
    
	/* estimate Which video format */
    switch (mode)
    {
        case VE_MODE_MPEG124: //mpeg124
        	tmp = *ve_top_reg;
        	tmp &= ~0xf;
        	tmp |= VE_MODE_MPEG124;
        	*ve_top_reg = tmp;
            break;
        default:
            break;
    }
    
    return;
}


void mp4_set_deblk_dram_buf(MP4_STATE* s)
{
    //* set VE Top 0x54 register.
	uint32_t      tmp;
	volatile vetop_reglist_t* listVeTop;

    listVeTop = ve_get_reglist(REG_GROUP_VETOP);

    tmp = GetRegValue(listVeTop->_50_deblk_intrapred_buf_ctrl);
    tmp &= ~0x3;
    tmp |= 0x2;
    SetRegValue(listVeTop->_50_deblk_intrapred_buf_ctrl, tmp);

    tmp = (uint32_t)AdapterMemGetPhysicAddress((void*)s->deblk_buf);

    SetRegValue(listVeTop->_54_deblk_dram_buf, tmp);
}

