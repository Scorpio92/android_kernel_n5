/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

/*
 * Toshiba MIPI-DSI-to-LVDS Bridge driver.
 * Device Model TC358764XBG/65XBG.
 * Reference document: TC358764XBG_65XBG_V119.pdf
 *
 * The Host sends a DSI Generic Long Write packet (Data ID = 0x29) over the
 * DSI link for each write access transaction to the chip configuration
 * registers.
 * Payload of this packet is 16-bit register address and 32-bit data.
 * Multiple data values are allowed for sequential addresses.
 *
 * The Host sends a DSI Generic Read packet (Data ID = 0x24) over the DSI
 * link for each read request transaction to the chip configuration
 * registers. Payload of this packet is further defined as follows:
 * 16-bit address followed by a 32-bit value (Generic Long Read Response
 * packet).
 *
 * The bridge supports 5 GPIO lines controlled via the GPC register.
 *
 * The bridge support I2C Master/Slave.
 * The I2C slave can be used for read/write to the bridge register instead of
 * using the DSI interface.
 * I2C slave address is 0x0F (read/write 0x1F/0x1E).
 * The I2C Master can be used for communication with the panel if
 * it has an I2C slave.
 *
 * NOTE: The I2C interface is not used in this driver.
 * Only the DSI interface is used for read/write the bridge registers.
 *
 * Pixel data can be transmitted in non-burst or burst fashion.
 * Non-burst refers to pixel data packet transmission time on DSI link
 * being roughly the same (to account for packet overhead time)
 * as active video line time on LVDS output (i.e. DE = 1).
 * And burst refers to pixel data packet transmission time on DSI link
 * being less than the active video line time on LVDS output.
 * Video mode transmission is further differentiated by the types of
 * timing events being transmitted.
 * Video pulse mode refers to the case where both sync start and sync end
 * events (for frame and line) are transmitted.
 * Video event mode refers to the case where only sync start events
 * are transmitted.
 * This is configured via register bit VPCTRL.EVTMODE.
 *
 */

#define DEBUG 1

/**
 * Use the I2C master to control the panel.
 */
/* #define TC358762_USE_I2C_MASTER */

#define DRV_NAME "mipi_tc358762"

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/pwm.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_tc358762_dsi2rgb.h"

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
#define LMS501_PANEL

#ifdef LMS501_PANEL
/* spi sw control to panel */
#define SPI_SW_CONTROL
/* spi hw control to panel */
//#define SPI_HW_CONTROL
#endif

#ifdef SPI_SW_CONTROL
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#endif

/* Registers definition */

/* DSI D-PHY Layer Registers */
#define D0W_DPHYCONTTX	0x0004	/* Data Lane 0 DPHY Tx Control */
#define CLW_DPHYCONTRX	0x0020	/* Clock Lane DPHY Rx Control */
#define D0W_DPHYCONTRX	0x0024	/* Data Lane 0 DPHY Rx Control */
#define D1W_DPHYCONTRX	0x0028	/* Data Lane 1 DPHY Rx Control */
#define D2W_DPHYCONTRX	0x002C	/* Data Lane 2 DPHY Rx Control */
#define D3W_DPHYCONTRX	0x0030	/* Data Lane 3 DPHY Rx Control */
#define COM_DPHYCONTRX	0x0038	/* DPHY Rx Common Control */
#define CLW_CNTRL	0x0040	/* Clock Lane Control */
#define D0W_CNTRL	0x0044	/* Data Lane 0 Control */
#define D1W_CNTRL	0x0048	/* Data Lane 1 Control */
#define D2W_CNTRL	0x004C	/* Data Lane 2 Control */
#define D3W_CNTRL	0x0050	/* Data Lane 3 Control */
#define DFTMODE_CNTRL	0x0054	/* DFT Mode Control */

/* DSI PPI Layer Registers */
#define PPI_STARTPPI	0x0104	/* START control bit of PPI-TX function. */
#define PPI_BUSYPPI	0x0108
#define PPI_LINEINITCNT	0x0110	/* Line Initialization Wait Counter  */
#define PPI_LPTXTIMECNT	0x0114
#define PPI_LANEENABLE	0x0134	/* Enables each lane at the PPI layer. */
#define PPI_TX_RX_TA	0x013C	/* DSI Bus Turn Around timing parameters */

/* Analog timer function enable */
#define PPI_CLS_ATMR	0x0140	/* Delay for Clock Lane in LPRX  */
#define PPI_D0S_ATMR	0x0144	/* Delay for Data Lane 0 in LPRX */
#define PPI_D1S_ATMR	0x0148	/* Delay for Data Lane 1 in LPRX */
#define PPI_D2S_ATMR	0x014C	/* Delay for Data Lane 2 in LPRX */
#define PPI_D3S_ATMR	0x0150	/* Delay for Data Lane 3 in LPRX */
#define PPI_D0S_CLRSIPOCOUNT	0x0164

#define PPI_D1S_CLRSIPOCOUNT	0x0168	/* For lane 1 */
#define PPI_D2S_CLRSIPOCOUNT	0x016C	/* For lane 2 */
#define PPI_D3S_CLRSIPOCOUNT	0x0170	/* For lane 3 */

#define CLS_PRE		0x0180	/* Digital Counter inside of PHY IO */
#define D0S_PRE		0x0184	/* Digital Counter inside of PHY IO */
#define D1S_PRE		0x0188	/* Digital Counter inside of PHY IO */
#define D2S_PRE		0x018C	/* Digital Counter inside of PHY IO */
#define D3S_PRE		0x0190	/* Digital Counter inside of PHY IO */
#define CLS_PREP	0x01A0	/* Digital Counter inside of PHY IO */
#define D0S_PREP	0x01A4	/* Digital Counter inside of PHY IO */
#define D1S_PREP	0x01A8	/* Digital Counter inside of PHY IO */
#define D2S_PREP	0x01AC	/* Digital Counter inside of PHY IO */
#define D3S_PREP	0x01B0	/* Digital Counter inside of PHY IO */
#define CLS_ZERO	0x01C0	/* Digital Counter inside of PHY IO */
#define D0S_ZERO	0x01C4	/* Digital Counter inside of PHY IO */
#define D1S_ZERO	0x01C8	/* Digital Counter inside of PHY IO */
#define D2S_ZERO	0x01CC	/* Digital Counter inside of PHY IO */
#define D3S_ZERO	0x01D0	/* Digital Counter inside of PHY IO */

#define PPI_CLRFLG	0x01E0	/* PRE Counters has reached set values */
#define PPI_CLRSIPO	0x01E4	/* Clear SIPO values, Slave mode use only. */
#define HSTIMEOUT	0x01F0	/* HS Rx Time Out Counter */
#define HSTIMEOUTENABLE	0x01F4	/* Enable HS Rx Time Out Counter */
#define DSI_STARTDSI	0x0204	/* START control bit of DSI-TX function */
#define DSI_BUSYDSI	0x0208
#define DSI_LANEENABLE	0x0210	/* Enables each lane at the Protocol layer. */
#define DSI_LANESTATUS0	0x0214	/* Displays lane is in HS RX mode. */
#define DSI_LANESTATUS1	0x0218	/* Displays lane is in ULPS or STOP state */

#define DSI_INTSTATUS	0x0220	/* Interrupt Status */
#define DSI_INTMASK	0x0224	/* Interrupt Mask */
#define DSI_INTCLR	0x0228	/* Interrupt Clear */
#define DSI_LPTXTO	0x0230	/* Low Power Tx Time Out Counter */

