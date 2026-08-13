/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_DMA_H
#define __ASM_ARM_DMA_H

/*
 * This is the maximum virtual address which can be DMA'd from.
 */
#ifndef CONFIG_ZONE_DMA
#define MAX_DMA_ADDRESS	0xffffffffUL
#else
#define MAX_DMA_ADDRESS	({ \
	extern phys_addr_t arm_dma_zone_size; \
	arm_dma_zone_size && arm_dma_zone_size < (0x100000000ULL - PAGE_OFFSET) ? \
		(PAGE_OFFSET + arm_dma_zone_size) : 0xffffffffUL; })

extern phys_addr_t arm_dma_limit;
#define ARCH_LOW_ADDRESS_LIMIT arm_dma_limit
#endif

#endif /* __ASM_ARM_DMA_H */
