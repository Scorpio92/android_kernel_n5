/*
 * include/media/radio-si4713.h
 *
 * Board related data definitions for Si4713 radio transmitter chip.
 *
 * Copyright (c) 2009 Nokia Corporation
 * Contact: Eduardo Valentin <eduardo.valentin@nokia.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 */

#ifndef RADIO_SI470X_H
#define RADIO_SI470X_H

#include <linux/i2c.h>

/*
 * Platform dependent definition
 */
struct radio_si470x_platform_data {
	unsigned int reset_pin;
	int (*power_on)(bool on);
	int (*clk_enable)(bool on);
};

#endif /* ifndef RADIO_SI470X_H*/