#define DSIERRCNT	0x0300	/* DSI Error Count */
#define APLCTRL		0x0400	/* Application Layer Control */
#define RDPKTLN		0x0404	/* Command Read Packet Length */
#define VPCTRL		0x0450	/* Video Path Control */
#define HTIM1		0x0454	/* Horizontal Timing Control 1 */
#define HTIM2		0x0458	/* Horizontal Timing Control 2 */
#define VTIM1		0x045C	/* Vertical Timing Control 1 */
#define VTIM2		0x0460	/* Vertical Timing Control 2 */
#define VFUEN		0x0464	/* Video Frame Timing Update Enable */

/* Mux Input Select for LVDS LINK Input */
#define LVMX0003	0x0480	/* Bit 0 to 3 */
#define LVMX0407	0x0484	/* Bit 4 to 7 */
#define LVMX0811	0x0488	/* Bit 8 to 11 */
#define LVMX1215	0x048C	/* Bit 12 to 15 */
#define LVMX1619	0x0490	/* Bit 16 to 19 */
#define LVMX2023	0x0494	/* Bit 20 to 23 */
#define LVMX2427	0x0498	/* Bit 24 to 27 */

#define LVCFG		0x049C	/* LVDS Configuration  */
#define LVPHY0		0x04A0	/* LVDS PHY 0 */
#define LVPHY1		0x04A4	/* LVDS PHY 1 */
#define SYSSTAT		0x0500	/* System Status  */
#define SYSRST		0x0504	/* System Reset  */

/* GPIO Registers */
#define GPIOC		0x0480	/* GPIO Control  */
#define GPIOO		0x0484	/* GPIO Output  */
#define GPIOI		0x0488	/* GPIO Input  */

/* I2C Registers */
#define I2CTIMCTRL	0x0540	/* I2C IF Timing and Enable Control */
#define I2CMADDR	0x0544	/* I2C Master Addressing */
#define WDATAQ		0x0548	/* Write Data Queue */
#define RDATAQ		0x054C	/* Read Data Queue */

/* Chip ID and Revision ID Register */
//#define IDREG		0x0580
#define IDREG		0x04A0

//#define TC358762XBG_ID	0x00006500
#define TC358762XBG_ID	0x00006200

/* Debug Registers */
#define DEBUG00		0x05A0	/* Debug */
#define DEBUG01		0x05A4	/* LVDS Data */

/* PWM */
#define PWM_FREQ_HZ	(66*1000)	/* 66 KHZ */
#define PWM_LEVEL 15
#define PWM_PERIOD_USEC (USEC_PER_SEC / PWM_FREQ_HZ)
#define PWM_DUTY_LEVEL (PWM_PERIOD_USEC / PWM_LEVEL)

#define CMD_DELAY 100
#define DSI_MAX_LANES 4
#define KHZ 1000
#define MHZ (1000*1000)

/*LCDC/DPI Host Registers*/
#define LCDCTRL		0x0420	/* Control the feature of LCDC interface */

#define WCMDQUE 	0x0500

struct dsi_rgb_bridge_instance {
	struct i2c_adapter *adap;
} *dsi_rgb_inst;


#define TC359762_ADDRESS 0x0b
/**
 * Command payload for DTYPE_GEN_LWRITE (0x29) / DTYPE_GEN_READ2 (0x24).
 */
struct wr_cmd_payload {
	u16 addr;
	u32 data;
} __packed;

/*
 * Driver state.
 */
static struct msm_panel_common_pdata *d2r_common_pdata;
struct msm_fb_data_type *d2r_mfd;
static struct dsi_buf d2r_tx_buf;
static struct dsi_buf d2r_rx_buf;
static int led_pwm;
static struct pwm_device *bl_pwm;
static int bl_level;
//static u32 d2r_gpio_out_mask;
//static u32 d2r_gpio_out_val;
static int mipi_d2r_init(void);


#ifdef LMS501_PANEL
unsigned char INDEX_b9[] =
{0xb9,
0xff, 0x83, 0x69}; 

unsigned char INDEX_b1[] =
{0xb1,
0x01,0x00,0x34,0x06,0x00,0x14,0x14,0x20,0x28,0x12,
0x12,0x17,0x0a,0x01,0xe6,0xe6,0xe6,0xe6,0xe6};

unsigned char INDEX_b2[] =
{0xb2,
0x00,0x2b,0x03,0x03,0x70,0x00,0xff,0x00,0x00,0x00,
0x00,0x03,0x03,0x00,0x01}; 

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213 */
/*0x09->0x01, for white vertical line at the right of lms501 panel*/
unsigned char INDEX_b3[] =
{0xb3, 
0x01};

unsigned char INDEX_b4[] =
{0xb4,
0x01,0x08,0x77,0x0e,0x06};
 
unsigned char INDEX_b6[] =
{0xb6,
0x4c,0x2e};

unsigned char INDEX_d5[] =
{0xd5,
0x00,0x05,0x03,0x29,0x01,0x07,0x17,0x68,0x13,0x37,
0x20,0x31,0x8a,0x46,0x9b,0x57,0x13,0x02,0x75,0xb9,
0x64,0xa8,0x07,0x0f,0x04,0x07};

unsigned char INDEX_cc[] =
{0xcc,
0x0a};

unsigned char INDEX_3a[] =
{0x3a,
0x77};

unsigned char INDEX_e0[] =
{0xe0,
0x00,0x04,0x09,0x0f,0x1f,0x3f,0x1f,0x2f,0x0a,0x0f,
0x10,0x16,0x18,0x16,0x17,0x0d,0x15,0x00,0x04,0x09,
0x0f,0x38,0x3f,0x20,0x39,0x0a,0x0f,0x10,0x16,0x18,
0x16,0x17,0x0d,0x15};

unsigned char INDEX_c1[] =
{0xc1,
0x01,0x03,0x07,0x0f,0x1a,0x22,0x2c,0x33,0x3c,0x46,
0x4f,0x58,0x60,0x69,0x71,0x79,0x82,0x89,0x92,0x9a,
0xa1,0xa9,0xb1,0xb9,0xc1,0xc9,0xcf,0xd6,0xde,0xe5,
0xec,0xf3,0xf9,0xff,0xdd,0x39,0x07,0x1c,0xcb,0xab,
0x5f,0x49,0x80,0x03,0x07,0x0f,0x19,0x20,0x2a,0x31,
0x39,0x42,0x4b,0x53,0x5b,0x63,0x6b,0x73,0x7b,0x83,
0x8a,0x92,0x9b,0xa2,0xaa,0xb2,0xba,0xc2,0xca,0xd0,
0xd8,0xe1,0xe8,0xf0,0xf8,0xff,0xf7,0xd8,0xbe,0xa7,
0x39,0x40,0x85,0x8c,0xc0,0x04,0x07,0x0c,0x17,0x1c,
0x23,0x2b,0x34,0x3b,0x43,0x4c,0x54,0x5b,0x63,0x6a,
0x73,0x7a,0x82,0x8a,0x91,0x98,0xa1,0xa8,0xb0,0xb7,
0xc1,0xc9,0xcf,0xd9,0xe3,0xea,0xf4,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char INDEX_36[] =
{0x36,
0x10};

unsigned char INDEX_11[] =
{0x11};

unsigned char INDEX_29[] =
{0x29};

#endif


#ifdef SPI_SW_CONTROL
static int spi_cs;
static int spi_sclk;
static int spi_mosi;
static int spi_miso;

#define SPI_CS_ASSERT 0
#define SPI_CS_DEASSERT 1

/*
extern void __gpio_set_value(unsigned gpio, int value);

static inline void gpio_set_value(unsigned gpio, int value)
{
	__gpio_set_value(gpio, value);
}
*/

static void spi_pin_assign(void)
{
	/* Setting the Default GPIO's */
#if 0
	spi_sclk = 5;
	spi_cs   = 3;
	spi_mosi  = 14;
	spi_miso  = 0;
#else /*for Z751*/
	spi_sclk = 54;
	spi_cs   = 53;
	spi_mosi  = 51;
	spi_miso  = 52;
#endif
}

static void toshiba_spi_write_byte(char dc, uint8 data)
{
	uint32 bit;
	int bnum;

	gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
	/* dc: 0 for command, 1 for parameter */
	gpio_set_value_cansleep(spi_mosi, dc);
	udelay(1);	/* at least 20 ns */
	gpio_set_value_cansleep(spi_sclk, 1); /* clk high */
	udelay(1);	/* at least 20 ns */
	bnum = 8;	/* 8 data bits */
	bit = 0x80;
	while (bnum) {
		gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
		if (data & bit)
			gpio_set_value_cansleep(spi_mosi, 1);
		else
			gpio_set_value_cansleep(spi_mosi, 0);
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); /* clk high */
		udelay(1);
		bit >>= 1;
		bnum--;
	}
	
}

