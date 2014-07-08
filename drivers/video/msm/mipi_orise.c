/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_orise.h"
#include "mdp4.h"
#include <linux/gpio.h>

//wangbing
//static struct mipi_dsi_panel_platform_data *mipi_orise_pdata;
static struct msm_panel_common_pdata *mipi_orise_pdata;

static struct dsi_buf orise_tx_buf;
static struct dsi_buf orise_rx_buf;


static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static char param_cabc_0x51[2] = {0x51,0xa6};
static char param_cabc_0x53[2] = {0x53,0x2c};
static char param_cabc_0x55[2] = {0x55,0x03};

/**************BOE,OTM1283*****************/
	static char boe_param0[4] = {0xff,0x12,0x83,0x01};	//EXTC=1
	static char boe_param1[2] = {0x00,0x80};	            //Orise mode enable
	static char boe_param2[3] = {0xff,0x12,0x83};
//-------------------- panel setting ------------------------------------//
	static char boe_param3[2] = {0x00,0x80};             //TCON Setting
	static char boe_param4[10] = {0xc0,0x00,0x64,0x00,0x0f,0x11,0x00,0x64,0x0f,0x11};

	static char boe_param5[2] = {0x00,0x90};             //Panel Timing Setting
	static char boe_param6[7] = {0xc0,0x00,0x5c,0x00,0x01,0x00,0x04};   //Charge time setting,gate and source 

    static char boe_param7[2] = {0x00,0xa4};             //source pre. 
	static char boe_param8[2] = {0xc0,0x1c};

	static char boe_param9[2] = {0x00,0xb3};             //Interval Scan Frame: 0 frame, column inversion
	static char boe_param10[3] = {0xc0,0x00,0x50};   // 50 column, 00 1dot

	static char boe_param11[2] = {0x00,0x81};             //frame rate:60Hz
	static char boe_param12[2] = {0xc1,0x55};

	static char boe_param13[2] = {0x00,0x81};             //source bias 0.75uA
	static char boe_param14[2] = {0xc4,0x82};

	static char boe_param15[2] = {0x00,0x90};             //clock delay for data latch 
	static char boe_param16[2] = {0xc4,0x49};

//-------------------------------------- BOE Power IC-----------------------------------------
	static char boe_param17[2] = {0x00,0x90};             //Mode-3
	static char boe_param18[5] = {0xf5,0x02,0x11,0x02,0x11};

	static char boe_param19[2] = {0x00,0x90};             //3xVPNL
	static char boe_param20[2] = {0xc5,0x50};

	static char boe_param21[2] = {0x00,0x94};             //2xVPNL
	static char boe_param22[2] = {0xc5,0x66};

	static char boe_param23[2] = {0x00,0xb2};             //VGLO1
	static char boe_param24[3] = {0xf5,0x00,0x00};

	static char boe_param25[2] = {0x00,0xb4};             //VGLO1_S
	static char boe_param26[3] = {0xf5,0x00,0x00};

	static char boe_param27[2] = {0x00,0xb6};             //VGLO2
	static char boe_param28[3] = {0xf5,0x00,0x00};

	static char boe_param29[2] = {0x00,0xb8};             //VGLO2_S
	static char boe_param30[3] = {0xf5,0x00,0x00};

	static char boe_param31[2] = {0x00,0xb4};             //VGLO1/2 Pull low setting
	static char boe_param32[2] = {0xc5,0xc0};				//d[7] vglo1 d[6] vglo2 => 0: pull vss, 1: pull vgl

	static char boe_param33[2] = {0x00,0xb2};             //C31 cap. not remove
	static char boe_param34[2] = {0xc5,0x40};

	static char boe_param35[2] = {0x00,0xC8};             
	static char boe_param36[2] = {0xB0,0x62};

	static char boe_param37[2] = {0x00,0x92};//AVEE移至和NVDD18 同時開
	static char boe_param38[2] = {0xf5,0x06};

	static char boe_param39[2] = {0x00,0x94};//VCL移至和NVDD18 同時開
	static char boe_param40[2] = {0xf5,0x06};

	static char boe_param41[2] = {0x00,0x96};//VGH移至和VCOM 同時開
	static char boe_param42[2] = {0xf5,0x0f};

	static char boe_param43[2] = {0x00,0x98};//VGH移至和VCOM 同時開
	static char boe_param44[2] = {0xf5,0x0f};
	
//-------------------- power setting -----------------------------------//
	static char boe_param45[2] = {0x00,0xa0};             //dcdc setting (PFM Fre)

	static char boe_param46[15] = {0xc4,0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10};

	static char boe_param47[2] = {0x00,0xb0};             //clamp voltage setting
	static char boe_param48[3] = {0xc4,0x00,0x00};         //VSP and VSN Change (5.6V,-5.6V)

	static char boe_param49[2] = {0x00,0x91};             //VGH=12V, VGL=-12V, pump ratio:VGH=6x, VGL=-5x
	static char boe_param50[3] = {0xc5,0x19,0x50};

	static char boe_param51[2] = {0x00,0x00};             //GVDD=4.87V, NGVDD=-4.87V
	static char boe_param52[3] = {0xd8,0xbc,0xbc};

	static char boe_param53[2] = {0x00,0xb0};             //VDD_18V=1.6V, LVDSVDD=1.55V
	static char boe_param54[3] = {0xc5,0x04,0xb8};

	static char boe_param55[2] = {0x00,0xbb};             //LVD voltage level setting
	static char boe_param56[2] = {0xc5,0x80};

	static char boe_param57[2] = {0x00,0xc3};             //Sample / Hold All on
	static char boe_param58[2] = {0xf5,0x81};

