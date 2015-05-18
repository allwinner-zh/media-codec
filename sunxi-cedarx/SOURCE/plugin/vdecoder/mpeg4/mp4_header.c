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


//* these defines are for 3gp H263 decoding;
/* picture types */
#define PCT_INTRA                       0
#define PCT_INTER                       1
#define PCT_IPB                         2
#define PCT_B                           3
#define PCT_EI                          4
#define PCT_EP                          5
#define PCT_PB                          6

/* pb frame type */
#define PB_FRAMES                       1
#define IM_PB_FRAMES                    2

#define verify(x) if (! (x)) return 0

int32_t divround(int32_t v1, int32_t v2) { return (v1+v2/2)/v2; }

int32_t log2ceil(int32_t arg)
{
	int32_t j=0;
	int32_t i=1;
	if(arg==0)
		return 0;
	while (arg>i)
	{
		i*=2;
		j++;
	}
	return j;
}

void mp4_next_start_code(MP4_STREAM * _ld)
{
	MP4_STREAM * ld = _ld;
	//MP4_STATE * mp4_state = _mp4_state;

	mp4_getbits(ld, 1);
	mp4_bytealign(ld);
}

void mp4_next_resync_marker(MP4_STREAM * ld)
{
	mp4_next_start_code(ld);
}

/************************************************************************/
/* Return: 0--no resync, 1--has resyc                                   */
/************************************************************************/
int16_t mp4_get_resync_marker(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;

	if (mp4_state->hdr.resync_marker_disable == 0)
	{
		int32_t code = mp4_showbits(ld, mp4_state->hdr.resync_length);
		if (code != 1)
		{
			return 0;
		}
		else
		{
			mp4_bytealign(ld); // it should already be mp4_bytealigned (mp4_next_resync_marker)
			mp4_getbits(ld, mp4_state->hdr.resync_length);

			return 1;
		}
	}

	return 0;
}

int32_t mp4_check_stuffingcode(MP4_STREAM * ld, int32_t skipcnt)
{
	int32_t code, i;

	// verify stuffing bits
	code = mp4_showbits(ld, skipcnt);
	for (i = 0; i < skipcnt-1; i++, code >>= 1) {
		if ((code & 1) != 1)
			return 0;
	}
	if ((code & 1) != 0)
		return 0;

	return 1; // valid stuffing code
}

// returns the next nbit bits startign from the next mp4_bytealigned position
// requires that the stuffing bits are correct (0111...) in mpeg-4 syntax
int32_t mp4_nextbits_bytealigned(MP4_STREAM * _ld, int32_t nbit, int32_t short_video_header,
                            int32_t *skip_cnt)
{
	MP4_STREAM * ld = _ld;

	int32_t code;
	int32_t skipcnt = 0;

	if (mp4_bytealigned(ld, skipcnt))
	{
		// stuffing bits
		if (mp4_showbits(ld, 8) == 127) {
			skipcnt += 8;
		}
	}
	else
	{
		// count skipbits until mp4_bytealign
		while (! mp4_bytealigned(ld, skipcnt)) {
			skipcnt += 1;
		}
	}

	// verify stuffing bits here
	if ((! short_video_header) && (! mp4_check_stuffingcode(ld, skipcnt)))
		return -1;

	code = mp4_showbits(ld, nbit + skipcnt);
    if(skip_cnt!=NULL)
    {
        *skip_cnt = skipcnt;
    }
	return (code & msk[nbit]);
}

void mp4_getusrhdr(MP4_STREAM * ld, MP4_STATE * mp4_state)
{
	int8_t aucBuffer[64];
//	int8_t * pBuffer = aucBuffer;
	int8_t *pucTmp = NULL;
	int32_t iBufferPos = 0;

	mp4_getbits(ld, 32); // user data start code
	memset(aucBuffer, 0, 64);

	mp4_state->history_prefixed = 0;
	mp4_state->prefixed = 0;

	while((mp4_showbits(ld, 24) != 0x01))
	{
		aucBuffer[iBufferPos] = (int8_t)mp4_getbits(ld, 8); // right now I just parse through the data without using it
		iBufferPos++;

		if (iBufferPos == 64) {
			iBufferPos = 0;
		}
	}
	
}

int16_t mp4_getvoshdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;

	int32_t code = mp4_showbits(ld, 32);
	if (code == VOS_START_CODE)
	{
		mp4_getbits(ld, 24);
		mp4_getbits(ld, 8);
		mp4_getbits(ld, 8); // profile and level indication
		while (mp4_showbits(ld, 24) != 0x000001)
			mp4_getbits(ld, 8);
		while (mp4_showbits(ld, 32) == USR_START_CODE)
			mp4_getusrhdr(ld, mp4_state);
	}

	return 0;
}

void mp4_video_signal_type(MP4_STREAM * ld)
{
	int32_t coulour_description;
	int32_t video_signal_type;

	video_signal_type = mp4_getbits(ld, 1);
	if (video_signal_type)
	{
		mp4_getbits(ld, 3);
		mp4_getbits(ld, 1);
		coulour_description = mp4_getbits(ld, 1);
		if (coulour_description)
		{
			mp4_getbits(ld, 8);
			mp4_getbits(ld, 8);
			mp4_getbits(ld, 8);
		}
	}
}

int16_t mp4_getvsohdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * p_mp4_state;
	
	int32_t video_id = 1, still_texture_id = 2/*,mesh_id = 3, face_id = 4*/;

	p_mp4_state = _mp4_state;

	int32_t code = mp4_showbits(ld, 32);
	if (code == VSO_START_CODE)
	{
		int32_t is_visual_object_id;
		int32_t visual_object_type;

		mp4_getbits(ld, 24);
		mp4_getbits(ld, 8);

		is_visual_object_id = mp4_getbits(ld, 1);
		if (is_visual_object_id)
		{
			mp4_getbits(ld, 4);
			mp4_getbits(ld, 3);
		}

		visual_object_type = mp4_getbits(ld, 4);

		if ((visual_object_type == video_id) ||
			(visual_object_type == still_texture_id))
		{
			mp4_video_signal_type(ld);
		}

		return 1;
	}

	return 0;
}

void mp4_get_complexityestimationhdr(MP4_STREAM * ld, MP4_STATE * mp4_state)
{
	mp4_state->hdr.estimation_method = mp4_getbits(ld, 2);
	if ((mp4_state->hdr.estimation_method == 0)  ||
		(mp4_state->hdr.estimation_method == 1))
	{
		mp4_state->hdr.shape_complexity_estimation_disable = mp4_getbits(ld, 1);
		if (!  mp4_state->hdr.shape_complexity_estimation_disable) {
			mp4_state->hdr.opaque = mp4_getbits(ld, 1);
			mp4_state->hdr.transparent = mp4_getbits(ld, 1);
			mp4_state->hdr.intra_cae = mp4_getbits(ld, 1);
			mp4_state->hdr.inter_cae = mp4_getbits(ld, 1);
			mp4_state->hdr.no_update = mp4_getbits(ld, 1);
			mp4_state->hdr.upsampling = mp4_getbits(ld, 1);
		}
		mp4_state->hdr.texture_complexity_estimation_set_1_disable = mp4_getbits(ld, 1);
		if (! mp4_state->hdr.texture_complexity_estimation_set_1_disable) {
			mp4_state->hdr.intra_blocks = mp4_getbits(ld, 1);
			mp4_state->hdr.inter_blocks = mp4_getbits(ld, 1);
			mp4_state->hdr.inter4v_blocks = mp4_getbits(ld, 1);
			mp4_state->hdr.not_coded_blocks = mp4_getbits(ld, 1);
		}
		mp4_getbits(ld, 1); // marker bit
		mp4_state->hdr.texture_complexity_estimation_set_2_disable = mp4_getbits(ld, 1);
		if (! mp4_state->hdr.texture_complexity_estimation_set_2_disable) {
			mp4_state->hdr.dct_coefs = mp4_getbits(ld, 1);
			mp4_state->hdr.dct_lines = mp4_getbits(ld, 1);
			mp4_state->hdr.vlc_symbols = mp4_getbits(ld, 1);
			mp4_state->hdr.vlc_bits = mp4_getbits(ld, 1);
		}
		mp4_state->hdr.motion_compensation_complexity_disable = mp4_getbits(ld, 1);
		if (! mp4_state->hdr.motion_compensation_complexity_disable) {
			mp4_state->hdr.apm = mp4_getbits(ld, 1);
			mp4_state->hdr.npm = mp4_getbits(ld, 1);
			mp4_state->hdr.interpolate_mc_q = mp4_getbits(ld, 1);
			mp4_state->hdr.forw_back_mc_q = mp4_getbits(ld, 1);
			mp4_state->hdr.halfpel2 = mp4_getbits(ld, 1);
			mp4_state->hdr.halfpel4 = mp4_getbits(ld, 1);
		}
		mp4_getbits(ld, 1); // marker bit

		if (mp4_state->hdr.estimation_method == 1)
		{
			mp4_state->hdr.version2_complexity_estimation_disable = mp4_getbits(ld, 1);
			if (! mp4_state->hdr.version2_complexity_estimation_disable) {
				mp4_state->hdr.sadct = mp4_getbits(ld, 1);
				mp4_state->hdr.quarterpel = mp4_getbits(ld, 1);
			}
		}
	}
}

#define EXTENDED_PAR 0x000f

