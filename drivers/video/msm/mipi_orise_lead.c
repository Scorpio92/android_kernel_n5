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
#include "mipi_orise_lead.h"
#include "mdp4.h"
#include <linux/gpio.h>

#define  IC_VERSION_DETECT

//wangbing
//static struct mipi_dsi_panel_platform_data *mipi_orise_lead_pdata;
static struct msm_panel_common_pdata *mipi_orise_lead_pdata;

static struct dsi_buf orise_lead_tx_buf;
static struct dsi_buf orise_lead_rx_buf;


static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static char param_cabc_0x55[2] = {0x55,0x93};   //ce:low(0x83),medium(0x93),high(0xb3)

/********lead yushun LCM,OTM1283************/
static char param00[2] = {0x00,0x00};
static char param0[4] = {0xFF,0x12,0x83,0x01};
static char param1[2] = {0x00,0x80};
static char param2[3] = {0xFF,0x12,0x83};

static char param5[2] = {0x00,0xA6};
static char param6[2] = {0xB3,0x0B};
//-------------------- panel setting --------------------//
static char param7[2] = {0x00,0x80};
static char param8[10] = {0xC0,0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10};

static char param9[2] = {0x00,0x90};
static char param10[7] = {0xC0,0x00,0x5C,0x00,0x01,0x00,0x04};

static char param10a[2] = {0x00,0xa2};
static char param10b[4] = {0xC0,0x01,0x00,0x00};

static char param11[2] = {0x00,0xB3};
static char param12[3] = {0xC0,0x00,0x50};

static char param13[2] = {0x00,0x81};
static char param14[2] = {0xC1,0x55};

static char param14a[2] = {0x00,0x90};             //clock delay for data latch 
static char param14b[2] = {0xc4,0x49};

//-------------------- power setting --------------------//
static char param15[2] = {0x00,0xA0};
static char param16[15] = {0xC4,0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x07,0x02,0x05,0x15,0x11};

static char param17[2] = {0x00,0xB0};
static char param18[3] = {0xC4,0x00,0x00};

static char param21[2] = {0x00,0x91};
static char param22[3] = {0xC5,0x29,0x50};

static char param23[2] = {0x00,0x00};
static char param24[3] = {0xD8,0x9d,0x9d};

static char param25[2] = {0x00,0x00};
static char param26[2] = {0xD9,0x57};

static char param26a[2] = {0x00,0x81};
static char param26b[2] = {0xc4,0x82};

static char param27[2] = {0x00,0xB0};
static char param28[3] = {0xC5,0x04,0xb8};//b8

static char param19[2] = {0x00,0xBB};
static char param20[2] = {0xC5,0x80};

static char param20a[2] = {0x00,0x82};
static char param20b[2] = {0xC4,0x02};

static char param20c[2] = {0x00,0xc6};
static char param20d[2] = {0xb0,0x03};

//-------------------- control setting --------------------//
static char param33[2] = {0x00,0x00};
static char param34[2] = {0xD0,0x40};

static char param35[2] = {0x00,0x00};
static char param36[3] = {0xD1,0x00,0x00};
//-------------------- power on setting --------------------//
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

static char param84a[2] = {0x00,0xBA}; 
static char param84b[2] = {0xF5,0x03}; 
 
static char param84c[2] = {0x00,0x94}; 
static char param84d[2] = {0xF5,0x02}; 

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
static char param122[15] = {0xCC,0x01,0x00,0x0B,0x0F,0x09,0x0D,0x17,0x13,0x15,0x11,0x07,0x05,0x00,0x00};
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
static char param136[15] = {0xCF,0x78,0x07,0x04,0xFF,0x00,0x18,0x28,0x78,0x06,0x05,0x00,0x00,0x18,0x28};

static char param137[2] = {0x00,0x90};
static char param138[15] = {0xCF,0x78,0x05,0x05,0x01,0x00,0x18,0x28,0x78,0x04,0x05,0x02,0x00,0x18,0x28};