//static int toshiba_spi_write(char cmd, uint32 data, int num)
static int toshiba_spi_write(char cmd, unsigned char *data, int num)
{
	char *bp;
#ifdef CONFIG_SPI_QSD
	char                tx_buf[4];
	int                 rc, i;
	struct spi_message  m;
	struct spi_transfer t;
	uint32 final_data = 0;

	if (!lms_spi_client) {
		printk(KERN_ERR "%s lcdc_toshiba_spi_client is NULL\n",
			__func__);
		return -EINVAL;
	}

	memset(&t, 0, sizeof t);
	t.tx_buf = tx_buf;
	spi_setup(lms_spi_client);
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	/* command byte first */
	final_data |= cmd << 23;
	t.len = num + 2;
	if (t.len < 4)
		t.bits_per_word = 8 * t.len;
	/* followed by parameter bytes */
	if (num) {
		bp = (char *)&data;;
		bp += (num - 1);
		i = 1;
		while (num) {
			final_data |= 1 << (((4 - i) << 3) - i - 1);
			final_data |= *bp << (((4 - i - 1) << 3) - i - 1);
			num--;
			bp--;
			i++;
		}
	}

	bp = (char *)&final_data;
	for (i = 0; i < t.len; i++)
		tx_buf[i] = bp[3 - i];
	t.rx_buf = NULL;
	rc = spi_sync(lms_spi_client, &m);
	if (rc)
		printk(KERN_ERR "spi_sync _write failed %d\n", rc);
	return rc;
#else

	printk("%s\n", __func__);

	gpio_set_value_cansleep(spi_cs, SPI_CS_ASSERT);	/* cs high */

	/* command byte first */
	toshiba_spi_write_byte(0, cmd);

	/* followed by parameter bytes */
	if (num) {
		bp = (char *)data;
		bp += 1;
		while (num) {
			toshiba_spi_write_byte(1, *bp);
			num--;
			bp++;
		}
	}


/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213 start*/
	gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
	udelay(5);

	gpio_set_value_cansleep(spi_cs, SPI_CS_DEASSERT);	/* cs low */
	udelay(5);

	gpio_set_value_cansleep(spi_sclk, 1); /* clk low */
	udelay(5);
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213 end*/

	return 0;
#endif
}

#endif


#define DSI_OP_INF
#ifdef DSI_OP_INF
/**
 * Read a bridge register
 *
 * @param mfd
 *
 * @return register data value
 */
static u32 mipi_d2r_read_reg(struct msm_fb_data_type *mfd, u16 reg)
{
	u32 data;
	int len = 4;
	struct dsi_cmd_desc cmd_read_reg = {
		DTYPE_GEN_READ2, 1, 0, 1, 0, /* cmd 0x24 */
			sizeof(reg), (char *) &reg};

	printk("%s: to read reg=0x%x.\n", __func__, reg);

	mipi_dsi_buf_init(&d2r_tx_buf);
	mipi_dsi_buf_init(&d2r_rx_buf);

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_lock(&mfd->dma->ov_mutex);
	len = mipi_dsi_cmds_rx(mfd, &d2r_tx_buf, &d2r_rx_buf,
			       &cmd_read_reg, len);
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_unlock(&mfd->dma->ov_mutex);

	data = *(u32 *)d2r_rx_buf.data;

	if (len != 4)
		pr_err("%s: invalid rlen=%d, expecting 4.\n", __func__, len);

	pr_debug("%s: reg=0x%x.data=0x%08x.\n", __func__, reg, data);

	printk("%s: reg=0x%x.data=0x%08x.\n", __func__, reg, data);

	return data;
}

/**
 * Write a bridge register
 *
 * @param mfd
 *
 * @return register data value
 */
static u32 mipi_d2r_write_reg(struct msm_fb_data_type *mfd, u16 reg, u32 data)
{
	struct wr_cmd_payload payload;
	struct dsi_cmd_desc cmd_write_reg = {
		DTYPE_GEN_LWRITE, 1, 0, 0, 0,
			sizeof(payload), (char *)&payload};

	printk("%s: to write reg=0x%x with data=0x%x.\n", __func__, reg, data);

	payload.addr = reg;
	payload.data = data;

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_lock(&mfd->dma->ov_mutex);
//wangtao fusion3-debug begin
//	mipi_dsi_cmds_tx(mfd, &d2r_tx_buf, &cmd_write_reg, 1);
mipi_dsi_cmds_tx(&d2r_tx_buf, &cmd_write_reg, 1);
//wangtao fusion3-debug end
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_unlock(&mfd->dma->ov_mutex);

	printk("%s: reg=0x%x. data=0x%x.\n", __func__, reg, data);
//	pr_debug("%s: reg=0x%x. data=0x%x.\n", __func__, reg, data);

	return data;
}


/**
 * Write a panel register via a WCMDQUE register of bridge
 *
 * @param mfd
 *
 * @return register data value
 */
static u32 mipi_d2r_write_panel_reg(struct msm_fb_data_type *mfd, u16 reg, u32 data)
{
	struct wr_cmd_payload payload;
	struct dsi_cmd_desc cmd_write_reg = {
		DTYPE_GEN_LWRITE, 1, 0, 0, 0,
			sizeof(payload)-1, (char *)&payload};

	//will transmit 5 byte of payload, the last byte of data member will be discarded.
	payload.addr = reg;
	payload.data = data;

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_lock(&mfd->dma->ov_mutex);
//wangtao fusion3-debug begin
//	mipi_dsi_cmds_tx(mfd, &d2r_tx_buf, &cmd_write_reg, 1);
mipi_dsi_cmds_tx(&d2r_tx_buf, &cmd_write_reg, 1);
//wangtao fusion3-debug end
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
//	mutex_unlock(&mfd->dma->ov_mutex);

	pr_debug("%s: reg=0x%x. data=0x%x.\n", __func__, reg, data);

	return data;
}


