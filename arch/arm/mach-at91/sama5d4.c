/*
 *  Chip-specific setup code for the SAMA5D4 family
 *
 *  Copyright (C) 2013 Atmel Corporation,
 *                     Nicolas Ferre <nicolas.ferre@atmel.com>
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk/at91_pmc.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/micrel_phy.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/phy.h>
#include <linux/clk-provider.h>

#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/at91_dbgu.h>
#include <mach/cpu.h>
#include <mach/hardware.h>
#include <mach/sama5d4.h>

#include "generic.h"
#include "sam9_smc.h"

static struct map_desc at91_io_desc[] __initdata = {
	{
	.virtual	= (unsigned long)AT91_ALT_VA_BASE_SYS,
	.pfn		= __phys_to_pfn(AT91_ALT_BASE_SYS),
	.length		= 24 * SZ_1K,
	.type		= MT_DEVICE,
	},
	{
	.virtual        = (unsigned long)AT91_ALT_IO_P2V(SAMA5D4_BASE_MPDDRC),
	.pfn            = __phys_to_pfn(SAMA5D4_BASE_MPDDRC),
	.length         = SZ_512,
	.type           = MT_DEVICE,
	},
	{
	.virtual        = (unsigned long)AT91_ALT_IO_P2V(SAMA5D4_BASE_PMC),
	.pfn            = __phys_to_pfn(SAMA5D4_BASE_PMC),
	.length         = SZ_512,
	.type           = MT_DEVICE,
	},
	{ /* On sama5d4, we use USART3 as serial console */
	.virtual        = (unsigned long)AT91_ALT_IO_P2V(SAMA5D4_BASE_USART3),
	.pfn            = __phys_to_pfn(SAMA5D4_BASE_USART3),
	.length         = SZ_256,
	.type           = MT_DEVICE,
	},
	{ /* A bunch of peripheral with fine grained IO space */
	.virtual        = (unsigned long)AT91_ALT_IO_P2V(SAMA5D4_BASE_SYS2),
	.pfn            = __phys_to_pfn(SAMA5D4_BASE_SYS2),
	.length         = SZ_2K,
	.type           = MT_DEVICE,
	},
};

static void __init sama5d4_map_io(void)
{
	const char *name = "unknown";
	void __iomem *dbgu_base = AT91_ALT_IO_P2V(AT91_BASE_DBGU2);

	iotable_init(at91_io_desc, ARRAY_SIZE(at91_io_desc));
	at91_init_sram(0, SAMA5D4_NS_SRAM_BASE, SAMA5D4_NS_SRAM_SIZE);

	at91_soc_initdata.type = AT91_SOC_SAMA5D4;
	at91_soc_initdata.cidr = __raw_readl(dbgu_base + AT91_DBGU_CIDR);
	at91_soc_initdata.exid = __raw_readl(dbgu_base + AT91_DBGU_EXID);

	switch (at91_soc_initdata.exid) {
	case ARCH_EXID_SAMA5D41:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D41;
		name = "sama5d41";
		break;
	case ARCH_EXID_SAMA5D42:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D42;
		name = "sama5d42";
		break;
	case ARCH_EXID_SAMA5D43:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D43;
		name = "sama5d43";
		break;
	case ARCH_EXID_SAMA5D44:
		at91_soc_initdata.subtype = AT91_SOC_SAMA5D44;
		name = "sama5d44";
		break;
	}
	pr_info("SAMA5: Detected soc subtype: %s\n", name);
}

static void __init sama5d4_init_early(void)
{
	at91_dt_ramc();
}

static const char *sama5_alt_dt_board_compat[] __initconst = {
	"atmel,sama5d4",
	NULL
};

DT_MACHINE_START(sama5_alt_dt, "Atmel SAMA5 (Device Tree)")
	/* Maintainer: Atmel */
	.map_io		= sama5d4_map_io,
	.init_early	= sama5d4_init_early,
	.dt_compat	= sama5_alt_dt_board_compat,
	.l2c_aux_mask	= ~0UL,
MACHINE_END
