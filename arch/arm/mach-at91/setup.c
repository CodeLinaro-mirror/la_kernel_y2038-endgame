/*
 * Copyright (C) 2007 Atmel Corporation.
 * Copyright (C) 2011 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * Under GPLv2
 */

#define pr_fmt(fmt)	"AT91: " fmt

#include <linux/module.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/pm.h>
#include <linux/of_address.h>
#include <linux/pinctrl/machine.h>
#include <linux/clk/at91_pmc.h>

#include <asm/system_misc.h>
#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <mach/cpu.h>
#include <mach/at91_dbgu.h>

#include "generic.h"
#include "pm.h"

struct at91_socinfo at91_soc_initdata;
EXPORT_SYMBOL(at91_soc_initdata);

void __iomem *at91_ramc_base[2];
EXPORT_SYMBOL_GPL(at91_ramc_base);

static struct map_desc sram_desc[2] __initdata;

void __init at91_init_sram(int bank, unsigned long base, unsigned int length)
{
	struct map_desc *desc = &sram_desc[bank];

	desc->virtual = (unsigned long)AT91_IO_VIRT_BASE - length;
	if (bank > 0)
		desc->virtual -= sram_desc[bank - 1].length;

	desc->pfn = __phys_to_pfn(base);
	desc->length = length;
	desc->type = MT_MEMORY_RWX_NONCACHED;

	pr_info("sram at 0x%lx of 0x%x mapped at 0x%lx\n",
		base, length, desc->virtual);

	iotable_init(desc, 1);
}

void __iomem *at91_matrix_base;
EXPORT_SYMBOL_GPL(at91_matrix_base);

void __init at91_ioremap_matrix(u32 base_addr)
{
	at91_matrix_base = ioremap(base_addr, 512);
	if (!at91_matrix_base)
		panic(pr_fmt("Impossible to ioremap at91_matrix_base\n"));
}

static struct of_device_id ramc_ids[] = {
	{ .compatible = "atmel,at91rm9200-sdramc", .data = at91rm9200_standby },
	{ .compatible = "atmel,at91sam9260-sdramc", .data = at91sam9_sdram_standby },
	{ .compatible = "atmel,at91sam9g45-ddramc", .data = at91_ddr_standby },
	{ .compatible = "atmel,sama5d3-ddramc", .data = at91_ddr_standby },
	{ /*sentinel*/ }
};

void at91_dt_ramc(void)
{
	struct device_node *np;
	const struct of_device_id *of_id;
	int idx = 0;
	const void *standby = NULL;

	for_each_matching_node_and_match(np, ramc_ids, &of_id) {
		at91_ramc_base[idx] = of_iomap(np, 0);
		if (!at91_ramc_base[idx])
			panic(pr_fmt("unable to map ramc[%d] cpu registers\n"), idx);

		if (!standby)
			standby = of_id->data;

		idx++;
	}

	if (!idx)
		panic(pr_fmt("unable to find compatible ram controller node in dtb\n"));

	if (!standby) {
		pr_warn("ramc no standby function available\n");
		return;
	}

	at91_pm_set_standby(standby);
}
