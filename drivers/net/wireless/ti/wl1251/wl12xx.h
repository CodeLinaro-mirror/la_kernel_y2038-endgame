/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * This file is part of wl12xx
 *
 * Copyright (C) 2009 Nokia Corporation
 *
 * Contact: Luciano Coelho <luciano.coelho@nokia.com>
 */

#ifndef _LINUX_WL12XX_H
#define _LINUX_WL12XX_H

#include <linux/err.h>

struct wl1251_platform_data {
	struct gpio_desc *power_gpio;
	/* SDIO only: IRQ number if WLAN_IRQ line is used, 0 for SDIO IRQs */
	int irq;
	bool use_eeprom;
};

#endif
