/*
 *  Chip-specific setup code for the SAMA5D3 family
 *
 *  Copyright (C) 2013 Atmel,
 *                2013 Ludovic Desroches <ludovic.desroches@atmel.com>
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/init.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/at91_dbgu.h>
#include <mach/cpu.h>
#include <mach/hardware.h>
#include <mach/sama5d3.h>

#include "generic.h"
#include "sam9_smc.h"

static struct map_desc at91_io_desc __initdata __maybe_unused = {
	.virtual	= (unsigned long)AT91_VA_BASE_SYS,
	.pfn		= __phys_to_pfn(AT91_BASE_SYS),
	.length		= SZ_16K,
	.type		= MT_DEVICE,
};

static void __init sama5d3_map_io(void)
{
	void __iomem *dbgu_base = AT91_IO_P2V(AT91_BASE_DBGU1);
	const char *name = "unknown";

	iotable_init(&at91_io_desc, 1);
	at91_init_sram(0, SAMA5D3_SRAM_BASE, SAMA5D3_SRAM_SIZE);

	at91_soc_initdata.type = AT91_SOC_SAMA5D3;
	at91_soc_initdata.exid = __raw_readl(dbgu_base + AT91_DBGU_EXID);
	at91_soc_initdata.cidr = __raw_readl(dbgu_base + AT91_DBGU_CIDR);

	switch (at91_soc_initdata.exid) {
	case ARCH_EXID_SAMA5D31:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D31;
		name = "sama5d31";
		break;
	case ARCH_EXID_SAMA5D33:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D33;
		name = "sama5d33";
		break;
	case ARCH_EXID_SAMA5D34:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D34;
		name = "sama5d34";
		break;
	case ARCH_EXID_SAMA5D35:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D35;
		name = "sama5d35";
		break;
	case ARCH_EXID_SAMA5D36:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D36;
		name = "sama5d36";
		break;
	}
	pr_info("SAMA5: Detected soc subtype: %s\n", name);
}

static void __init sama5d3_init_early(void)
{
	at91_dt_ramc();
	at91_sysirq_mask_rtc(SAMA5D3_BASE_RTC);
}

static const char *sama5d3_board_compat[] __initconst = {
	"atmel,sama5",
	NULL
};

DT_MACHINE_START(sama5_dt, "Atmel SAMA5 (Device Tree)")
	/* Maintainer: Atmel */
	.map_io		= sama5d3_map_io,
	.init_early	= sama5d3_init_early,
	.dt_compat	= sama5d3_board_compat,
MACHINE_END