//-------------------- control setting ---------------------------------------------------//

	static char boe_param59[2] = {0x00,0x80};             //PWM output enable
	static char boe_param60[2] = {0xc6,0x24};

	static char boe_param61[2] = {0x00,0x00};             //ID1
	static char boe_param62[2] = {0xd0,0x40};

	static char boe_param63[2] = {0x00,0x00};             //ID2, ID3
	static char boe_param64[3] = {0xd1,0x00,0x00};

//-------------------- panel timing state control ------------------------------------------//
	static char boe_param65[2] = {0x00,0x80};             //panel timing state control
	static char boe_param66[12] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param67[2] = {0x00,0x90};             //panel timing state control
	static char boe_param68[16] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param69[2] = {0x00,0xa0};             //panel timing state control
	static char boe_param70[16] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param71[2] = {0x00,0xb0};             //panel timing state control
	static char boe_param72[16] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param73[2] = {0x00,0xc0};             //panel timing state control
	static char boe_param74[16] = {0xcb,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param75[2] = {0x00,0xd0};             //panel timing state control
	static char boe_param76[16] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00};

	static char boe_param77[2] = {0x00,0xe0};             //panel timing state control
	static char boe_param78[15] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05};

	static char boe_param79[2] = {0x00,0xf0};             //panel timing state control
	static char boe_param80[12] = {0xcb,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//-------------------- panel pad mapping control --------------------//
	 static char boe_param81[2] = {0x00,0x80};             //panel pad mapping control
	 static char boe_param82[16] = {0xcc,0x0a,0x0c,0x0e,0x10,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	 static char boe_param83[2] = {0x00,0x90};             //panel pad mapping control
	 static char boe_param84[16] = {0xcc,0x00,0x00,0x00,0x00,0x00,0x2e,0x2d,0x09,0x0b,0x0d,0x0f,0x01,0x03,0x00,0x00};

	 static char boe_param85[2] = {0x00,0xa0};             //panel pad mapping control
	 static char boe_param86[15] = {0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2e,0x2d};

	 static char boe_param87[2] = {0x00,0xb0};             //panel pad mapping control
	 static char boe_param88[16] = {0xcc,0x0F,0x0D,0x0B,0x09,0x3,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; 

	 static char boe_param89[2] = {0x00,0xc0};             //panel pad mapping control
	 static char boe_param90[16] = {0xcc,0x00,0x00,0x00,0x00,0x00,0x2d,0x2e,0x10,0x0E,0x0C,0x0A,0x04,0x02,0x00,0x00}; 

	 static char boe_param91[2] = {0x00,0xd0};             //panel pad mapping control
	 static char boe_param92[15] = {0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2d,0x2e}; 

//-------------------- panel timing setting --------------------//
	static char boe_param93[2] = {0x00,0x80};             //panel VST setting
	static char boe_param94[13] = {0xce,0x8f,0x03,0x00,0x8e,0x03,0x00,0x8d,0x03,0x00,0x8c,0x03,0x00};

	static char boe_param95[2] = {0x00,0x90};             //panel VEND setting
	static char boe_param96[15] = {0xce,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param97[2] = {0x00,0xa0};             //panel CLKA1/2 setting
	static char boe_param98[15] = {0xce,0x38,0x0b,0x05,0x00,0x00,0x0a,0x0a,0x38,0x0a,0x05,0x01,0x00,0x0a,0x0a};

	static char boe_param99[2] = {0x00,0xb0};             //panel CLKA3/4 setting
	static char boe_param100[15] = {0xce,0x38,0x09,0x05,0x02,0x00,0x0a,0x0a,0x38,0x08,0x05,0x03,0x00,0x0a,0x0a};

	static char boe_param101[2] = {0x00,0xc0};             //panel CLKb1/2 setting
	static char boe_param102[15] = {0xce,0x38,0x07,0x05,0x04,0x00,0x0a,0x0a,0x38,0x06,0x05,0x05,0x00,0x0a,0x0a};

	static char boe_param103[2] = {0x00,0xd0};             //panel CLKb3/4 setting
	static char boe_param104[15] = {0xce,0x38,0x05,0x05,0x06,0x00,0x0a,0x0a,0x38,0x04,0x05,0x07,0x00,0x0a,0x0a};

	static char boe_param105[2] = {0x00,0x80};             //panel CLKc1/2 setting
	static char boe_param106[15] = {0xcf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param107[2] = {0x00,0x90};             //panel CLKc3/4 setting
	static char boe_param108[15] = {0xcf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param109[2] = {0x00,0xa0};             //panel CLKd1/2 setting
	static char boe_param110[15] = {0xcf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param111[2] = {0x00,0xb0};             //panel CLKd3/4 setting
	static char boe_param112[15] = {0xcf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	static char boe_param113[2] = {0x00,0xc0};             //panel ECLK setting, gate pre. ena.
	static char boe_param114[12] = {0xcf,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x00,0x08};

	static char boe_param115[2] = {0x00,0xb5};             //TCON_GOA_OUT Setting

	static char boe_param116[7] = {0xc5,0x33,0xf1,0xff,0x33,0xf1,0xff};  //normal output with VGH/VGL
//------------------------------------------VCOM Setting-------------------------//
	static char boe_param117[2] = {0x00,0x00};             //VCOMDC=-1.1      (up to down, down to up)
	static char boe_param118[2] = {0xd9,0x65};	
//------------------------------------------Gamma Tuning-----------------------//
	static char boe_param119[2] = {0x00,0x00};
	static char boe_param120[17] = {0xE1,0x0A,0x1D,0x23,0x0D,0x04,0x0F,0x0A,0x09,0x03,0x06,0x0A,0x05,0x0E,0x11,0x0D,0x01};

	static char boe_param121[2] = {0x00,0x00};
	static char boe_param122[17] = {0xE2,0x0A,0x1D,0x22,0x0E,0x04,0x0E,0x0B,0x0A,0x02,0x06,0x09,0x05,0x0E,0x11,0x0D,0x01};
 		
	static char boe_param123[2] = {0x00,0x00};             //Orise mode disable
	static char boe_param124[4] = {0xff,0xff,0xff,0xff};
	
	static char boe_param125[2] = {0x11,0x0};//sleep out
	//delay_ms(120);
	static char boe_param126[2] = {0x29,0x0};// display on
	//delay_ms(120);

static struct dsi_cmd_desc boe_otm1283_video_init_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param0),boe_param0},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param1),boe_param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param2),boe_param2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param3),boe_param3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param4),boe_param4},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param5),boe_param5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param6),boe_param6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param7),boe_param7},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param8),boe_param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param9),boe_param9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param10),boe_param10},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param11),boe_param11},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param12),boe_param12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param13),boe_param13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param14),boe_param14},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param15),boe_param15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param16),boe_param16},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param17),boe_param17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param18),boe_param18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param19),boe_param19},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param20),boe_param20},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param21),boe_param21},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param22),boe_param22},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param23),boe_param23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param24),boe_param24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param25),boe_param25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param26),boe_param26},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param27),boe_param27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param28),boe_param28},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param29),boe_param29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param30),boe_param30},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param31),boe_param31},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param32),boe_param32},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param33),boe_param33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param34),boe_param34},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param35),boe_param35},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param36),boe_param36},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37),boe_param37},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param38),boe_param38},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param39),boe_param39},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param40),boe_param40},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param41),boe_param41},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param42),boe_param42},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param43),boe_param43},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param44),boe_param44},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param45),boe_param45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param46),boe_param46},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param47),boe_param47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param48),boe_param48},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param49),boe_param49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param50),boe_param50},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param51),boe_param51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param52),boe_param52},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param53),boe_param53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param54),boe_param54},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param55),boe_param55},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param56),boe_param56},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param57),boe_param57},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param58),boe_param58},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param59),boe_param59},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param60),boe_param60},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param61),boe_param61},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param62),boe_param62},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param63),boe_param63},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param64),boe_param64},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param65),boe_param65},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param66),boe_param66},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param67),boe_param67},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param68),boe_param68},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param69),boe_param69},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param70),boe_param70},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param71),boe_param71},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param72),boe_param72},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param73),boe_param73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param74),boe_param74},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param75),boe_param75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param76),boe_param76},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param77),boe_param77},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param78),boe_param78},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param79),boe_param79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param80),boe_param80},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param81),boe_param81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param82),boe_param82},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param83),boe_param83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param84),boe_param84},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param85),boe_param85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param86),boe_param86},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param87),boe_param87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param88),boe_param88},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param89),boe_param89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param90),boe_param90},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param91),boe_param91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param92),boe_param92},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param93),boe_param93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param94),boe_param94},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param95),boe_param95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param96),boe_param96},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param97),boe_param97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param98),boe_param98},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param99),boe_param99},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param100),boe_param100},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param101),boe_param101},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param102),boe_param102},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param103),boe_param103},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param104),boe_param104},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param105),boe_param105},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param106),boe_param106},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param107),boe_param107},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param108),boe_param108},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param109),boe_param109},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param110),boe_param110},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param111),boe_param111},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param112),boe_param112},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param113),boe_param113},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param114),boe_param114},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param115),boe_param115},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param116),boe_param116},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param117),boe_param117},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param118),boe_param118},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param119),boe_param119},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param120),boe_param120},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param121),boe_param121},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param122),boe_param122},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param123),boe_param123},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param124),boe_param124},
//shihuiqin for cabc
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x51),param_cabc_0x51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x53),param_cabc_0x53},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},
	
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(boe_param125),boe_param125},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(boe_param126),boe_param126},

};
/*************BOE,OTM1283******************/

