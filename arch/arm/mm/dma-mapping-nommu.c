// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Based on linux/arch/arm/mm/dma-mapping.c
 *
 *  Copyright (C) 2000-2004 Russell King
 */

#include <linux/dma-map-ops.h>
#include <asm/cachetype.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#include <asm/cp15.h>

#include "dma.h"

void arch_sync_dma_for_device(phys_addr_t paddr, size_t size,
		enum dma_data_direction dir)
{
	dmac_map_area(__va(paddr), size, dir);

	if (dir == DMA_FROM_DEVICE)
		outer_inv_range(paddr, paddr + size);
	else
		outer_clean_range(paddr, paddr + size);
}

void arch_sync_dma_for_cpu(phys_addr_t paddr, size_t size,
		enum dma_data_direction dir)
{
	if (dir != DMA_TO_DEVICE) {
		outer_inv_range(paddr, paddr + size);
		dmac_unmap_area(__va(paddr), size, dir);
	}
}

void arch_setup_dma_ops(struct device *dev, bool coherent)
{
	/*
	 * Assume coherent DMA in case MMU/MPU has not been set up.
	 */
	dev_assign_dma_coherent(dev, (get_cr() & CR_M) ? coherent : true);
}
