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

static struct msm_panel_info pinfo;



static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
#if 0
    /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
//zhangqi add for sharp lcd begin 
	{0x103, 0x3b, 0x25, 0x00, 0x64, 0x73, 0x2b, 0x3f,
	0x48, 0x03, 0x04, 0xa0},
//zhangqi add for sharp lcd end
    /* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
    /* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
//zhangqi add for sharp lcd begin 

	{0x0, 0x24, 0x30, 0xc1, 0x00, 0x40, 0x03, 0x62,
	0x40, 0x07, 0x07,
	0x00, 0x1a, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },

//zhangqi add for sharp lcd end
#else
    /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
//zhangqi add for sharp lcd begin 
	{0x100, 0x3b, 0x25, 0x00, 0x63, 0x73, 0x2b, 0x3f,
	0x48, 0x03, 0x04, 0xa0},
//zhangqi add for sharp lcd end
    /* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
    /* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
//zhangqi add for sharp lcd begin 

	{0x0, 0x36, 0x30, 0xc2, 0x00, 0x40, 0x03, 0x62,
	0x40, 0x07, 0x07,
	0x00, 0x1a, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },

//zhangqi add for sharp lcd end
#endif
};

static int __init mipi_video_sharp_1080p_pt_init(void)
{
	int ret;
	printk("zhangqi add enter %s\n",__func__);
	if (msm_fb_detect_client("mipi_video_sharp_1080p"))
		return 0;
//zhangqi add for sharp lcd begin 
	pinfo.xres =1080;// 720;
	pinfo.yres =1920;// 1280;
//zhangqi add for sharp lcd end
	pinfo.lcdc.xres_pad = 0;
	pinfo.lcdc.yres_pad = 0;

	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;


	pinfo.lcdc.h_back_porch = 160;
	pinfo.lcdc.h_front_porch = 160;
	pinfo.lcdc.h_pulse_width = 8;
	pinfo.lcdc.v_back_porch = 10;
	pinfo.lcdc.v_front_porch = 10;
	pinfo.lcdc.v_pulse_width = 2;

	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 15;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = TRUE;
	pinfo.mipi.hbp_power_stop = TRUE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.t_clk_post = 0x04;
	pinfo.mipi.t_clk_pre = 0x1c;
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = 0;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
//zhangqi add for sharp lcd begin 
	pinfo.mipi.frame_rate = 60;
//zhangqi add for sharp lcd end
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	pinfo.mipi.tx_eot_append = TRUE;
	pinfo.mipi.esc_byte_ratio = 4;

	ret = mipi_sharp_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_720P_PT);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_sharp_1080p_pt_init);