int16_t mp4_getvolhdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;

	int32_t code = mp4_showbits(ld, 32);
	if ((code >= VO_START_CODE_MIN) &&
		(code <= VO_START_CODE_MAX))
	{
		mp4_getbits(ld, 24);
		mp4_getbits(ld, 8);
	}
	uint32_t nextCode;
	nextCode = mp4_showbits(ld, 24);
	while ((nextCode != 0x000001) && (nextCode != 0x0))
		mp4_getbits(ld, 8);

    //clear new frame rate
    mp4_state->newfrmrate = 0;

	code = mp4_showbits(ld, 28);
	if (code == VOL_START_CODE)
	{
		mp4_getbits(ld, 24);
		mp4_getbits(ld, 4);

		mp4_state->hdr.flag_keyframe = 1;

		mp4_state->hdr.ident = mp4_getbits(ld, 4); // vol_id
		mp4_state->hdr.random_accessible_vol = mp4_getbits(ld, 1);
		mp4_state->hdr.type_indication = mp4_getbits(ld, 8);
		mp4_state->hdr.is_object_layer_identifier = mp4_getbits(ld, 1);

		if (mp4_state->hdr.is_object_layer_identifier) {
			mp4_state->hdr.visual_object_layer_verid = mp4_getbits(ld, 4);
			mp4_state->hdr.visual_object_layer_priority = mp4_getbits(ld, 3);
		}
		else {
			mp4_state->hdr.visual_object_layer_verid = 1;
			mp4_state->hdr.visual_object_layer_priority = 1;
		}
		mp4_state->hdr.aspect_ratio_info = mp4_getbits(ld, 4);
		if (mp4_state->hdr.aspect_ratio_info == EXTENDED_PAR)
		{
			mp4_state->hdr.par_width = mp4_getbits(ld, 8);
			mp4_state->hdr.par_height = mp4_getbits(ld, 8);

			//assert(mp4_state->hdr.par_width != 0);
			//assert(mp4_state->hdr.par_height != 0);
		}
		mp4_state->hdr.vol_control_parameters = mp4_getbits(ld, 1);
		if (mp4_state->hdr.vol_control_parameters)
		{
			mp4_state->hdr.chroma_format = mp4_getbits(ld, 2);
			mp4_state->hdr.low_delay = mp4_getbits(ld, 1);
			mp4_state->hdr.vbv_parameters = mp4_getbits(ld, 1);
			if (mp4_state->hdr.vbv_parameters)
			{
				mp4_state->hdr.first_half_bit_rate = mp4_getbits(ld, 15);
				mp4_getbits1(ld); // marker
				mp4_state->hdr.latter_half_bit_rate = mp4_getbits(ld, 15);
				mp4_getbits1(ld); // marker
				mp4_state->hdr.first_half_vbv_buffer_size = mp4_getbits(ld, 15);
				mp4_getbits1(ld); // marker
				mp4_state->hdr.latter_half_vbv_buffer_size = mp4_getbits(ld, 3);
				mp4_state->hdr.first_half_vbv_occupancy = mp4_getbits(ld, 11);
				mp4_getbits1(ld); // marker
				mp4_state->hdr.latter_half_vbv_occupancy = mp4_getbits(ld, 15);
				mp4_getbits1(ld); // marker
			}
		}
		mp4_state->hdr.shape = mp4_getbits(ld, 2);
		if(mp4_state->hdr.shape != RECTANGULAR)
            return -1;

		if ((mp4_state->hdr.shape == GRAY_SCALE) &&
			(mp4_state->hdr.visual_object_layer_verid != 0x01)) {
			mp4_state->hdr.video_object_layer_shape_extension = mp4_getbits(ld, 4);
			return -1;
		}

		mp4_getbits1(ld); // marker
		mp4_state->hdr.time_increment_resolution = mp4_getbits(ld, 16);
		mp4_state->hdr.test_timeinc = -2;
		mp4_getbits1(ld); // marker
		mp4_state->hdr.fixed_vop_rate = mp4_getbits(ld, 1);

		if (mp4_state->hdr.fixed_vop_rate)
		{
			int32_t bits = log2ceil(mp4_state->hdr.time_increment_resolution);
			if (bits < 1)
				bits = 1;
			mp4_state->hdr.fixed_vop_time_increment = mp4_getbits(ld, bits);

            //update frame rate
            if(mp4_state->hdr.fixed_vop_time_increment)
            {
                mp4_state->newfrmrate = (int64_t)mp4_state->hdr.time_increment_resolution*1000 / mp4_state->hdr.fixed_vop_time_increment;
            }
		}

		if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY)
		{
			if(mp4_state->hdr.shape == RECTANGULAR)
			{
				mp4_getbits1(ld); // marker
				mp4_state->hdr.width = (uint16_t)mp4_getbits(ld, 13);
				mp4_getbits1(ld); // marker
				mp4_state->hdr.height = (uint16_t)mp4_getbits(ld, 13);
				mp4_getbits1(ld); // marker
			}

			mp4_state->hdr.interlaced = mp4_getbits(ld, 1);
			mp4_state->hdr.obmc_disable = mp4_getbits(ld, 1);

			if((mp4_state->hdr.time_increment_resolution == 0) ||
				(mp4_state->hdr.time_increment_resolution == 1))
			{
				mp4_state->hdr.obmc_disable = 1;
			}
			if (mp4_state->hdr.visual_object_layer_verid == 1) {
				mp4_state->hdr.sprite_usage = mp4_getbits(ld, 1);
			}
			else {
				mp4_state->hdr.sprite_usage = mp4_getbits(ld, 2);
			}

			if((mp4_state->hdr.sprite_usage == STATIC_SPRITE) || (mp4_state->hdr.sprite_usage == GMC_SPRITE))
			{
				if(mp4_state->hdr.sprite_usage != GMC_SPRITE)
				{
					mp4_getbits(ld, 13); // sprite_width
					mp4_getbits(ld, 1); // marker
					mp4_getbits(ld, 13); // sprite_height
					mp4_getbits(ld, 1); // marker
					mp4_getbits(ld, 13); // sprite_left_coordinate
					mp4_getbits(ld, 1); // marker
					mp4_getbits(ld, 13); // sprite_top_coordinate
					mp4_getbits(ld, 1); // marker
				}
				mp4_state->hdr.no_of_sprite_warping_points=mp4_getbits(ld, 6);
				mp4_state->hdr.sprite_warping_accuracy=mp4_getbits(ld, 2);
				// 00: halfpel
				// 01: 1/4 pel
				// 10: 1/8 pel
				// 11: 1/16 pel

				mp4_state->hdr.sprite_brightness_change=mp4_getbits(ld, 1);
				if(mp4_state->hdr.sprite_usage != GMC_SPRITE)
					mp4_state->hdr.low_latency_sprite_enable=mp4_getbits(ld, 1);
			}

            if(mp4_state->hdr.sprite_usage == STATIC_SPRITE)
                return -1;

			if ((mp4_state->hdr.visual_object_layer_verid != 0x01) &&
				(mp4_state->hdr.shape != RECTANGULAR)) {
				mp4_state->hdr.sadct_disable = mp4_getbits(ld, 1);
			}

			mp4_state->hdr.not_8_bit = mp4_getbits(ld, 1);
			if (mp4_state->hdr.not_8_bit)
			{
                return -1;
			}
			else
			{
				mp4_state->hdr.quant_precision = 5;
				mp4_state->hdr.bits_per_pixel = 8;
			}

			if(mp4_state->hdr.shape == GRAY_SCALE)
			{
				mp4_getbits(ld, 1); // no_gray_quant_update
				mp4_getbits(ld, 1); // composition_method
				mp4_getbits(ld, 1); // linear_composition
			}

			mp4_state->hdr.quant_type = mp4_getbits(ld, 1); // quant type

			if (mp4_state->hdr.quant_type)
			{
				mp4_state->hdr.load_intra_quant_matrix = mp4_getbits(ld, 1);
				if(mp4_state->hdr.load_intra_quant_matrix) 
					{
					// load intra quant matrix
					uint32_t val;
					int32_t i, k = 0;
					do {
						val = mp4_getbits(ld, 8);
						mp4_state->hdr.intra_quant_matrix[zig_zag_scan[k]] = val;
						k++;
					} while ((k < 64) && (val != 0));
					if(k<64)
						k--;
					for (i = k; i < 64; i++) {
						mp4_state->hdr.intra_quant_matrix[zig_zag_scan[i]] =
							mp4_state->hdr.intra_quant_matrix[zig_zag_scan[k-1]];
					}
				}
				else
					memcpy(mp4_state->hdr.intra_quant_matrix, intra_quant_matrix, sizeof(intra_quant_matrix));

				mp4_state->hdr.load_nonintra_quant_matrix = mp4_getbits(ld, 1);
				if (mp4_state->hdr.load_nonintra_quant_matrix) {
					// load nonintra quant matrix
					uint32_t val;
					int32_t i, k = 0;
					do {
						val = mp4_getbits(ld, 8);
						mp4_state->hdr.nonintra_quant_matrix[zig_zag_scan[k]] = val;
						k++;
					} while ((k < 64) && (val != 0));
					if(k<64)
						k--;
					for (i = k; i < 64; i++) {
						mp4_state->hdr.nonintra_quant_matrix[zig_zag_scan[i]] =
							mp4_state->hdr.nonintra_quant_matrix[zig_zag_scan[k-1]];
					}
				}
				else
					memcpy(mp4_state->hdr.nonintra_quant_matrix, nonintra_quant_matrix, sizeof(nonintra_quant_matrix));

				if(mp4_state->hdr.shape == GRAY_SCALE)
				{
				}
			}

			if (mp4_state->hdr.visual_object_layer_verid/*ident*/ != 1) {
				mp4_state->hdr.quarter_pixel = mp4_getbits(ld, 1);
			} else {
				mp4_state->hdr.quarter_pixel = 0;
			}

			mp4_state->hdr.complexity_estimation_disable = mp4_getbits(ld, 1);
			if (! mp4_state->hdr.complexity_estimation_disable)
				mp4_get_complexityestimationhdr(ld, mp4_state);

			mp4_state->hdr.resync_marker_disable = mp4_getbits(ld, 1);
			mp4_state->hdr.data_partitioning = mp4_getbits(ld, 1);
			if (mp4_state->hdr.data_partitioning) {
				mp4_state->hdr.reversible_vlc = mp4_getbits(ld, 1);
			}

			if (mp4_state->hdr.visual_object_layer_verid != 1) {
				mp4_state->hdr.newpred_enable = mp4_getbits(ld, 1); // newpred
				if (mp4_state->hdr.newpred_enable) {
                    return -1;
				}
				mp4_state->hdr.reduced_resolution_vop_enable = mp4_getbits(ld, 1);
			}

			mp4_state->hdr.intra_acdc_pred_disable = 0;

			mp4_state->hdr.scalability = mp4_getbits(ld, 1);
            if ((mp4_state->hdr.scalability) && (mp4_state->userdata_codec_version != 412))
                return -1;
		}

		mp4_bytealign(ld);
		if ((mp4_showbits(ld, 24) != 0x01) && ((mp4_showbits(ld, 32) & 0x00ffffff) == 0x01) )
			mp4_next_start_code(ld);
		if (mp4_showbits(ld, 32) == USR_START_CODE) {
			mp4_getusrhdr(ld, mp4_state);
		}
		mp4_state->hdr.tframe = -1;
		return 0;
	}

	return 0; // no VO start code
}

