/*
 *  Setup code for AT91RM9200
 *
 *  Copyright (C) 2005 SAN People
 *  Copyright (C) 2011 Atmel,
 *                2011 Nicolas Ferre <nicolas.ferre@atmel.com>
 *                2012 Joachim Eastwood <manabian@gmail.com>
 *
 * Licensed under GPLv2 or later.
 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/system_misc.h>

#include <mach/at91_st.h>
#include <mach/cpu.h>
#include <mach/hardware.h>

#include "generic.h"

static void at91rm9200_idle(void)
{
	/*
	 * Disable the processor clock.  The processor will be automatically
	 * re-enabled by an interrupt or by a reset.
	 */
	at91_pmc_write(AT91_PMC_SCDR, AT91_PMC_PCK);
}

static void at91rm9200_restart(enum reboot_mode reboot_mode, const char *cmd)
{
	/*
	 * Perform a hardware reset with the use of the Watchdog timer.
	 */
	at91_st_write(AT91_ST_WDMR, AT91_ST_RSTEN | AT91_ST_EXTEN | 1);
	at91_st_write(AT91_ST_CR, AT91_ST_WDRST);
}

static struct map_desc at91_io_desc __initdata __maybe_unused = {
	.virtual	= (unsigned long)AT91_VA_BASE_SYS,
	.pfn		= __phys_to_pfn(AT91_BASE_SYS),
	.length		= SZ_16K,
	.type		= MT_DEVICE,
};

static void __init at91rm9200_map_io(void)
{
	at91_soc_initdata.type = AT91_SOC_RM9200;
	at91_soc_initdata.subtype = AT91_SOC_RM9200_BGA;

	/* Map peripherals */
	iotable_init(&at91_io_desc, 1);
	at91_init_sram(0, AT91RM9200_SRAM_BASE, AT91RM9200_SRAM_SIZE);
}

static void __init at91rm9200_initialize(void)
{
	at91_dt_ramc();
	arm_pm_idle = at91rm9200_idle;
	arm_pm_restart = at91rm9200_restart;
}

static void __init at91rm9200_dt_timer_init(void)
{
	of_clk_init(NULL);
	at91rm9200_timer_init();
}

static const char *at91rm9200_dt_board_compat[] __initdata = {
	"atmel,at91rm9200",
	NULL
};

DT_MACHINE_START(at91rm9200_dt, "Atmel AT91RM9200 (Device Tree)")
	.init_time      = at91rm9200_dt_timer_init,
	.map_io		= at91rm9200_map_io,
	.init_early	= at91rm9200_initialize,
	.dt_compat	= at91rm9200_dt_board_compat,
MACHINE_END