/********lead yushun LCM,OTM1283************/
static char param0[4] = {0xFF,0x12,0x83,0x01};
static char param1[2] = {0x00,0x80};
static char param2[3] = {0xFF,0x12,0x83};
static char param3[2] = {0x00,0x92};
static char param4[3] = {0xFF,0x30,0x02};
static char param5[2] = {0x00,0xA6};
static char param6[2] = {0xB3,0x0B};
//-------------------- panel setting --------------------//
static char param7[2] = {0x00,0x80};
static char param8[10] = {0xC0,0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10};
static char param9[2] = {0x00,0x90};
static char param10[7] = {0xC0,0x00,0x5C,0x00,0x01,0x00,0x04};

static char param10a[2] = {0x00,0xa4};
static char param10b[2] = {0xc0,0x22};

static char param11[2] = {0x00,0xB3};
static char param12[3] = {0xC0,0x00,0x50};
static char param13[2] = {0x00,0x81};
static char param14[2] = {0xC1,0x55};

static char param14a[2] = {0x00,0x90};             //clock delay for data latch 
static char param14b[2] = {0xc4,0x49};

//-------------------- power setting --------------------//
static char param15[2] = {0x00,0xA0};
static char param16[15] = {0xC4,0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x05,0x02,0x05,0x15,0x11};
static char param17[2] = {0x00,0xB0};
static char param18[3] = {0xC4,0x00,0x00};