void mp4_read_vop_complexity_estimation_header(MP4_STREAM * ld, MP4_STATE * mp4_state)
{
	if ((mp4_state->hdr.estimation_method == 0) || (mp4_state->hdr.estimation_method == 1))
	{
		if (mp4_state->hdr.prediction_type == MP4_I_VOP)
		{
			if (mp4_state->hdr.opaque)
				mp4_state->hdr.dcecs_opaque = mp4_getbits(ld, 8);
			if (mp4_state->hdr.transparent)
				mp4_state->hdr.dcecs_transparent = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_cae)
				mp4_state->hdr.dcecs_intra_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter_cae)
				mp4_state->hdr.dcecs_inter_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.no_update)
				mp4_state->hdr.dcecs_no_update = mp4_getbits(ld, 8);
			if (mp4_state->hdr.upsampling)
				mp4_state->hdr.dcecs_upsampling = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_blocks)
				mp4_state->hdr.dcecs_intra_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.not_coded_blocks)
				mp4_state->hdr.dcecs_not_coded_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_coefs)
				mp4_state->hdr.dcecs_dct_coefs = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_lines)
				mp4_state->hdr.dcecs_dct_lines = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_symbols)
				mp4_state->hdr.dcecs_vlc_symbols = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_bits)
				mp4_state->hdr.dcecs_vlc_bits = mp4_getbits(ld, 4);
			if (mp4_state->hdr.sadct)
				mp4_state->hdr.dcecs_sadct = mp4_getbits(ld, 8);
		}
		if (mp4_state->hdr.prediction_type == MP4_P_VOP)
		{
			if (mp4_state->hdr.opaque)
				mp4_state->hdr.dcecs_opaque = mp4_getbits(ld, 8);
			if (mp4_state->hdr.transparent)
				mp4_state->hdr.dcecs_transparent = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_cae)
				mp4_state->hdr.dcecs_intra_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter_cae)
				mp4_state->hdr.dcecs_inter_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.no_update)
				mp4_state->hdr.dcecs_no_update = mp4_getbits(ld, 8);
			if (mp4_state->hdr.upsampling)
				mp4_state->hdr.dcecs_upsampling = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_blocks)
				mp4_state->hdr.dcecs_intra_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.not_coded_blocks)
				mp4_state->hdr.dcecs_not_coded_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_coefs)
				mp4_state->hdr.dcecs_dct_coefs = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_lines)
				mp4_state->hdr.dcecs_dct_lines = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_symbols)
				mp4_state->hdr.dcecs_vlc_symbols = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_bits)
				mp4_state->hdr.dcecs_vlc_bits = mp4_getbits(ld, 4);
			if (mp4_state->hdr.inter_blocks)
				mp4_state->hdr.dcecs_inter_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter4v_blocks)
				mp4_state->hdr.dcecs_inter4v_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.apm)
				mp4_state->hdr.dcecs_apm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.npm)
				mp4_state->hdr.dcecs_npm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.forw_back_mc_q)
				mp4_state->hdr.dcecs_forw_back_mc_q = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel2)
				mp4_state->hdr.dcecs_halfpel2 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel4)
				mp4_state->hdr.dcecs_halfpel4 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.sadct)
				mp4_state->hdr.dcecs_sadct = mp4_getbits(ld, 8);
			if (mp4_state->hdr.quarterpel)
				mp4_state->hdr.dcecs_quarterpel = mp4_getbits(ld, 8);
		}
		if (mp4_state->hdr.prediction_type == MP4_B_VOP)
		{
			if (mp4_state->hdr.opaque)
				mp4_state->hdr.dcecs_opaque = mp4_getbits(ld, 8);
			if (mp4_state->hdr.transparent)
				mp4_state->hdr.dcecs_transparent = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_cae)
				mp4_state->hdr.dcecs_intra_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter_cae)
				mp4_state->hdr.dcecs_inter_cae = mp4_getbits(ld, 8);
			if (mp4_state->hdr.no_update)
				mp4_state->hdr.dcecs_no_update = mp4_getbits(ld, 8);
			if (mp4_state->hdr.upsampling)
				mp4_state->hdr.dcecs_upsampling = mp4_getbits(ld, 8);
			if (mp4_state->hdr.intra_blocks)
				mp4_state->hdr.dcecs_intra_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.not_coded_blocks)
				mp4_state->hdr.dcecs_not_coded_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_coefs)
				mp4_state->hdr.dcecs_dct_coefs = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_lines)
				mp4_state->hdr.dcecs_dct_lines = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_symbols)
				mp4_state->hdr.dcecs_vlc_symbols = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_bits)
				mp4_state->hdr.dcecs_vlc_bits = mp4_getbits(ld, 4);
			if (mp4_state->hdr.inter_blocks)
				mp4_state->hdr.dcecs_inter_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter4v_blocks)
				mp4_state->hdr.dcecs_inter4v_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.apm)
				mp4_state->hdr.dcecs_apm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.npm)
				mp4_state->hdr.dcecs_npm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.forw_back_mc_q)
				mp4_state->hdr.dcecs_forw_back_mc_q = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel2)
				mp4_state->hdr.dcecs_halfpel2 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel4)
				mp4_state->hdr.dcecs_halfpel4 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.interpolate_mc_q)
				mp4_state->hdr.dcecs_interpolate_mc_q = mp4_getbits(ld, 8);
			if (mp4_state->hdr.sadct)
				mp4_state->hdr.dcecs_sadct = mp4_getbits(ld, 8);
			if (mp4_state->hdr.quarterpel)
				mp4_state->hdr.dcecs_quarterpel = mp4_getbits(ld, 8);
		}
		if ((mp4_state->hdr.prediction_type == MP4_S_VOP) &&
			(mp4_state->hdr.sprite_usage == STATIC_SPRITE))
		{
			if (mp4_state->hdr.intra_blocks)
				mp4_state->hdr.dcecs_intra_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.not_coded_blocks)
				mp4_state->hdr.dcecs_not_coded_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_coefs)
				mp4_state->hdr.dcecs_dct_coefs = mp4_getbits(ld, 8);
			if (mp4_state->hdr.dct_lines)
				mp4_state->hdr.dcecs_dct_lines = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_symbols)
				mp4_state->hdr.dcecs_vlc_symbols = mp4_getbits(ld, 8);
			if (mp4_state->hdr.vlc_bits)
				mp4_state->hdr.dcecs_vlc_bits = mp4_getbits(ld, 4);
			if (mp4_state->hdr.inter_blocks)
				mp4_state->hdr.dcecs_inter_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.inter4v_blocks)
				mp4_state->hdr.dcecs_inter4v_blocks = mp4_getbits(ld, 8);
			if (mp4_state->hdr.apm)
				mp4_state->hdr.dcecs_apm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.npm)
				mp4_state->hdr.dcecs_npm = mp4_getbits(ld, 8);
			if (mp4_state->hdr.forw_back_mc_q)
				mp4_state->hdr.dcecs_forw_back_mc_q = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel2)
				mp4_state->hdr.dcecs_halfpel2 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.halfpel4)
				mp4_state->hdr.dcecs_halfpel4 = mp4_getbits(ld, 8);
			if (mp4_state->hdr.interpolate_mc_q)
				mp4_state->hdr.dcecs_interpolate_mc_q = mp4_getbits(ld, 8);
		}
	}
}

/***/

int16_t mp4_getgophdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;
	int32_t h,m,s;

	if (mp4_nextbits(ld, 32) == GOP_START_CODE) // [Ag][Review] possible bug, it's not possible to read 32 bits
	{
		mp4_getbits(ld, 24);
		mp4_getbits(ld, 8);

		mp4_state->hdr.time_code = mp4_getbits(ld, 18);
		mp4_state->hdr.closed_gov = mp4_getbits(ld, 1);
		mp4_state->hdr.broken_link = mp4_getbits(ld, 1);

		//Add time code processing in gop header
		s = mp4_state->hdr.time_code & 0x3f;
		m = (mp4_state->hdr.time_code>>7) & 0x3f;
		h = (mp4_state->hdr.time_code>>13) & 0x1f;
		mp4_state->hdr.time_base = h*3600 + m*60 + s;
	}

	return 0;
}

/***/

int16_t mp4_getshvhdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;

	// end of sequence
	if (mp4_nextbits(ld, 22) == SHV_END_MARKER)
	{
		mp4_getbits(ld, 22);
		mp4_state->hdr.short_video_header = 0;

		return 0;
	}

	// start of sequence
	if (mp4_nextbits(ld, 22) != (int32_t) SHV_START_CODE)
	{
		mp4_state->hdr.short_video_header = 0;
		return -1;
	}

	mp4_getbits(ld, 22);
	mp4_state->hdr.short_video_header = 1;
	mp4_state->userdata_codec_version = 263;

	mp4_state->hdr.temporal_reference = mp4_getbits(ld, 8);
	mp4_getbits1(ld); // marker bit
	mp4_getbits1(ld); // zero_bit
	mp4_state->hdr.split_screen_indicator = mp4_getbits(ld, 1);
	mp4_state->hdr.document_camera_indicator = mp4_getbits(ld, 1);
	mp4_state->hdr.full_picture_freeze_release = mp4_getbits(ld, 1);
	mp4_state->hdr.source_format = mp4_getbits(ld, 3);

	switch (mp4_state->hdr.source_format)
	{
	case 1:
		mp4_state->hdr.width = 128;
		mp4_state->hdr.height = 96;
		mp4_state->hdr.num_mb_in_gob = 8;
		mp4_state->hdr.num_gobs_in_vop = 6;
		break;
	case 2:
		mp4_state->hdr.width = 176;
		mp4_state->hdr.height = 144;
		mp4_state->hdr.num_mb_in_gob = 11;
		mp4_state->hdr.num_gobs_in_vop = 9;
		break;
	case 3:
		mp4_state->hdr.width = 352;
		mp4_state->hdr.height = 288;
		mp4_state->hdr.num_mb_in_gob = 22;
		mp4_state->hdr.num_gobs_in_vop = 18;
		break;
	case 4:
		mp4_state->hdr.width = 704;
		mp4_state->hdr.height = 576;
		mp4_state->hdr.num_mb_in_gob = 88;
		mp4_state->hdr.num_gobs_in_vop = 18;
		break;
	case 5:
		mp4_state->hdr.width = 1408;
		mp4_state->hdr.height = 1152;
		mp4_state->hdr.num_mb_in_gob = 352;
		mp4_state->hdr.num_gobs_in_vop = 18;
		break;
	default:
		return -1; // no valid picture format found
	}

	mp4_state->hdr.picture_coding_type = mp4_getbits(ld, 1);
	if (mp4_state->hdr.picture_coding_type == 0) {
		mp4_state->hdr.prediction_type = MP4_I_VOP;
		mp4_state->hdr.flag_keyframe = 1;
	}
	else {
		mp4_state->hdr.prediction_type = MP4_P_VOP;
		mp4_state->hdr.flag_keyframe = 0;
	}

	//assert(mp4_state->hdr.picture_coding_type != B_VOP); // B-VOPs not supported in short header mode

	mp4_state->hdr.four_reserved_zero_bits = mp4_getbits(ld, 4);

	mp4_state->hdr.vop_quant = mp4_getbits(ld, 5);
	mp4_state->hdr.quantizer = mp4_state->hdr.vop_quant;

	mp4_getbits1(ld); // zero_bit

	while (mp4_getbits(ld, 1) == 1)
	{
		mp4_getbits(ld, 8); // pei + psupp mechanism
	}

	// fixed setting for short header
	mp4_state->hdr.shape = RECTANGULAR;
	mp4_state->hdr.obmc_disable = 1;
	mp4_state->hdr.quant_type = 0;
	mp4_state->hdr.resync_marker_disable = 1;
	mp4_state->hdr.data_partitioning = 0;
	mp4_state->hdr.reversible_vlc = 0;
	mp4_state->hdr.rounding_type = 0;
	mp4_state->hdr.fcode_for = 1;
	mp4_state->hdr.vop_coded = 1;
	mp4_state->hdr.interlaced = 0;
	mp4_state->hdr.complexity_estimation_disable = 1;
	mp4_state->hdr.use_intra_dc_vlc = 0;
	mp4_state->hdr.scalability = 0;
	mp4_state->hdr.not_8_bit = 0;
	mp4_state->hdr.bits_per_pixel = 8;

	/***

	  if (mp4_nextbits(22) == SHV_END_MARKER) {
	  mp4_getbits(22);
	  }
	  mp4_bytealign();

	***/

	return 0;
}