/*
 * Init the d2r bridge via the DSI interface for Video.
 *
 *	Register		Addr	Value
 *  ===================================================
 *  PPI_TX_RX_TA		0x013C	0x00040004
 *  PPI_LPTXTIMECNT	        0x0114	0x00000004
 *  PPI_D0S_CLRSIPOCOUNT	0x0164	0x00000003
 *  PPI_D1S_CLRSIPOCOUNT	0x0168	0x00000003
 *  PPI_D2S_CLRSIPOCOUNT	0x016C	0x00000003
 *  PPI_D3S_CLRSIPOCOUNT	0x0170	0x00000003
 *  PPI_LANEENABLE	        0x0134	0x0000001F
 *  DSI_LANEENABLE	        0x0210	0x0000001F
 *  PPI_STARTPPI	        0x0104	0x00000001
 *  DSI_STARTDSI	        0x0204	0x00000001
 *  VPCTRL			0x0450	0x01000120
 *  HTIM1			0x0454	0x002C0028
 *  VTIM1			0x045C	0x001E0008
 *  VFUEN			0x0464	0x00000001
 *  LVCFG			0x049C	0x00000001
 *
 *  the configuration of tc358762
 *  DSI_LANEENABLE	       	0x0210	0x00000003
 *  PPI_D0S_CLRSIPOCOUNT	0x0164	0x00000004
 *  PPI_D1S_CLRSIPOCOUNT	0x0168	0x00000004
 *  PPI_D0S_ATMR				0x0144	0x00000001
 *  PPI_D1S_ATMR				0x0148	0x00000001
 *  DSI_STARTDSI	        		0x0204	0x00000001
 *  LCDCTRL					0x0420	0x00000150
 *  VPCTRL					0x0450	0x00000062
 *  HTIM1						0x0454	0x00000122
 *  PPI_STARTPPI	        		0x0104	0x00000001
 *
 * VPCTRL.EVTMODE (0x20) configuration bit is needed to determine whether
 * video timing information is delivered in pulse mode or event mode.
 * In pulse mode, both Sync Start and End packets are required.
 * In event mode, only Sync Start packets are required.
 *
 * @param mfd
 *
 * @return register data value
 */
static int mipi_d2r_dsi_init_sequence(struct msm_fb_data_type *mfd)
{
//	struct mipi_panel_info *mipi = &mfd->panel_info.mipi;
	printk("%s.\n",__func__);

#if 0
	/* VESA format instead of JEIDA format for RGB888 */
	mipi_d2r_write_reg(mfd, DSI_LANEENABLE, 0x00000007); /* BTA */
	mipi_d2r_write_reg(mfd, PPI_D0S_CLRSIPOCOUNT, 0x00000004);
	mipi_d2r_write_reg(mfd, PPI_D1S_CLRSIPOCOUNT, 0x00000004);
	mipi_d2r_write_reg(mfd, PPI_D0S_ATMR, 0x00000001);
	mipi_d2r_write_reg(mfd, PPI_D1S_ATMR, 0x00000001);
	mipi_d2r_write_reg(mfd, DSI_STARTDSI, 0x00000001);
	mipi_d2r_write_reg(mfd, LCDCTRL, 0x00000150);
	mipi_d2r_write_reg(mfd, VPCTRL, 0x00000062);
	mipi_d2r_write_reg(mfd, HTIM1, 0x00000122);
	mipi_d2r_write_reg(mfd, PPI_STARTPPI, 0x00000001);


	/* initialisation of lcd panel */
	/* 
	    example for set panel reg,  100h=0x400, need to execute 2 commands

	   1. first command: set the index to 0x100
		payload byte(lengh is 5) order  00 05 70 01 00     
		explanation:
		[byte1][byte0]  0500	WCMDQUE register of the bridge
		[byte2]			70	set the index to pointer to register 0x100
	   2. second command: set the date as 0x400
		payload byte(lengh is 5) order  00 05 72 04 00     
		explanation:
		[byte1][byte0]  0500	WCMDQUE register of the bridge
		[byte2]			72	write the data with 0x0400
	*/
if (0) {
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff000170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff000472); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff000072); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff080070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff020072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff090070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff180072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff0c0070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff104072); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff030170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff000172); 
	msleep(10);
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff010170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff00c072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff200070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff400172); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff260070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff008872); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff010072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff190072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff020072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff790072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xfff90072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff030072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xff290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0xfff50372); 
	msleep(10);
} else {
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00000170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00000472); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00000072); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00080070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00020072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00090070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00180072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x000c0070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00104072); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00030170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00000172); 
	msleep(10);
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00010170); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x0000c072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00200070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00400172); 
	msleep(10);

	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00260070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00008872); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00010072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00190072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00020072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00790072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00f90072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00070070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00030072); 
	msleep(10);
	
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00290070); 
	mipi_d2r_write_panel_reg(mfd, WCMDQUE, 0x00f50372); 
	msleep(10);
}
#endif


#ifndef LMS501_PANEL /*attri*/
//	mipi_d2r_write_reg(mfd, 0x047c, 0x00000000);  /*Enter normal mode */
	mipi_d2r_write_reg(mfd, 0x0210, 0x00000007);  /*Enable both data lanes for HS mode */

	mipi_d2r_write_reg(mfd, 0x0164, 0x00000007);  /*Set lane0 Asserting Period  0x00000003*/
	mipi_d2r_write_reg(mfd, 0x0168, 0x00000007);  /*Set lane1 Asserting Period  0x00000003*/

	mipi_d2r_write_reg(mfd, 0x0144, 0x00000000);  /*Program ATMR0 register */
	mipi_d2r_write_reg(mfd, 0x0148, 0x00000000);  /*Program ATMR1 register */

	mipi_d2r_write_reg(mfd, 0x0114, 0x00000003);  /*LPTXTIMECNT 0x00000002*/
	mipi_d2r_write_reg(mfd, 0x0204, 0x00000001);  /*Start DSI Rx function */

/*DPI Registers*/	
	mipi_d2r_write_reg(mfd, 0x0420, 0x00000150);  /*Program LCDC Control Register, RGB888 */
	mipi_d2r_write_reg(mfd, 0x0424, 0x00240024);  /*HBPR_HSR*/
	mipi_d2r_write_reg(mfd, 0x0428, 0x00240320);  /*HFPR_HDISPR*/
	mipi_d2r_write_reg(mfd, 0x042C, 0x00020002);  /*VBPR_VSR*/
	mipi_d2r_write_reg(mfd, 0x0430, 0x02580002);  /*VFPR_VDISPR*/
	mipi_d2r_write_reg(mfd, 0x0434, 0x00000001);  /*VFUEN*/
	mipi_d2r_write_reg(mfd, 0x0450, 0x00000062);  /*SPIRCMR*/
	mipi_d2r_write_reg(mfd, 0x0454, 0x00000122);  /*SPITCR2_SPITCR1*/
	mipi_d2r_write_reg(mfd, 0x0464, 0x00000200);  /*SYSCTRL*/
	
/*Program Video Frame Update Register*/
	mipi_d2r_write_reg(mfd, 0x0104, 0x00000001);  /*Start the DSI-RX PPI function */
#else /*lms*/
//	mipi_d2r_write_reg(mfd, 0x047c, 0x00000000);  /*Enter normal mode */
	mipi_d2r_write_reg(mfd, 0x0210, 0x00000007);  /*Enable both data lanes for HS mode */

	mipi_d2r_write_reg(mfd, 0x0164, 0x00000007);  /*Set lane0 Asserting Period  0x00000003*/
	mipi_d2r_write_reg(mfd, 0x0168, 0x00000007);  /*Set lane1 Asserting Period  0x00000003*/

	mipi_d2r_write_reg(mfd, 0x0144, 0x00000000);  /*Program ATMR0 register */
	mipi_d2r_write_reg(mfd, 0x0148, 0x00000000);  /*Program ATMR1 register */

	mipi_d2r_write_reg(mfd, 0x0114, 0x00000002);  /*LPTXTIMECNT 0x00000002*/
	mipi_d2r_write_reg(mfd, 0x0204, 0x00000001);  /*Start DSI Rx function */

