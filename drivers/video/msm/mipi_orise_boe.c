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
#include "mipi_orise_boe.h"
#include "mdp4.h"
#include <linux/gpio.h>



//wangbing
//static struct mipi_dsi_panel_platform_data *mipi_orise_boe_pdata;
static struct msm_panel_common_pdata *mipi_orise_boe_pdata;

static struct dsi_buf orise_boe_tx_buf;
static struct dsi_buf orise_boe_rx_buf;


static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static char param_cabc_0x55[2] = {0x55,0x93};   //ce:low(0x83),medium(0x93),high(0xb3)

/**************BOE,OTM1283*****************/
static char boe_param0[2] = {0x00,0x00};
static char boe_param1[4] = {0xff,0x12,0x83,0x01};	//EXTC=1
static char boe_param2[2] = {0x00,0x80};	            //Orise mode enable
static char boe_param3[3] = {0xff,0x12,0x83};

/***************Panel Setting************************************/
static char boe_param4[2] = {0x00,0x80};   
static char boe_param5[10] = {0xC0,0x00,0x64,0x00,0x0F,0x11,0x00,0x64,0x0F,0x11};

static char boe_param6[2] = {0x00,0x90};   
static char boe_param7[7] = {0xC0,0x00,0x55,0x00,0x01,0x00,0x04};

static char boe_param8[2] = {0x00,0xa4};    
static char boe_param9[2] = {0xC0,0x00};

static char boe_param10[2] = {0x00,0xB3};   
static char boe_param11[3] = {0xC0,0x00,0x50};  

static char boe_param12[2] = {0x00,0x81};    
static char boe_param13[2] = {0xC1,0x55};

static char boe_param14[2] = {0x00,0x81};    
static char boe_param15[2] = {0xC4,0x82};	

static char boe_param16[2] = {0x00,0x82};   
static char boe_param17[2] = {0xC4,0x02};	

static char boe_param18[2] = {0x00,0x90};     
static char boe_param19[2] = {0xC4,0x49};

static char boe_param19a[2] = {0x00,0xC6};     
static char boe_param19b[2] = {0xB0,0x03};


/*********************BOE Power IC********************************/
static char boe_param20[2] = {0x00,0x90};          
static char boe_param21[5] = {0xF5,0x02,0x11,0x02,0x11};

static char boe_param22[2] = {0x00,0x90};          
static char boe_param23[2] = {0xC5,0x50};

static char boe_param24[2] = {0x00,0x94};       
static char boe_param25[2] = {0xC5,0x66};

static char boe_param26[2] = {0x00,0xB2};            
static char boe_param27[3] = {0xF5,0x00,0x00};

static char boe_param28[2] = {0x00,0xB4};             
static char boe_param29[3] = {0xF5,0x00,0x00};

static char boe_param30[2] = {0x00,0xB6};       
static char boe_param31[3] = {0xF5,0x00,0x00};

static char boe_param32[2] = {0x00,0xB8};       
static char boe_param33[3] = {0xF5,0x00,0x00};

static char boe_param34[2] = {0x00,0xB2};     
static char boe_param35[2] = {0xC5,0x40};	

static char boe_param36[2] = {0x00,0xB4};      
static char boe_param37[2] = {0xC5,0xC0};      

static char boe_param37a[2] = {0x00,0x94};     
static char boe_param37b[2] = {0xF5,0x02};	

static char boe_param37c[2] = {0x00,0xBA};      
static char boe_param37d[2] = {0xF5,0x03};      

/***************Power Setting*************************************/

static char boe_param38[2] = {0x00,0xA0};   
static char boe_param39[15] = {0xC4,0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10};

static char boe_param40[2] = {0x00,0xB0};    
static char boe_param41[3] = {0xC4,0x00,0x00}; 

static char boe_param42[2] = {0x00,0x91};   
static char boe_param43[3] = {0xC5,0x19,0x50};

static char boe_param44[2] = {0x00,0x00};    
static char boe_param45[3] = {0xD8,0xBC,0xBC};

static char boe_param46[2] = {0x00,0xB0}; 
static char boe_param47[3] = {0xC5,0x04,0xb8};

static char boe_param48[2] = {0x00,0xBB};   
static char boe_param49[2] = {0xC5,0x80};                  

/********************Control Setting**********************************/

static char boe_param50[2] = {0x00,0x00}; 
static char boe_param51[2] = {0xD0,0x40};

static char boe_param52[2] = {0x00,0x00};    
static char boe_param53[3] = {0xD1,0x00,0x00};  

/*************GAMMA TUNING*********************************************/

static char boe_param54[2] = {0x00,0x00};
static char boe_param55[17] = {0xE1,0x05,0x12,0x19,0x0F,0x07,0x10,0x0C,0x0B,0x00,0x06,0x0c,0x06,0x0f,0x11,0x0C,0x05};

static char boe_param56[2] = {0x00,0x00};
static char boe_param57[17] = {0xE2,0x05,0x11,0x16,0x0D,0x05,0x0C,0x0C,0x0B,0x04,0x06,0x10,0x09,0x10,0x11,0x0C,0x05};


