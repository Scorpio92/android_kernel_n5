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
#include "msm_sensor.h"
#include <linux/proc_fs.h>  //yaotong add sensor  id
#define SENSOR_NAME "imx132"
#define PLATFORM_DRIVER_NAME "msm_camera_imx132"
#define imx132_obj imx132_##obj

DEFINE_MUTEX(imx132_mut);
static struct msm_sensor_ctrl_t imx132_s_ctrl;

static struct msm_camera_i2c_reg_conf imx132_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf imx132_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx132_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx132_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf imx132_prev_settings[] = {
	/* 30fps 1/2 * 1/2 */
	/* PLL setting */
	{0x0305, 0x02}, /* pre_pll_clk_div[7:0] */
	{0x0307, 0x43}, /* pll_multiplier[7:0] */
	{0x30A4, 0x02},
	{0x303C, 0x4B},
	/* mode setting */
	{0x0340, 0x09}, /* frame_length_lines[15:8] */
	{0x0341, 0x50}, /* frame_length_lines[7:0] */
	{0x0342, 0x08}, /* line_length_pck[15:8] */
	{0x0343, 0xC8}, /* line_length_pck[7:0] */
	{0x0344, 0x00}, /* x_addr_start[15:8] */
	{0x0345, 0x1C}, /* x_addr_start[7:0] */
	{0x0346, 0x00}, /* y_addr_start[15:8] */
    //{0x0347, 0x3C}, /* y_addr_start[7:0] */ // zte-modify,fuyipeng modify for 1088 output
    {0x0347, 0x38}, /* y_addr_start[7:0] */
	{0x0348, 0x07}, /* x_addr_end[15:8] */
	{0x0349, 0x9B}, /* x_addr_end[7:0] */
	{0x034A, 0x04}, /* y_addr_end[15:8] */
    //{0x034B, 0x73}, /* y_addr_end[7:0] */  // zte-modify,fuyipeng modify for 1088 output
    {0x034B, 0x77}, /* y_addr_end[7:0] */
	{0x034C, 0x07}, /* x_output_size[15:8] */
	{0x034D, 0x80}, /* x_output_size[7:0] */
	{0x034E, 0x04}, /* y_output_size[15:8] */
    //{0x034F, 0x38}, /* y_output_size[7:0] */ // zte-modify,fuyipeng modify for 1088 output
    {0x034F, 0x40}, /* y_output_size[7:0] */
	{0x0381, 0x01}, /* x_even_inc[3:0] */
	{0x0383, 0x01}, /* x_odd_inc[3:0] */
	{0x0385, 0x01}, /* y_even_inc[7:0] */
	{0x0387, 0x01}, /* y_odd_inc[7:0] */
	{0x303D, 0x10},
	{0x303E, 0x4A},
	{0x3040, 0x08},
	{0x3041, 0x97},
	{0x3048, 0x00},
	{0x304C, 0x2F},
	{0x304D, 0x02},
	{0x3064, 0x92},
	{0x306A, 0x10},
	{0x309B, 0x00},
	{0x309E, 0x41},
	{0x30A0, 0x10},
	{0x30A1, 0x0B},
	{0x30B2, 0x00},
	{0x30D5, 0x00},
	{0x30D6, 0x00},
	{0x30D7, 0x00},
	{0x30D8, 0x00},
	{0x30D9, 0x00},
	{0x30DA, 0x00},
	{0x30DB, 0x00},
	{0x30DC, 0x00},
	{0x30DD, 0x00},
	{0x30DE, 0x00},
	{0x3102, 0x0C},
	{0x3103, 0x33},
	{0x3104, 0x30},
	{0x3105, 0x00},
	{0x3106, 0xCA},
	{0x3107, 0x00},
	{0x3108, 0x06},
	{0x3109, 0x04},
	{0x310A, 0x04},
	{0x315C, 0x3D},
	{0x315D, 0x3C},
	{0x316E, 0x3E},
	{0x316F, 0x3D},
	{0x3301, 0x00},
	{0x3304, 0x07},
	{0x3305, 0x06},
	{0x3306, 0x19},
	{0x3307, 0x03},
	{0x3308, 0x0F},
	{0x3309, 0x07},
	{0x330A, 0x0C},
	{0x330B, 0x06},
	{0x330C, 0x0B},
	{0x330D, 0x07},
	{0x330E, 0x03},
	{0x3318, 0x61},
	{0x3322, 0x09},
	{0x3342, 0x00},
	{0x3348, 0xE0},
	{0x0101, 0x03},   //yaotong 20121019 add 
	{0x0202, 0x02},   //shuttuer setting  yaotong 20121023    0x03
	{0x0203, 0x88},   //0xE8
	{0x0205, 0xE0},  //analog gain setting yaotong 20121023  0xE0
};

static struct msm_camera_i2c_reg_conf imx132_recommend_settings[] = {
	/* global setting */
	{0x3087, 0x53},
	{0x308B, 0x5A},
	{0x3094, 0x11},
	{0x309D, 0xA4},
	{0x30AA, 0x01},
	{0x30C6, 0x00},
	{0x30C7, 0x00},
	{0x3118, 0x2F},
	{0x312A, 0x00},
	{0x312B, 0x0B},
	{0x312C, 0x0B},
	{0x312D, 0x13},
	/* black level setting */
	//{0x3032, 0x40},//??????????
};