static char param19[2] = {0x00,0xBB};
static char param20[2] = {0xC5,0x80};

static char param21[2] = {0x00,0x91};
static char param22[3] = {0xC5,0x49,0x50};

static char param23[2] = {0x00,0x00};
static char param24[3] = {0xD8,0x9d,0x9d};

static char param25[2] = {0x00,0x00};
static char param26[2] = {0xD9,0x5e};

static char param26a[2] = {0x00,0x81};
static char param26b[2] = {0xc4,0x82};


static char param27[2] = {0x00,0xB0};
static char param28[3] = {0xC5,0x04,0xb8};

static char param29[2] = {0x00,0x82};
static char param30[3] = {0xF5,0x00,0x00};
static char param31[2] = {0x00,0x82};
static char param32[2] = {0xF4,0x00};
//-------------------- control setting --------------------//
static char param33[2] = {0x00,0x00};
static char param34[2] = {0xD0,0x40};
static char param35[2] = {0x00,0x00};
static char param36[3] = {0xD1,0x00,0x00};
//-------------------- power on setting --------------------//
static char param37[2] = {0x00,0x9B};
static char param38[2] = {0xCB,0xFF};
static char param39[2] = {0x00,0x9D};
static char param40[2] = {0xCB,0xFF};
static char param41[2] = {0x00,0xA0};
static char param42[2] = {0xCB,0xFF};
static char param43[2] = {0x00,0xA2};
static char param44[2] = {0xCB,0xFF};
static char param45[2] = {0x00,0xB3};
static char param46[2] = {0xCB,0xFF};
static char param47[2] = {0x00,0xB5};
static char param48[2] = {0xCB,0xFF};
static char param49[2] = {0x00,0xB7};
static char param50[2] = {0xCB,0xFF};
static char param51[2] = {0x00,0xB9};
static char param52[2] = {0xCB,0xFF};

static char param53[2] = {0x00,0x80};
static char param54[2] = {0xC4,0x00};

static char param55[2] = {0x00,0x98};
static char param56[2] = {0xC5,0x10};

static char param57[2] = {0x00,0x81};
static char param58[2] = {0xF5,0x15};
static char param59[2] = {0x00,0x83};
static char param60[2] = {0xF5,0x15};
static char param61[2] = {0x00,0x85};
static char param62[2] = {0xF5,0x15};
static char param63[2] = {0x00,0x87};
static char param64[2] = {0xF5,0x15};
static char param65[2] = {0x00,0x89};
static char param66[2] = {0xF5,0x15};
static char param67[2] = {0x00,0x8B};
static char param68[2] = {0xF5,0x15};

static char param69[2] = {0x00,0x95};
static char param70[2] = {0xF5,0x15};
static char param71[2] = {0x00,0x97};
static char param72[2] = {0xF5,0x15};
static char param73[2] = {0x00,0x99};
static char param74[2] = {0xF5,0x15};

static char param75[2] = {0x00,0xA1};
static char param76[2] = {0xF5,0x15};
static char param77[2] = {0x00,0xA3};
static char param78[2] = {0xF5,0x15};
static char param79[2] = {0x00,0xA5};
static char param80[2] = {0xF5,0x15};
static char param81[2] = {0x00,0xA7};
static char param82[2] = {0xF5,0x15};
static char param83[2] = {0x00,0xAB};
static char param84[2] = {0xF5,0x18};

static char param85[2] = {0x00,0xB1};
static char param86[14] = {0xF5,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x15,0x08,0x15};
static char param87[2] = {0x00,0xB4};
static char param88[2] = {0xC5,0xC0};

//-------------------- for Power IC ---------------------------------

