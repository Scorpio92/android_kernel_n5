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
#include <linux/proc_fs.h> //zhangzhao add sensor id 
#define SENSOR_NAME "ov9740"
#define PLATFORM_DRIVER_NAME "msm_camera_ov9740"
#define ov9740_obj ov9740_##obj

DEFINE_MUTEX(ov9740_mut);
static struct msm_sensor_ctrl_t ov9740_s_ctrl;

static struct msm_camera_i2c_reg_conf ov9740_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf ov9740_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf ov9740_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf ov9740_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf ov9740_prev_settings[] = {
};

static struct msm_camera_i2c_reg_conf ov9740_recommend_settings[] = {
{0x0103, 0x01},//	;; Software RESET
{0x0101, 0x02},//	;; Orientation    zhangzhao
//;; PLL setting
{0x3104, 0x20},
{0x0305, 0x06}, //[ECID:617001755692] zhangzhao  modified fps to 15,  //0x03
{0x0307, 0x4c},
{0x0303, 0x01},
{0x0301, 0x08},
{0x3010, 0x01},
//;; Timing setting	
{0x0340, 0x03},//	;; VTS
{0x0341, 0x07},//	;; VTS
{0x0342, 0x06},//	;; HTS
{0x0343, 0x62},//	;; HTS
{0x0344, 0x00},//	;; X start
{0x0345, 0x08},//	;; X start
{0x0346, 0x00},//	;; Y start
{0x0347, 0x05},//	;; Y start       zhangzhao  04----->05
{0x0348, 0x05},//	;; X end
{0x0349, 0x0c},//	;; X end
{0x034a, 0x02},//	;; Y end      
{0x034b, 0xd9},// 	;; Y end  zhangzhao  d8----->d9
{0x034c, 0x05},//	;; H output size
{0x034d, 0x00},//	;; H output size
{0x034e, 0x02},//	;; V output size
{0x034f, 0xd0},//	;; V output size
//;; Output select 	
{0x3002, 0x00},//	;; IO control
{0x3004, 0x00},//	;; IO control
{0x3005, 0x00},//	;; IO control
{0x3012, 0x70},//	;; MIPI control
{0x3013, 0x60},//	;; MIPI control
{0x3014, 0x01},//	;; MIPI control
{0x301f, 0x03},//	;; MIPI control
{0x3026, 0x00},//	;; Output select
{0x3027, 0x00},//	;; Output select
//;; Analog control	
{0x3601, 0x40},//	;; Analog control
{0x3602, 0x16},//	;; Analog control
{0x3603, 0xaa},//	;; Analog control
{0x3604, 0x0c},//	;; Analog control
{0x3610, 0xa1},//	;; Analog control
{0x3612, 0x24},//	;; Analog control
{0x3620, 0x66},//	;; Analog control
{0x3621, 0xc0},//	;; Analog control
{0x3622, 0x9f},//	;; Analog control
{0x3630, 0xd2},//	;; Analog control
{0x3631, 0x5e},//	;; Analog control
{0x3632, 0x27},//	;; Analog control
{0x3633, 0x50},//	;; Analog control
//;; Sensor control	
{0x3703, 0x42},//	;; Sensor control 
{0x3704, 0x10},//	;; Sensor control 
{0x3705, 0x45},//	;; Sensor control 
{0x3707, 0x11},//	;; Sensor control 
//;; Timing control	
{0x3817, 0x94},//	;; Internal timing control
{0x3819, 0x6e},//	;; Internal timing control
{0x3831, 0x40},//	;; Digital gain enable
{0x3833, 0x04},//	;; Internal timing control
{0x3835, 0x04},//	;; Internal timing control
{0x3837, 0x01},//	
//;; AEC/AGC control	
{0x3503, 0x10},//	;; AEC/AGC control
{0x3a18, 0x00},//{0x3a18, 0x01},//	;; Gain ceiling ----zhangzhao 2013-1-11 denoise to 2
{0x3a19, 0x80},//{0x3a19, 0xB5},//	;; Gain ceiling----------------zhangzhao 2013
{0x3a1a, 0x05},//	;; Max diff
{0x3a11, 0x90},// 	;; High threshold
{0x3a1b, 0x4a},// 	;; WPT 2 
{0x3a0f, 0x48},// 	;; WPT  
{0x3a10, 0x44},// 	;; BPT  
{0x3a1e, 0x42},// 	;; BPT 2 
{0x3a1f, 0x22},//  	;; Low threshold 
//;; Banding filter	
{0x3a08, 0x00},//	;; 50Hz banding step
{0x3a09, 0xe8},//	;; 50Hz banding step
{0x3a0e, 0x03},//	;; 50Hz banding Max
{0x3a14, 0x15},//	;; 50Hz Max exposure
{0x3a15, 0xc6},//	;; 50Hz Max exposure
{0x3a0a, 0x00},//	;; 60Hz banding step
{0x3a0b, 0xc0},//	;; 60Hz banding step
{0x3a0d, 0x04},//	;; 60Hz banding Max
{0x3a02, 0x18},//	;; 60Hz Max exposure
{0x3a03, 0x20},//	;; 60Hz Max exposure
//;;50/60 detection	
{0x3c0a, 0x9c},//	;; Number of samples
{0x3c0b, 0x3f},//	;; Number of samples
//:; BLC control	
{0x4002, 0x45},//	;; BLC auto enable
{0x4005, 0x18},//	;; BLC mode
//;; VFIFO control	
{0x4601, 0x16},//	;; VFIFO control
{0x460e, 0x82},//	;; VFIFO control
//;; DVP control	
{0x4702, 0x04},//	;; Vsync control
{0x4704, 0x00},//	;; Vsync mode 
{0x4706, 0x08},//	;; Vsync control

{0x4300, 0x31},//	;; -------------------------------------

//;; MIPI control	
{0x4800, 0x44},//	;; MIPI control
{0x4801, 0x0f},//	
{0x4803, 0x05},//	;; MIPI control
{0x4805, 0x10},//	
{0x4837, 0x20},//	
//;; ISP control	
{0x5000, 0xff},//	
{0x5001, 0xff},//	
{0x5003, 0xff},//
//;;AWB	
{0x5180, 0xf0},// 	;;AWB setting
{0x5181, 0x00},//  	;;AWB setting
{0x5182, 0x41},// 	;;AWB setting 
{0x5183, 0x42},// 	;;AWB setting
{0x5184, 0x80},// 	;;AWB setting
{0x5185, 0x68},// 	;;AWB setting
{0x5186, 0x93},// 	;;AWB setting 
{0x5187, 0xa8},// 	;;AWB setting
{0x5188, 0x17},// 	;;AWB setting
{0x5189, 0x45},// 	;;AWB setting
{0x518a, 0x27},// 	;;AWB setting
{0x518b, 0x41},// 	;;AWB setting
{0x518c, 0x2d},// 	;;AWB setting
{0x518d, 0xf0},// 	;;AWB setting
{0x518e, 0x10},//  	;;AWB setting
{0x518f, 0xff},// 	;;AWB setting
{0x5190, 0x0 },// 	;;AWB setting
{0x5191, 0xff},// 	;;AWB setting 
{0x5192, 0x00},//  	;;AWB setting
{0x5193, 0xff},// 	;;AWB setting 
{0x5194, 0x00},// 	;;AWB setting 
	
//;; DNS	
{0x529a, 0x06},//	;;DNS setting--------------zhangzhao 2013-1-11 denoise to 2
{0x529b, 0x18},//	;;DNS setting
{0x529c, 0x20},//	;;DNS setting
{0x529d, 0x30},//	;;DNS setting
{0x529e, 0x30},// 	;;DNS setting
//{0x529a, 0x02},//	;;DNS setting
//{0x529b, 0x08},//	;;DNS setting
//{0x529c, 0x0a},//	;;DNS setting
//{0x529d, 0x10},//	;;DNS setting
//{0x529e, 0x10},// 	;;DNS setting
{0x529f, 0x28},//   	;;DNS setting
{0x52a0, 0x32},// 	;;DNS setting
{0x52a2, 0x00},//	;;DNS setting 
{0x52a3, 0x02},//	;;DNS setting 
{0x52a4, 0x00},//	;;DNS setting 
{0x52a5, 0x04},//	;;DNS setting  
{0x52a6, 0x00},//	;;DNS setting  
{0x52a7, 0x08},//	;;DNS setting  
{0x52a8, 0x00},//	;;DNS setting  
{0x52a9, 0x10},//  	;;DNS setting
{0x52aa, 0x00},//	;;DNS setting  
{0x52ab, 0x38},//  	;;DNS setting
{0x52ac, 0x00},//	;;DNS setting  
{0x52ad, 0x3c},// 	;;DNS setting 
{0x52ae, 0x00},//	;;DNS setting   
{0x52af, 0x4c},// 	;;DNS setting
	
//;;CIP	
{0x530d, 0x06},//	;;CIP setting
	
//;;CMX	
{0x5380, 0x01},// 	;;CMX setting  
{0x5381, 0x00},// 	;;CMX setting  
{0x5382, 0x00},// 	;;CMX setting   
{0x5383, 0x0d},// 	;;CMX setting  
{0x5384, 0x00},// 	;;CMX setting  
{0x5385, 0x2f},// 	;;CMX setting   
{0x5386, 0x00},// 	;;CMX setting  
{0x5387, 0x00},// 	;;CMX setting   
{0x5388, 0x00},// 	;;CMX setting   
{0x5389, 0xd3},// 	;;CMX setting  
{0x538a, 0x00},// 	;;CMX setting   
{0x538b, 0x0f},// 	;;CMX setting   
{0x538c, 0x00},// 	;;CMX setting   
{0x538d, 0x00},// 	;;CMX setting  
{0x538e, 0x00},// 	;;CMX setting  
{0x538f, 0x32},// 	;;CMX setting   
{0x5390, 0x00},// 	;;CMX setting  
{0x5391, 0x94},// 	;;CMX setting    
{0x5392, 0x00},// 	;;CMX setting  
{0x5393, 0xa4},// 	;;CMX setting  
{0x5394, 0x18},// 	;;CMX setting  
	
//;; Contrast	
{0x5401, 0x2c},//	;; Contrast setting
{0x5403, 0x28},//	;; Contrast setting
{0x5404, 0x06},//	;; Contrast setting
{0x5405, 0xe0},//	;; Contrast setting
	
//;;Y Gamma	
{0x5480, 0x04},// 	;;Y Gamma setting  
{0x5481, 0x12},// 	;;Y Gamma setting 
{0x5482, 0x27},// 	;;Y Gamma setting  
{0x5483, 0x49},// 	;;Y Gamma setting  
{0x5484, 0x57},// 	;;Y Gamma setting  
{0x5485, 0x66},// 	;;Y Gamma setting  
{0x5486, 0x75},// 	;;Y Gamma setting  
{0x5487, 0x81},// 	;;Y Gamma setting 
{0x5488, 0x8c},// 	;;Y Gamma setting 
{0x5489, 0x95},// 	;;Y Gamma setting 
{0x548a, 0xa5},// 	;;Y Gamma setting 
{0x548b, 0xb2},// 	;;Y Gamma setting 
{0x548c, 0xc8},// 	;;Y Gamma setting 
{0x548d, 0xd9},// 	;;Y Gamma setting 
{0x548e, 0xec},// 	;;Y Gamma setting 
	
//;;UV Gamma	
{0x5490, 0x01},//  	;;UV Gamma setting 
{0x5491, 0xc0},//  	;;UV Gamma setting 
{0x5492, 0x03},//  	;;UV Gamma setting 
{0x5493, 0x00},//  	;;UV Gamma setting 
{0x5494, 0x03},//  	;;UV Gamma setting 
{0x5495, 0xe0},//  	;;UV Gamma setting 
{0x5496, 0x03},//  	;;UV Gamma setting 
{0x5497, 0x10},//  	;;UV Gamma setting 
{0x5498, 0x02},//  	;;UV Gamma setting 
{0x5499, 0xac},//  	;;UV Gamma setting 
{0x549a, 0x02},//  	;;UV Gamma setting 
{0x549b, 0x75},//  	;;UV Gamma setting 
{0x549c, 0x02},//   	;;UV Gamma setting 
{0x549d, 0x44},//  	;;UV Gamma setting 
{0x549e, 0x02},//  	;;UV Gamma setting 
{0x549f, 0x20},//  	;;UV Gamma setting 
{0x54a0, 0x02},//  	;;UV Gamma setting 
{0x54a1, 0x07},//  	;;UV Gamma setting 
{0x54a2, 0x01},//  	;;UV Gamma setting 
{0x54a3, 0xec},//  	;;UV Gamma setting 
{0x54a4, 0x01},//  	;;UV Gamma setting 
{0x54a5, 0xc0},//  	;;UV Gamma setting 
{0x54a6, 0x01},//  	;;UV Gamma setting 
{0x54a7, 0x9b},//  	;;UV Gamma setting 
{0x54a8, 0x01},//  	;;UV Gamma setting 
{0x54a9, 0x63},//  	;;UV Gamma setting 
{0x54aa, 0x01},//  	;;UV Gamma setting 
{0x54ab, 0x2b},//  	;;UV Gamma setting 
{0x54ac, 0x01},//  	;;UV Gamma setting 
{0x54ad, 0x22},//  	;;UV Gamma setting 
 	
//;; UV adjust	
{0x5501, 0x1c},//  	;;UV adjust setting 
{0x5502, 0x00},//   	;;UV adjust setting 
{0x5503, 0x40},//  	;;UV adjust setting 
{0x5504, 0x00},//  	;;UV adjust setting 
{0x5505, 0x80},//  	;;UV adjust setting 
	
//;; Lens correction	
{0x5800, 0x1c},//	;; Lens correction setting
{0x5801, 0x16},//	;; Lens correction setting
{0x5802, 0x15},//	;; Lens correction setting
{0x5803, 0x16},//	;; Lens correction setting
{0x5804, 0x18},//	;; Lens correction setting
{0x5805, 0x1a},//	;; Lens correction setting
{0x5806, 0x0c},//	;; Lens correction setting
{0x5807, 0x0a},//	;; Lens correction setting
{0x5808, 0x08},//	;; Lens correction setting
{0x5809, 0x08},//	;; Lens correction setting
{0x580a, 0x0a},//	;; Lens correction setting
{0x580b, 0x0b},//	;; Lens correction setting
{0x580c, 0x05},//	;; Lens correction setting
{0x580d, 0x02},//	;; Lens correction setting
{0x580e, 0x00},//	;; Lens correction setting
{0x580f, 0x00},//	;; Lens correction setting
{0x5810, 0x02},//	;; Lens correction setting
{0x5811, 0x05},//	;; Lens correction setting
{0x5812, 0x04},//	;; Lens correction setting
{0x5813, 0x01},//	;; Lens correction setting
{0x5814, 0x00},//	;; Lens correction setting
{0x5815, 0x00},//	;; Lens correction setting
{0x5816, 0x02},//	;; Lens correction setting
{0x5817, 0x03},//	;; Lens correction setting
{0x5818, 0x0a},//	;; Lens correction setting
{0x5819, 0x07},//	;; Lens correction setting
{0x581a, 0x05},//	;; Lens correction setting
{0x581b, 0x05},//	;; Lens correction setting
{0x581c, 0x08},//	;; Lens correction setting
{0x581d, 0x0b},//	;; Lens correction setting
{0x581e, 0x15},//	;; Lens correction setting
{0x581f, 0x14},//	;; Lens correction setting
{0x5820, 0x14},//	;; Lens correction setting
{0x5821, 0x13},//	;; Lens correction setting
{0x5822, 0x17},//	;; Lens correction setting
{0x5823, 0x16},//	;; Lens correction setting
{0x5824, 0x46},//	;; Lens correction setting
{0x5825, 0x4c},//	;; Lens correction setting
{0x5826, 0x6c},//	;; Lens correction setting
{0x5827, 0x4c},//	;; Lens correction setting
{0x5828, 0x80},//	;; Lens correction setting
{0x5829, 0x2e},//	;; Lens correction setting
{0x582a, 0x48},//	;; Lens correction setting
{0x582b, 0x46},//	;; Lens correction setting
{0x582c, 0x2a},//	;; Lens correction setting
{0x582d, 0x68},//	;; Lens correction setting
{0x582e, 0x08},//	;; Lens correction setting
{0x582f, 0x26},//	;; Lens correction setting
{0x5830, 0x44},//	;; Lens correction setting
{0x5831, 0x46},//	;; Lens correction setting
{0x5832, 0x62},//	;; Lens correction setting
{0x5833, 0x0c},//	;; Lens correction setting
{0x5834, 0x28},//	;; Lens correction setting
{0x5835, 0x46},//	;; Lens correction setting
{0x5836, 0x28},//	;; Lens correction setting
{0x5837, 0x88},//	;; Lens correction setting
{0x5838, 0x0e},//	;; Lens correction setting
{0x5839, 0x0e},//	;; Lens correction setting
{0x583a, 0x2c},//	;; Lens correction setting
{0x583b, 0x2e},//	;; Lens correction setting
{0x583c, 0x46},//	;; Lens correction setting
{0x583d, 0xca},//	;; Lens correction setting
{0x583e, 0xf0},//	;; Lens correction setting
{0x5842, 0x02},//	;; Lens correction setting
{0x5843, 0x5e},//	;; Lens correction setting
{0x5844, 0x04},//	;; Lens correction setting
{0x5845, 0x32},//	;; Lens correction setting
{0x5846, 0x03},//	;; Lens correction setting
{0x5847, 0x29},//	;; Lens correction setting
{0x5848, 0x02},//	;; Lens correction setting
{0x5849, 0xcc},//	;; Lens correction setting
{0x0100, 0x01},

};