int16_t mp4_get_use_intra_dc_vlc(int32_t quantizer, int32_t intra_dc_vlc_thr)
{
	int16_t use_intra_dc_vlc = 1;

	if (intra_dc_vlc_thr == 0) {
		return 1;
	}

	switch (intra_dc_vlc_thr&7)
	{
	case 1:
		if (quantizer >= 13)
			use_intra_dc_vlc = 0;
		break;
	case 2:
		if (quantizer >= 15)
			use_intra_dc_vlc = 0;
		break;
	case 3:
		if (quantizer >= 17)
			use_intra_dc_vlc = 0;
		break;
	case 4:
		if (quantizer >= 19)
			use_intra_dc_vlc = 0;
		break;
	case 5:
		if (quantizer >= 21)
			use_intra_dc_vlc = 0;
		break;
	case 6:
		if (quantizer >= 23)
			use_intra_dc_vlc = 0;
		break;
	case 7:
		use_intra_dc_vlc = 0;
		break;
	default:
		use_intra_dc_vlc = 1;
		break;
	}

	return use_intra_dc_vlc;
}

/***/
/****
Table B-33 -- Code table for the first trajectory point
dmv value	SSS	VLC	dmv_code
-16383 ?-8192, 8192 ?16383	14	111111111110	00000000000000...01111111111111, 10000000000000...11111111111111
-8191 ?-4096, 4096 ?8191	13	11111111110	0000000000000...0111111111111, 1000000000000...1111111111111
-4095 ?-2048, 2048 ?4095	12	1111111110	000000000000...011111111111, 100000000000...111111111111
-2047...-1024, 1024...2047	11	111111110	00000000000...01111111111, 10000000000...11111111111
-1023...-512, 512...1023	10	11111110	0000000000...0111111111, 1000000000...1111111111
-511...-256, 256...511	9	1111110	000000000...011111111, 100000000...111111111
-255...-128, 128...255	8	111110	00000000...01111111, 10000000...11111111
-127...-64, 64...127	7	11110	0000000...0111111, 1000000...1111111
-63...-32, 32...63	6	1110	000000...011111, 100000...111111
-31...-16, 16...31	5	110	00000...01111, 10000...1111
-15...-8, 8...15	4	101	0000...0111, 1000...1111
-7...-4, 4...7	3	100	000...011, 100...111
-3...-2, 2...3	2	011	00...01, 10...11
-1, 1	1	010	0, 1
0	0	00	-
****/

int32_t read_dmv_length(MP4_STREAM* ld)
{
	int32_t retval;
	switch(mp4_getbits(ld, 2))
	{
    default:
	case 0:
		return 0;
	case 1:
		return 1+mp4_getbits1(ld);
	case 2:
		return 3+mp4_getbits1(ld);
	case 3:
		retval=5;
		while(mp4_getbits1(ld) && (retval<=14))
			retval++;
		if(retval==15)
			return -1;
		return retval;
	}
}

int32_t read_dmv_code(MP4_STREAM* ld, int32_t len)
{
    int32_t base;
    if(!len)
		return 0;
    if(mp4_getbits1(ld)==1)
		base=1 << (len-1);
    else
		base=-(1 << len)+1;
    if(len > 1)
		base+=mp4_getbits(ld, len-1);
    return base;
}

int16_t decode_sprite_trajectory(MP4_STATE* mp4_state, MP4_STREAM* ld)
{
    int32_t i;
	int32_t flag=(mp4_state->userdata_codec_version==500)
		&& (mp4_state->userdata_build_number>=370)
		&& (mp4_state->userdata_build_number<=413)
		;
    if((mp4_state->hdr.no_of_sprite_warping_points<0) || (mp4_state->hdr.no_of_sprite_warping_points>3))
        return -1;
    for(i=0; i<mp4_state->hdr.no_of_sprite_warping_points; i++)
    {
        int32_t dmv_length=read_dmv_length(ld);
        if(dmv_length<0)
            return -1;
		mp4_state->hdr.warping_points[i][0]=read_dmv_code(ld, dmv_length);
		if(!flag)
			mp4_getbits1(ld);
        dmv_length=read_dmv_length(ld);
        if(dmv_length<0)
            return -1;
		mp4_state->hdr.warping_points[i][1]=read_dmv_code(ld, dmv_length);
		mp4_getbits1(ld);
    }
    mp4_state->hdr.iEffectiveWarpingPoints = mp4_state->hdr.no_of_sprite_warping_points;
   	while(mp4_state->hdr.iEffectiveWarpingPoints &&
		!(mp4_state->hdr.warping_points[mp4_state->hdr.iEffectiveWarpingPoints-1][0] || mp4_state->hdr.warping_points[mp4_state->hdr.iEffectiveWarpingPoints-1][1]))
		mp4_state->hdr.iEffectiveWarpingPoints--; 
    return 0;
}

/****
brightness_change_factor value	brightness_change_factor_length value	brightness_change_factor_length VLC	brightness_change_factor
-16...-1, 1...16	1	0	00000...01111, 10000...11111
-48...-17, 17...48	2	10	000000...011111, 100000...111111
112...-49, 49...112	3	110	0000000...0111111, 1000000...1111111
113?24	4	1110	000000000...111 111 111
625...1648	4	1111	0000000000?111111111
****/
int32_t decode_brightness_change_factor(MP4_STREAM* ld)
{
    int32_t length=0;
    while(mp4_getbits1(ld) && (length<4))
		length++;
    switch(length)
    {
    case 0:
		if(mp4_getbits1(ld))
			return 1+mp4_getbits(ld, 4);
		else
			return -16+(int32_t)mp4_getbits(ld, 4);
    case 1:
		if(mp4_getbits1(ld))
			return 17+mp4_getbits(ld, 5);
		else
			return -48+(int32_t)mp4_getbits(ld, 5);
    case 2:
		if(mp4_getbits1(ld))
			return 49+mp4_getbits(ld, 6);
		else
			return -112+(int32_t)mp4_getbits(ld, 6);
    case 3:
		return 113+mp4_getbits(ld, 9);
    case 4:
		return 625+mp4_getbits(ld, 10);
    }

	return 0;
}

int32_t div_twoslash(int32_t v1, int32_t v2)
{
    //assert(v2 > 0);
    //assert(!(v2 & 1));
    if(v1>0)
		return (v1 + v2/2) / v2;
    else
		return (v1 - v2/2) / v2;
}

int32_t div_threeslash(int32_t v1, int32_t v2)
{
//    assert(v2 > 0);
  //  assert(!(v2 & 1));
    return (v1 + v2/2) / v2;
}
/***

  Spec tells us the following

	'//' is an integer division with rounding to the nearest integer. Half-integers are rounded away from zero.
	'///' is an integer division with rounding to the nearest integer. Half-integers are rounded up.
	'////' is an integer division with truncation towards minus infinity.

	  i1'' = 16 (i0 + W') + ((W - W') (r i0' - 16 i0) + W' (r i1' - 16 i1)) // W
	  j1'' = 16 j0 + ((W - W') (r j0' - 16 j0) + W' (r j1' - 16 j1)) // W

		in 1/16 pel accuracy, and r = 16/s ( scaler from stream accuracy to 1/16 pel accuracy ).

		  I = i - i0 ( horizontal coordinate relative to p.0 )
		  J = j - j0 ( vertical coordinate relative to p.0 )
		  Ic = 4 ic  - 2 i0 + 1
		  Jc = 4 jc  - 2 j0 + 1

			in these equations W' and H' are lowest powers of 2 larger or equal to width and height of picture, respectively
***/
void calc_affine_transforms_1point(MP4_STATE* mp4_state)
{
	int32_t s = 2 << mp4_state->hdr.sprite_warping_accuracy;
	//int32_t r = 16 / s;

	// with 1 pel accuracy:
	int32_t i0 = 0, j0 = 0;
//	int32_t i1 = mp4_state->hdr.width;

	//int32_t W = mp4_state->hdr.width, H = mp4_state->hdr.height;
	//int32_t W_ = 1 << log2ceil(W), H_ = 1 << log2ceil(H);

	int32_t i0_, j0_;
//	int32_t  i1_, j1_;
	int32_t X0, Y0, XX, YX, XY, YY, rounder, shifter;
	// following equations are unchecked

	// with 1/s pel accuracy
	if((mp4_state->userdata_codec_version == 500) &&
		(mp4_state->userdata_build_number >= 370) &&
		(mp4_state->userdata_build_number <= 413)
		)
	{
		i0_ = s * i0 + mp4_state->hdr.warping_points[0][0];
		j0_ = s * j0 + mp4_state->hdr.warping_points[0][1];
//		i1_ = s * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0];
//		j1_ = s * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1];
	}
	else
	{
		i0_ = (s/2) * (2 * i0 + mp4_state->hdr.warping_points[0][0]);
		j0_ = (s/2) * (2 * j0 + mp4_state->hdr.warping_points[0][1]);
//		i1_ = (s/2) * (2 * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0]);
//		j1_ = (s/2) * (2 * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1]);
	}

 /**
    luminance:
 **/

	XX = s;
	YX = 0;
	XY = 0;
	YY = s;
	shifter = 0;
	rounder = 0;


	mp4_state->at_lum.X0 = i0_;
	mp4_state->at_lum.XX = XX;
	mp4_state->at_lum.YX = YX;
	mp4_state->at_lum.Y0 = j0_;
	mp4_state->at_lum.XY = XY;
	mp4_state->at_lum.YY = YY;
	mp4_state->at_lum.shifter = shifter;
	mp4_state->at_lum.rounder1 = mp4_state->at_lum.rounder2 = rounder;

 /**
    chrominance:
 **/
	XX = s;
	YX = 0;
	XY = 0;
	YY = s;
	shifter = 0;
	rounder = 0;
	X0 = ((i0_>>1)|(i0_&1)) - s*i0/2;
	Y0 = ((j0_>>1)|(j0_&1)) - s*j0/2;


	mp4_state->at_chrom.XX = XX;
	mp4_state->at_chrom.YX = YX;
	mp4_state->at_chrom.XY = XY;
	mp4_state->at_chrom.YY = YY;
	mp4_state->at_chrom.shifter = shifter;
	mp4_state->at_chrom.rounder1 = mp4_state->at_chrom.rounder2 = rounder;
	mp4_state->at_chrom.X0 = X0;
	mp4_state->at_chrom.Y0 = Y0;
}