static char param89[2] = {0x00,0x90};
static char param90[5] = {0xf5,0x02,0x11,0x02,0x11};

static char param91[2] = {0x00,0x90};
static char param92[2] = {0xc5,0x50};

static char param93[2] = {0x00,0x94};
static char param94[2] = {0xc5,0x66};
//-------------------- panel timing state control --------------------//
static char param95[2] = {0x00,0x80};
static char param96[12] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char param97[2] = {0x00,0x90};
static char param98[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x00};

static char param99[2] = {0x00,0xA0};
static char param100[16] = {0xCB,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char param101[2] = {0x00,0xB0};
static char param102[16] = {0xCB,0x00,0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00};

static char param103[2] = {0x00,0xC0};
static char param104[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x00,0x05,0x05,0x05,0x05,0x05};

static char param105[2] = {0x00,0xD0};
static char param106[16] = {0xCB,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05};

static char param107[2] = {0x00,0xE0};
static char param108[15] = {0xCB,0x05,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00};

static char param109[2] = {0x00,0xF0};
static char param110[12] = {0xCB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//-------------------- panel pad mapping control --------------------//
static char param111[2] = {0x00,0x80};
static char param112[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x07,0x00,0x0D,0x09,0x0F,0x0B,0x11};

static char param113[2] = {0x00,0x90};
static char param114[16] = {0xCC,0x15,0x13,0x17,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06};

static char param115[2] = {0x00,0xA0};
static char param116[15] = {0xCC,0x08,0x00,0x0E,0x0A,0x10,0x0C,0x12,0x16,0x14,0x18,0x02,0x04,0x00,0x00};

static char param117[2] = {0x00,0xB0};
static char param118[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0x00,0x0C,0x10,0x0A,0x0E,0x18};

static char param119[2] = {0x00,0xC0};
static char param120[16] = {0xCC,0x14,0x16,0x12,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03};

static char param121[2] = {0x00,0xD0};
static char param122[16] = {0xCC,0x01,0x00,0x0B,0x0F,0x09,0x0D,0x17,0x13,0x15,0x11,0x07,0x05,0x00,0x00};
//-------------------- panel timing setting --------------------//
static char param123[2] = {0x00,0x80};
static char param124[13] = {0xCE,0x87,0x03,0x28,0x86,0x03,0x28,0x85,0x03,0x28,0x84,0x03,0x28};

static char param125[2] = {0x00,0x90};
static char param126[15] = {0xCE,0x34,0xFC,0x28,0x34,0xFD,0x28,0x34,0xFe,0x28,0x34,0xFF,0x28,0x00,0x00};

static char param127[2] = {0x00,0xA0};
static char param128[15] = {0xCE,0x38,0x07,0x05,0x00,0x00,0x28,0x00,0x38,0x06,0x05,0x01,0x00,0x28,0x00};

static char param129[2] = {0x00,0xB0};
static char param130[15] = {0xCE,0x38,0x05,0x05,0x02,0x00,0x28,0x00,0x38,0x04,0x05,0x03,0x00,0x28,0x00};

static char param131[2] = {0x00,0xC0};
static char param132[15] = {0xCE,0x38,0x03,0x05,0x04,0x00,0x28,0x00,0x38,0x02,0x05,0x05,0x00,0x28,0x00};

static char param133[2] = {0x00,0xD0};
static char param134[15] = {0xCE,0x38,0x01,0x05,0x06,0x00,0x28,0x00,0x38,0x00,0x05,0x07,0x00,0x28,0x00};

static char param135[2] = {0x00,0x80};
static char param136[15] = {0xCF,0x78,0x07,0x04,0xFF,0x00,0x18,0x10,0x78,0x06,0x05,0x00,0x00,0x18,0x10};

static char param137[2] = {0x00,0x90};
static char param138[15] = {0xCF,0x78,0x05,0x05,0x01,0x00,0x18,0x10,0x78,0x04,0x05,0x02,0x00,0x18,0x10};

static char param139[2] = {0x00,0xA0};
static char param140[15] = {0xCF,0x70,0x00,0x05,0x00,0x00,0x18,0x10,0x70,0x01,0x05,0x01,0x00,0x18,0x10};

static char param141[2] = {0x00,0xB0};
static char param142[15] = {0xCF,0x70,0x02,0x05,0x02,0x00,0x18,0x10,0x70,0x03,0x05,0x03,0x00,0x18,0x10};

static char param143[2] = {0x00,0xC0};
static char param144[12] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x81,0x00,0x03,0x08};

static char param145[2] = {0x00,0x00};
static char param146[17] = {0xE1,0x00,0x04,0x08,0x0b,0x04,0x0d,0x0c,0x0c,0x03,0x06,0x12,0x0F,0x17,0x30,0x25,0x06};

static char param147[2] = {0x00,0x00};
static char param148[17] = {0xE2,0x00,0x04,0x08,0x0b,0x04,0x0d,0x0C,0x0c,0x03,0x06,0x12,0x0F,0x17,0x30,0x25,0x06};

static char param149[2] = {0x00,0xa0};
static char param150[13] = {0xd6,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD};
static char param151[2] = {0x00,0xB0};
static char param152[13] = {0xd6,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD};
static char param153[2] = {0x00,0xC0};
static char param154[13] = {0xD6,0x89,0x11,0x89,0x89,0x11,0x89,0x89,0x11,0x89,0x89,0x11,0x89};
static char param155[2] = {0x00,0xD0};
static char param156[7] = {0xD6,0x89,0x11,0x89,0x89,0x11,0x89};
static char param157[2] = {0x00,0xE0};
static char param158[13] = {0xD6,0x44,0x11,0x44,0x44,0x11,0x44,0x44,0x11,0x44,0x44,0x11,0x44};
static char param159[2] = {0x00,0xF0};
static char param160[7] = {0xD6,0x44,0x11,0x44,0x44,0x11,0x44};
static char param161[2] = {0x00,0x90};
static char param162[2] = {0xd6,0x00};
static char param163[2] = {0x00,0x00};
static char param164[2] = {0x55,0xff}; 

static char param165[2] = {0x00,0x00};
static char param166[4] = {0xFF,0xFF,0xFF,0xFF};

static char param169[2] = {0x35,0x00};
static char param167[2] = {0x11,0x0};
static char param168[2] = {0x29,0x0};

static struct dsi_cmd_desc otm1283_video_init_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param0),param0},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param1),param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param2),param2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param3),param3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param4),param4},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param5),param5},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param6),param6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param7),param7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param8),param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param9),param9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param10),param10},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param10a),param10a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param10b),param10b},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param11),param11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param12),param12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param13),param13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param14),param14},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param14a),param14a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param14b),param14b},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param15),param15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param16),param16},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param17),param17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param18),param18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param19),param19},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20),param20},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param21),param21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param22),param22},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param23),param23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param24),param24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param25),param25},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param26),param26},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param26a),param26a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param26b),param26b},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param27),param27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param28),param28},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param29),param29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param30),param30},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param31),param31},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param32),param32},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param33),param33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param34),param34},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param35),param35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param36),param36},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param37),param37},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param38),param38},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param39),param39},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param40),param40},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param41),param41},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param42),param42},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param43),param43},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param44),param44},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param45),param45},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param46),param46},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param47),param47},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param48),param48},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param49),param49},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param50),param50},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param51),param51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param52),param52},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param53),param53},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param54),param54},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param55),param55},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param56),param56},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param57),param57},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param58),param58},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param59),param59},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param60),param60},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param61),param61},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param62),param62},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param63),param63},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param64),param64},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param65),param65},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param66),param66},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param67),param67},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param68),param68},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param69),param69},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param70),param70},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param71),param71},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param72),param72},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param73),param73},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param74),param74},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param75),param75},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param76),param76},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param77),param77},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param78),param78},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param79),param79},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param80),param80},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param81),param81},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param82),param82},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param83),param83},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84),param84},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param85),param85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param86),param86},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param87),param87},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param88),param88},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param89),param89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param90),param90},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param91),param91},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param92),param92},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param93),param93},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param94),param94},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param95),param95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param96),param96},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param97),param97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param98),param98},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param99),param99},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param100),param100},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param101),param101},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param102),param102},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param103),param103},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param104),param104},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param105),param105},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param106),param106},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param107),param107},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param108),param108},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param109),param109},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param110),param110},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param111),param111},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param112),param112},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param113),param113},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param114),param114},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param115),param115},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param116),param116},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param117),param117},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param118),param118},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param119),param119},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param120),param120},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param121),param121},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param122),param122},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param123),param123},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param124),param124},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param125),param125},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param126),param126},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param127),param127},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param128),param128},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param129),param129},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param130),param130},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param131),param131},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param132),param132},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param133),param133},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param134),param134},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param135),param135},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param136),param136},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param137),param137},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param138),param138},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param139),param139},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param140),param140},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param141),param141},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param142),param142},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param143),param143},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param144),param144},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param145),param145},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param146),param146},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param147),param147},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param148),param148},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param149),param149},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param150),param150},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param151),param151},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param152),param152},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param153),param153},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param154),param154},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param155),param155},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param156),param156},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param157),param157},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param158),param158},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param159),param159},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param160),param160},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param161),param161},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param162),param162},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param163),param163},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param164),param164},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param165),param165},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param166),param166},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param169),param169},
//shihuiqin for cabc
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x51),param_cabc_0x51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x53),param_cabc_0x53},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},
	
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(param167),param167},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,sizeof(param168),param168},

};

