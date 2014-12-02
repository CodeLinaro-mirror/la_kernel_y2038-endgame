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

struct at91_init_soc {
	void (*map_io)(void);
	void (*init)(void);
};

static struct at91_init_soc __initdata at91_boot_soc;

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

static void __init at91sam9261_map_io(void)
{
	if (cpu_is_at91sam9g10())
		at91_init_sram(0, AT91SAM9G10_SRAM_BASE, AT91SAM9G10_SRAM_SIZE);
	else
		at91_init_sram(0, AT91SAM9261_SRAM_BASE, AT91SAM9261_SRAM_SIZE);
}

static void __init at91sam9263_map_io(void)
{
	at91_init_sram(0, AT91SAM9263_SRAM0_BASE, AT91SAM9263_SRAM0_SIZE);
	at91_init_sram(1, AT91SAM9263_SRAM1_BASE, AT91SAM9263_SRAM1_SIZE);
}

static void __init at91sam9g45_map_io(void)
{
	at91_init_sram(0, AT91SAM9G45_SRAM_BASE, AT91SAM9G45_SRAM_SIZE);
}

static void __init at91sam9n12_map_io(void)
{
	at91_init_sram(0, AT91SAM9N12_SRAM_BASE, AT91SAM9N12_SRAM_SIZE);
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

static void __init at91sam9x5_map_io(void)
{
	at91_init_sram(0, AT91SAM9X5_SRAM_BASE, AT91SAM9X5_SRAM_SIZE);
}

static void __init at91sam9260_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9260_BASE_RTT);
}

static void __init at91sam9261_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9261_BASE_RTT);
}

static void __init at91sam9263_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtt(AT91SAM9263_BASE_RTT0);
	at91_sysirq_mask_rtt(AT91SAM9263_BASE_RTT1);
}

static void __init at91sam9g45_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtc(AT91SAM9G45_BASE_RTC);
	at91_sysirq_mask_rtt(AT91SAM9G45_BASE_RTT);
}


static void __init at91sam9n12_initialize(void)
{
	at91_sysirq_mask_rtc(AT91SAM9N12_BASE_RTC);
}

static void __init at91sam9rl_initialize(void)
{
	arm_pm_idle = at91sam9_idle;

	at91_sysirq_mask_rtc(AT91SAM9RL_BASE_RTC);
	at91_sysirq_mask_rtt(AT91SAM9RL_BASE_RTT);
}

static void __init at91sam9x5_initialize(void)
{
	at91_sysirq_mask_rtc(AT91SAM9X5_BASE_RTC);
}

#define AT91_SOC_START(_name)				\
static struct at91_init_soc __initdata _name##_soc = {	\

#define AT91_SOC_END					\
};

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

static struct map_desc at91_io_desc __initdata __maybe_unused = {
	.virtual	= (unsigned long)AT91_VA_BASE_SYS,
	.pfn		= __phys_to_pfn(AT91_BASE_SYS),
	.length		= SZ_16K,
	.type		= MT_DEVICE,
};

static void __init soc_detect(u32 dbgu_base)
{
	u32 cidr, socid;

	cidr = __raw_readl(AT91_IO_P2V(dbgu_base) + AT91_DBGU_CIDR);
	socid = cidr & ~AT91_CIDR_VERSION;

	switch (socid) {
	case ARCH_ID_AT91SAM9260:
		at91_soc_initdata.type = AT91_SOC_SAM9260;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9260_soc;
		break;

	case ARCH_ID_AT91SAM9261:
		at91_soc_initdata.type = AT91_SOC_SAM9261;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9261_soc;
		break;

	case ARCH_ID_AT91SAM9263:
		at91_soc_initdata.type = AT91_SOC_SAM9263;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9263_soc;
		break;

	case ARCH_ID_AT91SAM9G20:
		at91_soc_initdata.type = AT91_SOC_SAM9G20;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9260_soc;
		break;

	case ARCH_ID_AT91SAM9G45:
		at91_soc_initdata.type = AT91_SOC_SAM9G45;
		if (cidr == ARCH_ID_AT91SAM9G45ES)
			at91_soc_initdata.subtype = AT91_SOC_SAM9G45ES;
		at91_boot_soc = at91sam9g45_soc;
		break;

	case ARCH_ID_AT91SAM9RL64:
		at91_soc_initdata.type = AT91_SOC_SAM9RL;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9rl_soc;
		break;

	case ARCH_ID_AT91SAM9X5:
		at91_soc_initdata.type = AT91_SOC_SAM9X5;
		at91_boot_soc = at91sam9x5_soc;
		break;

	case ARCH_ID_AT91SAM9N12:
		at91_soc_initdata.type = AT91_SOC_SAM9N12;
		at91_boot_soc = at91sam9n12_soc;
		break;
	}

	/* at91sam9g10 */
	if ((socid & ~AT91_CIDR_EXT) == ARCH_ID_AT91SAM9G10) {
		at91_soc_initdata.type = AT91_SOC_SAM9G10;
		at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_NONE;
		at91_boot_soc = at91sam9261_soc;
	}
	/* at91sam9xe */
	else if ((cidr & AT91_CIDR_ARCH) == ARCH_FAMILY_AT91SAM9XE) {
		at91_soc_initdata.type = AT91_SOC_SAM9260;
		at91_soc_initdata.subtype = AT91_SOC_SAM9XE;
		at91_boot_soc = at91sam9260_soc;
	}

	if (!at91_soc_is_detected())
		return;

	at91_soc_initdata.cidr = cidr;

	/* sub version of soc */
	if (!at91_soc_initdata.exid)
		at91_soc_initdata.exid = __raw_readl(AT91_IO_P2V(dbgu_base) + AT91_DBGU_EXID);

	if (at91_soc_initdata.type == AT91_SOC_SAM9G45) {
		switch (at91_soc_initdata.exid) {
		case ARCH_EXID_AT91SAM9M10:
			at91_soc_initdata.subtype = AT91_SOC_SAM9M10;
			break;
		case ARCH_EXID_AT91SAM9G46:
			at91_soc_initdata.subtype = AT91_SOC_SAM9G46;
			break;
		case ARCH_EXID_AT91SAM9M11:
			at91_soc_initdata.subtype = AT91_SOC_SAM9M11;
			break;
		}
	}

	if (at91_soc_initdata.type == AT91_SOC_SAM9X5) {
		switch (at91_soc_initdata.exid) {
		case ARCH_EXID_AT91SAM9G15:
			at91_soc_initdata.subtype = AT91_SOC_SAM9G15;
			break;
		case ARCH_EXID_AT91SAM9G35:
			at91_soc_initdata.subtype = AT91_SOC_SAM9G35;
			break;
		case ARCH_EXID_AT91SAM9X35:
			at91_soc_initdata.subtype = AT91_SOC_SAM9X35;
			break;
		case ARCH_EXID_AT91SAM9G25:
			at91_soc_initdata.subtype = AT91_SOC_SAM9G25;
			break;
		case ARCH_EXID_AT91SAM9X25:
			at91_soc_initdata.subtype = AT91_SOC_SAM9X25;
			break;
		}
	}
}