/***

  Spec tells us the following

	'//' is an integer division with rounding to the nearest integer. Half-integers are rounded away from zero.
	'///' is an integer division with rounding to the nearest integer. Half-integers are rounded up.
	'////' is an integer division with truncation towards minus infinity.

	  i1'' = 16 (i0 + W') + ((W - W') (r i0' - 16 i0) + W' (r i1' - 16 i1)) // W
	  j1'' = 16 j0 + ((W - W') (r j0' - 16 j0) + W' (r j1' - 16 j1)) // W

		in 1/16 pel accuracy, and r = 16/s ( scaler from stream accuracy to 1/16 pel accuracy ).

		  I = i - i0 ( horizontal coordinate relative to p.0 )
		  J = j - j0 ( vertical coordinate relative to p.0 )
		  Ic = 4 ic  - 2 i0 + 1
		  Jc = 4 jc  - 2 j0 + 1

			in these equations W' and H' are lowest powers of 2 larger or equal to width and height of picture, respectively
***/
void calc_affine_transforms_2point(MP4_STATE* mp4_state)
{
	int32_t s = 2 << mp4_state->hdr.sprite_warping_accuracy;
	int32_t r = 16 / s;

	// with 1 pel accuracy:
	int32_t i0 = 0, j0 = 0;
	int32_t i1 = mp4_state->hdr.width, j1 = 0;

	int32_t W = mp4_state->hdr.width/*, H = mp4_state->hdr.height*/;
	int32_t W_ = 1 << log2ceil(W)/*, H_ = 1 << log2ceil(H)*/;

	int32_t i0_, j0_, i1_, j1_;
	int32_t i1__, j1__;
	int32_t X0, Y0, XX, YX, XY, YY, rounder, shifter;
	// following equations are unchecked

	// with 1/s pel accuracy
	if((mp4_state->userdata_codec_version == 500) &&
		(mp4_state->userdata_build_number >= 370) &&
		(mp4_state->userdata_build_number <= 413)
		)
	{
		i0_ = s * i0 + mp4_state->hdr.warping_points[0][0];
		j0_ = s * j0 + mp4_state->hdr.warping_points[0][1];
		i1_ = s * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0];
		j1_ = s * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1];
	}
	else
	{
		i0_ = (s/2) * (2 * i0 + mp4_state->hdr.warping_points[0][0]);
		j0_ = (s/2) * (2 * j0 + mp4_state->hdr.warping_points[0][1]);
		i1_ = (s/2) * (2 * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0]);
		j1_ = (s/2) * (2 * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1]);
	}

	// with 1/16 pel accuracy
	i1__ = 16*(i0+W_) + div_twoslash((W-W_)*(r*i0_-16*i0) + W_*(r*i1_-16*i1), W);
	// (i0 + W_) + ((W-W_)*(i0_-i0) + W_*(i1_-i1)) / W
	j1__ = 16*j0 + div_twoslash((W-W_)*(r*j0_-16*j0) + W_*(r*j1_-16*j1), W);

	/**
    luminance:
    F(i, j) = i0' + ((-r i0' + i1'') I + (r j0' - j1'') J) /// (W' r)
    G(i, j) = j0' + ((-r j0' + j1'') I + (-r i0' + i1'') J) /// (W' r)
	**/

	XX = (-r*i0_ + i1__);
	YX = (r*j0_ - j1__);
	XY = (-r*j0_ + j1__);
	YY = (-r*i0_ + i1__);
	shifter = (int16_t)log2ceil(W_ * r);
	rounder = 1 << (shifter-1);

	while(!((XX | YX | XY | YY | rounder) & 1))
	{
		if(shifter==0)
			break;
		XX >>=1;
		YX >>=1;
		XY >>=1;
		YY >>=1;
		rounder >>=1;
		shifter--;
	}

	mp4_state->at_lum.X0 = i0_;
	mp4_state->at_lum.XX = XX;
	mp4_state->at_lum.YX = YX;
	mp4_state->at_lum.Y0 = j0_;
	mp4_state->at_lum.XY = XY;
	mp4_state->at_lum.YY = YY;
	mp4_state->at_lum.shifter = shifter;
	mp4_state->at_lum.rounder1 = mp4_state->at_lum.rounder2 = rounder;

	/**
    chrominance:
    Fc(ic, jc) = ((-r i0' + i1 '') Ic  + (r j0' - j1'') Jc  + 2 W' r i0' - 16W') /// (4 W' r)
    Gc(ic, jc) = ((-r j0' + j1'') Ic  + (-r i0' + i1'') Jc  + 2 W' r j0' - 16W') /// (4 W' r)
	**/
	XX = (-r*i0_ + i1__);
	YX = (r*j0_ - j1__);
	XY = (-r*j0_ + j1__);
	YY = (-r*i0_ + i1__);
	shifter = (int16_t)log2ceil(4 * W_ * r);
	rounder = 1 << (shifter-1);
	X0 = 2*W_*r*i0_ - 16*W_ + rounder;
	Y0 = 2*W_*r*j0_ - 16*W_ + rounder;

	while(!((X0 | Y0 | XX | YX | XY | YY | rounder) & 1))
	{
		if(shifter==0)
			break;
		X0 >>=1;
		Y0 >>=1;
		XX >>=1;
		YX >>=1;
		XY >>=1;
		YY >>=1;
		rounder >>=1;
		shifter--;
	}

	mp4_state->at_chrom.XX = XX;
	mp4_state->at_chrom.YX = YX;
	mp4_state->at_chrom.XY = XY;
	mp4_state->at_chrom.YY = YY;
	mp4_state->at_chrom.shifter = shifter;
	mp4_state->at_chrom.rounder1 = mp4_state->at_chrom.rounder2 = rounder;
	mp4_state->at_chrom.X0 = X0;
	mp4_state->at_chrom.Y0 = Y0;
}

