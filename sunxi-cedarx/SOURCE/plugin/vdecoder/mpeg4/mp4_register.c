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
#include "mpeg4_config.h"
#include "mp4_register.h"

mp4regVECORE_MODESEL mp4vecore_modesel_reg00;
mp4regVECORE_STATUS mp4vecore_status_reg1c;


mp4regMPHR mp4mphr_reg00;
mp4regMVOPHR mp4mvophr_reg04;
mp4regFSIZE mp4fsize_reg08;
mp4regPICSIZE	mp4picsize_reg0c;
mp4regMBADDR mp4mbaddr_reg10;
mp4regVECTRL mp4vectrl_reg14;
mp4regVETRIGGER mp4vetrigger_reg18;
mp4regVESTAT mp4vestat_reg1c;
mp4regTRBTRDFLD mp4trbtrdfld_reg20;
mp4regTRBTRDFRM mp4trbtrdfrm_reg24;
mp4regVLDBADDR mp4vldbaddr_reg28;
mp4regVLDOFFSET mp4vldoffset_reg2c;
mp4regVLDLEN mp4vldlen_reg30;
mp4regVBVENDADDR mp4vbvsize_reg34;
mp4regMBHADDR	mp4mbhaddr_reg38;
mp4regVLDOFFSET mp4vldoffset_reg38;
mp4regVLDLEN mp4vldlen_reg3c;
mp4regDecBufADDR mp4dcacaddr_reg3c;
mp4regDecBufADDR mp4dblkaddr_reg40;
mp4regDecBufADDR mp4ncfaddr_reg44;
mp4regFRMADDR mp4rec_yframaddr_reg48,mp4rec_cframaddr_reg4c;
mp4regFRMADDR mp4for_yframaddr_reg50,mp4for_cframaddr_reg54;
mp4regFRMADDR mp4back_yframaddr_reg58,mp4back_cframaddr_reg5c;
mp4regSOCX mp4socx_reg60;
mp4regSOCY mp4socy_reg64;
mp4regSO mp4sol_reg68;
mp4regSDX mp4sdlx_reg6c;
mp4regSDY mp4sdly_reg70;
mp4regSSR mp4spriteshifter_reg74;
mp4regSDX mp4sdcx_reg78;
mp4regSDY mp4sdcy_reg7c;
mp4regIQMINPUT mp4iqminput_reg80;
mp4regQCINPUT mp4qcinput_reg84;
mp4regIQIDCTINPUT mp4iqidctinput_reg90;
mp4regMBH mp4mbh_reg94;
mp4regMV1234 mp4mv1_reg98,mp4mv2_reg9c,mp4mv3_rega0,mp4mv4_rega4;
mp4regMV5678 mp4mv5_rega8,mp4mv6_regac,mp4mv7_regb0,mp4mv8_regb4;
mp4regERRFLAG mp4errflag_regc4;
mp4regCRTMBADDR mp4crtmb_regc8;
mp4regFRMADDR mp4rotf_yfrmaddr_regcc;
mp4regFRMADDR mp4rotf_cfrmaddr_regd0;
//mp4regSDROTDBLKCTRL mp4extra_func_ctrl_regd4;