static char param139[2] = {0x00,0xA0};
static char param140[15] = {0xCF,0x70,0x00,0x05,0x00,0x00,0x18,0x28,0x70,0x01,0x05,0x01,0x00,0x18,0x28};

static char param141[2] = {0x00,0xB0};
static char param142[15] = {0xCF,0x70,0x02,0x05,0x02,0x00,0x18,0x28,0x70,0x03,0x05,0x03,0x00,0x18,0x28};

static char param143[2] = {0x00,0xC0};
static char param144[12] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x81,0x00,0x03,0x08};

//GAMMA 2.2+ 
static char param145[2] = {0x00,0x00};
static char param146[17] = {0xE1,0x00,0x04,0x09,0x0C,0x04,0x0F,0x0B,0x0B,0x02,0x05,0x11,0x0E,0x17,0x30,0x25,0x06};

//GAMMA 2.2- 
static char param147[2] = {0x00,0x00};
static char param148[17] = {0xE2,0x00,0x04,0x09,0x0C,0x04,0x0F,0x0B,0x0B,0x02,0x05,0x11,0x0E,0x17,0x30,0x25,0x06};

//CABC
static char param_cabc1[2] = {0x00,0x80};
static char param_cabc2[29] = {0xCA,1,142,149,157,164,171,178,186,193,200,207,215,222,229,236,244,232,255,135,255,135,
	                                          255,5,3,5,3,5,3};
//**********************	CABC 1
	static char param_cabc3[2] = {0x00,0x00};
	static char param_cabc4[2] = {0xC6,0x10};
	static char param_cabc5[19] = {0xC7,144, 169, 170, 170, 170, 170, 170, 170, 153, 136, 136, 136, 119, 102,  85,  85,  85,  85};

//**********************	CABC 2
	static char param_cabc6[2] = {0x00,0x00};
	static char param_cabc7[2] = {0xC6,0x11};
	static char param_cabc8[19] = {0xC7,144, 153, 170, 170, 170, 170, 170, 154, 153, 136, 136, 136, 119, 102, 102,  85,  85,  85};

//**********************	CABC 3
	static char param_cabc9[2] = {0x00,0x00};
	static char param_cabc10[2] = {0xC6,0x12};
	static char param_cabc11[19] = {0xC7,144, 153, 169, 170, 170, 170, 170, 153, 153, 136, 136, 136, 119, 102, 102, 102,  85,  85};

//**********************	CABC 4
	static char param_cabc12[2] = {0x00,0x00};
	static char param_cabc13[2] = {0xC6,0x13};
	static char param_cabc14[19] = {0xC7,144, 153, 153, 170, 170, 170, 154, 153, 153, 136, 136, 136, 119, 102, 102, 102, 102,  85};

//**********************	CABC 5
	static char param_cabc15[2] = {0x00,0x00};
	static char param_cabc16[2] = {0xC6,0x14};
	static char param_cabc17[19] = {0xC7,144, 153, 153, 169, 170, 170, 153, 153, 153, 136, 136, 136, 119, 102, 102, 102, 102, 102};

//**********************	CABC 6
	static char param_cabc18[2] = {0x00,0x00};
	static char param_cabc19[2] = {0xC6,0x15};
	static char param_cabc20[19] = {0xC7,144, 153, 153, 153, 170, 154, 153, 153, 153, 136, 136, 136, 119, 119, 102, 102, 102, 102};

//**********************	CABC 7
	static char param_cabc21[2] = {0x00,0x00};
	static char param_cabc22[2] = {0xC6,0x16};
	static char param_cabc23[19] = {0xC7,144, 153, 153, 153, 169, 153, 153, 153, 153, 136, 136, 136, 119, 119, 119, 102, 102, 102};