/***
i1'' = 16 (i0 + W') + ((W - W') (r i0' - 16 i0) + W' (r i1' - 16 i1)) // W
j1'' = 16 j0 + ((W - W') (r j0' - 16 j0) + W' (r j1' - 16 j1)) // W

  i2'' = 16 i0 + ((H - H') (r i0' - 16 i0) + H' (r i2' - 16 i2)) // H,
  j2'' = 16 (j0 + H') + ((H - H') (r j0' - 16 j0) + H' (r j2' - 16 j2)) // H

    luminance:
    F(i, j) = (i0' + ((-r i0' + i1'') H' I + (-r i0'+ i2'')W' J) /// (W'H'r)
    G(i, j) = j0' + ((-r j0' + j1'') H' I + (-r j0'+ j2'')W' J) /// (W'H'r)
    chrominance:
    Fc(ic, jc) = ((-r i0' + i1'') H' Ic  + (-r i0'+ i2'')W' Jc  + 2 W'H'r i0' - 16W'H') /// (4W'H'r)
    Gc(ic, jc) = ((-r j0' + j1'') H' Ic  + (-r j0'+ j2'')W' Jc  + 2 W'H'r j0' - 16W'H') /// (4W'H'r)

***/
void calc_affine_transforms_3point(MP4_STATE* mp4_state)
{
	int32_t s = 2 << mp4_state->hdr.sprite_warping_accuracy;
	//	    const int s = 2;
	int32_t r = 16 / s;
	//	    const int r = 8;

	// with 1 pel accuracy:
	int32_t i0 = 0, j0 = 0;
	int32_t i1 = mp4_state->hdr.width, j1 = 0;
	int32_t i2 = 0, j2 = mp4_state->hdr.height;

	int32_t W = mp4_state->hdr.width, H = mp4_state->hdr.height;
	int32_t W_ = 1 << log2ceil(W), H_ = 1 << log2ceil(H);

	// following equations are unchecked

	int32_t i0_, j0_, i1_, j1_, i2_, j2_;
	int32_t i1__, j1__, i2__, j2__;

    int32_t XX, YX, XY, YY;
	//int32_t X0, Y0;
	int64_t X1, Y1;

	int32_t shifter=log2ceil(W_ * H_ * r);
	int32_t rounder=1 << (shifter-1);

	// with 1/s pel accuracy
	if((mp4_state->userdata_codec_version == 500) &&
		(mp4_state->userdata_build_number >= 370) &&
		(mp4_state->userdata_build_number <= 413)
		)
	{
		i0_ = s * i0 + mp4_state->hdr.warping_points[0][0];
		j0_ = s * j0 + mp4_state->hdr.warping_points[0][1];
		i1_ = s * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0];
		j1_ = s * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1];
		i2_ = s * i2 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[2][0];
		j2_ = s * j2 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[2][1];
	}
	else // mpeg-4 compliant
	{
		i0_ = (s/2) * (2  * i0 + mp4_state->hdr.warping_points[0][0]);
		j0_ = (s/2) * (2  * j0 + mp4_state->hdr.warping_points[0][1]);
		i1_ = (s/2) * (2  * i1 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[1][0]);
		j1_ = (s/2) * (2  * j1 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[1][1]);
		i2_ = (s/2) * (2  * i2 + mp4_state->hdr.warping_points[0][0] + mp4_state->hdr.warping_points[2][0]);
		j2_ = (s/2) * (2 * j2 + mp4_state->hdr.warping_points[0][1] + mp4_state->hdr.warping_points[2][1]);
	}

	// with 1/16 pel accuracy
	i1__ = 16*(i0+W_) + div_twoslash((W-W_)*(r*i0_-16*i0) + W_*(r*i1_-16*i1), W);
	// (i0 + W_) + ((W-W_)*(i0_-i0) + W_*(i1_-i1)) / W
	j1__ = 16*j0 + div_twoslash((W-W_)*(r*j0_-16*j0) + W_*(r*j1_-16*j1), W);

	i2__ = 16*i0 + div_twoslash((H-H_)*(r*i0_-16*i0) + H_*(r*i2_-16*i2), H);
	j2__ = 16*(j0+H_) + div_twoslash((H-H_)*(r*j0_-16*j0) + H_*(r*j2_-16*j2), H);

	XX = ((-r*i0_ + i1__) * H_);
	YX = ((-r*i0_ + i2__) * W_);
	XY = ((-r*j0_ + j1__) * H_);
	YY = ((-r*j0_ + j2__) * W_);

	/**
    luminance:
    F(i, j) = i0' + ((-r i0' + i1'') H' I + (-r i0'+ i2'')W' J) /// (W'H'r)
    G(i, j) = j0' + ((-r j0' + j1'') H' I + (-r j0'+ j2'')W' J) /// (W'H'r)
	**/

	// fixme ( minor )
	// sometimes 16-bit precision is still not enough
	while(!((XX | YX | XY | YY | rounder) & 1))
	{
		if(shifter==0)
			break;
		XX >>=1;
		YX >>=1;
		XY >>=1;
		YY >>=1;
		rounder >>=1;
		shifter--;
	}

	mp4_state->at_lum.X0 = i0_;
	mp4_state->at_lum.XX = XX;
	mp4_state->at_lum.YX = YX;
	mp4_state->at_lum.Y0 = j0_;
	mp4_state->at_lum.XY = XY;
	mp4_state->at_lum.YY = YY;
	mp4_state->at_lum.shifter = shifter;
	mp4_state->at_lum.rounder1 = mp4_state->at_lum.rounder2 = rounder;
	/**
    chrominance:
    Fc(ic, jc) = ((-r i0' + i1'') H' Ic  + (-r i0'+ i2'')W' Jc  + 2 W'H'r i0' - 16W'H') /// (4W'H'r)
    Gc(ic, jc) = ((-r j0' + j1'') H' Ic  + (-r j0'+ j2'')W' Jc  + 2 W'H'r j0' - 16W'H') /// (4W'H'r)
	**/
	XX = ((-r*i0_ + i1__)*H_);
	YX = ((-r*i0_ + i2__)*W_);
	XY = ((-r*j0_ + j1__)*H_);
	YY = ((-r*j0_ + j2__)*W_);

	shifter=log2ceil(4 * W_ * H_ * r);
	rounder=1 << (shifter-1);
//	X0=2*W_*H_*r*i0_ - 16*W_*H_ + rounder;
//	Y0=2*W_*H_*r*j0_ - 16*W_*H_ + rounder;
	X1 = i0_;
	X1 *= 2*W_*H_*r;
	X1 -= 16*W_*H_;
	X1 += rounder;
	Y1 = j0_;
	Y1 *= 2*W_*H_*r;
	Y1 -= 16*W_*H_;
	Y1 += rounder;

	while(!((XX | YX | XY | YY | X1 | Y1 | rounder) & 1))
	{
		if(shifter==0)
			break;
		XX >>=1;
		YX >>=1;
		XY >>=1;
		YY >>=1;
//		X0 >>=1;
//		Y0 >>=1;
		X1 >>=1;
		Y1 >>=1;
		rounder >>=1;
		shifter--;
	}

	mp4_state->at_chrom.XX = XX;
	mp4_state->at_chrom.YX = YX;
	mp4_state->at_chrom.XY = XY;
	mp4_state->at_chrom.YY = YY;
	mp4_state->at_chrom.shifter = shifter;
	mp4_state->at_chrom.rounder1 = mp4_state->at_chrom.rounder2 = rounder;
//	mp4_state->at_chrom.X0 = X0;
//	mp4_state->at_chrom.Y0 = Y0;
	mp4_state->at_chrom.X0 = X1;
	mp4_state->at_chrom.Y0 = Y1;
}