static struct v4l2_subdev_info ov9740_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ov9740_init_conf[] = {
	{&ov9740_recommend_settings[0],
	ARRAY_SIZE(ov9740_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov9740_confs[] = {
	{&ov9740_prev_settings[0],
	ARRAY_SIZE(ov9740_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t ov9740_dimensions[] = {
	{
		.x_output = 0x500, /* 1296 */
		.y_output = 0x2d0, /* 808 */
		.line_length_pclk = 0x500, /* 1664 */
		.frame_length_lines = 0x2d0, /* 961 */
		.vt_pixel_clk = 96000000,
		.op_pixel_clk = 96000000,
		.binning_factor = 1,
	},
};



static struct msm_camera_csid_vc_cfg ov9740_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov9740_csi_params = {
	.csid_params = {
		.lane_cnt = 1,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = ov9740_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,
		.settle_cnt = 0x12,
	},
};

static struct msm_camera_csi2_params *ov9740_csi_params_array[] = {
	&ov9740_csi_params,
	&ov9740_csi_params,
};

static struct msm_sensor_output_reg_addr_t ov9740_reg_addr = {
	.x_output = 0x034c,
	.y_output = 0x034e,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t ov9740_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x9740,
};


static const struct i2c_device_id ov9740_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov9740_s_ctrl},
	{ }
};

//zhangzhao add sensor id start
static ssize_t camera_id_read_proc(char *page,char **start,off_t off,int count,int *eof,void* data)
{		 	
	int ret;	
       unsigned char *camera_status = "Front Camera ID:0x20,ov9740-720P";	
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
static int32_t OV9740_sensor_i2c_probe(struct i2c_client *client,
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
static struct i2c_driver ov9740_i2c_driver = {
	.id_table = ov9740_i2c_id,
	.probe  = OV9740_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};
//zhangzhao add sensor id end
static struct msm_camera_i2c_client ov9740_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	pr_err("%s: init cam sensor\n",__func__);
	return i2c_add_driver(&ov9740_i2c_driver);
}

static struct v4l2_subdev_core_ops ov9740_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov9740_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov9740_subdev_ops = {
	.core = &ov9740_subdev_core_ops,
	.video  = &ov9740_subdev_video_ops,
};

static struct msm_sensor_fn_t ov9740_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t ov9740_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov9740_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov9740_start_settings),
	.stop_stream_conf = ov9740_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov9740_stop_settings),
	.group_hold_on_conf = ov9740_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov9740_groupon_settings),
	.group_hold_off_conf = ov9740_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(ov9740_groupoff_settings),
	.init_settings = &ov9740_init_conf[0],
	.init_size = ARRAY_SIZE(ov9740_init_conf),
	.mode_settings = &ov9740_confs[0],
	.output_settings = &ov9740_dimensions[0],
	.num_conf = ARRAY_SIZE(ov9740_confs),
};

static struct msm_sensor_ctrl_t ov9740_s_ctrl = {
	.msm_sensor_reg = &ov9740_regs,
	.sensor_i2c_client = &ov9740_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.sensor_output_reg_addr = &ov9740_reg_addr,
	.sensor_id_info = &ov9740_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &ov9740_csi_params_array[0],
	.msm_sensor_mutex = &ov9740_mut,
	.sensor_i2c_driver = &ov9740_i2c_driver,
	.sensor_v4l2_subdev_info = ov9740_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov9740_subdev_info),
	.sensor_v4l2_subdev_ops = &ov9740_subdev_ops,
	.func_tbl = &ov9740_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision WXGA Bayer sensor driver");
MODULE_LICENSE("GPL v2");