//**********************	CABC 8
	static char param_cabc24[2] = {0x00,0x00};
	static char param_cabc25[2] = {0xC6,0x17};
	static char param_cabc26[19] = {0xC7,128, 153, 153, 153, 153, 153, 153, 153, 153, 136, 136, 136, 119, 119, 119, 119, 102, 102};

//**********************	CABC 9
	static char param_cabc27[2] = {0x00,0x00};
	static char param_cabc28[2] = {0xC6,0x18};
	static char param_cabc29[19] = {0xC7,128, 152, 153, 153, 153, 152, 153, 153, 153, 136, 136, 136, 119, 119, 119, 119, 119, 102};

//**********************	CABC 10
	static char param_cabc30[2] = {0x00,0x00};
	static char param_cabc31[2] = {0xC6,0x19};
	static char param_cabc32[19] = {0xC7,128, 136, 153, 153, 153, 136, 153, 153, 153, 136, 136, 136, 119, 119, 119, 119, 119, 119};

//**********************	CABC 11
	static char param_cabc33[2] = {0x00,0x00};
	static char param_cabc34[2] = {0xC6,0x1A};
	static char param_cabc35[19] = {0xC7,128, 136, 152, 153, 153, 136, 152, 153, 153, 136, 136, 136, 136, 119, 119, 119, 119, 119};

//**********************	CABC 12
	static char param_cabc36[2] = {0x00,0x00};
	static char param_cabc37[2] = {0xC6,0x1B};
	static char param_cabc38[19] = {0xC7,128, 136, 136, 153, 153, 136, 136, 153, 153, 136, 136, 136, 136, 136, 119, 119, 119, 119};

//**********************	CABC 13
	static char param_cabc39[2] = {0x00,0x00};
	static char param_cabc40[2] = {0xC6,0x1C};
	static char param_cabc41[19] = {0xC7,128, 136, 136, 152, 153, 136, 136, 152, 153, 136, 136, 136, 136, 136, 136, 119, 119, 119};

//**********************	CABC 14
	static char param_cabc42[2] = {0x00,0x00};
	static char param_cabc43[2] = {0xC6,0x1D};
	static char param_cabc44[19] = {0xC7,128, 136, 136, 136, 153, 136, 136, 136, 153, 136, 136, 136, 136, 136, 136, 136, 119, 119};

//**********************	CABC 15
	static char param_cabc45[2] = {0x00,0x00};
	static char param_cabc46[2] = {0xC6,0x1E};
	static char param_cabc47[19] = {0xC7,128, 136, 136, 136, 152, 136, 136, 136, 152, 136, 136, 136, 136, 136, 136, 136, 120, 120};

//**********************	CABC 16
	static char param_cabc48[2] = {0x00,0x00};
	static char param_cabc49[2] = {0xC6,0x1F};
	static char param_cabc50[19] = {0xC7,128, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136};


	static char param_cabc51[2] = {0x00,0x00};
	static char param_cabc52[2] = {0XC6,0x00};
//CABC	

/**************CE**********************/
#if 1
 
/**2.21 para*/
        static char ce_param1[2] = {0x00,0xa0};             // 
        static char ce_param2[13] = {0xD6,0x01,0x1a,0x01,0x1a,0x01,0xe6,0x01,0xb3,0x01,0xb3,0x01,0xb3}; 
 
        static char ce_param3[2] = {0x00,0xB0};             // 
        static char ce_param4[13] = {0xD6,0x01,0xb3,0x01,0xcd,0x01,0xcd,0x01,0xcd,0x01,0xcd,0x01,0x66}; 
 
        static char ce_param5[2] = {0x00,0xC0};             // 
        static char ce_param6[13] = {0xD6,0x66,0x11,0x11,0x9a,0x11,0x77,0x77,0x11,0x77,0x77,0x11,0x89}; 
 
        static char ce_param7[2] = {0x00,0xD0};             // 
        static char ce_param8[7] = {0xD6,0x89,0x11,0x89,0x89,0x11,0x44}; 
 
        static char ce_param9[2] = {0x00,0xE0};             // 
        static char ce_param10[13] = {0xD6,0x33,0x11,0x44,0x4d,0x11,0x3c,0x3c,0x11,0x3c,0x3c,0x11,0x44}; 
 
        static char ce_param11[2] = {0x00,0xF0};             // 
        static char ce_param12[7] = {0xD6,0x44,0x11,0x44,0x44,0x11,0x22}; 
 
        static char ce_param13[2] = {0x00,0x90};             //Clever CMD1 
        static char ce_param14[2] = {0xd6,0x00}; 
 
        static char ce_param15[2] = {0x00,0x00};             //CE -80: Low; 90: Mid; B0: High; 
        static char ce_param16[2] = {0x55,0x93}; 

        static char ce_param17[2] = {0x00,0x00};    

