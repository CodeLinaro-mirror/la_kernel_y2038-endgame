/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * max14577.h - Driver for the Maxim 14577/77836
 *
 * Copyright (C) 2014 Samsung Electronics
 * Chanwoo Choi <cw00.choi@samsung.com>
 * Krzysztof Kozlowski <krzk@kernel.org>
 *
 * This driver is based on max8997.h
 *
 * MAX14577 has MUIC, Charger devices.
 * The devices share the same I2C bus and interrupt line
 * included in this mfd driver.
 *
 * MAX77836 has additional PMIC and Fuel-Gauge on different I2C slave
 * addresses.
 */

#ifndef __MAX14577_H__
#define __MAX14577_H__

#include <linux/regulator/consumer.h>

/* MAX14577 regulator IDs */
enum max14577_regulators {
	MAX14577_SAFEOUT = 0,
	MAX14577_CHARGER,

	MAX14577_REGULATOR_NUM,
};

/* MAX77836 regulator IDs */
enum max77836_regulators {
	MAX77836_SAFEOUT = 0,
	MAX77836_CHARGER,
	MAX77836_LDO1,
	MAX77836_LDO2,

	MAX77836_REGULATOR_NUM,
};

/*
 * Valid limits of current for max14577 and max77836 chargers.
 * They must correspond to MBCICHWRCL and MBCICHWRCH fields in CHGCTRL4
 * register for given chipset.
 */
struct maxim_charger_current {
	/* Minimal current, set in CHGCTRL4/MBCICHWRCL, uA */
	unsigned int min;
	/*
	 * Minimal current when high setting is active,
	 * set in CHGCTRL4/MBCICHWRCH, uA
	 */
	unsigned int high_start;
	/* Value of one step in high setting, uA */
	unsigned int high_step;
	/* Maximum current of high setting, uA */
	unsigned int max;
};

extern const struct maxim_charger_current maxim_charger_currents[];
extern int maxim_charger_calc_reg_current(const struct maxim_charger_current *limits,
		unsigned int min_ua, unsigned int max_ua, u8 *dst);

#endif /* __MAX14577_H__ */
