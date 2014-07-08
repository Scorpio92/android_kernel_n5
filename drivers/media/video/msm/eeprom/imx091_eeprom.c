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

#include <linux/module.h>
#include "msm_camera_eeprom.h"
#include "msm_camera_i2c.h"
#include <media/msm_camera.h> //zhangzhao


DEFINE_MUTEX(imx091_eeprom_mutex);
static struct msm_eeprom_ctrl_t imx091_eeprom_t;

static const struct i2c_device_id imx091_eeprom_i2c_id[] = {
	{"imx091_eeprom", (kernel_ulong_t)&imx091_eeprom_t},
	{ }
};

static struct i2c_driver imx091_eeprom_i2c_driver = {
	.id_table = imx091_eeprom_i2c_id,
	.probe  = msm_eeprom_i2c_probe,
	.remove = __exit_p(imx091_eeprom_i2c_remove),
	.driver = {
		.name = "imx091_eeprom",
	},
};

static int __init imx091_eeprom_i2c_add_driver(void)
{
	int rc = 0;
	rc = i2c_add_driver(imx091_eeprom_t.i2c_driver);
	return rc;
}

static struct v4l2_subdev_core_ops imx091_eeprom_subdev_core_ops = {
	.ioctl = msm_eeprom_subdev_ioctl,
};

static struct v4l2_subdev_ops imx091_eeprom_subdev_ops = {
	.core = &imx091_eeprom_subdev_core_ops,
};

uint8_t imx091_wbcalib_data[6];
uint8_t imx091_afcalib_data[6];
uint8_t imx091_module_id[6];

struct msm_calib_wb imx091_wb_data;
struct msm_calib_af imx091_af_data;

static struct msm_camera_eeprom_info_t imx091_calib_supp_info = {
	{FALSE, 6, 1, 1},
	{FALSE, 6, 0, 32768},
	{FALSE, 0, 0, 1},
	{FALSE, 0, 0, 1},
};

static struct msm_camera_eeprom_read_t imx091_eeprom_read_tbl[] = {
	{0x00, &imx091_module_id[0], 6, 0},
	{0x00, &imx091_wbcalib_data[0], 6, 0},
	{0x00, &imx091_afcalib_data[0], 6, 0},
};


static struct msm_camera_eeprom_data_t imx091_eeprom_data_tbl[] = {
	{&imx091_wb_data, sizeof(struct msm_calib_wb)},
	{&imx091_af_data, sizeof(struct msm_calib_af)},
};

static void imx091_format_wbdata(void)
{
	imx091_wb_data.r_over_g = (uint16_t)(imx091_wbcalib_data[1] << 8) |
		(imx091_wbcalib_data[0] - 0x32);
	imx091_wb_data.b_over_g = (uint16_t)(imx091_wbcalib_data[3] << 8) |
		(imx091_wbcalib_data[2] - 0x32);
	imx091_wb_data.gr_over_gb = (uint16_t)(imx091_wbcalib_data[5] << 8) |
		(imx091_wbcalib_data[4] - 0x32);

    pr_err("the imx091_format_wbdata info  is %x,%x\n", imx091_wbcalib_data[0],imx091_wbcalib_data[1]);
}

static void imx091_format_afdata(void)
{
	imx091_af_data.inf_dac = (uint16_t)(imx091_afcalib_data[1] << 8) |
		imx091_afcalib_data[0];
	imx091_af_data.macro_dac = (uint16_t)(imx091_afcalib_data[3] << 8) |
		imx091_afcalib_data[2];
	imx091_af_data.start_dac = (uint16_t)(imx091_afcalib_data[5] << 8) |
		imx091_afcalib_data[4];
    pr_err("the imx091_format_afdata info  is %x,%x\n", imx091_afcalib_data[0],imx091_afcalib_data[1]);
}

static void imx091_print_module_info(void)
{

    pr_err("the module info  is %x,%x\n", imx091_module_id[0],imx091_module_id[1]);
    pr_err("the module info 2  is %x,%x\n", imx091_module_id[2],imx091_module_id[3]);
    pr_err("the module info 3  is %x,%x\n", imx091_module_id[4],imx091_module_id[5]);
	
}

void imx091_format_calibrationdata(void)
{
	imx091_format_wbdata();
	imx091_format_afdata();
	imx091_print_module_info();	
}

uint16_t imx091_get_module_id(void)
{
    uint16_t module_id = 0;
    //int16_t rc = 0;
    module_id = (uint16_t)(imx091_module_id[1] << 8) |(imx091_module_id[0]);

	pr_err("the module id is %x\n",module_id);
	return module_id;
}

EXPORT_SYMBOL(imx091_get_module_id);
	
static struct msm_eeprom_ctrl_t imx091_eeprom_t = {
	.i2c_driver = &imx091_eeprom_i2c_driver,
	.i2c_addr = 0xA0,
	.eeprom_v4l2_subdev_ops = &imx091_eeprom_subdev_ops,

	.i2c_client = {
		.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	},

	.eeprom_mutex = &imx091_eeprom_mutex,

	.func_tbl = {
		.eeprom_init = NULL,
		.eeprom_release = NULL,
		.eeprom_get_info = msm_camera_eeprom_get_info,
		.eeprom_get_data = msm_camera_eeprom_get_data,
		.eeprom_set_dev_addr = NULL,
		.eeprom_format_data = imx091_format_calibrationdata,
	},
	.info = &imx091_calib_supp_info,
	.info_size = sizeof(struct msm_camera_eeprom_info_t),
	.read_tbl = imx091_eeprom_read_tbl,
	.read_tbl_size = ARRAY_SIZE(imx091_eeprom_read_tbl),
	.data_tbl = imx091_eeprom_data_tbl,
	.data_tbl_size = ARRAY_SIZE(imx091_eeprom_data_tbl),
};

subsys_initcall(imx091_eeprom_i2c_add_driver);
MODULE_DESCRIPTION("imx091 EEPROM");
MODULE_LICENSE("GPL v2");
