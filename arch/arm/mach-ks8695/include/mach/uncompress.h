/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * arch/arm/mach-ks8695/include/mach/uncompress.h
 *
 * Copyright (C) 2006 Ben Dooks <ben@simtec.co.uk>
 * Copyright (C) 2006 Simtec Electronics
 *
 * KS8695 - Kernel uncompressor
 */

#ifndef __ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#include <linux/io.h>

#define KS8695_UART_OFFSET     (0xF0000 + 0xE000)
#define KS8695_UART_PA          (KS8695_IO_PA + KS8695_UART_OFFSET)
#define KS8695_URRB     (0x00)          /* Receive Buffer Register */
#define KS8695_URTH     (0x04)          /* Transmit Holding Register */
#define KS8695_URFC     (0x08)          /* FIFO Control Register */
#define KS8695_URLC     (0x0C)          /* Line Control Register */
#define KS8695_URMC     (0x10)          /* Modem Control Register */
#define KS8695_URLS     (0x14)          /* Line Status Register */
#define KS8695_URMS     (0x18)          /* Modem Status Register */
#define KS8695_URBD     (0x1C)          /* Baud Rate Divisor Register */
#define KS8695_USR      (0x20)          /* Status Register */

#define URLS_URTE       (1 << 6)
#define URLS_URTHRE     (1 << 5)

static inline void putc(char c)
{
	while (!(__raw_readl((void __iomem*)KS8695_UART_PA + KS8695_URLS) & URLS_URTHRE))
		barrier();

	__raw_writel(c, (void __iomem*)KS8695_UART_PA + KS8695_URTH);
}

static inline void flush(void)
{
	while (!(__raw_readl((void __iomem*)KS8695_UART_PA + KS8695_URLS) & URLS_URTE))
		barrier();
}

#define arch_decomp_setup()

#endif
