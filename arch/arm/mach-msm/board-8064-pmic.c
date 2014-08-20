/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include "devices.h"
#include "board-8064.h"

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8821_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8821_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8921_GPIO_DISABLE(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8921_GPIO_OUTPUT(_gpio, _val, _strength) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_BUFCONF(_gpio, _val, _strength, _bufconf) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT,\
			PM_GPIO_OUT_BUF_##_bufconf, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_INPUT(_gpio, _pull) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8921_GPIO_OUTPUT_VIN(_gpio, _val, _vin) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, _vin, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

/* Initial PM8921 GPIO configurations */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/* HDMI Mux Selector */
	PM8921_GPIO_OUTPUT(23, 0, HIGH),	/* touchscreen power FET */
	PM8921_GPIO_OUTPUT(24, 0, LOW),	/* zhangzhao add for p864a20 CAMERA 1v2 FET*/
	PM8921_GPIO_OUTPUT(29, 0, LOW),     /*ZTEBSP yaotong 20121102 for p864a20*/
	PM8921_GPIO_OUTPUT(31, 0, LOW),     /*ZTEBSP yaotong 20121102 for p864a20*/
	PM8921_GPIO_OUTPUT_BUFCONF(25, 1, HIGH, CMOS), /* DISP_RESET_N */  //zhangqi add for logo
	PM8921_GPIO_OUTPUT_FUNC(26, 0, PM_GPIO_FUNC_2), /* Bl: Off, PWM mode */
	PM8921_GPIO_OUTPUT_VIN(30, 0, PM_GPIO_VIN_S4), /*[ECID:000000] ZTEBSP wanghaifei 20121227, avoid flash when power on*/
	PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)		//jiaobaocun deleted for FM
	PM8921_GPIO_OUTPUT_FUNC(43, 0, PM_GPIO_FUNC_1),	//jiaobaocun
#endif
//[ECID:000000] ZTEBSP wanghaifei start 20121120, for OTG
#ifdef CONFIG_PROJECT_P864H01
	PM8921_GPIO_OUTPUT(42, 0, HIGH),
#endif
//[ECID:000000] ZTEBSP wanghaifei end 20121120, for OTG
	PM8921_GPIO_OUTPUT_FUNC(44, 0, PM_GPIO_FUNC_2),
	PM8921_GPIO_OUTPUT(33, 0, HIGH),
	PM8921_GPIO_OUTPUT(19, 0, HIGH),    //doumingming add
	PM8921_GPIO_OUTPUT(20, 0, HIGH),
	PM8921_GPIO_INPUT(35, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_OUTPUT(36, 0,LOW),                      /* vrep  *///ztebsp zhangjing add for otg,20120705
	/* TABLA CODEC RESET */
	PM8921_GPIO_OUTPUT(34, 1, MED),
	PM8921_GPIO_OUTPUT(13, 0, HIGH),               /* PCIE_CLK_PWR_EN */
};

static struct pm8xxx_gpio_init pm8921_mtp_kp_gpios[] __initdata = {
//shihuiqin for key
	PM8921_GPIO_INPUT(1, PM_GPIO_PULL_UP_30),   /*vol down key*/
	PM8921_GPIO_INPUT(2, PM_GPIO_PULL_UP_30),   /*vol up key*/
//shihuiqin for key
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),    /*cam snapshot key*/
//wangtao fusion3-debug begin	
	//PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),
       PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
#ifndef CONFIG_PROJECT_P864H01 //[ECID:000000] ZTEBSP wanghaifei start 20121120, for OTG
	PM8921_GPIO_INPUT(42, PM_GPIO_PULL_UP_30),
#endif
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/* SD_WP */
//wangtao fusion3-debug end
};

static struct pm8xxx_gpio_init pm8921_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(42, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/* SD_WP */
};

