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

#include <mach/sama5d3.h>
#include <mach/cpu.h>

#include "soc.h"
#include "sam9_smc.h"
#include "generic.h"


/* --------------------------------------------------------------------
 *  AT91SAM9x5 processor initialization
 * -------------------------------------------------------------------- */

static void __init sama5d3_map_io(void)
{
	at91_init_sram(0, SAMA5D3_SRAM_BASE, SAMA5D3_SRAM_SIZE);
}

static void __init sama5d3_initialize(void)
{
	at91_sysirq_mask_rtc(SAMA5D3_BASE_RTC);
}

AT91_SOC_START(sama5d3)
	.map_io = sama5d3_map_io,
	.init = sama5d3_initialize,
AT91_SOC_END

static const char *sama5_dt_board_compat[] __initconst = {
	"atmel,sama5",
	NULL
};

DT_MACHINE_START(sama5_dt, "Atmel SAMA5 (Device Tree)")
	/* Maintainer: Atmel */
	.map_io		= at91_map_io,
	.init_early	= at91_dt_initialize,
	.dt_compat	= sama5_dt_board_compat,
MACHINE_END