/*DPI Registers*/	
	mipi_d2r_write_reg(mfd, 0x0420, 0x00000150);  /*Program LCDC Control Register, RGB888 */
	mipi_d2r_write_reg(mfd, 0x0424, 0x00240024);  /*HBPR_HSR*/
	mipi_d2r_write_reg(mfd, 0x0428, 0x00240320);  /*HFPR_HDISPR*/
	mipi_d2r_write_reg(mfd, 0x042C, 0x00020002);  /*VBPR_VSR*/
	mipi_d2r_write_reg(mfd, 0x0430, 0x02580002);  /*VFPR_VDISPR*/
	mipi_d2r_write_reg(mfd, 0x0434, 0x00000001);  /*VFUEN*/
	mipi_d2r_write_reg(mfd, 0x0450, 0x00000063);  /*SPIRCMR 0x00000062*/
	mipi_d2r_write_reg(mfd, 0x0454, 0x00000122);  /*SPITCR2_SPITCR1*/
	mipi_d2r_write_reg(mfd, 0x0464, 0x00000200);  /*SYSCTRL*/
	
/*Program Video Frame Update Register*/
	mipi_d2r_write_reg(mfd, 0x0104, 0x00000001);  /*Start the DSI-RX PPI function */


#endif

/*Program the display device*/
#ifdef LMS501_PANEL
	msleep(100);
	toshiba_spi_write(INDEX_b9[0], &INDEX_b9[0], sizeof(INDEX_b9)-1);
	toshiba_spi_write(INDEX_b1[0], &INDEX_b1[0], sizeof(INDEX_b1)-1);
	toshiba_spi_write(INDEX_b2[0], &INDEX_b2[0], sizeof(INDEX_b2)-1);
	toshiba_spi_write(INDEX_b3[0], &INDEX_b3[0], sizeof(INDEX_b3)-1);
	toshiba_spi_write(INDEX_b4[0], &INDEX_b4[0], sizeof(INDEX_b4)-1);
	toshiba_spi_write(INDEX_b6[0], &INDEX_b6[0], sizeof(INDEX_b6)-1);
	toshiba_spi_write(INDEX_d5[0], &INDEX_d5[0], sizeof(INDEX_d5)-1);
	toshiba_spi_write(INDEX_cc[0], &INDEX_cc[0], sizeof(INDEX_cc)-1);
	toshiba_spi_write(INDEX_3a[0], &INDEX_3a[0], sizeof(INDEX_3a)-1);
	toshiba_spi_write(INDEX_e0[0], &INDEX_e0[0], sizeof(INDEX_e0)-1);
	toshiba_spi_write(INDEX_c1[0], &INDEX_c1[0], sizeof(INDEX_c1)-1);
	toshiba_spi_write(INDEX_36[0], &INDEX_36[0], sizeof(INDEX_36)-1);
	toshiba_spi_write(INDEX_11[0], &INDEX_11[0], sizeof(INDEX_11)-1);
	msleep(120);
	toshiba_spi_write(INDEX_29[0], &INDEX_29[0], sizeof(INDEX_29)-1);


//for compile, temp
if (0)
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000170);   /*reg 100h=400h */

#else /*original panel--attri*/
//exit standby mode , turn on operation amplifier
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000170);   /*reg 100h=400h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000472);
	msleep(10);

//power on reset
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00070070);  /*reg 007h=000h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000072);
	msleep(10);
	
//display off
//	mipi_d2r_write_reg(mfd, 0x0500, 0x00010070);   /*reg 001h=200h */
//	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000272);  
//	msleep(10);

//display off
//	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00070070);   /*reg 007h=000h */
//	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000072);  
//	msleep(10);

//4 lines for vertical back porch period
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00080070);    /*reg 008h=002h ->008h*/
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00080072);
	msleep(10);

// clks for horizontal back porch period
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00090070);    /*reg 009h=00Ch */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x000C0072);
	msleep(10);

//24 bit RGB interface
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x000c0070);    /*reg 00ch=4010h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00104072);
	msleep(100);

//Power Control 4, power supply circuit operation setting
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00030170);    /*reg 103h=100h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000172);
	msleep(100);

//Control power supply circuit 1 operation
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00010170);    /*reg 101h=c000h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x0000c072);
	msleep(100);

//Panel interface control 1
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00200070);    /*reg 020h=140h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00400172);
	msleep(10);

//Panel interface control 7
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00260070);    /*reg 026h=8800h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00008872);
	msleep(10);

//display control
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00070070);    /*reg 007h=0001h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00010072);
	msleep(10);

//panel interface control 10
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00290070);    /*reg 029h=0019h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00190072);
	msleep(1000);

//display control
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00070070);    /*reg 007h=0002h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00020072);
	msleep(10);

//panel interface control 10
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00290070);    /*reg 029h=0079h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00790072);
	msleep(10);

//panel interface control 10
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00290070);    /*reg 029h=00f9h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00f90072);
	msleep(10);

//display control
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00070070);    /*reg 007h=0003h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00030072);
	msleep(10);

//panel interface control 10
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00290070);    /*reg 029h=03f5h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00f50372);
	msleep(10);

//..
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00010070);    /*reg 001h=0200h */
	mipi_d2r_write_panel_reg(mfd, 0x0500, 0x00000272);
	msleep(10);
#endif

	return 0;
}
#endif /*DSI_OP_INF*/

/**
 * Set Backlight level.
 *
 * @param pwm
 * @param level
 *
 * @return int
 */
static int mipi_d2r_set_backlight_level(struct pwm_device *pwm, int level)
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

/* ************************************************************************* *\
 * FUNCTION: __DSI_I2C_ByteWrite
 *
 * DESCRIPTION:  Local functions to process write req for I2C registers
 *
\* ************************************************************************* */
static int __DSI_I2C_ByteWrite(u16 reg, u32 data, int count)
{
	char txData[6] = {0};
	struct i2c_msg msg[] = {
		{
		 .addr = TC359762_ADDRESS,
		 .flags = 0,
		 .len = count,
		 },
	};

	/*Set the register */
	txData[0] = (reg & 0xFF00) >> 8;
	txData[1] = reg & 0xFF;

	if (count == 6) {
		/*Set the data */
		txData[2] = (data & 0xFF);
		txData[3] = (data & 0xFF00) >> 8;
		txData[4] = (data & 0xFF0000) >> 16;
		txData[5] = (data & 0xFF000000) >> 24;
	} else if (count == 5) {
		/*Set the data */
		txData[2] = (data & 0xFF);
		txData[3] = (data & 0xFF00) >> 8;
		txData[4] = (data & 0xFF0000) >> 16;
	} else {
		/* Not valid for this bridge chipset */
	}
	msleep(50);
	printk(KERN_INFO "[DISPLAY] %s: reg = %x, data = %x\n",
		__func__, reg, data);

	msg[0].buf = txData;

	if (i2c_transfer(dsi_rgb_inst->adap, msg, 1) < 0) {
		printk(KERN_ERR "[DISPLAY] %s: transfer error\n", __func__);
		return -EIO;
	} else {
		return 0;
	}
}

/* ************************************************************************* *\
 * FUNCTION: DSI_I2C_ByteWrite
 *
 * DESCRIPTION:  Local functions to issue write req for I2C registers
 *
\* ************************************************************************* */
static int DSI_I2C_ByteWrite(u16 reg, u32 data, int count)
{
	if (dsi_rgb_inst->adap)
		return __DSI_I2C_ByteWrite(reg, data, count);
	else
		return -EIO;
}