/* Initial PM8XXX MPP configurations */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	PM8921_MPP_INIT(3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW),
	PM8921_MPP_INIT(8, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
	/*MPP9 is used to detect docking station connection/removal on Liquid*/
	PM8921_MPP_INIT(9, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
	/* PCIE_RESET_N */
	PM8921_MPP_INIT(1, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_HIGH),
};

void __init apq8064_pm8xxx_gpio_mpp_init(void)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(pm8921_gpios); i++) {
		rc = pm8xxx_gpio_config(pm8921_gpios[i].gpio,
					&pm8921_gpios[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
			break;
		}
	}

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		for (i = 0; i < ARRAY_SIZE(pm8921_cdp_kp_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_cdp_kp_gpios[i].gpio,
						&pm8921_cdp_kp_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}

	if (machine_is_apq8064_mtp())
		for (i = 0; i < ARRAY_SIZE(pm8921_mtp_kp_gpios); i++) {
			rc = pm8xxx_gpio_config(pm8921_mtp_kp_gpios[i].gpio,
						&pm8921_mtp_kp_gpios[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_gpio_config: rc=%d\n",
					__func__, rc);
				break;
			}
		}

	for (i = 0; i < ARRAY_SIZE(pm8xxx_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8xxx_mpps[i].mpp,
					&pm8xxx_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}
}

static struct pm8xxx_pwrkey_platform_data apq8064_pm8921_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 15625,
	.wakeup			= 1,
};

static struct pm8xxx_misc_platform_data apq8064_pm8921_misc_pdata = {
	.priority		= 0,
};

#define PM8921_LC_LED_MAX_CURRENT	4	/* I = 4mA */
#define PM8921_LC_LED_LOW_CURRENT	1	/* I = 1mA */
#define PM8XXX_LED_PWM_PERIOD		1000
/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 */
#ifdef CONFIG_LEDS_PM8XXX_HW_BLINK
#define PM8XXX_LED_PWM_DUTY_MS		50
#else
#define PM8XXX_LED_PWM_DUTY_MS		20
#endif
/**
 * PM8XXX_PWM_CHANNEL_NONE shall be used when LED shall not be
 * driven using PWM feature.
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

static struct led_info pm8921_led_info[] = {
	[0] = {
		.name			= "red",   /*led:red*/
		.default_trigger	= "none",   /*ac-online*/
	},
/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 +++*/
	[1] = {
		.name			= "green",
		.default_trigger	= "none",
	},
	[2] = {
		.name			= "keyboard-backlight",
		.default_trigger	= "none",
	},
/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 ---*/		
};

static struct led_platform_data pm8921_led_core_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};

/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 */
#ifdef CONFIG_LEDS_PM8XXX_HW_BLINK
static int pm8921_led0_pwm_duty_pcts[51] = {
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0,
};
#else
#if 0
static int pm8921_led0_pwm_duty_pcts[56] = {
	1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
	40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
	80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
	92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
	58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
	14, 10, 6, 4, 1
};
#endif
#endif

#ifdef CONFIG_LEDS_PM8XXX_HW_BLINK
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 1,
};
#else
#if 0
/*
 * Note: There is a bug in LPG module that results in incorrect
 * behavior of pattern when LUT index 0 is used. So effectively
 * there are 63 usable LUT entries.
 */
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 1,
};
#endif
#endif

static struct pm8xxx_led_config pm8921_led_configs[] = {
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5, //according to 80_N4420_1 Rev.E 3.8.1
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
#ifdef CONFIG_LEDS_PM8XXX_HW_BLINK
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#else
#if 0
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#endif		
#endif		
	},
/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 +++*/
	[1] = {
		.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_PWM3,
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
		.max_current = 20, 
#else
		.max_current = PM8921_LC_LED_MAX_CURRENT, 
#endif		
		.pwm_channel = 6,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
#ifdef CONFIG_LEDS_PM8XXX_HW_BLINK
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#else
#if 0
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#endif		
#endif		
	},
	[2] = {
		.id = PM8XXX_ID_LED_2,
		.mode = PM8XXX_LED_MODE_MANUAL,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 7,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
//		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
/*[ECID:000000] ZTEBSP wangbing, for pm8921 led, 20120828 ---*/		
};

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata = {
		.led_core = &pm8921_led_core_pdata,
		.configs = pm8921_led_configs,
		.num_configs = ARRAY_SIZE(pm8921_led_configs),
};
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 start*/
#ifdef CONFIG_PMIC8XXX_VIBRATOR
static struct pm8xxx_vibrator_platform_data pmic_vibrator_pdata = {
	.initial_vibrate_ms = 200,
	.max_timeout_ms = 15000,
	.level_mV = 2800, //[ECID:000000] ZTEBSP wanghaifei 20121204, modify to 2.8V
};
#endif
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 end*/
static struct pm8xxx_adc_amux apq8064_pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
};

static struct pm8xxx_adc_properties apq8064_pm8921_adc_data = {
	.adc_vdd_reference	= 1800, /* milli-voltage for this adc */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static struct pm8xxx_adc_platform_data apq8064_pm8921_adc_pdata = {
	.adc_channel		= apq8064_pm8921_adc_channels_data,
	.adc_num_board_channel	= ARRAY_SIZE(apq8064_pm8921_adc_channels_data),
	.adc_prop		= &apq8064_pm8921_adc_data,
	.adc_mpp_base		= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8921_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_gpio_platform_data
apq8064_pm8921_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_irq_platform_data
apq8064_pm8921_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(74),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
	.dev_id			= 0,
};

//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
static struct pm8xxx_rtc_platform_data
apq8064_pm8921_rtc_pdata = {
	.rtc_write_enable       = true,
	.rtc_alarm_powerup      = true,
};
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.

static int apq8064_pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	325,
};
/*[ECID:000000] ZTBSP zhangbo change for charging, start*/
#define MAX_VOLTAGE_MV          4370
#ifdef CONFIG_ZTE_BATTERY_4350MV_3200MAH
	#define CHG_TERM_MA		120
	#define ZTE_MAX_BATT_CHG_CURRENT 1200