/************************************************
return:
  1---not coded
  0---normal
  -1 ----- error
*************************************************/
int16_t mp4_getvophdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
	MP4_STREAM * ld = _ld;
	MP4_STATE * mp4_state = _mp4_state;

	int32_t display_time, i = 0, ptype;

	mp4_bytealign(ld);

	while(mp4_showbits(ld, 32) != (int32_t) VOP_START_CODE)
	{
		i++;
		mp4_next_start_code(ld);
		if(i>100)
		{
		    logw("mp4_getvophdr 0\n");
			return -1; // vop start code not found
		}
	}
	mp4_flushbits(ld, 32);

	ptype = mp4_getbits(ld, 2);
	mp4_state->hdr.picture_coding_type = ptype;
	if (ptype == MP4_I_VOP)
		mp4_state->hdr.flag_keyframe = 1;
	else
		mp4_state->hdr.flag_keyframe = 0;
	if(ptype != MP4_B_VOP)
	{
		mp4_state->hdr.old_time_base=mp4_state->hdr.time_base;
		display_time = mp4_state->hdr.time_base;
	}
	else
		display_time = mp4_state->hdr.old_time_base;
	while (mp4_getbits(ld, 1) == 1) // temporal time base
	{
		if(ptype != MP4_B_VOP)
			mp4_state->hdr.time_base++;
		display_time++;
	}

	mp4_getbits1(ld); // marker bit
	{
		int32_t bits = log2ceil(mp4_state->hdr.time_increment_resolution);
        
        //logd("mp4_state->hdr.time_increment_resolution(%d)", mp4_state->hdr.time_increment_resolution);
		if (bits < 1)
		{
/*		
			for(bits=1;bits<16;bits++)
			{
				if(mp4_showbits(ld,bits+1)&1)
					break;
			}
*/
            for (bits = 1;
                 bits < 16;
                 bits++) {
                if (mp4_state->hdr.picture_coding_type == MP4_P_VOP ||
                    (mp4_state->hdr.picture_coding_type == MP4_S_VOP &&
                     mp4_state->hdr.sprite_usage == GMC_SPRITE)) {
                    if ((mp4_showbits(ld, bits + 6) & 0x37) == 0x30)
                        break;
                } else if ((mp4_showbits(ld, bits + 5) & 0x1F) == 0x18)
                    break;
            }
            
			mp4_state->hdr.time_increment_resolution = 1<<bits;
		}
        //logd("bits(%d), mp4_state->hdr.time_increment_resolution(%d)", bits, mp4_state->hdr.time_increment_resolution);
		mp4_state->hdr.time_inc = mp4_getbits(ld, bits); // vop_time_increment (1-16 bits)
	}
	mp4_getbits1(ld); // marker bit

	// trb/trd - display_time calculation

	display_time = display_time * mp4_state->hdr.time_increment_resolution + mp4_state->hdr.time_inc;
	if (ptype != MP4_B_VOP) {

		mp4_state->hdr.display_time_prev = mp4_state->hdr.display_time_next;
		mp4_state->hdr.display_time_next = display_time;

		if (display_time != mp4_state->hdr.display_time_prev)
			mp4_state->hdr.trd = mp4_state->hdr.display_time_next - mp4_state->hdr.display_time_prev;
	}
	else {
		mp4_state->hdr.trb = display_time - mp4_state->hdr.display_time_prev;
		// remove from interlaced case to all case
		if(mp4_state->hdr.tframe == -1)
			//Changed interlaced B direct time calculation
			//mp4_state->hdr.tframe = mp4_state->hdr.display_time_next - display_time;
			mp4_state->hdr.tframe = display_time - mp4_state->hdr.display_time_prev;
		if(mp4_state->hdr.tframe == 0)
			mp4_state->hdr.tframe = 1;
		if(mp4_state->hdr.interlaced)
		{

			mp4_state->hdr.trbi=
				2*(divround(display_time,mp4_state->hdr.tframe) - divround(mp4_state->hdr.display_time_prev, mp4_state->hdr.tframe));
			mp4_state->hdr.trdi=
				2*(divround(mp4_state->hdr.display_time_next,mp4_state->hdr.tframe) - divround(mp4_state->hdr.display_time_prev, mp4_state->hdr.tframe));
		}
	}

	mp4_state->hdr.vop_coded = mp4_getbits(ld, 1);
	if (mp4_state->hdr.vop_coded == 0)
	{
		mp4_next_start_code(ld);
        mp4_state->hdr.prediction_type = ptype;
		return 1;
	}

	if(mp4_state->hdr.last_coded_prediction_type != MP4_B_VOP)
	    mp4_state->hdr.old_prediction_type = mp4_state->hdr.last_coded_prediction_type;
	mp4_state->hdr.last_coded_prediction_type = mp4_state->hdr.prediction_type = ptype;

	if(mp4_state->hdr.newpred_enable)
	{
		int32_t bit_length;
//        int32_t vop_id;
		int32_t vop_id_for_prediction_indication;

		bit_length = ((mp4_state->hdr.time_inc+3) < 15) ? (mp4_state->hdr.time_inc+3) : 15;
//		vop_id = mp4_getbits(ld, bit_length);
        mp4_getbits(ld, bit_length);
        vop_id_for_prediction_indication = mp4_getbits(ld, 1);
        if (vop_id_for_prediction_indication)
        {
            /*int32_t vop_id_for_prediction = */mp4_getbits(ld, bit_length);
            mp4_getbits1(ld); // marker bit
        }
	}

	if ((mp4_state->hdr.shape != BINARY_SHAPE_ONLY) &&
		((mp4_state->hdr.prediction_type == MP4_P_VOP) ||
		((mp4_state->hdr.prediction_type == MP4_S_VOP) && mp4_state->hdr.sprite_usage == GMC_SPRITE)))
	{
		mp4_state->hdr.rounding_type = mp4_getbits(ld, 1);
	} else {
		mp4_state->hdr.rounding_type = 0;
	}

	verify(mp4_state->hdr.shape == RECTANGULAR);

	if (mp4_state->hdr.shape != RECTANGULAR)
	{
		if (! (mp4_state->hdr.sprite_usage == STATIC_SPRITE &&
			mp4_state->hdr.prediction_type == MP4_I_VOP) )
		{
			mp4_state->hdr.width = (uint16_t)mp4_getbits(ld, 13);
			mp4_getbits1(ld);
			mp4_state->hdr.height = (uint16_t)mp4_getbits(ld, 13);
			mp4_getbits1(ld);
			mp4_state->hdr.hor_spat_ref = mp4_getbits(ld, 13);
			mp4_getbits1(ld);
			mp4_state->hdr.ver_spat_ref = mp4_getbits(ld, 13);
			mp4_getbits1(ld); // corr
		}

		mp4_state->hdr.change_CR_disable = mp4_getbits(ld, 1);

		mp4_state->hdr.constant_alpha = mp4_getbits(ld, 1);
		if (mp4_state->hdr.constant_alpha) {
			mp4_state->hdr.constant_alpha_value = mp4_getbits(ld, 8);
		}
	}

	//assert(mp4_state->hdr.complexity_estimation_disable == 1);
	if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY)
	{
		if (! mp4_state->hdr.complexity_estimation_disable)
			mp4_read_vop_complexity_estimation_header(ld, mp4_state);
	}

	if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY)
	{
		mp4_state->hdr.intra_dc_vlc_thr = mp4_getbits(ld, 3);

		if(mp4_state->hdr.interlaced == 1)
		{
			mp4_state->hdr.top_field_first = mp4_getbits(ld, 1);
			mp4_state->hdr.alternate_vertical_scan_flag = mp4_getbits(ld, 1);
		}
	}

	if((mp4_state->hdr.prediction_type == MP4_S_VOP) &&
		((mp4_state->hdr.sprite_usage==STATIC_SPRITE) || (mp4_state->hdr.sprite_usage==GMC_SPRITE)))
	{
		if(mp4_state->hdr.no_of_sprite_warping_points>0)
        {
            int16_t err=decode_sprite_trajectory(mp4_state, ld);
            if(err!=0)
            {
                logw("mp4_getvophdr 1\n");
                return err;
            }
        }
		if(mp4_state->hdr.sprite_brightness_change)
			mp4_state->hdr.sprite_brightness_change_factor=decode_brightness_change_factor(ld);
		if(mp4_state->hdr.sprite_usage==STATIC_SPRITE)
		{
		    logw("mp4_getvophdr 2\n");
			return -1;
		}
	}

	if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY)
	{
		mp4_state->hdr.quantizer = mp4_getbits(ld, mp4_state->hdr.quant_precision); // vop quant
		mp4_state->hdr.use_intra_dc_vlc = mp4_get_use_intra_dc_vlc(mp4_state->hdr.quantizer, mp4_state->hdr.intra_dc_vlc_thr);

		if (mp4_state->hdr.prediction_type != MP4_I_VOP)
		{
			mp4_state->hdr.fcode_for = mp4_getbits(ld, 3);
			if(mp4_state->hdr.fcode_for==0)
			{
			    logw("mp4_getvophdr 3\n");
				return -1;
			}
		}
		if (mp4_state->hdr.prediction_type == MP4_B_VOP)
		{
			mp4_state->hdr.fcode_back = mp4_getbits(ld, 3);
		}

		if (! mp4_state->hdr.scalability)
        {
			if (mp4_state->hdr.shape && mp4_state->hdr.prediction_type != MP4_I_VOP)
				mp4_state->hdr.shape_coding_type = mp4_getbits(ld, 1); // vop shape coding type

			/* motion_shape_texture() */
		}
	}

	if(mp4_state->hdr.prediction_type == MP4_B_VOP)
	{
		mp4_state->hdr.resync_length = mmax((16 + mmax(mp4_state->hdr.fcode_for,mp4_state->hdr.fcode_back)),18);
	}else
	{
		mp4_state->hdr.resync_length = (mp4_state->hdr.prediction_type == MP4_I_VOP) ?
			17 : (16 + mp4_state->hdr.fcode_for);
	}

	if(mp4_state->hdr.prediction_type == MP4_S_VOP)
	{
		switch(mp4_state->hdr.no_of_sprite_warping_points)
		{
		case 0:
			/* do nothing */
			break;
		case 1:
			//Add no_of_sprite_warping_points = 1
			calc_affine_transforms_1point(mp4_state);
			break;
		case 2:
			calc_affine_transforms_2point(mp4_state);
			break;
		case 3:
			calc_affine_transforms_3point(mp4_state);
			break;
		default:
			// spec forbids this value to be more than 3 for GMC frames
			logw("mp4_getvophdr 4\n");
			return -1;
		}
		// luma part motion vectors for hw GMC
		mp4_state->hdr.gmc_lum_mv_x = 	mp4_state->at_lum.X0;
		mp4_state->hdr.gmc_lum_mv_y = 	mp4_state->at_lum.Y0;
		mp4_state->hdr.gmc_lum_mv_x <<= (3 - mp4_state->hdr.sprite_warping_accuracy);
		mp4_state->hdr.gmc_lum_mv_y <<= (3 - mp4_state->hdr.sprite_warping_accuracy);
		// chroma part motion vectors for hw GMC
		if(mp4_state->hdr.no_of_sprite_warping_points >= 2)
		{
			mp4_state->hdr.gmc_chrom_mv_x = mp4_state->at_chrom.X0 + mp4_state->at_chrom.XX + mp4_state->at_chrom.YX;
			mp4_state->hdr.gmc_chrom_mv_y = mp4_state->at_chrom.Y0 + mp4_state->at_chrom.XY + mp4_state->at_chrom.YY;
			mp4_state->hdr.gmc_chrom_mv_x >>= mp4_state->at_chrom.shifter;
			mp4_state->hdr.gmc_chrom_mv_y >>= mp4_state->at_chrom.shifter;
		}
		else
		{
			mp4_state->hdr.gmc_chrom_mv_x = mp4_state->at_chrom.X0;
			mp4_state->hdr.gmc_chrom_mv_y = mp4_state->at_chrom.Y0;
		}
		mp4_state->hdr.gmc_chrom_mv_x <<= (3 - mp4_state->hdr.sprite_warping_accuracy);
		mp4_state->hdr.gmc_chrom_mv_y <<= (3 - mp4_state->hdr.sprite_warping_accuracy);
	}
	return 0;
}

/***/

#if 0
void sprite_trajectory(MP4_STREAM * ld, MP4_STATE * mp4_state)
{
}
#endif

// errors in the header must be considered unrecoverable (return value is 0)
int16_t mp4_getpackethdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
    MP4_STREAM * ld = _ld;
    MP4_STATE * mp4_state = _mp4_state;
	
    mp4_get_resync_marker(ld, mp4_state);
    mp4_state->hdr.mba = mp4_getbits(ld, mp4_state->hdr.mb_in_vop_length);
    if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY) 
    {
        mp4_state->hdr.quant_scale = mp4_getbits(ld, 5);
        //It seems it should not be update for B-VOP. Need to be verify
        if(mp4_state->hdr.prediction_type != MP4_B_VOP)
        {
            mp4_state->hdr.quantizer = mp4_state->hdr.quant_scale;
            mp4_state->hdr.use_intra_dc_vlc 
                = mp4_get_use_intra_dc_vlc(mp4_state->hdr.quantizer, 
                                        mp4_state->hdr.intra_dc_vlc_thr);
        }
    }

    mp4_state->hdr.header_extension_code = mp4_getbits(ld, 1);
    if (mp4_state->hdr.header_extension_code)
    {
        int32_t vop_coding_type, intra_dc_vlc_thr, vop_fcode_forward, vop_fcode_backward;

        while (mp4_getbits(ld, 1) == 1) // temporal time base
        {
            //It should be masked since it is same meaning as in picture header
            //mp4_state->hdr.time_base++;
        }
        mp4_getbits1(ld); // marker bit
        {
            int32_t bits = log2ceil(mp4_state->hdr.time_increment_resolution);
            if (bits < 1) bits = 1;

            mp4_state->hdr.time_inc = mp4_getbits(ld, bits); // vop_time_increment (1-16 bits)
        }
        mp4_getbits1(ld); // marker bit

        if (mp4_state->hdr.shape_coding_type != RECTANGULAR) {
        }

        vop_coding_type = mp4_getbits(ld, 2);

        if (vop_coding_type != mp4_state->hdr.prediction_type)
        {
            // unrecovereable error
            return -1;
        }

        if (mp4_state->hdr.shape != BINARY_SHAPE_ONLY)
        {
            intra_dc_vlc_thr = mp4_getbits(ld, 3);
#if 0
            if ((mp4_state->hdr.prediction_type == MP4_S_VOP) && (mp4_state->hdr.no_of_sprite_warping_points > 0))
            {
                sprite_trajectory(ld, mp4_state);
            }
#endif
            if (intra_dc_vlc_thr != mp4_state->hdr.intra_dc_vlc_thr)
            {
                mp4_state->hdr.intra_dc_vlc_thr = intra_dc_vlc_thr; // [Ag][Review]
                mp4_state->hdr.use_intra_dc_vlc = mp4_get_use_intra_dc_vlc(mp4_state->hdr.quantizer, mp4_state->hdr.intra_dc_vlc_thr);
            }
            if (mp4_state->hdr.prediction_type != MP4_I_VOP) {
                vop_fcode_forward = mp4_getbits(ld, 3);

                if (vop_fcode_forward != mp4_state->hdr.fcode_for)
                {
                    // unrecovereable error
                    return -1;
                }
            }
            if (mp4_state->hdr.prediction_type == MP4_B_VOP)
            {
                vop_fcode_backward = mp4_getbits(ld, 3);
                if (vop_fcode_backward != mp4_state->hdr.fcode_back)
                {
                    // unrecovereable error
                    return -1;
                }
            }
        }
    }

    if (mp4_state->hdr.newpred_enable)
    {
        int32_t bit_length;
//      int32_t vop_id;
        int32_t vop_id_for_prediction_indication;

        bit_length = ((mp4_state->hdr.time_inc+3) < 15) ? (mp4_state->hdr.time_inc+3) : 15;
//      vop_id = mp4_getbits(ld, bit_length);
        mp4_getbits(ld, bit_length);
        vop_id_for_prediction_indication = mp4_getbits(ld, 1);
        if (vop_id_for_prediction_indication)
        {
            /*int32_t vop_id_for_prediction = */mp4_getbits(ld, bit_length);
            mp4_getbits1(ld); // marker bit
        }
    }

    return 0;
}