#endif
/**************CE**********************/

static char param165[2] = {0x00,0x00};
static char param166[4] = {0xFF,0xFF,0xFF,0xFF};


#ifdef IC_VERSION_DETECT
static char para5[2] = {0x00,0xa6};
static char para6[2] = {0xb3,0x0b};
//-------------------- panel setting --------------------//
static char para7[2] = {0x00,0x80};
static char para8[10] = {0xc0,0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10};

static char para9[2] = {0x00,0x90};
static char para10[7] = {0xc0,0x00,0x5c,0x00,0x01,0x00,0x04};

static char para11[2] = {0x00,0xb3};
static char para12[3] = {0xc0,0x00,0x50};

static char para13[2] = {0x00,0x81};
static char para14[2] = {0xc1,0x55};

static char para14a[2] = {0x00,0x90};             //clock delay for data latch 
static char para14b[2] = {0xc4,0x49};

//-------------------- power setting --------------------//
static char para15[2] = {0x00,0xa0};
static char para16[15] = {0xc4,0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x05,0x02,0x05,0x15,0x11};

static char para17[2] = {0x00,0xb0};
static char para18[3] = {0xc4,0x00,0x00};

static char para21[2] = {0x00,0x91};
static char para22[3] = {0xc5,0x19,0x50};

static char para23[2] = {0x00,0x00};
static char para24[3] = {0xd8,0x9d,0x9d};

static char para25[2] = {0x00,0x00};
static char para26[2] = {0xd9,0x57};

static char para27[2] = {0x00,0xb0};
static char para28[3] = {0xc5,0x04,0x38};//b8

static char para19[2] = {0x00,0xbb};
static char para20[2] = {0xc5,0x80};

static char para20a[2] = {0x00,0x82};
static char para20b[2] = {0xC4,0x02};

static char para20c[2] = {0x00,0xc6};
static char para20d[2] = {0xb0,0x03};

//-------------------- control setting --------------------//
static char para33[2] = {0x00,0x00};
static char para34[2] = {0xd0,0x40};

static char para35[2] = {0x00,0x00};
static char para36[3] = {0xd1,0x00,0x00};
//-------------------- power on setting --------------------//
//-------------------- for Power IC ---------------------------------
//-------------------- panel timing state control --------------------//

//-------------------- panel pad mapping control --------------------//
//-------------------- panel timing setting --------------------//
static char para123[2] = {0x00,0x80};
static char para124[13] = {0xCE,0x87,0x03,0x28,0x86,0x03,0x28,0x85,0x03,0x28,0x84,0x03,0x28};

static char para125[2] = {0x00,0x90};
static char para126[15] = {0xCE,0x34,0xFC,0x28,0x34,0xFD,0x28,0x34,0xFe,0x28,0x34,0xFF,0x28,0x00,0x00};

static char para127[2] = {0x00,0xA0};
static char para128[15] = {0xCE,0x38,0x07,0x05,0x00,0x00,0x28,0x00,0x38,0x06,0x05,0x01,0x00,0x28,0x00};

static char para129[2] = {0x00,0xB0};
static char para130[15] = {0xCE,0x38,0x05,0x05,0x02,0x00,0x28,0x00,0x38,0x04,0x05,0x03,0x00,0x28,0x00};

