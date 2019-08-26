/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __TIMER_IXP4XX_H
#define __TIMER_IXP4XX_H

#include <linux/ioport.h>

void __init ixp4xx_timer_setup(resource_size_t timerbase,
			       int timer_irq,
			       unsigned int timer_freq);

#ifdef CONFIG_IXP4XX_TIMER
void ixp4xx_wdt_set(int heartbeat);
int ixp4xx_boot_status(void);
#else
static inline void ixp4xx_wdt_set(int heartbeat)
{
}
static inline int ixp4xx_boot_status(void)
{
	return -ENXIO;
}
#endif

#endif
