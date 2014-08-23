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

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/msm_ion.h>
#include <asm/mach-types.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/ion.h>
#include <mach/msm_bus_board.h>
#include <mach/socinfo.h>

#include "devices.h"
#include "board-8064.h"

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
/* prim = 1366 x 768 x 3(bpp) x 3(pages) */
#if defined  CONFIG_PROJECT_P864A10 || defined(CONFIG_PROJECT_P864H01)
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 3, 0x10000)
#elif defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 3*3, 0x10000) //zhangqi add for 1080p
#else
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 3, 0x10000)
#endif
#else
/* prim = 1366 x 768 x 3(bpp) x 2(pages) */
#define MSM_FB_PRIM_BUF_SIZE roundup(1920 * 1088 * 4 * 2, 0x10000)
#endif

#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE, 4096)

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1376 * 768 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */


static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

#define LVDS_CHIMEI_PANEL_NAME "lvds_chimei_wxga"
#define LVDS_FRC_PANEL_NAME "lvds_frc_fhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME "mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME "mipi_video_chimei_wxga"
#define HDMI_PANEL_NAME "hdmi_msm"
#define TVOUT_PANEL_NAME "tvout_msm"
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213*/
#define MIPI_TOSHIBA_ATTRI_PANEL_NAME  "mipi_video_attri_wxga"
/*[ECID:000000] ZTEBSP wangbing, for orise lcd, 20120823*/
#define MIPI_ORISE_720P_PANEL_NAME  "mipi_video_orise_720p"
#define MIPI_ORISE_LEAD_720P_PANEL_NAME  "mipi_video_orise_lead_720p"
#define MIPI_ORISE_BOE_720P_PANEL_NAME  "mipi_video_orise_boe_720p"
#define MIPI_ORISE_CPT_720P_PANEL_NAME  "mipi_video_orise_cpt_720p"

#define MIPI_SHARP_1080P_PANEL_NAME  "mipi_video_sharp_1080p"

#define LVDS_PIXEL_MAP_PATTERN_1	1
#define LVDS_PIXEL_MAP_PATTERN_2	2

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
static unsigned char hdmi_is_primary = 1;
#else
static unsigned char hdmi_is_primary;
#endif

unsigned char apq8064_hdmi_as_primary_selected(void)
{
	return hdmi_is_primary;
}

static void set_mdp_clocks_for_wuxga(void);

//[ECID:000000] ZTEBSP zhangqi add begin
typedef enum {
	LCD_PANEL_NOPANEL,
       OTM1283_BOE_HD_LCM,	//P864A10
       OTM1283_LEAD_HD_LCM,	//P864A10
       OTM1283_CPT_HD_LCM,	//P864A10
       E63311_SHARP_FHD_LCM,//P864A20
	LCD_PANEL_MAX
} LCD_PANEL_ID;
 int lcd_enum;

static int __init panel_num_setup(char *str)
{
	int cal = simple_strtol(str, NULL, 0);
	printk("zhangqi add for logo panel_num=%d \n",cal);
	lcd_enum = cal;
	return 1;
}

__setup("panel=", panel_num_setup);