/**
 * LCD ON.
 *
 * Set LCD On via MIPI interface or I2C-Slave interface.
 * Set Backlight on.
 *
 * @param pdev
 *
 * @return int
 */
static int mipi_d2r_lcd_on(struct platform_device *pdev)
{
	int ret = 0;
	u32 chip_id;
	struct msm_fb_data_type *mfd;

//	pr_info("%s.\n", __func__);

//	dev_WARN(&pdev->dev, "%s.\n", __func__);

	/* wait for valid clock before sending data over DSI or I2C. */
	msleep(30);

	mfd = platform_get_drvdata(pdev);
	d2r_mfd = mfd;

	pr_info("%s, mfd %d, key %d.\n", __func__, (int)mfd, mfd->key);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;


	if(d2r_common_pdata->panel_config_gpio == NULL){
		pr_err("%s: d2r_common_pdata ->panel_config_gpio NULL.\n", __func__);
		return -ENODEV;
	}
	d2r_common_pdata->panel_config_gpio(1);
	
	msleep(100);

/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213*/
/* 0 path means i2c op for tc358762*/
	if (0) {
 /*DSI registers*/
	DSI_I2C_ByteWrite(0x047c, 0x00000000, 6);  /*Enter normal mode */
	DSI_I2C_ByteWrite(0x0210, 0x00000007, 6);  /*Enable both data lanes for HS mode */

	DSI_I2C_ByteWrite(0x0164, 0x00000007, 6);  /*Set lane0 Asserting Period  0x00000003*/
	DSI_I2C_ByteWrite(0x0168, 0x00000007, 6);  /*Set lane1 Asserting Period  0x00000003*/

	DSI_I2C_ByteWrite(0x0144, 0x00000000, 6);  /*Program ATMR0 register */
	DSI_I2C_ByteWrite(0x0148, 0x00000000, 6);  /*Program ATMR1 register */

	DSI_I2C_ByteWrite(0x0114, 0x00000003, 6);  /*LPTXTIMECNT 0x00000002*/
	DSI_I2C_ByteWrite(0x0204, 0x00000001, 6);  /*Start DSI Rx function */

/*DPI Registers*/	
	DSI_I2C_ByteWrite(0x0420, 0x00000150, 6);  /*Program LCDC Control Register, RGB888 */
	DSI_I2C_ByteWrite(0x0424, 0x00240024, 6);  /*HBPR_HSR*/
	DSI_I2C_ByteWrite(0x0428, 0x00240320, 6);  /*HFPR_HDISPR*/
	DSI_I2C_ByteWrite(0x042C, 0x00020002, 6);  /*VBPR_VSR*/
	DSI_I2C_ByteWrite(0x0430, 0x02580002, 6);  /*VFPR_VDISPR*/
	DSI_I2C_ByteWrite(0x0434, 0x00000001, 6);  /*VFUEN*/
	DSI_I2C_ByteWrite(0x0450, 0x00000062, 6);  /*SPIRCMR*/
	DSI_I2C_ByteWrite(0x0454, 0x00000122, 6);  /*SPITCR2_SPITCR1*/
	DSI_I2C_ByteWrite(0x0464, 0x00000200, 6);  /*SYSCTRL*/
	
/*Program Video Frame Update Register*/
	DSI_I2C_ByteWrite(0x0104, 0x00000001, 6);  /*Start the DSI-RX PPI function */

/*Program the display device*/
//exit standby mode , turn on operation amplifier
	DSI_I2C_ByteWrite(0x0500, 0x00000170, 5);   /*reg 100h=400h */
	DSI_I2C_ByteWrite(0x0500, 0x00000472, 5);
	msleep(10);

//power on reset
	DSI_I2C_ByteWrite(0x0500, 0x00070070, 5);  /*reg 007h=000h */
	DSI_I2C_ByteWrite(0x0500, 0x00000072, 5);
	msleep(10);
	
//display off
//	DSI_I2C_ByteWrite(0x0500, 0x00010070, 5);   /*reg 001h=200h */
//	DSI_I2C_ByteWrite(0x0500, 0x00000272, 5);  
//	msleep(10);

//display off
//	DSI_I2C_ByteWrite(0x0500, 0x00070070, 5);   /*reg 007h=000h */
//	DSI_I2C_ByteWrite(0x0500, 0x00000072, 5);  
//	msleep(10);

//4 lines for vertical back porch period
	DSI_I2C_ByteWrite(0x0500, 0x00080070, 5);    /*reg 008h=002h ->008h*/
	DSI_I2C_ByteWrite(0x0500, 0x00080072, 5);
	msleep(10);

// clks for horizontal back porch period
	DSI_I2C_ByteWrite(0x0500, 0x00090070, 5);    /*reg 009h=00Ch */
	DSI_I2C_ByteWrite(0x0500, 0x000C0072, 5);
	msleep(10);

//24 bit RGB interface
	DSI_I2C_ByteWrite(0x0500, 0x000c0070, 5);    /*reg 00ch=4010h */
	DSI_I2C_ByteWrite(0x0500, 0x00104072, 5);
	msleep(100);

//Power Control 4, power supply circuit operation setting
	DSI_I2C_ByteWrite(0x0500, 0x00030170, 5);    /*reg 103h=100h */
	DSI_I2C_ByteWrite(0x0500, 0x00000172, 5);
	msleep(100);

//Control power supply circuit 1 operation
	DSI_I2C_ByteWrite(0x0500, 0x00010170, 5);    /*reg 101h=c000h */
	DSI_I2C_ByteWrite(0x0500, 0x0000c072, 5);
	msleep(100);

//Panel interface control 1
	DSI_I2C_ByteWrite(0x0500, 0x00200070, 5);    /*reg 020h=140h */
	DSI_I2C_ByteWrite(0x0500, 0x00400172, 5);
	msleep(10);

//Panel interface control 7
	DSI_I2C_ByteWrite(0x0500, 0x00260070, 5);    /*reg 026h=8800h */
	DSI_I2C_ByteWrite(0x0500, 0x00008872, 5);
	msleep(10);

//display control
	DSI_I2C_ByteWrite(0x0500, 0x00070070, 5);    /*reg 007h=0001h */
	DSI_I2C_ByteWrite(0x0500, 0x00010072, 5);
	msleep(10);

//panel interface control 10
	DSI_I2C_ByteWrite(0x0500, 0x00290070, 5);    /*reg 029h=0019h */
	DSI_I2C_ByteWrite(0x0500, 0x00190072, 5);
	msleep(1000);

//display control
	DSI_I2C_ByteWrite(0x0500, 0x00070070, 5);    /*reg 007h=0002h */
	DSI_I2C_ByteWrite(0x0500, 0x00020072, 5);
	msleep(10);

//panel interface control 10
	DSI_I2C_ByteWrite(0x0500, 0x00290070, 5);    /*reg 029h=0079h */
	DSI_I2C_ByteWrite(0x0500, 0x00790072, 5);
	msleep(10);

//panel interface control 10
	DSI_I2C_ByteWrite(0x0500, 0x00290070, 5);    /*reg 029h=00f9h */
	DSI_I2C_ByteWrite(0x0500, 0x00f90072, 5);
	msleep(10);

//display control
	DSI_I2C_ByteWrite(0x0500, 0x00070070, 5);    /*reg 007h=0003h */
	DSI_I2C_ByteWrite(0x0500, 0x00030072, 5);
	msleep(10);

//panel interface control 10
	DSI_I2C_ByteWrite(0x0500, 0x00290070, 5);    /*reg 029h=03f5h */
	DSI_I2C_ByteWrite(0x0500, 0x00f50372, 5);
	msleep(10);

//..
	DSI_I2C_ByteWrite(0x0500, 0x00010070, 5);    /*reg 001h=0200h */
	DSI_I2C_ByteWrite(0x0500, 0x00000272, 5);
	msleep(10);

//..
//	DSI_I2C_ByteWrite(0x0500, 0x00010070, 5);    /*reg 001h=0300h */
//	DSI_I2C_ByteWrite(0x0500, 0x00000272, 5);
//	msleep(10);

//for test J608.4
#if 0 
	DSI_I2C_ByteWrite(0x0450, 0x00000000, 6);
	DSI_I2C_ByteWrite(0x0480, 0x0000001f, 6);  
	DSI_I2C_ByteWrite(0x0484, 0x00000008, 6);
	msleep(50);
	DSI_I2C_ByteWrite(0x0480, 0x0000001f, 6);  
	DSI_I2C_ByteWrite(0x0484, 0x00000000, 6);
	msleep(50);
	DSI_I2C_ByteWrite(0x0480, 0x0000001f, 6);  
	DSI_I2C_ByteWrite(0x0484, 0x00000008, 6);
#endif
		}

//	mipi_d2r_write_reg(mfd, 0x0480, 0x1f);
	/* Set GPIOs: gpio#4=U/D=0 , gpio#3=L/R=1 , gpio#2,1=CABC=0. */
//	mipi_d2r_write_reg(mfd, 0x0484, 0x08);



#ifdef DSI_OP_INF
	/* prerequisite of dsi read operation*/
if (0) {

#if 0
	mipi_d2r_write_reg(mfd, DSI_LANEENABLE, 0x00000007); /*3->7 */
	mipi_d2r_write_reg(mfd, 0x0164, 0x00000003); 
	mipi_d2r_write_reg(mfd, 0x0168, 0x00000003); 
	mipi_d2r_write_reg(mfd, 0x0144, 0x00000000); 
	mipi_d2r_write_reg(mfd, 0x0148, 0x00000000); 
	mipi_d2r_write_reg(mfd, 0x0114, 0x00000010); 
	mipi_d2r_write_reg(mfd, 0x0410, 0x00000003); 
#endif

	mipi_d2r_write_reg(mfd, DSI_LANEENABLE, 0x00000007); /* BTA */
	mipi_d2r_write_reg(mfd, PPI_D0S_CLRSIPOCOUNT, 0x00000004);
	mipi_d2r_write_reg(mfd, PPI_D1S_CLRSIPOCOUNT, 0x00000004);
	mipi_d2r_write_reg(mfd, PPI_D0S_ATMR, 0x00000001);
	mipi_d2r_write_reg(mfd, PPI_D1S_ATMR, 0x00000001);
	mipi_d2r_write_reg(mfd, DSI_STARTDSI, 0x00000001);
	mipi_d2r_write_reg(mfd, LCDCTRL, 0x00000150);

//	mipi_d2r_write_reg(mfd, VPCTRL, 0x00000062);
	mipi_d2r_write_reg(mfd, VPCTRL, 0x00000060);

	mipi_d2r_write_reg(mfd, HTIM1, 0x00000122);
	mipi_d2r_write_reg(mfd, PPI_STARTPPI, 0x00000001);


	mipi_d2r_write_reg(mfd, 0x0480, 0x1f);
	/* Set GPIOs: gpio#4=U/D=0 , gpio#3=L/R=1 , gpio#2,1=CABC=0. */
	mipi_d2r_write_reg(mfd, 0x0484, 0x08);

//	mipi_dsi_cmd_bta_sw_trigger();

	chip_id = mipi_d2r_read_reg(mfd, IDREG);

	//wangbing
	printk("%s, chip id %d.\n", __func__, chip_id);

	if (chip_id != TC358762XBG_ID) {
		pr_err("%s: invalid chip_id=0x%x", __func__, chip_id);
		return -ENODEV;
	}

	//wangbing
	printk("%s, chip id %d.\n", __func__, chip_id);
}

if (1) {
//	mipi_set_tx_power_mode(1);
	ret = mipi_d2r_dsi_init_sequence(mfd);
}

//	mipi_d2r_write_reg(mfd, GPIOC, 0x1f);   //d2r_gpio_out_mask 
	/* Set GPIOs: gpio#4=U/D=0 , gpio#3=L/R=1 , gpio#2,1=CABC=0. */
//	mipi_d2r_write_reg(mfd, GPIOO, 0x08);  //d2r_gpio_out_val
	
	if (ret)
		return ret;

	if (bl_level == 0)
		bl_level = PWM_LEVEL * 2 / 3 ; /* Default ON value */

	/* Set backlight via PWM */
	if (bl_pwm) {
		ret = mipi_d2r_set_backlight_level(bl_pwm, bl_level);
		if (ret)
			pr_err("%s.mipi_d2r_set_backlight_level.ret=%d",
			       __func__, ret);
	}

#endif
	pr_info("%s.ret=%d.\n", __func__, ret);

	return ret;
}

