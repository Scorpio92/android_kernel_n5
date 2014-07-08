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
#include "mipi_sharp.h"
#include "mdp4.h"
//#include <mach/gpio.h>
//#include <mach/gpiomux.h>

//wangtao P864A20 bug-detect-lcd-crash begin
#ifdef CONFIG_PROJECT_P864A20
uint8 lcd_crash_trap = 0;
#endif 
//wangtao P864A20 bug-detect-lcd-crash end

//wangbing
//static struct mipi_dsi_panel_platform_data *mipi_sharp_pdata;
static struct msm_panel_common_pdata *mipi_sharp_pdata;

static struct dsi_buf sharp_tx_buf;
static struct dsi_buf sharp_rx_buf;


static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

//zhangqi add for sharp lcd begin 
/*************SHARP R63311****************/
static char index_b0_04[2] = {0xb0,0x04};
//static char index_b0_03[2] = {0xb0,0x03};
static char index_00[2] = {0x00,0x00};
static char index_d6[2] = {0xd6,0x01};
static char index_51[3] = {0x51,0x0f,0xff};
static char index_53[2] = {0x53,0x2C};
static char index_55[2] = {0x55,0x01};
static char index_ce[8] ={0xce,0x00,0x01,0x88,0xc1,0x00,0x1e,0x04};
static struct dsi_cmd_desc sharp_video_on_cmds[] = {
#if 0
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_b0_04), index_b0_04},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_d6),index_d6},

	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_b0_03), index_b0_03},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_53), index_53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_55), index_55},
	//{DTYPE_DCS_WRITE, 1, 0, 0, 200, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on), display_on},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(index_51), index_51},
#else
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_b0_04), index_b0_04},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(index_00), index_00},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_d6),index_d6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(index_51), index_51},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_53), index_53},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_55), index_55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_ce), index_ce},
	//{DTYPE_DCS_WRITE, 1, 0, 0, 200, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on), display_on},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
#endif
};


static struct dsi_cmd_desc sharp_video_off_cmds[] = {

//shihuiqin for cabc
	{DTYPE_DCS_WRITE, 1, 0, 0, 10,sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,sizeof(enter_sleep), enter_sleep},
};

//zhangqi add for sharp lcd end

//[ECID 000000]ZTEBSP zhangqi add for CABC begin
#ifdef CONFIG_BACKLIGHT_CABC

static struct dsi_cmd_desc cabc_cmds[] = {
{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(index_ce), index_ce},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(index_51), index_51},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_53), index_53},
{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(index_55), index_55},
};

//static int bl_level;

void mipi_sharp_set_backlight(struct msm_fb_data_type *mfd)
{
         /*value range is 1--32*/
	 int current_lel = mfd->bl_level;
	 uint16 level_to_map=0;
	 //unsigned long flags;
	 printk("zhangqi add for CABC level=%d in %s func \n ",current_lel,__func__);

	   if(current_lel==0)
	   {
		index_51[1]=0x00;
		index_51[2]=0x00;
	   }
	   else
	   {
		//zhangqi add for sharp lcd begin 
		level_to_map=current_lel*(4096/15);
		index_51[1]=(level_to_map & 0xff00)>>8;
		index_51[2]=level_to_map & 0x00ff;
		//zhangqi add for sharp lcd end
	   }
	   	 printk("zhangqi add r63311 backlightcabc index_51=%x %x  \n",index_51[1],index_51[2]);
		 mipi_set_tx_power_mode(0);
		 mipi_dsi_cmds_tx(&sharp_tx_buf, cabc_cmds,ARRAY_SIZE(cabc_cmds));
		 mipi_set_tx_power_mode(1);

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

static void mipi_sharp_set_backlight(struct msm_fb_data_type *mfd)
{
	int level = mfd->bl_level;

	pr_debug("%s.lvl=%d.\n", __func__, level);

	set_backlight_level(bl_pwm, level);

	bl_level = level;
}
#endif




static int mipi_sharp_lcd_on(struct platform_device *pdev)
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
		//mipi_sharp_pdata->panel_config_gpio(1);
	}


	if(first_time_panel_on)
	{
		printk("zhangqi add %s  first_time_panel_on=%d\n",__func__,first_time_panel_on);
	}

