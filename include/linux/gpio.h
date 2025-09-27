/* SPDX-License-Identifier: GPL-2.0 */
/*
 * NOTE: This header *must not* be included.
 *
 * If you're implementing a GPIO driver, only include <linux/gpio/driver.h>
 * If you're implementing a GPIO consumer, only include <linux/gpio/consumer.h>
 * If you're using the legacy interfaces, include <linux/gpio/legacy.h>
 */
#ifndef __LINUX_GPIO_H
#define __LINUX_GPIO_H

#include <linux/types.h>
#ifdef CONFIG_GPIOLIB
#include <linux/gpio/consumer.h>
#endif

#endif /* __LINUX_GPIO_H */