/*****************Panel Timing state control**************************/

static char boe_param60[2] = {0x00,0x80}; 
static char boe_param61[12] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param62[2] = {0x00,0x90};  
static char boe_param63[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  
static char boe_param64[2] = {0x00,0xA0};  
static char boe_param65[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param66[2] = {0x00,0xB0};  
static char boe_param67[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param68[2] = {0x00,0xC0};  
static char boe_param69[16] = {0xCB,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param70[2] = {0x00,0xD0}; 
static char boe_param71[16] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00};

static char boe_param72[2] = {0x00,0xE0};  
static char boe_param73[15] = {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05};

static char boe_param74[2] = {0x00,0xF0};
static char boe_param75[12] = {0xCB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/****************panel pad mapping control***************************/

static char boe_param76[2] = {0x00,0x80};
static char boe_param77[16] = {0xCC,0x0A,0x0C,0x0E,0x10,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param78[2] = {0x00,0x90};
static char boe_param79[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x2E,0x2D,0x09,0x0B,0x0D,0x0F,0x01,0x03,0x00,0x00};

static char boe_param80[2] = {0x00,0xA0};
static char boe_param81[15] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2E,0x2D};

static char boe_param82[2] = {0x00,0xB0};
static char boe_param83[16] = {0xCC,0x0F,0x0D,0x0B,0x09,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param84[2] = {0x00,0xC0};
static char boe_param85[16] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x2D,0x2E,0x10,0x0E,0x0C,0x0A,0x04,0x02,0x00,0x00};

static char boe_param86[2] = {0x00,0xD0};
static char boe_param87[15] = {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2D,0x2E};

/****************************panel timing setting*********************/
static char boe_param88[2] = {0x00,0x80};  
static char boe_param89[13] = {0xCE,0x8D,0x03,0x00,0x8C,0x03,0x00,0x8B,0x03,0x00,0x8A,0x03,0x00};

static char boe_param90[2] = {0x00,0x90}; 
static char boe_param91[15] = {0xCE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param92[2] = {0x00,0xA0};
static char boe_param93[15] = {0xCE,0x38,0x0B,0x04,0xFC,0x00,0x10,0x10,0x38,0x0A,0x04,0xFD,0x00,0x10,0x10};        

static char boe_param94[2] = {0x00,0xB0};  
static char boe_param95[15] = {0xCE,0x38,0x09,0x04,0xFE,0x00,0x10,0x10,0x38,0x08,0x04,0xFF,0x00,0x10,0x10};

static char boe_param96[2] = {0x00,0xC0};  
static char boe_param97[15] = {0xCE,0x38,0x07,0x05,0x00,0x00,0x10,0x10,0x38,0x06,0x05,0x01,0x00,0x10,0x10};

static char boe_param98[2] = {0x00,0xD0};  
static char boe_param99[15] = {0xCE,0x38,0x05,0x05,0x02,0x00,0x10,0x10,0x38,0x04,0x05,0x03,0x00,0x10,0x10};

static char boe_param100[2] = {0x00,0x80};   
static char boe_param101[15] = {0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param102[2] = {0x00,0x90};  
static char boe_param103[15] = {0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param104[2] = {0x00,0xA0};  
static char boe_param105[15] = {0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param106[2] = {0x00,0xB0};   
static char boe_param107[15] = {0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char boe_param108[2] = {0x00,0xC0};   
static char boe_param109[12] = {0xCF,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x00,0x08};

static char boe_param110[2] = {0x00,0xB5};   
static char boe_param111[7] = {0xC5,0x33,0xF1,0xFF,0x33,0xF1,0xFF};   

static char boe_param112[2] = {0x00,0x00};   
static char boe_param113[2] = {0x36,0x00}; 
/**************CE**********************/
#if 1
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
	static char boe_param123[2] = {0x00,0x00};             //Orise mode disable
	static char boe_param124[4] = {0xff,0xff,0xff,0xff};
	

static struct dsi_cmd_desc boe_otm1283_video_init_cmds[] = {
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param0),boe_param0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param1),boe_param1},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param2),boe_param2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param3),boe_param3},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param4),boe_param4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param5),boe_param5},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param6),boe_param6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param7),boe_param7},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param8),boe_param8},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param9),boe_param9},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param10),boe_param10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param11),boe_param11},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param12),boe_param12},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param13),boe_param13},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param14),boe_param14},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param15),boe_param15},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param16),boe_param16},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param17),boe_param17},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param18),boe_param18},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param19),boe_param19},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param19a),boe_param19a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param19b),boe_param19b},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param20),boe_param20},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param21),boe_param21},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param22),boe_param22},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param23),boe_param23},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param24),boe_param24},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param25),boe_param25},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param26),boe_param26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param27),boe_param27},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param28),boe_param28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param29),boe_param29},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param30),boe_param30},
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param31),boe_param31},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param32),boe_param32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param33),boe_param33},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param34),boe_param34},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param35),boe_param35},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param36),boe_param36},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37),boe_param37},

	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37a),boe_param37a},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37b),boe_param37b},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37c),boe_param37c},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param37d),boe_param37d},
	
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param38),boe_param38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param39),boe_param39},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param40),boe_param40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param41),boe_param41},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param42),boe_param42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param43),boe_param43},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param44),boe_param44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param45),boe_param45},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param46),boe_param46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param47),boe_param47},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param48),boe_param48},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param49),boe_param49},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param50),boe_param50},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param51),boe_param51},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param52),boe_param52},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param53),boe_param53},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param54),boe_param54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param55),boe_param55},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param56),boe_param56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param57),boe_param57},
	//{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param58),boe_param58},
	//{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param59),boe_param59},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param60),boe_param60},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param61),boe_param61},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param62),boe_param62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param63),boe_param63},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param64),boe_param64},
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param65),boe_param65},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param66),boe_param66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param67),boe_param67},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param68),boe_param68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param69),boe_param69},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param70),boe_param70},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param71),boe_param71},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param72),boe_param72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param73),boe_param73},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param74),boe_param74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param75),boe_param75},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param76),boe_param76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param77),boe_param77},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param78),boe_param78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param79),boe_param79},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param80),boe_param80},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param81),boe_param81},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param82),boe_param82},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param83),boe_param83},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param84),boe_param84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param85),boe_param85},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param86),boe_param86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param87),boe_param87},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param88),boe_param88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param89),boe_param89},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param90),boe_param90},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param91),boe_param91},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param92),boe_param92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param93),boe_param93},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param94),boe_param94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param95),boe_param95},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param96),boe_param96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param97),boe_param97},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param98),boe_param98},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param99),boe_param99},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param100),boe_param100},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param101),boe_param101},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param102),boe_param102},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param103),boe_param103},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param104),boe_param104},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param105),boe_param105},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param106),boe_param106},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param107),boe_param107},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param108),boe_param108},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param109),boe_param109},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param110),boe_param110},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param111),boe_param111},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param112),boe_param112},
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param113),boe_param113},

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
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(boe_param123),boe_param123},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0,sizeof(boe_param124),boe_param124},
//cabc&ce switch
	{DTYPE_GEN_WRITE2, 1, 0, 0, 0,sizeof(param_cabc_0x55),param_cabc_0x55},
	
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(exit_sleep),exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(display_on),display_on},

};
/*************BOE,OTM1283******************/