/********lead yushun LCM,OTM1283************/

static struct dsi_cmd_desc orise_cmd_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(display_on), display_on},
};

static char param_cabc_0x51_off[2]={0x51,0x0};
static char param_cabc_0x53_off[2]={0x53,0x0c};

static struct dsi_cmd_desc otm1283_display_off_cmds[] = {

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x51_off),param_cabc_0x51_off},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x53_off),param_cabc_0x53_off},


	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(enter_sleep), enter_sleep},
		
	
};


#define CONFIG_LCD_READ_ID 1 
#ifdef CONFIG_LCD_READ_ID
u32 LcdPanelID = LCD_PANEL_NOPANEL;

//shihuiqin for lcd 20121009
#define LCD_IC_ID    82
#define LCD_IC_ID1  85
static uint32 mipi_get_hardwareid(struct msm_fb_data_type *mfd )
{
       int id,id1;

      id = gpio_get_value(LCD_IC_ID);
      id1 = gpio_get_value(LCD_IC_ID1);

	printk("hardware id is %d,id1 is %d---\r\n",id,id1);

	if(id==1)
				LcdPanelID = OTM1283_BOE_HD_LCM;
	else if(id==0)
				LcdPanelID = OTM1283_LEAD_HD_LCM;
	else
				LcdPanelID = (u32)LCD_PANEL_NOPANEL;
		
	return LcdPanelID;
}
#endif


