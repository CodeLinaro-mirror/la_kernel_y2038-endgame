/*
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - Memory map definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_MAP_H
#define __ASM_ARCH_MAP_H __FILE__

/* Fit all our registers in at 0xF6000000 upwards, trying to use as
 * little of the VA space as possible so vmalloc and friends have a
 * better chance of getting memory.
 *
 * we try to ensure stuff like the IRQ registers are available for
 * an single MOVS instruction (ie, only 8 bits of set data)
 */

#define S3C_ADDR_BASE	0xF6000000

#ifndef __ASSEMBLY__
#define S3C_ADDR(x)	((void __iomem __force *)S3C_ADDR_BASE + (x))
#else
#define S3C_ADDR(x)	(S3C_ADDR_BASE + (x))
#endif

#define S3C_VA_IRQ	S3C_ADDR(0x00000000)	/* irq controller(s) */
#define S3C_VA_SYS	S3C_ADDR(0x00100000)	/* system control */
#define S3C_VA_MEM	S3C_ADDR(0x00200000)	/* memory control */
#define S3C_VA_TIMER	S3C_ADDR(0x00300000)	/* timer block */
#define S3C_VA_WATCHDOG	S3C_ADDR(0x00400000)	/* watchdog */
#define S3C_VA_UART	S3C_ADDR(0x01000000)	/* UART */

/* This is used for the CPU specific mappings that may be needed, so that
 * they do not need to directly used S3C_ADDR() and thus make it easier to
 * modify the space for mapping.
 */
#define S3C_ADDR_CPU(x)	S3C_ADDR(0x00500000 + (x))

/*
 * EXYNOS4 UART offset is 0x10000 but the older S5P SoCs are 0x400.
 * So need to define it, and here is to avoid redefinition warning.
 */
#define S3C_UART_OFFSET			(0x10000)

#define S5P_VA_CHIPID		S3C_ADDR(0x02000000)
#define S5P_VA_CMU		S3C_ADDR(0x02100000)
#define S5P_VA_GPIO		S3C_ADDR(0x02200000)
#define S5P_VA_GPIO1		S5P_VA_GPIO
#define S5P_VA_GPIO2		S3C_ADDR(0x02240000)
#define S5P_VA_GPIO3		S3C_ADDR(0x02280000)

#define S5P_VA_SYSRAM		S3C_ADDR(0x02400000)
#define S5P_VA_SYSRAM_NS	S3C_ADDR(0x02410000)
#define S5P_VA_DMC0		S3C_ADDR(0x02440000)
#define S5P_VA_DMC1		S3C_ADDR(0x02480000)
#define S5P_VA_SROMC		S3C_ADDR(0x024C0000)

#define S5P_VA_SYSTIMER		S3C_ADDR(0x02500000)
#define S5P_VA_L2CC		S3C_ADDR(0x02600000)

#define S5P_VA_COMBINER_BASE	S3C_ADDR(0x02700000)
#define S5P_VA_COMBINER(x)	(S5P_VA_COMBINER_BASE + ((x) >> 2) * 0x10)

#define S5P_VA_COREPERI_BASE	S3C_ADDR(0x02800000)
#define S5P_VA_COREPERI(x)	(S5P_VA_COREPERI_BASE + (x))
#define S5P_VA_SCU		S5P_VA_COREPERI(0x0)
#define S5P_VA_TWD		S5P_VA_COREPERI(0x600)

#define S5P_VA_GIC_CPU		S3C_ADDR(0x02810000)
#define S5P_VA_GIC_DIST		S3C_ADDR(0x02820000)

#define VA_VIC(x)		(S3C_VA_IRQ + ((x) * 0x10000))
#define VA_VIC0			VA_VIC(0)
#define VA_VIC1			VA_VIC(1)
#define VA_VIC2			VA_VIC(2)
#define VA_VIC3			VA_VIC(3)

#define S5P_VA_UART(x)		(S3C_VA_UART + ((x) * S3C_UART_OFFSET))
#define S5P_VA_UART0		S5P_VA_UART(0)
#define S5P_VA_UART1		S5P_VA_UART(1)
#define S5P_VA_UART2		S5P_VA_UART(2)
#define S5P_VA_UART3		S5P_VA_UART(3)

#ifndef S3C_UART_OFFSET
#define S3C_UART_OFFSET		(0x400)
#endif

#define S3C64XX_VA_USB_HSPHY	S3C_ADDR_CPU(0x00200000)

#define S3C_VA_USB_HSPHY	S3C64XX_VA_USB_HSPHY

#define EXYNOS_PA_CHIPID		0x10000000

#define EXYNOS4_PA_SYSCON		0x10010000
#define EXYNOS5_PA_SYSCON		0x10050100

#define EXYNOS4_PA_CMU			0x10030000
#define EXYNOS5_PA_CMU			0x10010000

#define EXYNOS4_PA_SYSTIMER		0x10050000

#define EXYNOS4_PA_WATCHDOG		0x10060000
#define EXYNOS5_PA_WATCHDOG		0x101D0000

#define EXYNOS4_PA_DMC0			0x10400000
#define EXYNOS4_PA_DMC1			0x10410000

#define EXYNOS4_PA_COMBINER		0x10440000
#define EXYNOS5_PA_COMBINER		0x10440000

#define EXYNOS4_PA_GIC_CPU		0x10480000
#define EXYNOS4_PA_GIC_DIST		0x10490000
#define EXYNOS5_PA_GIC_CPU		0x10482000
#define EXYNOS5_PA_GIC_DIST		0x10481000

#define EXYNOS4_PA_COREPERI		0x10500000
#define EXYNOS4_PA_L2CC			0x10502000

#define EXYNOS4_PA_SROMC		0x12570000
#define EXYNOS5_PA_SROMC		0x12250000

#define EXYNOS4_PA_HSPHY		0x125B0000

#define EXYNOS4_PA_UART			0x13800000
#define EXYNOS5_PA_UART			0x12C00000

#define EXYNOS4_PA_TIMER		0x139D0000
#define EXYNOS5_PA_TIMER		0x12DD0000

/* Compatibility UART */

#define EXYNOS5440_PA_UART0		0x000B0000

#define S3C_VA_UARTx(x)			(S3C_VA_UART + ((x) * S3C_UART_OFFSET))

#endif /* __ASM_ARCH_MAP_H */
