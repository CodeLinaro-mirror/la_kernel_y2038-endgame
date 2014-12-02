/*
 * Atmel AT91SAM9 support
 *
 *  Copyright (C) 2005-2006 SAN People
 *  Copyright (C) 2007-2012 Atmel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/clk-provider.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/types.h>

#include <mach/at91_dbgu.h>
#include <mach/cpu.h>
#include <mach/hardware.h>

#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/system_misc.h>

#include "generic.h"
#include "soc.h"

static void __init at91sam9xe_map_io(void)
{
	unsigned long sram_size;

	switch (at91_soc_initdata.cidr & AT91_CIDR_SRAMSIZ) {
		case AT91_CIDR_SRAMSIZ_32K:
			sram_size = 2 * SZ_16K;
			break;
		case AT91_CIDR_SRAMSIZ_16K:
		default:
			sram_size = SZ_16K;
	}

	at91_init_sram(0, AT91SAM9XE_SRAM_BASE, sram_size);
}

static void __init at91sam9260_map_io(void)
{
	if (cpu_is_at91sam9xe())
		at91sam9xe_map_io();
	else if (cpu_is_at91sam9g20())
		at91_init_sram(0, AT91SAM9G20_SRAM_BASE, AT91SAM9G20_SRAM_SIZE);
	else
		at91_init_sram(0, AT91SAM9260_SRAM_BASE, AT91SAM9260_SRAM_SIZE);
}

static void __init at91sam9260_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9260_BASE_RTT);
}

static void __init at91sam9261_map_io(void)
{
	if (cpu_is_at91sam9g10())
		at91_init_sram(0, AT91SAM9G10_SRAM_BASE, AT91SAM9G10_SRAM_SIZE);
	else
		at91_init_sram(0, AT91SAM9261_SRAM_BASE, AT91SAM9261_SRAM_SIZE);
}

static void __init at91sam9261_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9261_BASE_RTT);
}

static void __init at91sam9263_map_io(void)
{
	at91_init_sram(0, AT91SAM9263_SRAM0_BASE, AT91SAM9263_SRAM0_SIZE);
	at91_init_sram(1, AT91SAM9263_SRAM1_BASE, AT91SAM9263_SRAM1_SIZE);
}

static void __init at91sam9263_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9263_BASE_RTT0);
	at91_sysirq_mask_rtt(AT91SAM9263_BASE_RTT1);
}

static void __init at91sam9g45_map_io(void)
{
	at91_init_sram(0, AT91SAM9G45_SRAM_BASE, AT91SAM9G45_SRAM_SIZE);
}

static void __init at91sam9g45_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtc(AT91SAM9G45_BASE_RTC);
	at91_sysirq_mask_rtt(AT91SAM9G45_BASE_RTT);
}

static void __init at91sam9n12_map_io(void)
{
	at91_init_sram(0, AT91SAM9N12_SRAM_BASE, AT91SAM9N12_SRAM_SIZE);
}

static void __init at91sam9n12_initialize(void)
{
	at91_sysirq_mask_rtc(AT91SAM9N12_BASE_RTC);
}

static void __init at91sam9rl_map_io(void)
{
	unsigned long sram_size;

	switch (at91_soc_initdata.cidr & AT91_CIDR_SRAMSIZ) {
		case AT91_CIDR_SRAMSIZ_32K:
			sram_size = 2 * SZ_16K;
			break;
		case AT91_CIDR_SRAMSIZ_16K:
		default:
			sram_size = SZ_16K;
	}

	at91_init_sram(0, AT91SAM9RL_SRAM_BASE, sram_size);
}

static void __init at91sam9rl_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtc(AT91SAM9RL_BASE_RTC);
	at91_sysirq_mask_rtt(AT91SAM9RL_BASE_RTT);
}

static void __init at91sam9x5_map_io(void)
{
	at91_init_sram(0, AT91SAM9X5_SRAM_BASE, AT91SAM9X5_SRAM_SIZE);
}

static void __init at91sam9x5_initialize(void)
{
	at91_sysirq_mask_rtc(AT91SAM9X5_BASE_RTC);
}

AT91_SOC_START(at91sam9260)
	.map_io = at91sam9260_map_io,
	.init = at91sam9260_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9261)
	.map_io = at91sam9261_map_io,
	.init = at91sam9261_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9263)
	.map_io = at91sam9263_map_io,
	.init = at91sam9263_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9g45)
	.map_io = at91sam9g45_map_io,
	.init = at91sam9g45_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9n12)
	.map_io = at91sam9n12_map_io,
	.init = at91sam9n12_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9rl)
	.map_io = at91sam9rl_map_io,
	.init = at91sam9rl_initialize,
AT91_SOC_END

AT91_SOC_START(at91sam9x5)
	.map_io = at91sam9x5_map_io,
	.init = at91sam9x5_initialize,
AT91_SOC_END

static const char *at91_dt_board_compat[] __initdata = {
	"atmel,at91sam9",
	NULL
};

DT_MACHINE_START(at91sam_dt, "Atmel AT91SAM (Device Tree)")
	/* Maintainer: Atmel */
	.map_io		= at91_map_io,
	.init_early	= at91_dt_initialize,
	.dt_compat	= at91_dt_board_compat,
MACHINE_END