#elif defined CONFIG_ZTE_BATTERY_4350MV_1735MAH
	#define CHG_TERM_MA		60
	#define ZTE_MAX_BATT_CHG_CURRENT 900
#else
	#error "no battery information is defined."
#endif
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.safety_time		=400, /* min*/ 
	.ttrkl_time	= 60, /*min zhangbo add*/
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= 3200,
	.uvd_thresh_voltage	= 4050,
	.resume_voltage_delta	= 50, //zhangbo
	.term_current		= CHG_TERM_MA,
	.cool_temp		= -20, //zhangbo
	.warm_temp		= 45,
	.temp_check_period	= 1,
	.max_bat_chg_current	= ZTE_MAX_BATT_CHG_CURRENT,
	.cool_bat_chg_current	= 350,
	.warm_bat_chg_current	= 350,
	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.trkl_current = 180, //zhangbo
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.rconn_mohm		= 18,
	.hot_thr = PM_SMBC_BATT_TEMP_HOT_THR__HIGH, //zhangbo
};
/*[ECID:000000] ZTBSP zhangbo change for charging, end*/

static struct pm8xxx_ccadc_platform_data
apq8064_pm8xxx_ccadc_pdata = {
	.r_sense		= 10,
	.calib_delay_ms		= 600000,
};

static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
	.battery_type	= BATT_ZTE, 
	.r_sense			= 10,
	.v_cutoff			= 3400,
	.max_voltage_uv			= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 18,
	.shutdown_soc_valid_limit	= 20,
	.adjust_soc_low_threshold	= 25,
	.chg_term_ua			= CHG_TERM_MA * 1000,
};

static struct pm8921_platform_data
apq8064_pm8921_platform_data __devinitdata = {
	.regulator_pdatas	= msm8064_pm8921_regulator_pdata,
	.irq_pdata		= &apq8064_pm8921_irq_pdata,
	.gpio_pdata		= &apq8064_pm8921_gpio_pdata,
	.mpp_pdata		= &apq8064_pm8921_mpp_pdata,
	.rtc_pdata		= &apq8064_pm8921_rtc_pdata,
	.pwrkey_pdata		= &apq8064_pm8921_pwrkey_pdata,
	.misc_pdata		= &apq8064_pm8921_misc_pdata,
	.leds_pdata		= &apq8064_pm8921_leds_pdata,
	.adc_pdata		= &apq8064_pm8921_adc_pdata,
	.charger_pdata		= &apq8064_pm8921_chg_pdata,
	.bms_pdata		= &apq8064_pm8921_bms_pdata,
	.ccadc_pdata		= &apq8064_pm8xxx_ccadc_pdata,
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 start*/
	#ifdef CONFIG_PMIC8XXX_VIBRATOR
	.vibrator_pdata      = &pmic_vibrator_pdata,
#endif
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 end*/
};

static struct pm8xxx_irq_platform_data
apq8064_pm8821_irq_pdata __devinitdata = {
	.irq_base		= PM8821_IRQ_BASE,
	.devirq			= PM8821_SEC_IRQ_N,
	.irq_trigger_flag	= IRQF_TRIGGER_HIGH,
	.dev_id			= 1,
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8821_mpp_pdata __devinitdata = {
	.mpp_base	= PM8821_MPP_PM_TO_SYS(1),
};

static struct pm8821_platform_data
apq8064_pm8821_platform_data __devinitdata = {
	.irq_pdata	= &apq8064_pm8821_irq_pdata,
	.mpp_pdata	= &apq8064_pm8821_mpp_pdata,
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8921-core",
		.platform_data	= &apq8064_pm8921_platform_data,
	},
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8821_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8821-core",
		.platform_data	= &apq8064_pm8821_platform_data,
	},
};

void __init apq8064_init_pmic(void)
{
	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;

	apq8064_device_ssbi_pmic1.dev.platform_data =
						&apq8064_ssbi_pm8921_pdata;
	apq8064_device_ssbi_pmic2.dev.platform_data =
				&apq8064_ssbi_pm8821_pdata;
	apq8064_pm8921_platform_data.num_regulators =
					msm8064_pm8921_regulator_pdata_len;

	/*[ECID:000000] ZTBSP zhangbo change for battery type, start*/
	if (machine_is_apq8064_rumi3()) {
		apq8064_pm8921_irq_pdata.devirq = 0;
		apq8064_pm8821_irq_pdata.devirq = 0;
	} else if (machine_is_apq8064_mtp()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_PALLADIUM;
	} else if (machine_is_apq8064_liquid()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_DESAY;
	} else if (machine_is_apq8064_cdp()) {
		apq8064_pm8921_chg_pdata.has_dc_supply = true;
	}
	apq8064_pm8921_bms_pdata.battery_type = BATT_ZTE; //zhangbo
	/*[ECID:000000] ZTBSP zhangbo change for battery type, end*/
}