/***/

int16_t mp4_nextbits_resync_marker(MP4_STREAM * ld, MP4_STATE * mp4_state)
{
	if (mp4_state->hdr.resync_marker_disable == 0)
	{
        int32_t skip_cnt = 0;
		int32_t code = mp4_nextbits_bytealigned(ld, mp4_state->hdr.resync_length, 
            mp4_state->hdr.short_video_header, &skip_cnt);
        if(code == 0)
        {
            return 2;
        }
        else if(code == 1)
        {
            if(skip_cnt!=0)
            {
                mp4_getbits(ld, skip_cnt);
            }
            return 1;
        }
	}

	return 0;
}

/***/

int16_t check_sync_marker(MP4_STREAM* ld)
{
    uint8_t* ptr = ld->rdptr + (ld->bitcnt+7)/8 - 8;
    int32_t skipcnt = 0;
    // check that will return us to caller in 99.95% of cases
    if (ptr[0] || ptr[1] || (ptr[2] & ~1))
        return 0;
    // but it is not sufficient if we are less than 8 bits from the next frame
	if (mp4_bytealigned(ld, 0))
	{
		// stuffing bits
		if (mp4_showbits(ld, 8) == 127) {
			skipcnt = 8;
		}
	}
	else
	{
		// count skipbits until mp4_bytealign
		while (! mp4_bytealigned(ld, skipcnt)) {
			skipcnt += 1;
		}
	}
	if (! mp4_check_stuffingcode(ld, skipcnt)) // there was a check for short_video_header here but i removed it
		return 0;
    return 1;
}


int16_t mp4_get_h263_pic_hdr(MP4_STREAM * _ld, MP4_STATE * _mp4_state)
{
    uint32_t code, gob;
    uint32_t tmp;
    MP4_STREAM *ld = _ld;
    MP4_STATE  *s  = _mp4_state;

    uint32_t UFEP;
    uint32_t temp_ref;
    int32_t trd = 0;
    uint32_t source_format = 0;

    int32_t lines[7] = {-1,128,176,352,704,1408,-1};
 	int32_t pels[7]  = {-1,96,144,288,576,1152,-1};
    //* start code;
    code = mp4_getbits(ld, 17);
    if (code != 0x1)
        return -1;

    gob  = mp4_getbits (ld, 5);
    if (gob == 0x1f)
        return -1;      //* end of stream;

    if (gob != 0)
        return -1;       //* no picture header;


    temp_ref = mp4_getbits (ld, 8);
    if (trd < 0)
        trd += 256;

    tmp = mp4_getbits (ld, 5); /* 1, 0, split_screen_indicator, document_camera_indicator,  freeze_picture_release*/

    tmp = mp4_getbits (ld, 3);

    if(tmp == 0)		//forbidden format
    {
    	return -1;
    }

    if (tmp != 0x7)
    {
        s->hdr.rounding_type = 0;

        source_format = tmp;
        s->FormatPlus = source_format;
        s->hdr.width  = lines[source_format];
        s->hdr.height = pels[source_format];

	    s->hdr.short_video_header           = 1;
	    s->userdata_codec_version           = 263;

	    s->hdr.split_screen_indicator       = 0;
	    s->hdr.document_camera_indicator    = 0;
	    s->hdr.full_picture_freeze_release  = 0;

	    switch(s->FormatPlus)
	    {
	    case SQCIF:
	    	s->hdr.num_mb_in_gob    = 8;
	    	s->hdr.num_gobs_in_vop  = 6;
	    	break;

	    case QCIF:
	    	s->hdr.num_mb_in_gob    = 11;
	    	s->hdr.num_gobs_in_vop  = 9;
	    	break;

	    case CIF:
	    	s->hdr.num_mb_in_gob    = 22;
	    	s->hdr.num_gobs_in_vop  = 18;
	    	break;

	    case fCIF:
	    	s->hdr.num_mb_in_gob    = 44;
	    	s->hdr.num_gobs_in_vop  = 36;
	    	break;

	    case ssCIF:
	    	s->hdr.num_mb_in_gob    = 88;
	    	s->hdr.num_gobs_in_vop  = 72;
	    	break;

	    case CUSTOM:
	    	s->hdr.num_mb_in_gob    = (s->hdr.width+15)>>4;
	    	s->hdr.num_gobs_in_vop  = (s->hdr.height+15)>>4;
	    	break;

	    default:
	    	s->hdr.num_mb_in_gob    = 0;
	    	s->hdr.num_gobs_in_vop  = 0;
	    	break;
	    }

        tmp = mp4_getbits (ld, 1);

        if(tmp == 0)
            s->hdr.picture_coding_type = 0;
        else if(tmp == 1)
            s->hdr.picture_coding_type = 2;
        else
            return 1;   //* tell outside to skip this frame;

        //* 4 bits for unrestricted_mv_mode, syntax_based_ac;
        //* advance_prediction_mode and pb_frame.
        tmp = mp4_getbits(ld, 4);
        if(tmp)
            return -1;

	    // fixed setting for short header T
	    s->hdr.shape                            = RECTANGULAR;
	    s->hdr.obmc_disable                     = 1;
	    s->hdr.quant_type                       = 0;
	    s->hdr.resync_marker_disable            = 1;
	    s->hdr.data_partitioning                = 0;
	    s->hdr.reversible_vlc                   = 0;
	    s->hdr.rounding_type                    = 0;
	    s->hdr.fcode_for                        = 1;
	    s->hdr.fcode_back                       = 1;
	    s->hdr.vop_coded                        = 1;
	    s->hdr.interlaced                       = 0;
	    s->hdr.complexity_estimation_disable    = 1;
	    s->hdr.use_intra_dc_vlc                 = 0;
	    s->hdr.scalability                      = 0;
	    s->hdr.not_8_bit                        = 0;
        s->hdr.bits_per_pixel                   = 8;

        s->hdr.display_time_prev = s->hdr.display_time_next;
		s->hdr.display_time_next = temp_ref;
		s->hdr.trd = temp_ref - s->hdr.display_time_prev;
        if (s->hdr.trd < 0)
            s->hdr.trd += 256;

        s->hdr.vop_quant = mp4_getbits (ld, 5);
        s->hdr.quantizer = s->hdr.vop_quant;

        tmp = mp4_getbits (ld, 1);
        if (tmp)
            return -1;
    }
    else
    {
         UFEP = mp4_getbits (ld, 3);

         if (UFEP == 1)
         {
             /* OPPTYPE */
             source_format = mp4_getbits (ld, 3);

             //* optional custom picture clock frequency, unrestricted_mv_mode,
             //* syntax_based_ac and advance_prediction_mode bits;
             tmp = mp4_getbits(ld, 4);
             if(tmp)
                return -1;

             s->hdr.h263_aic    = mp4_getbits (ld, 1); //* advanced_intra_coding;
             tmp                = mp4_getbits (ld, 1); //* deblocking_filter_mode;

             //* slice structured mode, reference_picture_selection_mode,
             //* independently_segmented_decoding_mode;
             tmp = mp4_getbits(ld, 3);
             if(tmp)
                 return -1;

             tmp = mp4_getbits (ld, 1);     //* alternate vlc mode
             if(tmp)
                return -1;

             s->hdr.iModifiedQantization = mp4_getbits (ld, 1);

             tmp = mp4_getbits (ld, 4);      //* OPPTYPE : bit15=1, bit16,bit17,bit18=0
             if (tmp != 8)
                 return -1;
         }

         if ((UFEP == 1) || (UFEP == 0))
         {
             /* MMPTYPE */
             tmp = mp4_getbits (ld, 3);

             if(tmp == 0)
                 s->hdr.picture_coding_type = 0;
             else if(tmp == 1)
                 s->hdr.picture_coding_type = 2;
             else
                 return 1;  //* tell outside to skip this frame;

             s->hdr.display_time_prev = s->hdr.display_time_next;
		     s->hdr.display_time_next = temp_ref;
		     s->hdr.trd = temp_ref - s->hdr.display_time_prev;
             if (s->hdr.trd < 0)
                 s->hdr.trd += 256;

             //* reference_picture_resampling_mode, reduced_resolution_update_mode
             tmp = mp4_getbits(ld, 2);
             if (tmp)
                 return -1;

             s->hdr.rounding_type = mp4_getbits (ld, 1);      /* rounding type */

             tmp = mp4_getbits (ld, 3);
             if (tmp != 1)
                 return -1;
         }
         else
         {
             //* UFEP error;
             return -1;
         }

         tmp = mp4_getbits (ld, 1);      //* CPM, not support;
         if (tmp)
             return -1;

         if (UFEP && (source_format == CUSTOM))
         {
             /* Read custom picture format */
             s->hdr.aspect_ratio_info = mp4_getbits (ld, 4);      //* CP_PAR_code

             tmp = mp4_getbits (ld, 9);
             s->hdr.width = (tmp + 1 ) * 4;

             tmp = mp4_getbits (ld, 1);
             if (!tmp)
                 return -1;

             tmp = mp4_getbits (ld, 9);
             s->hdr.height = tmp * 4;

             if ((s->hdr.width%16) || (s->hdr.height%16))
                 return -1;

             if (s->hdr.aspect_ratio_info == EXTENDED_PAR)
             {
                 s->hdr.par_width  = mp4_getbits (ld, 8);
                 s->hdr.par_height = mp4_getbits (ld, 8);
             }
         }

         if (source_format != CUSTOM)
         {
             s->hdr.width  = lines[source_format];
             s->hdr.height = pels[source_format];
         }

         s->hdr.vop_quant = mp4_getbits (ld, 5);
         s->hdr.quantizer = s->hdr.vop_quant;
    }

    s->hdr.trb = 0;
    tmp = mp4_getbits (ld, 1);    //* pei

pspare:
    if (tmp)
    {
        /* extra info for possible future backward compatible additions */
        mp4_getbits (ld, 8);                /* not used */
        tmp = mp4_getbits (ld, 1);  //* pei
        if (tmp)
            goto pspare;              /* keep on reading pspare until pei=0 */
    }

    return 0;
}