static char para131[2] = {0x00,0xC0};
static char para132[15] = {0xCE,0x38,0x03,0x05,0x04,0x00,0x28,0x00,0x38,0x02,0x05,0x05,0x00,0x28,0x00};

static char para133[2] = {0x00,0xD0};
static char para134[15] = {0xCE,0x38,0x01,0x05,0x06,0x00,0x28,0x00,0x38,0x00,0x05,0x07,0x00,0x28,0x00};

static char para135[2] = {0x00,0x80};
static char para136[15] = {0xCF,0x78,0x07,0x04,0xFF,0x00,0x18,0x10,0x78,0x06,0x05,0x00,0x00,0x18,0x10};

static char para137[2] = {0x00,0x90};
static char para138[15] = {0xCF,0x78,0x05,0x05,0x01,0x00,0x18,0x10,0x78,0x04,0x05,0x02,0x00,0x18,0x10};

static char para139[2] = {0x00,0xA0};
static char para140[15] = {0xCF,0x70,0x00,0x05,0x00,0x00,0x18,0x10,0x70,0x01,0x05,0x01,0x00,0x18,0x10};

static char para141[2] = {0x00,0xB0};
static char para142[15] = {0xCF,0x70,0x02,0x05,0x02,0x00,0x18,0x10,0x70,0x03,0x05,0x03,0x00,0x18,0x10};

static char para143[2] = {0x00,0xC0};
static char para144[11] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x04,0x00,0x00};

//GAMMA 2.2+ 
static char para145[2] = {0x00,0x00};
static char para146[17] = {0xE1,0x00,0x04,0x09,0x0C,0x04,0x0F,0x0B,0x0B,0x02,0x05,0x11,0x0E,0x17,0x30,0x25,0x06};

//GAMMA 2.2- 
static char para147[2] = {0x00,0x00};
static char para148[17] = {0xE2,0x00,0x04,0x09,0x0C,0x04,0x0F,0x0B,0x0B,0x02,0x05,0x11,0x0E,0x17,0x30,0x25,0x06};


static struct dsi_cmd_desc otm1283_video_init_cmds0[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param00),param00},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param0),param0},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param1),param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param2),param2},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para5),para5},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para6),para6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para7),para7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para8),para8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para9),para9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para10),para10},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para11),para11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para12),para12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para13),para13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para14),para14},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para14a),para14a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para14b),para14b},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para15),para15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para16),para16},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para17),para17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para18),para18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para21),para21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para22),para22},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para23),para23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para24),para24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para25),para25},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para26),para26},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para27),para27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para28),para28},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para19),para19},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para20),para20},


	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para20a),para20a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para20b),para20b},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para20c),para20c},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para20d),para20d},

	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para33),para33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para34),para34},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para35),para35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para36),para36},
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
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84a),param84a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84b),param84b},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84c),param84c},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84d),param84d},
	
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
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para123),para123},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para124),para124},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para125),para125},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para126),para126},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para127),para127},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para128),para128},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para129),para129},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para130),para130},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para131),para131},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para132),para132},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para133),para133},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para134),para134},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para135),para135},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para136),para136},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para137),para137},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para138),para138},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para139),para139},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para140),para140},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para141),para141},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para142),para142},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para143),para143},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para144),para144},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para145),para145},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para146),para146},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(para147),para147},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(para148),para148},
//CABC
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc1),param_cabc1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc2),param_cabc2},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc3),param_cabc3},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc4),param_cabc4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc5),param_cabc5},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc6),param_cabc6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc7),param_cabc7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc8),param_cabc8},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc9),param_cabc9},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc10),param_cabc10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc11),param_cabc11},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc12),param_cabc12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc13),param_cabc13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc14),param_cabc14},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc15),param_cabc15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc16),param_cabc16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc17),param_cabc17},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc18),param_cabc18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc19),param_cabc19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc20),param_cabc20},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc21),param_cabc21},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc22),param_cabc22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc23),param_cabc23},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc24),param_cabc24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc25),param_cabc25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc26),param_cabc26},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc27),param_cabc27},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc28),param_cabc28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc29),param_cabc29},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc30),param_cabc30},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc31),param_cabc31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc32),param_cabc32},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc33),param_cabc33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc34),param_cabc34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc35),param_cabc35},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc36),param_cabc36},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc37),param_cabc37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc38),param_cabc38},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc39),param_cabc39},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc40),param_cabc40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc41),param_cabc41},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc42),param_cabc42},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc43),param_cabc43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc44),param_cabc44},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc45),param_cabc45},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc46),param_cabc46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc47),param_cabc47},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc48),param_cabc48},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc49),param_cabc49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc50),param_cabc50},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc51),param_cabc51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc52),param_cabc52},