//[ECID:000000] ZTEBSP zhangqi add end
static int msm_fb_detect_panel(const char *name)
{
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
	//u32 version;

#if 0
	if (machine_is_apq8064_liquid()) {
		version = socinfo_get_platform_version();
		if ((SOCINFO_VERSION_MAJOR(version) == 1) &&
			(SOCINFO_VERSION_MINOR(version) == 1)) {
			if (!strncmp(name, MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
				return 0;
		} else {
			if (!strncmp(name, LVDS_CHIMEI_PANEL_NAME,
				strnlen(LVDS_CHIMEI_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
				return 0;
		}
	} else if (machine_is_apq8064_mtp()) {
		if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
			strnlen(MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
			return 0;
	} else if (machine_is_apq8064_cdp()) {
		if (!strncmp(name, LVDS_CHIMEI_PANEL_NAME,
			strnlen(LVDS_CHIMEI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
			return 0;
	} else if (machine_is_mpq8064_dtv()) {
		if (!strncmp(name, LVDS_FRC_PANEL_NAME,
			strnlen(LVDS_FRC_PANEL_NAME,
			PANEL_NAME_MAX_LEN))) {
			set_mdp_clocks_for_wuxga();
			return 0;
		}
	}
#else
/*[ECID:000000] ZTEBSP wangbing, for orise lcd, 20120823*/
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
	switch (lcd_enum)
	{
		case OTM1283_BOE_HD_LCM : 
			if (!strncmp(name, MIPI_ORISE_BOE_720P_PANEL_NAME,strnlen(MIPI_ORISE_BOE_720P_PANEL_NAME,PANEL_NAME_MAX_LEN)))
				return 0;
			break;
		case OTM1283_LEAD_HD_LCM : 
			if (!strncmp(name, MIPI_ORISE_LEAD_720P_PANEL_NAME,strnlen(MIPI_ORISE_LEAD_720P_PANEL_NAME,PANEL_NAME_MAX_LEN)))
				return 0;
			break ;
		case OTM1283_CPT_HD_LCM : 
			if (!strncmp(name, MIPI_ORISE_CPT_720P_PANEL_NAME,strnlen(MIPI_ORISE_CPT_720P_PANEL_NAME,PANEL_NAME_MAX_LEN)))
				return 0;
			break ;
		 default :
			if (!strncmp(name, MIPI_ORISE_LEAD_720P_PANEL_NAME,strnlen(MIPI_ORISE_LEAD_720P_PANEL_NAME,PANEL_NAME_MAX_LEN)))
				return 0;
		 	printk("%s no match panel detected\n",__func__);
			break ;
	}
#elif defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
	if (!strncmp(name, MIPI_SHARP_1080P_PANEL_NAME,
			strnlen(MIPI_SHARP_1080P_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	//if (!strncmp(name, MIPI_ORISE_720P_PANEL_NAME,strnlen(MIPI_ORISE_720P_PANEL_NAME,PANEL_NAME_MAX_LEN)))
	//	return 0;
#else
	if (!strncmp(name, MIPI_TOSHIBA_ATTRI_PANEL_NAME,
			strnlen(MIPI_TOSHIBA_ATTRI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;
#endif
#endif
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 end*/

	if (!strncmp(name, HDMI_PANEL_NAME,
			strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
		if (apq8064_hdmi_as_primary_selected())
			set_mdp_clocks_for_wuxga();
		return 0;
	}


	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name              = "msm_fb",
	.id                = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

void __init apq8064_allocate_fb_region(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));
}

#define MDP_VSYNC_GPIO 0

static struct msm_bus_vectors mdp_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors mdp_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 230400000 * 2,
		.ib = 288000000 * 2,
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 334080000 * 2,
		.ib = 417600000 * 2,
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};

static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
	.mdp_max_clk = 200000000,
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
	.mdp_rev = MDP_REV_44,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = BIT(ION_CP_MM_HEAP_ID),
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
	.cont_splash_enabled=1,//zhangqi add for cont_splash
	.mdp_iommu_split_domain = 1,
};

void __init apq8064_mdp_writeback(struct memtype_reserve* reserve_table)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}

static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
static int hdmi_cec_power(int on);
static int hdmi_gpio_config(int on);
static int hdmi_panel_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
	.panel_power = hdmi_panel_power,
	.gpio_config = hdmi_gpio_config,
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};

static char wfd_check_mdp_iommu_split_domain(void)
{
	return mdp_pdata.mdp_iommu_split_domain;
}

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
static struct msm_wfd_platform_data wfd_pdata = {
	.wfd_check_mdp_iommu_split = wfd_check_mdp_iommu_split_domain,
};

static struct platform_device wfd_panel_device = {
	.name = "wfd_panel",
	.id = 0,
	.dev.platform_data = NULL,
};

static struct platform_device wfd_device = {
	.name          = "msm_wfd",
	.id            = -1,
	.dev.platform_data = &wfd_pdata,
};
#endif

/* HDMI related GPIOs */
#define HDMI_CEC_VAR_GPIO	69
#define HDMI_DDC_CLK_GPIO	70
#define HDMI_DDC_DATA_GPIO	71
#define HDMI_HPD_GPIO		72

/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120611 start*/
/**
 * ATTRI panel on/off
 *
 * @param on
 *
 * @return int
 */
#if defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
 //zhangqi add for sharp lcd begin 
#define TPS61135_EN 51
#endif
#define LMS_RESET PM8921_GPIO_PM_TO_SYS(25)
#define LMS_BL_CTRL PM8921_GPIO_PM_TO_SYS(26)
#define CABC_EN  1
//#define b2r_1v2_en PM8921_GPIO_PM_TO_SYS(6)
//#define b2r_1v8_en PM8921_GPIO_PM_TO_SYS(7)
static bool dsi_power_on;

#if defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
static int mipi_dsi_lms_panel_power(int on)
{
//	static struct regulator *reg_l8, *reg_s4, *reg_l23, *reg_l2, *reg_l17, *reg_l9;
	static struct regulator *reg_l23, *reg_l2,*reg_l11;
	static int first_time_panel_on=1;
	int rc;
//	static int lms_reset;
	pr_info("%s: on=%d\n", __func__, on);
	if (!dsi_power_on) {
#if 0
	lms_reset = PM8921_GPIO_PM_TO_SYS(18); /* Displays  (rst_n)*/
#endif		
		rc = gpio_request(LMS_RESET, "disp_rst_n");
		if (rc) {
			pr_err("request lms_reset failed, rc=%d\n", rc);
		}
		gpio_direction_output(LMS_RESET, 1);//zhangqi add for logo
		
 //zhangqi add for sharp lcd begin 
		rc = gpio_request(TPS61135_EN, "TPS61135_EN");
		if (rc) {
			pr_err("request TPS61135_EN failed, rc=%d\n", rc);
		}
		gpio_direction_output(TPS61135_EN, 0);
 //zhangqi add for sharp lcd end

           //not need l2
             reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}
		
		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_voltage(reg_l11, 3000000, 3000000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

/*lcd panel power++*/
		reg_l23 = regulator_get(NULL,
				"lms_panel_1p8");
		if (IS_ERR(reg_l23)) {
			pr_err("could not get 8921_l23, rc = %ld\n",
				PTR_ERR(reg_l23));
			return -ENODEV;
		}		
		rc = regulator_set_voltage(reg_l23, 1800000, 1800000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}
/*lcd panel power--*/

		dsi_power_on = true;
	}

	if (on) {
		

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		
	       rc = regulator_set_optimum_mode(reg_l23, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l23);
		if (rc) {
			pr_err("enable l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		
	       msleep(200);
		gpio_set_value_cansleep(TPS61135_EN, 1); /* disp disable (resx=0) */

		if (first_time_panel_on==0)
		{
			msleep(100);
			gpio_set_value_cansleep(LMS_RESET, 0); /* disp disable (resx=0) */
	 		msleep(20);
			
			gpio_set_value_cansleep(LMS_RESET, 1); /* disp enable */
			msleep(20);
		}
		first_time_panel_on=0;

	}		
	
	else {
		
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		
/*[ECID:000000] ZTEBSP wangbing, for orise lcd powerdown timging, 20120827*/
		gpio_set_value_cansleep(LMS_RESET, 0);

		rc = regulator_disable(reg_l23);
		if (rc) {
			pr_err("disable reg_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio_set_value_cansleep(TPS61135_EN, 0); /* disp disable (resx=0) */
		//gpio_set_value_cansleep(LMS_BL_CTRL, 0); /* BACKLIGHT CTRL */
 		msleep(100);

		rc = regulator_set_optimum_mode(reg_l23, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_optimum_mode(reg_l2, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
/*[ECID:000000] ZTEBSP wangbing, for orise lcd powerdown timging, 20120827*/
//		gpio_set_value_cansleep(LMS_RESET, 0);

	}
	return 0;
}
#endif
#if defined  CONFIG_PROJECT_P864A10 || defined(CONFIG_PROJECT_P864H01)
static int mipi_dsi_lms_panel_power(int on)
{
//	static struct regulator *reg_l8, *reg_s4, *reg_l23, *reg_l2, *reg_l17, *reg_l9;
	static struct regulator *reg_l23, *reg_l2,*reg_l11;
	static int first_time_panel_on=1;
	int rc;
//	static int lms_reset;
	pr_info("%s: on=%d\n", __func__, on);
	if (!dsi_power_on) {
#if 0
	lms_reset = PM8921_GPIO_PM_TO_SYS(18); /* Displays  (rst_n)*/
#endif		
		rc = gpio_request(LMS_RESET, "disp_rst_n");
		if (rc) {
			pr_err("request lms_reset failed, rc=%d\n", rc);
		}
		gpio_direction_output(LMS_RESET, 1);//zhangqi add for logo

		rc = gpio_request(CABC_EN, "cabc_en");
		if (rc) {
			pr_err("request cabc_en failed, rc=%d\n", rc);
		}
		gpio_direction_output(CABC_EN, 0);
		
           //not need l2
             reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}
		
		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_set_voltage(reg_l11, 3000000, 3000000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

		reg_l23 = regulator_get(NULL,
				"lms_panel_1p8");
		if (IS_ERR(reg_l23)) {
			pr_err("could not get 8921_l23, rc = %ld\n",
				PTR_ERR(reg_l23));
			return -ENODEV;
		}		
		rc = regulator_set_voltage(reg_l23, 1800000, 1800000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}


		dsi_power_on = true;
	}

	if (on) {
		
		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	       rc = regulator_set_optimum_mode(reg_l23, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l23);
		if (rc) {
			pr_err("enable l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	 	msleep(5);
		
		rc = regulator_set_optimum_mode(reg_l11, 110000);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l11);
		if (rc) {
			pr_err("enable l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}

	 	msleep(5);
		
		if (first_time_panel_on==0)
		{
			gpio_set_value_cansleep(LMS_RESET, 1); /* disp disable (resx=0) */
	 		msleep(50);
			
			gpio_set_value_cansleep(LMS_RESET, 0); /* disp disable (resx=0) */
	 		msleep(10);
			
			gpio_set_value_cansleep(LMS_RESET, 1); /* disp enable */
			msleep(10);
		}
		first_time_panel_on=0;
	}		
	
	else {
	
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_l2, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
	
		

/*[ECID:000000] ZTEBSP wangbing, for orise lcd powerdown timging, 20120827*/
		gpio_set_value_cansleep(LMS_RESET, 0);
	
	 	msleep(5);
		
		rc = regulator_disable(reg_l11);
		if (rc) {
			pr_err("disable reg_l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_l11, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		
	 	msleep(5);
		rc = regulator_disable(reg_l23);
		if (rc) {
			pr_err("disable reg_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_set_optimum_mode(reg_l23, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}

	}
	return 0;
}
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 end*/
#endif
static int mipi_dsi_panel_power(int on)
{
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
#if 1
mipi_dsi_lms_panel_power(on);

#else
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
	static struct regulator *reg_lvs7, *reg_l2, *reg_l11, *reg_ext_3p3v;
	static int gpio36, gpio25, gpio26, mpp3;
	int rc;

	pr_debug("%s: on=%d\n", __func__, on);

	if (!dsi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev,
						"dsi1_avdd");
		if (IS_ERR(reg_l11)) {
				pr_err("could not get 8921_l11, rc = %ld\n",
						PTR_ERR(reg_l11));
				return -ENODEV;
		}
		rc = regulator_set_voltage(reg_l11, 3000000, 3000000);
		if (rc) {
				pr_err("set_voltage l11 failed, rc=%d\n", rc);
				return -EINVAL;
		}

		if (machine_is_apq8064_liquid()) {
			reg_ext_3p3v = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vccs_3p3v");
			if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
				pr_err("could not get reg_ext_3p3v, rc = %ld\n",
					PTR_ERR(reg_ext_3p3v));
				reg_ext_3p3v = NULL;
				return -ENODEV;
			}
			mpp3 = PM8921_MPP_PM_TO_SYS(3);
			rc = gpio_request(mpp3, "backlight_en");
			if (rc) {
				pr_err("request mpp3 failed, rc=%d\n", rc);
				return -ENODEV;
			}
		}

		gpio25 = PM8921_GPIO_PM_TO_SYS(25);
		rc = gpio_request(gpio25, "disp_rst_n");
		if (rc) {
			pr_err("request gpio 25 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio26 = PM8921_GPIO_PM_TO_SYS(26);
		rc = gpio_request(gpio26, "pwm_backlight_ctrl");
		if (rc) {
			pr_err("request gpio 26 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio36 = PM8921_GPIO_PM_TO_SYS(36); /* lcd1_pwr_en_n */
		rc = gpio_request(gpio36, "lcd1_pwr_en_n");
		if (rc) {
			pr_err("request gpio 36 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		dsi_power_on = true;
	}

	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l11, 110000);
		if (rc < 0) {
			pr_err("set_optimum_mode l11 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l11);
		if (rc) {
			pr_err("enable l11 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		if (machine_is_apq8064_liquid()) {
			rc = regulator_enable(reg_ext_3p3v);
			if (rc) {
				pr_err("enable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
			gpio_set_value_cansleep(mpp3, 1);
		}

		gpio_set_value_cansleep(gpio36, 0);
		gpio_set_value_cansleep(gpio25, 1);
	} else {
		gpio_set_value_cansleep(gpio25, 0);
		gpio_set_value_cansleep(gpio36, 1);

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(mpp3, 0);

			rc = regulator_disable(reg_ext_3p3v);
			if (rc) {
				pr_err("disable reg_ext_3p3v failed, rc=%d\n",
					rc);
				return -ENODEV;
			}
		}

		rc = regulator_disable(reg_l11);
		if (rc) {
			pr_err("disable reg_l1 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}
#endif    //shihuiqin--
	return 0;
}
//zhangqi add for cons_splash begin 
static char mipi_dsi_splash_is_enabled(void)
{
	return mdp_pdata.cont_splash_enabled;
}
//zhangqi add for cons_splash end
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.dsi_power_save = mipi_dsi_panel_power,
	.splash_is_enabled = mipi_dsi_splash_is_enabled,//zhangqi add for cons_splash
};

static bool lvds_power_on;
static int lvds_panel_power(int on)
{
	static struct regulator *reg_lvs7, *reg_l2, *reg_ext_3p3v;
	static int gpio36, gpio26, mpp3;
	int rc;

	pr_debug("%s: on=%d\n", __func__, on);

	if (!lvds_power_on) {
		reg_lvs7 = regulator_get(&msm_lvds_device.dev,
				"lvds_vdda");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_lvds_device.dev,
				"lvds_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
				PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		reg_ext_3p3v = regulator_get(&msm_lvds_device.dev,
			"lvds_vccs_3p3v");
		if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
			pr_err("could not get reg_ext_3p3v, rc = %ld\n",
			       PTR_ERR(reg_ext_3p3v));
		    return -ENODEV;
		}

		gpio26 = PM8921_GPIO_PM_TO_SYS(26);
		rc = gpio_request(gpio26, "pwm_backlight_ctrl");
		if (rc) {
			pr_err("request gpio 26 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio36 = PM8921_GPIO_PM_TO_SYS(36); /* lcd1_pwr_en_n */
		rc = gpio_request(gpio36, "lcd1_pwr_en_n");
		if (rc) {
			pr_err("request gpio 36 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		mpp3 = PM8921_MPP_PM_TO_SYS(3);
		rc = gpio_request(mpp3, "backlight_en");
		if (rc) {
			pr_err("request mpp3 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		lvds_power_on = true;
	}

	if (on) {
		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_enable(reg_ext_3p3v);
		if (rc) {
			pr_err("enable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}

		gpio_set_value_cansleep(gpio36, 0);
		gpio_set_value_cansleep(mpp3, 1);
	} else {
		gpio_set_value_cansleep(mpp3, 0);
		gpio_set_value_cansleep(gpio36, 1);

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}

	return 0;
}

static int lvds_pixel_remap(void)
{
	if (machine_is_apq8064_cdp() ||
	    machine_is_apq8064_liquid()) {
		u32 ver = socinfo_get_version();
		if ((SOCINFO_VERSION_MAJOR(ver) == 1) &&
		    (SOCINFO_VERSION_MINOR(ver) == 0))
			return LVDS_PIXEL_MAP_PATTERN_1;
	} else if (machine_is_mpq8064_dtv()) {
		return LVDS_PIXEL_MAP_PATTERN_2;
	}
	return 0;
}

static struct lcdc_platform_data lvds_pdata = {
	.lcdc_power_save = lvds_panel_power,
	.lvds_pixel_remap = lvds_pixel_remap
};

#define LPM_CHANNEL 2
static int lvds_chimei_gpio[] = {LPM_CHANNEL};

static struct lvds_panel_platform_data lvds_chimei_pdata = {
	.gpio = lvds_chimei_gpio,
};

static struct platform_device lvds_chimei_panel_device = {
	.name = "lvds_chimei_wxga",
	.id = 0,
	.dev = {
		.platform_data = &lvds_chimei_pdata,
	}
};

#define FRC_GPIO_UPDATE	(SX150X_EXP4_GPIO_BASE + 8)
#define FRC_GPIO_RESET	(SX150X_EXP4_GPIO_BASE + 9)
#define FRC_GPIO_PWR	(SX150X_EXP4_GPIO_BASE + 10)

//wangtao fusion3-debug begin
/*
static int lvds_frc_gpio[] = {FRC_GPIO_UPDATE, FRC_GPIO_RESET, FRC_GPIO_PWR};
*/
//wangtao fusion3-debug end

/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
#define d2r_reset    0//PM8921_GPIO_PM_TO_SYS(43)
void lcd_d2r_config_gpio(int on)
{
	static struct regulator *d2r_1v2, *d2r_1v8,*d2r_1v8_pull_up;
	int rc;
	static bool initialised = 0;
	printk("%s %d\n", __func__, on);

	if (!initialised)
	{
		rc = gpio_request(d2r_reset, "d2r_rst_n");
		if (rc) {
			pr_err("request d2r_reset failed, rc=%d\n", rc);
		}
		gpio_direction_output(d2r_reset, 0);		

	       //LVS2 SMPS
		d2r_1v2 = regulator_get(NULL,
				"d2r_1v2");
		if (IS_ERR(d2r_1v2)) {
			pr_err("could not get d2r_1v2, rc = %ld\n",
				PTR_ERR(d2r_1v2));
//			return -ENODEV;
		}

		//L14  LDO
		d2r_1v8 = regulator_get(NULL,
				"d2r_1v8");
		if (IS_ERR(d2r_1v8)) {
			pr_err("could not get d2r_1v8, rc = %ld\n",
				PTR_ERR(d2r_1v8));
//			return -ENODEV;
		}

		//S4  SMPS   s4 should be always on debfined in msm_rpm_regulator_init_data
		d2r_1v8_pull_up = regulator_get(NULL,
				"d2r_1v8_pull_up");
		if (IS_ERR(d2r_1v8_pull_up)) {
			pr_err("could not get d2r_1v8_pull_up, rc = %ld\n",
				PTR_ERR(d2r_1v8_pull_up));
//			return -ENODEV;
		}

	#if 0	//lvs not support the voltage setting.
		rc = regulator_set_voltage(d2r_1v2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage d2r_1v2 failed, rc=%d\n", rc);
//			return -EINVAL;
		}
	#endif
		rc = regulator_set_voltage(d2r_1v8, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage d2r_1v8 failed, rc=%d\n", rc);
//			return -EINVAL;
		}

		rc = regulator_set_voltage(d2r_1v8_pull_up, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -EINVAL;
		}
		
		
		initialised = 1;
	}
		
#if 0	
	rc = gpio_request(LCD_3V3EN, "LCD_3VEN");
	if (rc) {
		pr_err("request gpio LCD_3V3EN failed, rc=%d\n", rc);
		return ;
	}

	rc = gpio_request(LCD_12V_BOOST_EN, "12V_BOOST_EN");
	if (rc) {
		pr_err("request gpio 12V_BOOST_EN failed, rc=%d\n", rc);
		return ;
	}

		rc = gpio_request(LCD_POWER_EN, "12V_BOOST_EN");
	if (rc) {
		pr_err("request gpio 12V_BOOST_EN failed, rc=%d\n", rc);
		return ;
	}
#endif

	if(on){
#if 0
		gpio_set_value_cansleep(LCD_3V3EN, 1);
		msleep(50);

		rc = gpio_direction_output(LCD_POWER_EN, 1);
		if (rc) {
			pr_err("%s: unable to set_direction for mxt_ldo_en gpio [%d]\n",
			__func__, LCD_POWER_EN);

		}
		msleep(50);
//		gpio_set_value_cansleep(LCD_12V_BOOST_EN, 1);
#endif		
		rc = regulator_enable(d2r_1v2);
		if (rc) {
			pr_err("enable d2r_1v2 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

		rc = regulator_enable(d2r_1v8);
		if (rc) {
			pr_err("enable d2r_1v8 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

		rc = regulator_enable(d2r_1v8_pull_up);
		if (rc) {
			pr_err("enable d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -ENODEV;
		}
		
		
		msleep(5);
		gpio_set_value_cansleep(d2r_reset, 1);
		msleep(10);
	}
	else
	{
#if 0	
//		gpio_set_value_cansleep(LCD_12V_BOOST_EN, 0);
		gpio_set_value_cansleep(LCD_3V3EN, 0);
//		gpio_set_value_cansleep(LCD_POWER_EN,0);
#endif
		gpio_set_value_cansleep(d2r_reset, 0);
		msleep(5);

		rc = regulator_disable(d2r_1v8);
		if (rc) {
			pr_err("disable d2r_1v8 failed, rc=%d\n", rc);
//			return -ENODEV;
		}		

		rc = regulator_disable(d2r_1v2);
		if (rc) {
			pr_err("disable d2r_1v2 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

#if 0
//s4 should be always on debfined in msm_rpm_regulator_init_data
		rc = regulator_disable(d2r_1v8_pull_up);
		if (rc) {
			pr_err("disable d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -ENODEV;
		}
#endif
		

	}
#if 0	
	gpio_free(LCD_3V3EN);
//	gpio_free(LCD_12V_BOOST_EN);
//	gpio_free(LCD_POWER_EN);
#endif

}

/* [ECID¡êo000000000000] ZTEBSP wangtao139815 add for P864A20, begin */
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)||defined(CONFIG_PROJECT_P864A20) \
|| defined(CONFIG_PROJECT_P864G02)
/* [ECID¡êo000000000000] ZTEBSP wangtao139815 add for P864A20, end */
#else
static int dsi2rgb_gpio[2] = {
	LPM_CHANNEL,/* Backlight PWM-ID=0 for PMIC-GPIO#24 */
	0x1F08 /* DSI2LVDS Bridge GPIO Output, mask=0x1f, out=0x08 */
	};
static struct msm_panel_common_pdata mipi_dsi2rgb_pdata = {
	.gpio_num = dsi2rgb_gpio,
.panel_config_gpio = lcd_d2r_config_gpio,
};
static struct platform_device mipi_dsi2rgb_bridge_device = {
	.name = "mipi_tc358762",
	.id = 0,
	.dev.platform_data = &mipi_dsi2rgb_pdata,
};
#endif
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 end*/

//wangtao fusion3-debug begin
/*
static struct lvds_panel_platform_data lvds_frc_pdata = {
	.gpio = lvds_frc_gpio,
};

static struct platform_device lvds_frc_panel_device = {
	.name = "lvds_frc_fhd",
	.id = 0,
	.dev = {
		.platform_data = &lvds_frc_pdata,
	}
};
*/
//wangtao fusion3-debug end

static int lvds_frc_gpio[] = {FRC_GPIO_UPDATE, FRC_GPIO_RESET, FRC_GPIO_PWR};
static struct lvds_panel_platform_data lvds_frc_pdata = {
	.gpio = lvds_frc_gpio,
};

static struct platform_device lvds_frc_panel_device = {
	.name = "lvds_frc_fhd",
	.id = 0,
	.dev = {
		.platform_data = &lvds_frc_pdata,
	}
};

static int dsi2lvds_gpio[2] = {
	LPM_CHANNEL,/* Backlight PWM-ID=0 for PMIC-GPIO#24 */
	0x1F08 /* DSI2LVDS Bridge GPIO Output, mask=0x1f, out=0x08 */
};
static struct msm_panel_common_pdata mipi_dsi2lvds_pdata = {
	.gpio_num = dsi2lvds_gpio,
};

static struct platform_device mipi_dsi2lvds_bridge_device = {
	.name = "mipi_tc358764",
	.id = 0,
	.dev.platform_data = &mipi_dsi2lvds_pdata,
};

static int toshiba_gpio[] = {LPM_CHANNEL};
static struct mipi_dsi_panel_platform_data toshiba_pdata = {
	.gpio = toshiba_gpio,
};

static struct platform_device mipi_dsi_toshiba_panel_device = {
	.name = "mipi_toshiba",
	.id = 0,
	.dev = {
			.platform_data = &toshiba_pdata,
	}
};

/*[ECID:000000] ZTEBSP wangbing, for orise lcd, 20120823*/
#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
//#define orise_reset    0//PM8921_GPIO_PM_TO_SYS(43)
void lcd_sharp_config_gpio(int on)
{
	printk("%s %d do nothing\n", __func__, on);
}

static int sharp_gpio[2] = {
	LPM_CHANNEL,/* Backlight PWM-ID=0 for PMIC-GPIO#24 */
	0x1F08 /* DSI2LVDS Bridge GPIO Output, mask=0x1f, out=0x08 */
	};

static struct msm_panel_common_pdata mipi_sharp_pdata = {
	.gpio_num =sharp_gpio,
	.panel_config_gpio = lcd_sharp_config_gpio,
};

static struct platform_device mipi_sharp_device = {
	.name = "mipi_sharp",
	.id = 0,
	.dev.platform_data = &mipi_sharp_pdata,
};
#endif
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
#if 0
#define orise_reset    0//PM8921_GPIO_PM_TO_SYS(43)
#endif
void lcd_orise_config_gpio(int on)
{
	static struct regulator *d2r_1v2, *d2r_1v8,*d2r_1v8_pull_up;
	int rc;
	static bool initialised = 0;
	printk("%s %d\n", __func__, on);

	if (!initialised)
	{
#if 0
		rc = gpio_request(orise_reset, "orise_rst_n");
		if (rc) {
			pr_err("request orise_reset failed, rc=%d\n", rc);
		}
		gpio_direction_output(orise_reset, 0);		
#endif
	       //LVS2 SMPS
		d2r_1v2 = regulator_get(NULL,
				"d2r_1v2");
		if (IS_ERR(d2r_1v2)) {
			pr_err("could not get d2r_1v2, rc = %ld\n",
				PTR_ERR(d2r_1v2));
//			return -ENODEV;
		}

		//L14  LDO
		d2r_1v8 = regulator_get(NULL,
				"d2r_1v8");
		if (IS_ERR(d2r_1v8)) {
			pr_err("could not get d2r_1v8, rc = %ld\n",
				PTR_ERR(d2r_1v8));
//			return -ENODEV;
		}

		//S4  SMPS   s4 should be always on debfined in msm_rpm_regulator_init_data
		d2r_1v8_pull_up = regulator_get(NULL,
				"d2r_1v8_pull_up");
		if (IS_ERR(d2r_1v8_pull_up)) {
			pr_err("could not get d2r_1v8_pull_up, rc = %ld\n",
				PTR_ERR(d2r_1v8_pull_up));
//			return -ENODEV;
		}

	#if 0	//lvs not support the voltage setting.
		rc = regulator_set_voltage(d2r_1v2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage d2r_1v2 failed, rc=%d\n", rc);
//			return -EINVAL;
		}
	#endif
		rc = regulator_set_voltage(d2r_1v8, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage d2r_1v8 failed, rc=%d\n", rc);
//			return -EINVAL;
		}

		rc = regulator_set_voltage(d2r_1v8_pull_up, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -EINVAL;
		}
		
		
		initialised = 1;
	}
		
#if 0	
	rc = gpio_request(LCD_3V3EN, "LCD_3VEN");
	if (rc) {
		pr_err("request gpio LCD_3V3EN failed, rc=%d\n", rc);
		return ;
	}

	rc = gpio_request(LCD_12V_BOOST_EN, "12V_BOOST_EN");
	if (rc) {
		pr_err("request gpio 12V_BOOST_EN failed, rc=%d\n", rc);
		return ;
	}

		rc = gpio_request(LCD_POWER_EN, "12V_BOOST_EN");
	if (rc) {
		pr_err("request gpio 12V_BOOST_EN failed, rc=%d\n", rc);
		return ;
	}
#endif

	if(on){
#if 0
		gpio_set_value_cansleep(LCD_3V3EN, 1);
		msleep(50);

		rc = gpio_direction_output(LCD_POWER_EN, 1);
		if (rc) {
			pr_err("%s: unable to set_direction for mxt_ldo_en gpio [%d]\n",
			__func__, LCD_POWER_EN);

		}
		msleep(50);
//		gpio_set_value_cansleep(LCD_12V_BOOST_EN, 1);
#endif		
		rc = regulator_enable(d2r_1v2);
		if (rc) {
			pr_err("enable d2r_1v2 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

		rc = regulator_enable(d2r_1v8);
		if (rc) {
			pr_err("enable d2r_1v8 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

		rc = regulator_enable(d2r_1v8_pull_up);
		if (rc) {
			pr_err("enable d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -ENODEV;
		}
		
		
		msleep(5);
#if 0
		gpio_set_value_cansleep(orise_reset, 1);
		msleep(10);
#endif		
	}
	else
	{
#if 0	
//		gpio_set_value_cansleep(LCD_12V_BOOST_EN, 0);
		gpio_set_value_cansleep(LCD_3V3EN, 0);
//		gpio_set_value_cansleep(LCD_POWER_EN,0);
		gpio_set_value_cansleep(orise_reset, 0);
		msleep(5);
#endif
		rc = regulator_disable(d2r_1v8);
		if (rc) {
			pr_err("disable d2r_1v8 failed, rc=%d\n", rc);
//			return -ENODEV;
		}		

		rc = regulator_disable(d2r_1v2);
		if (rc) {
			pr_err("disable d2r_1v2 failed, rc=%d\n", rc);
//			return -ENODEV;
		}

#if 0
//s4 should be always on debfined in msm_rpm_regulator_init_data
		rc = regulator_disable(d2r_1v8_pull_up);
		if (rc) {
			pr_err("disable d2r_1v8_pull_up failed, rc=%d\n", rc);
//			return -ENODEV;
		}
#endif
		

	}
#if 0	
	gpio_free(LCD_3V3EN);
//	gpio_free(LCD_12V_BOOST_EN);
//	gpio_free(LCD_POWER_EN);
#endif

}

static int orise_gpio[2] = {
	LPM_CHANNEL,/* Backlight PWM-ID=0 for PMIC-GPIO#24 */
	0x1F08 /* DSI2LVDS Bridge GPIO Output, mask=0x1f, out=0x08 */
	};

static struct msm_panel_common_pdata mipi_orise_pdata = {
	.gpio_num = orise_gpio,
	.panel_config_gpio = lcd_orise_config_gpio,
};

static struct platform_device mipi_orise_lead_device = {
	.name = "mipi_orise_lead",
	.id = 0,
	.dev.platform_data = &mipi_orise_pdata,
};

static struct platform_device mipi_orise_boe_device = {
	.name = "mipi_orise_boe",
	.id = 0,
	.dev.platform_data = &mipi_orise_pdata,
};

static struct platform_device mipi_orise_cpt_device = {
	.name = "mipi_orise_cpt",
	.id = 0,
	.dev.platform_data = &mipi_orise_pdata,
};
#endif

static struct msm_bus_vectors dtv_bus_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors dtv_bus_def_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 566092800 * 2,
		.ib = 707616000 * 2,
	},
};

static struct msm_bus_paths dtv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_def_vectors),
		dtv_bus_def_vectors,
	},
};
static struct msm_bus_scale_pdata dtv_bus_scale_pdata = {
	dtv_bus_scale_usecases,
	ARRAY_SIZE(dtv_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_pdata = {
	.bus_scale_table = &dtv_bus_scale_pdata,
	.lcdc_power_save = hdmi_panel_power,
};

static int hdmi_panel_power(int on)
{
	int rc;

	pr_debug("%s: HDMI Core: %s\n", __func__, (on ? "ON" : "OFF"));
	rc = hdmi_core_power(on, 1);
	if (rc)
		rc = hdmi_cec_power(on);

	pr_debug("%s: HDMI Core: %s Success\n", __func__, (on ? "ON" : "OFF"));
	return rc;
}

static int hdmi_enable_5v(int on)
{
	/* TBD: PM8921 regulator instead of 8901 */
	static struct regulator *reg_8921_hdmi_mvs;	/* HDMI_5V */
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8921_hdmi_mvs) {
		reg_8921_hdmi_mvs = regulator_get(&hdmi_msm_device.dev,
			"hdmi_mvs");
		if (IS_ERR(reg_8921_hdmi_mvs)) {
			pr_err("could not get reg_8921_hdmi_mvs, rc = %ld\n",
				PTR_ERR(reg_8921_hdmi_mvs));
			reg_8921_hdmi_mvs = NULL;
			return -ENODEV;
		}
	}

	if (on) {
		rc = regulator_enable(reg_8921_hdmi_mvs);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
			return rc;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_hdmi_mvs);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"8921_hdmi_mvs", rc);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8921_lvs7, *reg_8921_s4, *reg_ext_3p3v;
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	/* TBD: PM8921 regulator instead of 8901 */
	if (!reg_ext_3p3v) {
		reg_ext_3p3v = regulator_get(&hdmi_msm_device.dev,
					     "hdmi_mux_vdd");
		if (IS_ERR_OR_NULL(reg_ext_3p3v)) {
			pr_err("could not get reg_ext_3p3v, rc = %ld\n",
			       PTR_ERR(reg_ext_3p3v));
			reg_ext_3p3v = NULL;
			return -ENODEV;
		}
	}

	if (!reg_8921_lvs7) {
		reg_8921_lvs7 = regulator_get(&hdmi_msm_device.dev,
					      "hdmi_vdda");
		if (IS_ERR(reg_8921_lvs7)) {
			pr_err("could not get reg_8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_8921_lvs7));
			reg_8921_lvs7 = NULL;
			return -ENODEV;
		}
	}
	if (!reg_8921_s4) {
		reg_8921_s4 = regulator_get(&hdmi_msm_device.dev,
					    "hdmi_lvl_tsl");
		if (IS_ERR(reg_8921_s4)) {
			pr_err("could not get reg_8921_s4, rc = %ld\n",
				PTR_ERR(reg_8921_s4));
			reg_8921_s4 = NULL;
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_s4, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_s4, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	if (on) {
		/*
		 * Configure 3P3V_BOOST_EN as GPIO, 8mA drive strength,
		 * pull none, out-high
		 */
		rc = regulator_set_optimum_mode(reg_ext_3p3v, 290000);
		if (rc < 0) {
			pr_err("set_optimum_mode ext_3p3v failed, rc=%d\n", rc);
			return -EINVAL;
		}

		rc = regulator_enable(reg_ext_3p3v);
		if (rc) {
			pr_err("enable reg_ext_3p3v failed, rc=%d\n", rc);
			return rc;
		}
		rc = regulator_enable(reg_8921_lvs7);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_vdda", rc);
			goto error1;
		}
		rc = regulator_enable(reg_8921_s4);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_lvl_tsl", rc);
			goto error2;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_ext_3p3v);
		if (rc) {
			pr_err("disable reg_ext_3p3v failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_lvs7);
		if (rc) {
			pr_err("disable reg_8921_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_s4);
		if (rc) {
			pr_err("disable reg_8921_s4 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;

error2:
	regulator_disable(reg_8921_lvs7);
error1:
	regulator_disable(reg_ext_3p3v);
	return rc;
}

static int hdmi_gpio_config(int on)
{
	int rc = 0;
	static int prev_on;
	int pmic_gpio14 = PM8921_GPIO_PM_TO_SYS(14);

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(HDMI_DDC_CLK_GPIO, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", HDMI_DDC_CLK_GPIO, rc);
			goto error1;
		}
		rc = gpio_request(HDMI_DDC_DATA_GPIO, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", HDMI_DDC_DATA_GPIO, rc);
			goto error2;
		}
		rc = gpio_request(HDMI_HPD_GPIO, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", HDMI_HPD_GPIO, rc);
			goto error3;
		}
		if (machine_is_apq8064_liquid()) {
			rc = gpio_request(pmic_gpio14, "PMIC_HDMI_MUX_SEL");
			if (rc) {
				pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
					"PMIC_HDMI_MUX_SEL", 14, rc);
				goto error4;
			}
			gpio_set_value_cansleep(pmic_gpio14, 0);
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_DDC_CLK_GPIO);
		gpio_free(HDMI_DDC_DATA_GPIO);
		gpio_free(HDMI_HPD_GPIO);

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(pmic_gpio14, 1);
			gpio_free(pmic_gpio14);
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;
	return 0;

error4:
	gpio_free(HDMI_HPD_GPIO);
error3:
	gpio_free(HDMI_DDC_DATA_GPIO);
error2:
	gpio_free(HDMI_DDC_CLK_GPIO);
error1:
	return rc;
}

static int hdmi_cec_power(int on)
{
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(HDMI_CEC_VAR_GPIO, "HDMI_CEC_VAR");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_CEC_VAR", HDMI_CEC_VAR_GPIO, rc);
			goto error;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_CEC_VAR_GPIO);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
}

void __init apq8064_init_fb(void)
{
	platform_device_register(&msm_fb_device);
	platform_device_register(&lvds_chimei_panel_device);

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	platform_device_register(&wfd_panel_device);
	platform_device_register(&wfd_device);
#endif

/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
if (0) 
{
	if (machine_is_apq8064_liquid())
		platform_device_register(&mipi_dsi2lvds_bridge_device);
	if (machine_is_apq8064_mtp())
		platform_device_register(&mipi_dsi_toshiba_panel_device);
	if (machine_is_mpq8064_dtv())
		platform_device_register(&lvds_frc_panel_device);
}
else 
{
/*[ECID:000000] ZTEBSP wangbing, for orise lcd, 20120823*/
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
	switch (lcd_enum)
	{
		case OTM1283_BOE_HD_LCM : 
			platform_device_register(&mipi_orise_boe_device);
			break;
		case OTM1283_LEAD_HD_LCM : 
			platform_device_register(&mipi_orise_lead_device);
			break ;
		case OTM1283_CPT_HD_LCM : 
			platform_device_register(&mipi_orise_cpt_device);
			break ;		
		 default :
		 	platform_device_register(&mipi_orise_lead_device);
		 	printk("%s no match panel detected \n",__func__);
			break ;
	}
#elif defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
		platform_device_register(&mipi_sharp_device);
#else
		platform_device_register(&mipi_dsi2rgb_bridge_device);
#endif
}
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 end*/

	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lvds", &lvds_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
	platform_device_register(&hdmi_msm_device);
	msm_fb_register_device("dtv", &dtv_pdata);
}

/**
 * Set MDP clocks to high frequency to avoid DSI underflow
 * when using high resolution 1200x1920 WUXGA panels
 */
static void set_mdp_clocks_for_wuxga(void)
{
	mdp_ui_vectors[0].ab = 2000000000;
	mdp_ui_vectors[0].ib = 2000000000;
	mdp_vga_vectors[0].ab = 2000000000;
	mdp_vga_vectors[0].ib = 2000000000;
	mdp_720p_vectors[0].ab = 2000000000;
	mdp_720p_vectors[0].ib = 2000000000;
	mdp_1080p_vectors[0].ab = 2000000000;
	mdp_1080p_vectors[0].ib = 2000000000;

	if (apq8064_hdmi_as_primary_selected()) {
		dtv_bus_def_vectors[0].ab = 2000000000;
		dtv_bus_def_vectors[0].ib = 2000000000;
	}
}

void __init apq8064_set_display_params(char *prim_panel, char *ext_panel,
		unsigned char resolution)
{
	/*
	 * For certain MPQ boards, HDMI should be set as primary display
	 * by default, with the flexibility to specify any other panel
	 * as a primary panel through boot parameters.
	 */
	if (machine_is_mpq8064_hrd() || machine_is_mpq8064_cdp()) {
		pr_debug("HDMI is the primary display by default for MPQ\n");
		if (!strnlen(prim_panel, PANEL_NAME_MAX_LEN))
			strlcpy(msm_fb_pdata.prim_panel_name, HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN);
	}

	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);

		if (!strncmp((char *)msm_fb_pdata.prim_panel_name,
			HDMI_PANEL_NAME, strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("HDMI is the primary display by"
				" boot parameter\n");
			hdmi_is_primary = 1;
			set_mdp_clocks_for_wuxga();
		}
	}
	if (strnlen(ext_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.ext_panel_name, ext_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.ext_panel_name %s\n",
			msm_fb_pdata.ext_panel_name);
	}
	#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02) 
	set_mdp_clocks_for_wuxga();//zhangqi add for blue screen test 2012.10.24
	#endif
	msm_fb_pdata.ext_resolution = resolution;
}
