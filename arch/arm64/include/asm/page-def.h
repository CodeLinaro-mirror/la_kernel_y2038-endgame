/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/page.h
 *
 * Copyright (C) 1995-2003 Russell King
 * Copyright (C) 2017 ARM Ltd.
 */
#ifndef __ASM_PAGE_DEF_H
#define __ASM_PAGE_DEF_H

#include <linux/const.h>

#include <vdso/page.h>

/*
 * Size of the PCI I/O space. This must remain a power of two so that
 * IO_SPACE_LIMIT acts as a mask for the low bits of I/O addresses.
 */
#define PCI_IO_SIZE		SZ_16M

/*
 * VMEMMAP_SIZE - allows the whole linear region to be covered by
 *                a struct page array
 *
 * If we are configured with a 52-bit kernel VA then our VMEMMAP_SIZE
 * needs to cover the memory region from the beginning of the 52-bit
 * PAGE_OFFSET all the way to PAGE_END for 48-bit. This allows us to
 * keep a constant PAGE_OFFSET and "fallback" to using the higher end
 * of the VMEMMAP where 52-bit support is not available in hardware.
 */
#define VMEMMAP_RANGE	(_PAGE_END(VA_BITS_MIN) - PAGE_OFFSET)
#define VMEMMAP_SIZE	((VMEMMAP_RANGE >> PAGE_SHIFT) * sizeof(struct page))

/*
 * PAGE_OFFSET - the virtual address of the start of the linear map, at the
 *               start of the TTBR1 address space.
 * PAGE_END - the end of the linear map, where all other kernel mappings begin.
 * KIMAGE_VADDR - the virtual address of the start of the kernel image.
 * VA_BITS - the maximum number of bits for virtual addresses.
 */
#define VA_BITS			(CONFIG_ARM64_VA_BITS)
#define _PAGE_OFFSET(va)	(-(UL(1) << (va)))
#define PAGE_OFFSET		(_PAGE_OFFSET(VA_BITS))
#define KIMAGE_VADDR		(MODULES_END)
#define MODULES_END		(MODULES_VADDR + MODULES_VSIZE)
#define MODULES_VADDR		(_PAGE_END(VA_BITS_MIN))
#define MODULES_VSIZE		(SZ_2G)
#define VMEMMAP_START		(VMEMMAP_END - VMEMMAP_SIZE)
#define VMEMMAP_END		(-UL(SZ_1G))
#define PCI_IO_START		(VMEMMAP_END + SZ_8M)
#define PCI_IO_END		(PCI_IO_START + PCI_IO_SIZE)
#define FIXADDR_TOP		(-UL(SZ_8M))

#if VA_BITS > 48
#ifdef CONFIG_ARM64_16K_PAGES
#define VA_BITS_MIN		(47)
#else
#define VA_BITS_MIN		(48)
#endif
#else
#define VA_BITS_MIN		(VA_BITS)
#endif

#define _PAGE_END(va)		(-(UL(1) << ((va) - 1)))

#endif /* __ASM_PAGE_DEF_H */