//CABC
#if 1
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param1),ce_param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param2),ce_param2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param3),ce_param3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param4),ce_param4},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param5),ce_param5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param6),ce_param6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param7),ce_param7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param8),ce_param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param9),ce_param9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param10),ce_param10},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param11),ce_param11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param12),ce_param12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param13),ce_param13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param14),ce_param14},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param15),ce_param15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param16),ce_param16},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param17),ce_param17},
#endif
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param165),param165},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param166),param166},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},

	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(exit_sleep),exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(display_on),display_on},
};
#endif

static struct dsi_cmd_desc otm1283_video_init_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param00),param00},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param0),param0},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param1),param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param2),param2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param5),param5},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param6),param6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param7),param7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param8),param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param9),param9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param10),param10},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param10a),param10a},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param10b),param10b},
	
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
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param19),param19},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20),param20},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20a),param20a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20b),param20b},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20c),param20c},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param20d),param20d},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param33),param33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param34),param34},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param35),param35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param36),param36},
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
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84a),param84a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84b),param84b},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84c),param84c},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param84d),param84d},

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
//CABC
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc1),param_cabc1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc2),param_cabc2},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc3),param_cabc3},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc4),param_cabc4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc5),param_cabc5},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc6),param_cabc6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc7),param_cabc7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc8),param_cabc8},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc9),param_cabc9},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc10),param_cabc10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc11),param_cabc11},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc12),param_cabc12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc13),param_cabc13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc14),param_cabc14},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc15),param_cabc15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc16),param_cabc16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc17),param_cabc17},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc18),param_cabc18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc19),param_cabc19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc20),param_cabc20},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc21),param_cabc21},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc22),param_cabc22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc23),param_cabc23},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc24),param_cabc24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc25),param_cabc25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc26),param_cabc26},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc27),param_cabc27},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc28),param_cabc28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc29),param_cabc29},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc30),param_cabc30},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc31),param_cabc31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc32),param_cabc32},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc33),param_cabc33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc34),param_cabc34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc35),param_cabc35},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc36),param_cabc36},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc37),param_cabc37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc38),param_cabc38},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc39),param_cabc39},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc40),param_cabc40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc41),param_cabc41},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc42),param_cabc42},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc43),param_cabc43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc44),param_cabc44},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc45),param_cabc45},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc46),param_cabc46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc47),param_cabc47},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc48),param_cabc48},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc49),param_cabc49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param_cabc50),param_cabc50},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc51),param_cabc51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc52),param_cabc52},

//CABC
#if 1
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param1),ce_param1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param2),ce_param2},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param3),ce_param3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param4),ce_param4},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param5),ce_param5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param6),ce_param6},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param7),ce_param7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param8),ce_param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param9),ce_param9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param10),ce_param10},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param11),ce_param11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(ce_param12),ce_param12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param13),ce_param13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param14),ce_param14},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param15),ce_param15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param16),ce_param16},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(ce_param17),ce_param17},
#endif
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param165),param165},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(param166),param166},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},
	
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(exit_sleep),exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 100,sizeof(display_on),display_on},

};
/********lead yushun LCM,OTM1283************/