/**
 * LCD OFF.
 *
 * @param pdev
 *
 * @return int
 */
static int mipi_d2r_lcd_off(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	pr_info("%s.\n", __func__);

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213  1*/
	ret = mipi_d2r_set_backlight_level(bl_pwm, 0);
	d2r_common_pdata->panel_config_gpio(0);
	ret = 0;

	pr_info("%s.ret=%d.\n", __func__, ret);

	return ret;
}

static void mipi_d2r_set_backlight(struct msm_fb_data_type *mfd)
{
	int level = mfd->bl_level;

	pr_debug("%s.lvl=%d.\n", __func__, level);

	mipi_d2r_set_backlight_level(bl_pwm, level);

	bl_level = level;
}

static struct msm_fb_panel_data d2r_panel_data = {
	.on = mipi_d2r_lcd_on,
	.off = mipi_d2r_lcd_off,
	.set_backlight = mipi_d2r_set_backlight,
};


#ifdef SPI_HW_CONTROL

#define LMS_SPI_DEVICE_NAME "lms501_spi"
static struct spi_device *lms_spi_client;

static int __devinit lms_spi_probe(struct spi_device *spi)
{
	lms_spi_client = spi;
	lms_spi_client->bits_per_word = 32;
	return 0;
}
static int __devexit lms_spi_remove(struct spi_device *spi)
{
	lms_spi_client = NULL;
	return 0;
}