static struct dsi_cmd_desc orise_boe_cmd_on_cmds[] = {
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
	   mipi_dsi_cmds_tx(&orise_boe_tx_buf, cabc_cmds,ARRAY_SIZE(cabc_cmds));
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

static int mipi_orise_boe_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct msm_panel_info *pinfo;

	static int first_time_panel_on = 1;


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
		           // mipi_dsi_cmds_tx(&orise_boe_tx_buf, otm1283_video_init_cmds,ARRAY_SIZE(otm1283_video_init_cmds));
		           mipi_set_tx_power_mode(0);	
	       }
	       else
	       { 
			     mipi_dsi_cmds_tx(&orise_boe_tx_buf, boe_otm1283_video_init_cmds,ARRAY_SIZE(boe_otm1283_video_init_cmds));
	       }
	   }
	   else
	   {
		            mipi_dsi_cmds_tx(&orise_boe_tx_buf, orise_boe_cmd_on_cmds,ARRAY_SIZE(orise_boe_cmd_on_cmds));
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

static int mipi_orise_boe_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	printk("%s\n", __func__);
	
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	  mipi_dsi_cmds_tx(&orise_boe_tx_buf, otm1283_display_off_cmds,ARRAY_SIZE(otm1283_display_off_cmds));

	return 0;
}

static int __devinit mipi_orise_boe_lcd_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct platform_device *current_pdev;

	printk(KERN_ERR "%s\n", __func__);
	
	if (pdev->id == 0) {
		mipi_orise_boe_pdata = pdev->dev.platform_data;
		mipi_orise_boe_pdata->panel_config_gpio(1);
#ifdef CONFIG_BACKLIGHT_PM8921
		led_pwm = mipi_orise_boe_pdata->gpio_num[0];
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
	.probe  = mipi_orise_boe_lcd_probe,
	.driver = {
		.name   = "mipi_orise_boe",
	},
};

static struct msm_fb_panel_data orise_boe_panel_data = {
	.on		= mipi_orise_boe_lcd_on,
	.off		= mipi_orise_boe_lcd_off,
/*[ECID:000000] ZTEBSP wangbing, for orise_boe lcd cabc, 20120820*/
#if defined(CONFIG_BACKLIGHT_CABC) || defined(CONFIG_BACKLIGHT_PM8921)
	.set_backlight	 = mipi_set_backlight, 
#endif	
};

static int ch_used[3];

int mipi_orise_boe_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	
	printk(KERN_ERR "%s\n", __func__);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_orise_boe", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	orise_boe_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &orise_boe_panel_data,
		sizeof(orise_boe_panel_data));
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

static int __init mipi_orise_boe_lcd_init(void)
{
	printk("%s\n", __func__);

	mipi_dsi_buf_alloc(&orise_boe_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&orise_boe_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_orise_boe_lcd_init);