static struct v4l2_subdev_info imx132_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array imx132_init_conf[] = {
	{&imx132_recommend_settings[0],
	ARRAY_SIZE(imx132_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

#if 0
static struct msm_camera_i2c_conf_array imx132_confs[] = {
	{&imx132_snap_settings[0],
	ARRAY_SIZE(imx132_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx132_prev_settings[0],
	ARRAY_SIZE(imx132_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};
#endif

static struct msm_camera_i2c_conf_array imx132_confs[] = {
	{&imx132_prev_settings[0],
	ARRAY_SIZE(imx132_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	//{&imx132_prev_settings[0],
	//ARRAY_SIZE(imx132_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};
static struct msm_sensor_output_info_t imx132_dimensions[] = {
	{
	/* full size */
		.x_output = 0x780, /* 1920 */
		.y_output = 0x440, /* 1088 */
		.line_length_pclk = 0x08C8, /*  */
		.frame_length_lines = 0x0950, /*  */
		.vt_pixel_clk = 206400000, //??????
		.op_pixel_clk = 206400000, //?????
		.binning_factor = 1,
	},
};

static struct msm_camera_csid_vc_cfg imx132_cid_cfg[] = {
	{0, CSI_RAW10, CSI_DECODE_10BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params imx132_csi_params = {
	.csid_params = {
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = imx132_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x12,
	},
};

static struct msm_camera_csi2_params *imx132_csi_params_array[] = {
	&imx132_csi_params,
	&imx132_csi_params,
};

static struct msm_sensor_output_reg_addr_t imx132_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t imx132_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x0132,
};

static struct msm_sensor_exp_gain_info_t imx132_exp_gain_info = {
	.coarse_int_time_addr = 0x0202,
	.global_gain_addr = 0x0204,
	.vert_offset = 5,
};

static const struct i2c_device_id imx132_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx132_s_ctrl},
	{ }
};

/*ZTEBSP yaotong 20121105 start*/
static ssize_t camera_id_read_proc(char *page,char **start,off_t off,int count,int *eof,void* data)
{		 	
	int ret;	
       unsigned char *camera_status = "Front Camera ID:0x6C,IMX132-2.0Mp";	
       ret = strlen(camera_status);	 	 
       sprintf(page,"%s\n",camera_status);	 	 
       return (ret + 1);	
}
static void camera_proc_file(void)
{	
    struct proc_dir_entry *proc_file  = create_proc_entry("driver/camera_id_front",0644,NULL);	
    if(proc_file)	
     {		
  	     proc_file->read_proc = camera_id_read_proc;			
     }	
    else	
     {		
        printk(KERN_INFO "camera_proc_file error!\r\n");	
     }
}


static int32_t imx132_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	rc = msm_sensor_i2c_probe(client,id);
	if(rc == 0)
		{
	camera_proc_file();
	}
	return rc;
}

/*ZTEBSP yaotong 20121105 end*/

static struct i2c_driver imx132_i2c_driver = {
	.id_table = imx132_i2c_id,
	//.probe  = msm_sensor_i2c_probe,
	.probe = imx132_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx132_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};


static int __init imx132_sensor_init_module(void)
{
	return i2c_add_driver(&imx132_i2c_driver);
}

static struct v4l2_subdev_core_ops imx132_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops imx132_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx132_subdev_ops = {
	.core = &imx132_subdev_core_ops,
	.video  = &imx132_subdev_video_ops,
};

static struct msm_sensor_fn_t imx132_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_write_snapshot_exp_gain = msm_sensor_write_exp_gain1,
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t imx132_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx132_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx132_start_settings),
	.stop_stream_conf = imx132_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx132_stop_settings),
	.group_hold_on_conf = imx132_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx132_groupon_settings),
	.group_hold_off_conf = imx132_groupoff_settings,
	.group_hold_off_conf_size = ARRAY_SIZE(imx132_groupoff_settings),
	.init_settings = &imx132_init_conf[0],
	.init_size = ARRAY_SIZE(imx132_init_conf),
	.mode_settings = &imx132_confs[0],
	.output_settings = &imx132_dimensions[0],
	.num_conf = ARRAY_SIZE(imx132_confs),
};

static struct msm_sensor_ctrl_t imx132_s_ctrl = {
	.msm_sensor_reg = &imx132_regs,
	.sensor_i2c_client = &imx132_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.sensor_output_reg_addr = &imx132_reg_addr,
	.sensor_id_info = &imx132_id_info,
	.sensor_exp_gain_info = &imx132_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &imx132_csi_params_array[0],
	.msm_sensor_mutex = &imx132_mut,
	.sensor_i2c_driver = &imx132_i2c_driver,
	.sensor_v4l2_subdev_info = imx132_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx132_subdev_info),
	.sensor_v4l2_subdev_ops = &imx132_subdev_ops,
	.func_tbl = &imx132_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(imx132_sensor_init_module);
MODULE_DESCRIPTION("SONY 2MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