static const char *soc_name[] = {
	[AT91_SOC_SAM9260]	= "at91sam9260",
	[AT91_SOC_SAM9261]	= "at91sam9261",
	[AT91_SOC_SAM9263]	= "at91sam9263",
	[AT91_SOC_SAM9G10]	= "at91sam9g10",
	[AT91_SOC_SAM9G20]	= "at91sam9g20",
	[AT91_SOC_SAM9G45]	= "at91sam9g45",
	[AT91_SOC_SAM9RL]	= "at91sam9rl",
	[AT91_SOC_SAM9X5]	= "at91sam9x5",
	[AT91_SOC_SAM9N12]	= "at91sam9n12",
	[AT91_SOC_UNKNOWN]	= "Unknown",
};

static const char *at91_get_soc_type(struct at91_socinfo *c)
{
	return soc_name[c->type];
}

static const char *soc_subtype_name[] = {
	[AT91_SOC_SAM9XE]	= "at91sam9xe",
	[AT91_SOC_SAM9G45ES]	= "at91sam9g45es",
	[AT91_SOC_SAM9M10]	= "at91sam9m10",
	[AT91_SOC_SAM9G46]	= "at91sam9g46",
	[AT91_SOC_SAM9M11]	= "at91sam9m11",
	[AT91_SOC_SAM9G15]	= "at91sam9g15",
	[AT91_SOC_SAM9G35]	= "at91sam9g35",
	[AT91_SOC_SAM9X35]	= "at91sam9x35",
	[AT91_SOC_SAM9G25]	= "at91sam9g25",
	[AT91_SOC_SAM9X25]	= "at91sam9x25",
	[AT91_SOC_SUBTYPE_NONE]	= "None",
	[AT91_SOC_SUBTYPE_UNKNOWN] = "Unknown",
};

static const char *at91_get_soc_subtype(struct at91_socinfo *c)
{
	return soc_subtype_name[c->subtype];
}

static void __init at91_map_io(void)
{
	/* Map peripherals */
	iotable_init(&at91_io_desc, 1);

	at91_soc_initdata.type = AT91_SOC_UNKNOWN;
	at91_soc_initdata.subtype = AT91_SOC_SUBTYPE_UNKNOWN;

	soc_detect(AT91_BASE_DBGU0);
	if (!at91_soc_is_detected())
		soc_detect(AT91_BASE_DBGU1);

	if (!at91_soc_is_detected())
		panic(pr_fmt("Impossible to detect the SOC type"));

	pr_info("Detected soc type: %s\n",
		at91_get_soc_type(&at91_soc_initdata));
	if (at91_soc_initdata.subtype != AT91_SOC_SUBTYPE_NONE)
		pr_info("Detected soc subtype: %s\n",
			at91_get_soc_subtype(&at91_soc_initdata));

	if (at91_boot_soc.map_io)
		at91_boot_soc.map_io();
}

static void __init at91_dt_initialize(void)
{
	at91_dt_ramc();

	if (at91_boot_soc.init)
		at91_boot_soc.init();
}

DT_MACHINE_START(at91sam_dt, "Atmel AT91SAM (Device Tree)")
	/* Maintainer: Atmel */
	.map_io		= at91_map_io,
	.init_early	= at91_dt_initialize,
	.dt_compat	= at91_dt_board_compat,
MACHINE_END