//[ECID 000000]ZTEBSP zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
static struct dsi_cmd_desc cabc_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 5,sizeof(param_cabc_0x51),param_cabc_0x51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x53),param_cabc_0x53},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},
};

void mipi_set_backlight(struct msm_fb_data_type *mfd)
{
         /*value range is 1--32*/
	 int current_lel = mfd->bl_level;
	 //unsigned long flags;
	 printk("zhangqi add for CABC level=%d in %s func \n ",current_lel,__func__);

	   if(current_lel==0)
	   {
	   	   if(LcdPanelID == (u32)OTM1283_BOE_HD_LCM)
	   	   {
	                     param_cabc_0x51[1]=0x00;
	   	   }
		   else
	   	   {
	                     param_cabc_0x51[1]=0x00;
	   	   }
		   	
	   }
	   else
	   {
	   	   if(LcdPanelID == (u32)OTM1283_BOE_HD_LCM)
	   	   {
				param_cabc_0x51[1] = current_lel;
	   	   }
		   else
	   	   {
	                     param_cabc_0x51[1]=current_lel;
	   	   }
	   }
	   mipi_set_tx_power_mode(0);
	   if(LcdPanelID == (u32)OTM1283_BOE_HD_LCM)
	   {
	         mipi_dsi_cmds_tx(&orise_tx_buf, cabc_cmds,ARRAY_SIZE(cabc_cmds));
	   }
	   else
	   {
	         mipi_dsi_cmds_tx(&orise_tx_buf, cabc_cmds,ARRAY_SIZE(cabc_cmds));
	   }
	   
	   return;
	 
}
#endif
//[ECID 000000]ZTEBSP zhangqi add for CABC end	


#ifdef CONFIG_BACKLIGHT_PM8921
#include <linux/delay.h>
#include <linux/pwm.h>
static struct pwm_device *bl_pwm;
static int bl_level;
static int led_pwm;
/* PWM */
#define PWM_FREQ_HZ	(66*1000)	/* 66 KHZ */
#define PWM_LEVEL 15
#define PWM_PERIOD_USEC (USEC_PER_SEC / PWM_FREQ_HZ)
#define PWM_DUTY_LEVEL (PWM_PERIOD_USEC / PWM_LEVEL)


/**
 * Set Backlight level.
 *
 * @param pwm
 * @param level
 *
 * @return int
 */
static int set_backlight_level(struct pwm_device *pwm, int level)
{
	int ret = 0;

	pr_debug("%s: level=%d.\n", __func__, level);

	if ((pwm == NULL) || (level > PWM_LEVEL) || (level < 0)) {
		pr_err("%s.pwm=NULL.\n", __func__);
		return -EINVAL;
	}

	ret = pwm_config(pwm, PWM_DUTY_LEVEL * level, PWM_PERIOD_USEC);
	if (ret) {
		pr_err("%s: pwm_config() failed err=%d.\n", __func__, ret);
		return ret;
	}

	ret = pwm_enable(pwm);
	if (ret) {
		pr_err("%s: pwm_enable() failed err=%d\n",
		       __func__, ret);
		return ret;
	}


	return 0;
}

static void mipi_set_backlight(struct msm_fb_data_type *mfd)
{
	int level = mfd->bl_level;

	pr_debug("%s.lvl=%d.\n", __func__, level);

	set_backlight_level(bl_pwm, level);

	bl_level = level;
}
#endif

static int mipi_orise_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct msm_panel_info *pinfo;
#ifdef CONFIG_LCD_READ_ID
	static int first_time_panel_on = 1;
#endif

	printk("%s\n", __func__);

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	pinfo = &mfd->panel_info;
	mipi  = &mfd->panel_info.mipi;