//wangtao P864A20 bug-detect-lcd-crash begin
#ifdef CONFIG_PROJECT_P864A20
     lcd_crash_trap = 0;
#endif
//wangtao P864A20 bug-detect-lcd-crash end	


//zhangqi add for sharp lcd begin 
	printk("R63311_SHARP\n");
	if(first_time_panel_on)
	{
		first_time_panel_on = 0;
		mipi_set_tx_power_mode(1);	
		mipi_dsi_cmds_tx(&sharp_tx_buf,sharp_video_on_cmds,ARRAY_SIZE(sharp_video_on_cmds));
		mipi_set_tx_power_mode(0);	
	}
	else
	{
					  
		mipi_set_tx_power_mode(1);	
		mipi_dsi_cmds_tx(&sharp_tx_buf,sharp_video_on_cmds, ARRAY_SIZE(sharp_video_on_cmds));
		mipi_set_tx_power_mode(0);	
	}
				
//mipi_dsi_cmd_bta_sw_trigger(); /* clean up ack_err_status */				
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

static int mipi_sharp_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	printk("%s\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	printk("R63311_SHARP power off \n");

//wangtao P864A20 bug-detect-lcd-crash begin
#ifdef CONFIG_PROJECT_P864A20
     lcd_crash_trap = 1;
#endif
//wangtao P864A20 bug-detect-lcd-crash end
	
	mipi_dsi_cmds_tx(&sharp_tx_buf, sharp_video_off_cmds,ARRAY_SIZE(sharp_video_off_cmds));
	return 0;
}

static void mipi_sharp_shutdown(struct platform_device *pdev)
{
	printk("%s\n", __func__);
	printk("R63311_SHARP shutdown \n");
	mipi_dsi_cmds_tx(&sharp_tx_buf, sharp_video_off_cmds,ARRAY_SIZE(sharp_video_off_cmds));
	//return 0;
}
//static struct msm_fb_data_type *sharp_mfd;


static int __devinit mipi_sharp_lcd_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct platform_device *current_pdev;
//	static struct mipi_dsi_phy_ctrl *phy_settings;

	printk(KERN_ERR "%s\n", __func__);
	
	if (pdev->id == 0) {

	printk(KERN_ERR "pdev->id == 0\n");
		
		mipi_sharp_pdata = pdev->dev.platform_data;

#if 0
		if (mipi_sharp_pdata
			&& mipi_sharp_pdata->phy_ctrl_settings) {
			phy_settings = (mipi_sharp_pdata->phy_ctrl_settings);
		}
#endif

		mipi_sharp_pdata->panel_config_gpio(1);



#ifdef CONFIG_BACKLIGHT_PM8921
		led_pwm = mipi_sharp_pdata->gpio_num[0];
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


#if 0
msleep(200);
		mipi_get_icpanelid(mfd);
		if(LcdPanelID != (u32)HIMAX8394_BOE_HD_LCM)
		            change_mipi_para(mfd->panel_info);
#endif
//shihuiqin for test	


		mipi  = &mfd->panel_info.mipi;
#if 0
		if (phy_settings != NULL)
			mipi->dsi_phy_db = phy_settings;
#endif		
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
	.probe  = mipi_sharp_lcd_probe,
	.shutdown =mipi_sharp_shutdown,
	.driver = {
		.name   = "mipi_sharp",
	},
};

static struct msm_fb_panel_data sharp_panel_data = {
	.on		= mipi_sharp_lcd_on,
	.off		= mipi_sharp_lcd_off,
/*[ECID:000000] ZTEBSP wangbing, for sharp lcd cabc, 20120820*/
#if defined(CONFIG_BACKLIGHT_CABC) || defined(CONFIG_BACKLIGHT_PM8921)
	.set_backlight	 = mipi_sharp_set_backlight, 
#endif	
};

static int ch_used[3];

int mipi_sharp_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	
	printk(KERN_ERR "%s\n", __func__);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_sharp", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	sharp_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &sharp_panel_data,
		sizeof(sharp_panel_data));
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

static int __init mipi_sharp_lcd_init(void)
{
	printk("%s\n", __func__);

	mipi_dsi_buf_alloc(&sharp_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&sharp_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_sharp_lcd_init);