static struct dsi_cmd_desc orise_lead_cmd_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc otm1283_display_off_cmds[] = {


	{DTYPE_DCS_WRITE, 1, 0, 0, 10,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,
		sizeof(enter_sleep), enter_sleep},
		
	
};


#ifdef IC_VERSION_DETECT
/*about otm1283 chip version*/
static char ic_version;
#define IC_VERSION_1P0   0
#define IC_VERSION_1P1   1

static char orise_ic_ver_para0[2]={0x00,0x00};
static char orise_ic_ver_para1[4]={0xff,0x12,0x83,0x01};
static char orise_ic_ver_para2[2]={0x00,0x80};
static char orise_ic_Ver_para3[3]={0xff,0x12,0x83};
static char orise_ic_ver_para4[2]={0x00,0x00};
static char otm1283_ic_ver_rd_para[2] = {0xf8,0x00};//0xa1; 
   
static struct dsi_cmd_desc otm1283_ic_ver_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(otm1283_ic_ver_rd_para), otm1283_ic_ver_rd_para
};


static struct dsi_cmd_desc otm1283_setpassword_cmd[] = 
{
	{DTYPE_GEN_WRITE2, 1, 0, 0, 1, sizeof(orise_ic_ver_para0),orise_ic_ver_para0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(orise_ic_ver_para1),orise_ic_ver_para1},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 1, sizeof(orise_ic_ver_para2),orise_ic_ver_para2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(orise_ic_Ver_para3),orise_ic_Ver_para3},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 1, sizeof(orise_ic_ver_para4),orise_ic_ver_para4},
};

void mipi_get_ic_version(struct msm_fb_data_type *mfd)
{
	u32 icid[2] = {0,0};
	int len;
	
		mipi_dsi_buf_init(&orise_lead_tx_buf);
		mipi_dsi_buf_init(&orise_lead_rx_buf);
		
              if (1)
		mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_setpassword_cmd,ARRAY_SIZE(otm1283_setpassword_cmd));
		mipi_dsi_cmd_bta_sw_trigger(); 
		
	       mipi_set_tx_power_mode(1);	
		len = mipi_dsi_cmds_rx(mfd,&orise_lead_tx_buf,&orise_lead_rx_buf, &otm1283_ic_ver_rd_cmd,5);
		mipi_set_tx_power_mode(0);	

		icid[0] = *(u32 *)(orise_lead_rx_buf.data);
		icid[1] = *((u32 *)(orise_lead_rx_buf.data)+1);
		
		printk("debug :read ic version = %x,%x\n",icid[0],icid[1]);

		if((icid[1])& 0xff)
		{
			ic_version = IC_VERSION_1P1;
		}
		else
		{
			ic_version = IC_VERSION_1P0;
		}
}
#endif

//[ECID 000000]ZTEBSP zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC
static char param_cabc_0x51[2] = {0x51,0xa6};
static char param_cabc_0x53[2] = {0x53,0x2c};

static struct dsi_cmd_desc cabc_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 5,sizeof(param_cabc_0x51),param_cabc_0x51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x53),param_cabc_0x53},
};

