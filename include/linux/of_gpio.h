/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * OF helpers for the GPIO API
 *
 * Copyright (c) 2007-2008  MontaVista Software, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#ifndef __LINUX_OF_GPIO_H
#define __LINUX_OF_GPIO_H

#include <linux/compiler.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>		/* FIXME: Shouldn't be here */
#include <linux/of.h>
#include <linux/container_of.h>
#include <linux/errno.h>

struct device_node;

/* Drivers may not strictly depend on the GPIO support, so let them link. */
#ifdef CONFIG_OF_GPIO
extern int of_get_named_gpio(const struct device_node *np,
		const char *list_name, int index);
#else /* CONFIG_OF_GPIO */
static inline int of_get_named_gpio(const struct device_node *np,
		const char *list_name, int index)
{
	return -ENOSYS;
}
#endif /* CONFIG_OF_GPIO */

/**
 * of_gpio_named_count() - Count GPIOs for a device
 * @np:		device node to count GPIOs for
 * @propname:	property name containing gpio specifier(s)
 *
 * The function returns the count of GPIOs specified for a node.
 * Note that the empty GPIO specifiers count too. Returns either
 *   Number of gpios defined in property,
 *   -EINVAL for an incorrectly formed gpios property, or
 *   -ENOENT for a missing gpios property
 *
 * Example:
 * gpios = <0
 *          &gpio1 1 2
 *          0
 *          &gpio2 3 4>;
 *
 * The above example defines four GPIOs, two of which are not specified.
 * This function will return '4'
 */
static inline int of_gpio_named_count(const struct device_node *np,
				      const char *propname)
{
	return of_count_phandle_with_args(np, propname, "#gpio-cells");
}

/**
 * of_gpio_count() - Count GPIOs for a device
 * @np:		device node to count GPIOs for
 *
 * Same as of_gpio_named_count, but hard coded to use the 'gpios' property
 */
static inline int of_gpio_count(const struct device_node *np)
{
	return of_gpio_named_count(np, "gpios");
}

#endif /* __LINUX_OF_GPIO_H */