#ifdef CONFIG_LCD_READ_ID
	if(first_time_panel_on)
	{
		printk("zhangqi add %s  first_time_panel_on=%d\n",__func__,first_time_panel_on);
		mipi_get_hardwareid(mfd);
	       msleep(200);
        }

	if(LcdPanelID == (u32)OTM1283_BOE_HD_LCM)
	{

		printk("OTM1283_BOE_HD_LCM\n");
              if(first_time_panel_on)
	       {
			first_time_panel_on = 0;

		    mipi_set_tx_power_mode(1);	
		    mipi_dsi_cmds_tx(&orise_tx_buf,
			boe_otm1283_video_init_cmds,
			ARRAY_SIZE(boe_otm1283_video_init_cmds));
		    mipi_set_tx_power_mode(0);	
	       }
	       else
	       {
		      mipi_dsi_cmds_tx(&orise_tx_buf,
			     boe_otm1283_video_init_cmds,
			    ARRAY_SIZE(boe_otm1283_video_init_cmds));
	       }
	}
	else
	{
		printk("OTM1283_HD_LCM\n");
	       if (mipi->mode == DSI_VIDEO_MODE) 
		{
	            msleep(5);
	            if(first_time_panel_on)
	            {
			     first_time_panel_on = 0;
		            mipi_set_tx_power_mode(1);
		            mipi_dsi_cmds_tx(&orise_tx_buf, otm1283_video_init_cmds,
			                  ARRAY_SIZE(otm1283_video_init_cmds));
		            mipi_set_tx_power_mode(0);	
	             }
	             else
	             { 
			     mipi_dsi_cmds_tx(&orise_tx_buf, otm1283_video_init_cmds,
			     ARRAY_SIZE(otm1283_video_init_cmds));
	             }
	        }
		 else
		 {
		            mipi_dsi_cmds_tx(&orise_tx_buf, orise_cmd_on_cmds,
			              ARRAY_SIZE(orise_cmd_on_cmds));
		            mipi_dsi_cmd_bta_sw_trigger(); /* clean up ack_err_status */
	        }
	}
#endif


#ifdef CONFIG_BACKLIGHT_PM8921
 	if (bl_level == 0)
		bl_level = PWM_LEVEL * 2 / 3 ; /* Default ON value PWM_LEVEL*/

	/* Set backlight via PWM */
	if (bl_pwm) {
		int ret=0;		
		ret = set_backlight_level(bl_pwm, bl_level);
		if (ret)
			pr_err("%s.mipi_d2r_set_backlight_level.ret=%d",
			       __func__, ret);
	}
#endif

	return 0;
}

static int mipi_orise_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	printk("%s\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

#ifdef CONFIG_LCD_READ_ID
	if(LcdPanelID == (u32)OTM1283_BOE_HD_LCM)
	{
		printk("OTM1283_BOE_HD_LCM\n");
	       mipi_dsi_cmds_tx(&orise_tx_buf, otm1283_display_off_cmds,
			   ARRAY_SIZE(otm1283_display_off_cmds));
	}
	else
	{
		printk("OTM1283_HD_LCM\n");
	       mipi_dsi_cmds_tx(&orise_tx_buf, otm1283_display_off_cmds,
			    ARRAY_SIZE(otm1283_display_off_cmds));
	}
#endif
	return 0;
}

static int __devinit mipi_orise_lcd_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct platform_device *current_pdev;

	printk(KERN_ERR "%s\n", __func__);
	
	if (pdev->id == 0) {
		mipi_orise_pdata = pdev->dev.platform_data;
		mipi_orise_pdata->panel_config_gpio(1);
#ifdef CONFIG_BACKLIGHT_PM8921
		led_pwm = mipi_orise_pdata->gpio_num[0];
#endif
		return 0;
	}

	current_pdev = msm_fb_add_device(pdev);

	if (current_pdev) {
		mfd = platform_get_drvdata(current_pdev);
		
		if (!mfd)
			return -ENODEV;
		
		if (mfd->key != MFD_KEY)
			return -EINVAL;
		
		mipi  = &mfd->panel_info.mipi;
	}


#ifdef CONFIG_BACKLIGHT_PM8921
	bl_pwm = NULL;
	if (led_pwm >= 0) {
		bl_pwm = pwm_request(led_pwm, "lcd-backlight");
		if (bl_pwm == NULL || IS_ERR(bl_pwm)) {
			pr_err("%s pwm_request() failed.id=%d.bl_pwm=%d.\n",
			       __func__, led_pwm, (int) bl_pwm);
			bl_pwm = NULL;
			return -EIO;
		} else {
			pr_debug("%s.pwm_request() ok.pwm-id=%d.\n",
			       __func__, led_pwm);

		}
	} else {
		pr_info("%s. led_pwm is invalid.\n", __func__);
	}
#endif
	
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_orise_lcd_probe,
	.driver = {
		.name   = "mipi_orise",
	},
};

static struct msm_fb_panel_data orise_panel_data = {
	.on		= mipi_orise_lcd_on,
	.off		= mipi_orise_lcd_off,
/*[ECID:000000] ZTEBSP wangbing, for orise lcd cabc, 20120820*/
#if defined(CONFIG_BACKLIGHT_CABC) || defined(CONFIG_BACKLIGHT_PM8921)
	.set_backlight	 = mipi_set_backlight, 
#endif	
};

static int ch_used[3];

int mipi_orise_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	
	printk(KERN_ERR "%s\n", __func__);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_orise", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	orise_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &orise_panel_data,
		sizeof(orise_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_orise_lcd_init(void)
{
	printk("%s\n", __func__);

	mipi_dsi_buf_alloc(&orise_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&orise_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_orise_lcd_init);
