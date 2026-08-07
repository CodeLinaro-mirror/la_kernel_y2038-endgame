/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * max77693.h - Driver for the Maxim 77693
 *
 *  Copyright (C) 2012 Samsung Electronics
 *  SangYoung Son <hello.son@samsung.com>
 *
 * This program is not provided / owned by Maxim Integrated Products.
 *
 * This driver is based on max8997.h
 *
 * MAX77693 has PMIC, Charger, Flash LED, Haptic, MUIC devices.
 * The devices share the same I2C bus and included in
 * this mfd driver.
 */

#ifndef __LINUX_MFD_MAX77693_H
#define __LINUX_MFD_MAX77693_H

/* MAX77693 regulator IDs */
enum max77693_regulators {
	MAX77693_ESAFEOUT1 = 0,
	MAX77693_ESAFEOUT2,
	MAX77693_CHARGER,
	MAX77693_REG_MAX,
};

/* MAX77693 led flash */

/* triggers */
enum max77693_led_trigger {
	MAX77693_LED_TRIG_OFF,
	MAX77693_LED_TRIG_FLASH,
	MAX77693_LED_TRIG_TORCH,
	MAX77693_LED_TRIG_EXT,
	MAX77693_LED_TRIG_SOFT,
};

/* trigger types */
enum max77693_led_trigger_type {
	MAX77693_LED_TRIG_TYPE_EDGE,
	MAX77693_LED_TRIG_TYPE_LEVEL,
};

/* boost modes */
enum max77693_led_boost_mode {
	MAX77693_LED_BOOST_NONE,
	MAX77693_LED_BOOST_ADAPTIVE,
	MAX77693_LED_BOOST_FIXED,
};

#endif	/* __LINUX_MFD_MAX77693_H */