static void mipi_set_backlight(struct msm_fb_data_type *mfd)
{

		 int current_lel;

		 current_lel = mfd->bl_level;

	   printk("zhangqi add for CABC level=%d in %s func \n ",current_lel,__func__);

	   if(current_lel==0)
	   {
	       param_cabc_0x51[1]=0x00;
	   }
	   else
	   {
	       param_cabc_0x51[1]=current_lel*15;
	   }
	   mipi_set_tx_power_mode(0);
	   mipi_dsi_cmds_tx(&orise_lead_tx_buf, cabc_cmds,ARRAY_SIZE(cabc_cmds));
	   
	 
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

static int mipi_orise_lead_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct msm_panel_info *pinfo;

	static int first_time_panel_on = 1;

#ifdef IC_VERSION_DETECT
       static int ic_version_detect = 1;
#endif

	printk("%s\n", __func__);

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	pinfo = &mfd->panel_info;
	mipi  = &mfd->panel_info.mipi;

	if(first_time_panel_on)
	{
		printk("zhangqi add %s  first_time_panel_on=%d\n",__func__,first_time_panel_on);
	  	msleep(200);
  	}


	  if (mipi->mode == DSI_VIDEO_MODE) 
	  {
	            //msleep(5);
	       if(first_time_panel_on)
	       {
			     		 first_time_panel_on = 0;
		           mipi_set_tx_power_mode(1);
		           // mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_video_init_cmds,ARRAY_SIZE(otm1283_video_init_cmds));
		           mipi_set_tx_power_mode(0);	
	       }
	       else
	       { 
	       	    #ifdef IC_VERSION_DETECT
			    if(ic_version_detect)
			    {
			           mipi_get_ic_version(mfd);
	                         mipi_set_tx_power_mode(1);	
			    }

			    if(ic_version == IC_VERSION_1P0)
			    {
		             printk("ic_version == IC_VERSION_1P0\n");
			         mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_video_init_cmds0,ARRAY_SIZE(otm1283_video_init_cmds0));

			    }
			    else
			    {
			    		             printk("ic_version == IC_VERSION_1P1\n");
			         mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_video_init_cmds,ARRAY_SIZE(otm1283_video_init_cmds));
			     }
			    if(ic_version_detect)
			    {
			           ic_version_detect = 0;
	                         mipi_set_tx_power_mode(0);	
			    }
			    #else
			     mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_video_init_cmds,ARRAY_SIZE(otm1283_video_init_cmds));
			    #endif
	       }
	   }
	   else
	   {
		            mipi_dsi_cmds_tx(&orise_lead_tx_buf, orise_lead_cmd_on_cmds,ARRAY_SIZE(orise_lead_cmd_on_cmds));
		            mipi_dsi_cmd_bta_sw_trigger(); /* clean up ack_err_status */
	   }

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

static int mipi_orise_lead_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	printk("%s\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	  mipi_dsi_cmds_tx(&orise_lead_tx_buf, otm1283_display_off_cmds,ARRAY_SIZE(otm1283_display_off_cmds));

	return 0;
}

static int __devinit mipi_orise_lead_lcd_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct platform_device *current_pdev;

	printk(KERN_ERR "%s\n", __func__);
	
	if (pdev->id == 0) {
		mipi_orise_lead_pdata = pdev->dev.platform_data;
		mipi_orise_lead_pdata->panel_config_gpio(1);
#ifdef CONFIG_BACKLIGHT_PM8921
		led_pwm = mipi_orise_lead_pdata->gpio_num[0];
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
	.probe  = mipi_orise_lead_lcd_probe,
	.driver = {
		.name   = "mipi_orise_lead",
	},
};

static struct msm_fb_panel_data orise_lead_panel_data = {
	.on		= mipi_orise_lead_lcd_on,
	.off		= mipi_orise_lead_lcd_off,
/*[ECID:000000] ZTEBSP wangbing, for orise_lead lcd cabc, 20120820*/
#if defined(CONFIG_BACKLIGHT_CABC) || defined(CONFIG_BACKLIGHT_PM8921)
	.set_backlight	 = mipi_set_backlight, 
#endif	
};

static int ch_used[3];

int mipi_orise_lead_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	
	printk(KERN_ERR "%s\n", __func__);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_orise_lead", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	orise_lead_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &orise_lead_panel_data,
		sizeof(orise_lead_panel_data));
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

static int __init mipi_orise_lead_lcd_init(void)
{
	printk("%s\n", __func__);

	mipi_dsi_buf_alloc(&orise_lead_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&orise_lead_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_orise_lead_lcd_init);