static struct spi_driver lms_spi_driver = {
	.driver = {
		.name  = LMS_SPI_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe         = lms_spi_probe,
	.remove        = __devexit_p(lms_spi_remove),
};

static int toshiba_spi_write(char cmd, unsigned char *data, int num)
{
	char *bp;
#ifdef SPI_HW_CONTROL
	char                tx_buf[4];
	int                 rc, i;
	struct spi_message  m;
	struct spi_transfer t;
	uint32 final_data = 0;

	if (!lms_spi_client) {
		printk(KERN_ERR "%s lcdc_toshiba_spi_client is NULL\n",
			__func__);
		return -EINVAL;
	}

	memset(&t, 0, sizeof t);
	t.tx_buf = tx_buf;
	spi_setup(lms_spi_client);
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	/* command byte first */
	final_data |= cmd << 23;
	t.len = num + 2;
	if (t.len < 4)
		t.bits_per_word = 8 * t.len;
	/* followed by parameter bytes */
	if (num) {
		bp = (char *)&data;;
		bp += (num - 1);
		i = 1;
		while (num) {
			final_data |= 1 << (((4 - i) << 3) - i - 1);
			final_data |= *bp << (((4 - i - 1) << 3) - i - 1);
			num--;
			bp--;
			i++;
		}
	}

	bp = (char *)&final_data;
	for (i = 0; i < t.len; i++)
		tx_buf[i] = bp[3 - i];
	t.rx_buf = NULL;
	rc = spi_sync(lms_spi_client, &m);
	if (rc)
		printk(KERN_ERR "spi_sync _write failed %d\n", rc);
	return rc;
#else

	printk("%s\n", __func__);

	gpio_set_value_cansleep(spi_cs, SPI_CS_ASSERT);	/* cs high */

	/* command byte first */
	toshiba_spi_write_byte(0, cmd);

	/* followed by parameter bytes */
	if (num) {
		bp = (char *)data;
		bp += 1;
		while (num) {
			toshiba_spi_write_byte(1, *bp);
			num--;
			bp++;
		}
	}


/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213 start*/
	gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
	udelay(5);

	gpio_set_value_cansleep(spi_cs, SPI_CS_DEASSERT);	/* cs low */
	udelay(5);

	gpio_set_value_cansleep(spi_sclk, 1); /* clk low */
	udelay(5);
/*[ECID:000000] ZTEBSP wangbing, for video_lms501, 20120213 end*/

	return 0;
#endif
}
#endif

/**
 * Probe for device.
 *
 * Both the "target" and "panel" device use the same probe function.
 * "Target" device has id=0, "Panel" devic has non-zero id.
 * Target device should register first, passing msm_panel_common_pdata.
 * Panel device passing msm_panel_info.
 *
 * @param pdev
 *
 * @return int
 */
static int __devinit mipi_d2r_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct msm_panel_info *pinfo = NULL;

#ifdef SPI_SW_CONTROL 
	int rc;
#endif

	pr_debug("%s.id=%d.\n", __func__, pdev->id);

	if (pdev->id == 0) {
		/* d2r_common_pdata = platform_get_drvdata(pdev); */
		d2r_common_pdata = pdev->dev.platform_data;

		if (d2r_common_pdata == NULL) {
			pr_err("%s: no PWM gpio specified.\n", __func__);
			return 0;
		}

		led_pwm = d2r_common_pdata->gpio_num[0];
//		d2r_gpio_out_mask = d2r_common_pdata->gpio_num[1] >> 8;
//		d2r_gpio_out_val = d2r_common_pdata->gpio_num[1] & 0xFF;

		mipi_dsi_buf_alloc(&d2r_tx_buf, DSI_BUF_SIZE);
		mipi_dsi_buf_alloc(&d2r_rx_buf, DSI_BUF_SIZE);

		return 0;
	}

	if (d2r_common_pdata == NULL) {
		pr_err("%s: d2r_common_pdata is NULL.\n", __func__);
		return -ENODEV;
	}

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

	/* pinfo = platform_get_drvdata(pdev); */
	pinfo = pdev->dev.platform_data;

	if (pinfo == NULL) {
		pr_err("%s: pinfo is NULL.\n", __func__);
		return -ENODEV;
	}

#ifdef SPI_SW_CONTROL
	spi_pin_assign();

	printk("%s, configure spi gpio.\n", __func__);

	rc = gpio_request(spi_cs, "spi_cs");
	if (rc) {
		pr_err("request gpio %d failed, rc=%d\n", spi_cs, rc);
	}

	rc = gpio_request(spi_sclk, "spi_sclk");
	if (rc) {
		pr_err("request gpio %d failed, rc=%d\n", spi_sclk, rc);
	}

	rc = gpio_request(spi_mosi, "spi_mosi");
	if (rc) {
		pr_err("request gpio %d failed, rc=%d\n", spi_mosi, rc);
	}

	rc = gpio_request(spi_miso, "spi_miso");
	if (rc) {
		pr_err("request gpio %d failed, rc=%d\n", spi_miso, rc);
	}

		rc = gpio_direction_output(spi_cs, SPI_CS_DEASSERT);
		if (rc) {
			pr_err("%s: unable to set_direction for gpio [%d]\n",
				__func__, spi_cs);
		}

		rc = gpio_direction_output(spi_sclk, 1);
		if (rc) {
			pr_err("%s: unable to set_direction for gpio [%d]\n",
				__func__, spi_sclk);
		}

		rc = gpio_direction_output(spi_mosi, 0);
		if (rc) {
			pr_err("%s: unable to set_direction for gpio [%d]\n",
				__func__, spi_mosi);
		}

		rc = gpio_direction_output(spi_miso, 0);
		if (rc) {
			pr_err("%s: unable to set_direction for gpio [%d]\n",
				__func__, spi_miso);
		}

#if 0	
	gpio_set_value(spi_cs, 0);	/* low */
	gpio_set_value(spi_sclk, 1);	/* high */
	gpio_set_value(spi_mosi, 0);
	gpio_set_value(spi_miso, 0);
#endif
#endif

#ifdef  SPI_HW_CONTROL
	ret = spi_register_driver(&lms501_spi_driver);

	if (ret) {
		printk(KERN_ERR "%s not able to register spi\n", __func__);
		goto fail_device;
	}

#endif


	d2r_panel_data.panel_info = *pinfo;

	pdev->dev.platform_data = &d2r_panel_data;

	msm_fb_add_device(pdev);

	return ret;
}

/**
 * Device removal notification handler.
 *
 * @param pdev
 *
 * @return int
 */
static int __devexit mipi_d2r_remove(struct platform_device *pdev)
{
	/* Note: There are no APIs to remove fb device and free DSI buf. */
	pr_debug("%s.\n", __func__);

	if (bl_pwm) {
		pwm_free(bl_pwm);
		bl_pwm = NULL;
	}

	return 0;
}

/**
 * Register the panel device.
 *
 * @param pinfo
 * @param channel_id
 * @param panel_id
 *
 * @return int
 */
int mipi_tc358762_dsi2rgb_register(struct msm_panel_info *pinfo,
					   u32 channel_id, u32 panel_id)
{
	struct platform_device *pdev = NULL;
	int ret;
	/* Use DSI-to-rgb bridge */
	const char driver_name[] = "mipi_tc358762";

	pr_debug("%s.\n", __func__);
	ret = mipi_d2r_init();
	if (ret) {
		pr_err("mipi_d2r_init() failed with ret %u\n", ret);
		return ret;
	}

	/* Note: the device id should be non-zero */
	pdev = platform_device_alloc(driver_name, (panel_id << 8)|channel_id);
	if (pdev == NULL)
		return -ENOMEM;

	pdev->dev.platform_data = pinfo;

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	
	
	dsi_rgb_inst = kzalloc(sizeof(struct dsi_rgb_bridge_instance), GFP_KERNEL);

	if (dsi_rgb_inst == NULL) {
		printk(KERN_ERR "[DISPLAY] %s: Can not allocate memory.\n", __func__);
		return -ENOMEM;
	}

	dsi_rgb_inst->adap = i2c_get_adapter(10);


	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static struct platform_driver d2r_driver = {
	.probe  = mipi_d2r_probe,
	.remove = __devexit_p(mipi_d2r_remove),
	.driver = {
		.name   = DRV_NAME,
	},
};

/**
 * Module Init
 *
 * @return int
 */
static int mipi_d2r_init(void)
{
	pr_debug("%s.\n", __func__);
	return platform_driver_register(&d2r_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Toshiba MIPI-DSI-to-DPI bridge driver");
MODULE_AUTHOR("Amir Samuelov <amirs@codeaurora.org>");
