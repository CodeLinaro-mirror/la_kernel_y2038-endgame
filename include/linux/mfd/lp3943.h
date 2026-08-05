/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * TI/National Semiconductor LP3943 Device
 *
 * Copyright 2013 Texas Instruments
 *
 * Author: Milo Kim <milo.kim@ti.com>
 */

#ifndef __MFD_LP3943_H__
#define __MFD_LP3943_H__

#include <linux/gpio/consumer.h>
#include <linux/regmap.h>

/* Registers */
#define LP3943_REG_GPIO_A		0x00
#define LP3943_REG_GPIO_B		0x01
#define LP3943_REG_PRESCALE0		0x02
#define LP3943_REG_PWM0			0x03
#define LP3943_REG_PRESCALE1		0x04
#define LP3943_REG_PWM1			0x05
#define LP3943_REG_MUX0			0x06
#define LP3943_REG_MUX1			0x07
#define LP3943_REG_MUX2			0x08
#define LP3943_REG_MUX3			0x09

/* Bit description for LP3943_REG_MUX0 ~ 3 */
#define LP3943_GPIO_IN			0x00
#define LP3943_GPIO_OUT_HIGH		0x00
#define LP3943_GPIO_OUT_LOW		0x01
#define LP3943_DIM_PWM0			0x02
#define LP3943_DIM_PWM1			0x03

#define LP3943_NUM_PWMS			2

/*
 * struct lp3943_reg_cfg
 * @reg: Register address
 * @mask: Register bit mask to be updated
 * @shift: Register bit shift
 */
struct lp3943_reg_cfg {
	u8 reg;
	u8 mask;
	u8 shift;
};

/*
 * struct lp3943
 * @dev: Parent device pointer
 * @regmap: Used for I2C communication on accessing registers
 * @mux_cfg: Register configuration for pin MUX
 * @pin_used: Bit mask for output pin used.
 *	      This bitmask is used for pin assignment management.
 *	      1 = pin used, 0 = available.
 *	      Only LSB 16 bits are used, but it is unsigned long type
 *	      for atomic bitwise operations.
 */
struct lp3943 {
	struct device *dev;
	struct regmap *regmap;
	const struct lp3943_reg_cfg *mux_cfg;
	unsigned long pin_used;
};

int lp3943_read_byte(struct lp3943 *lp3943, u8 reg, u8 *read);
int lp3943_write_byte(struct lp3943 *lp3943, u8 reg, u8 data);
int lp3943_update_bits(struct lp3943 *lp3943, u8 reg, u8 mask, u8 data);
#endif
