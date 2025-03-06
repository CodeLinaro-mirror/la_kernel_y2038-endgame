// SPDX-License-Identifier: GPL-2.0
/*
 * Re-map IO memory to kernel address space so that we can access it.
 * This is needed for high PCI addresses that aren't mapped in the
 * 640k-1MB IO memory area on PC's
 *
 * (C) Copyright 1995 1996 Linus Torvalds
 */
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/ioremap.h>

void __iomem *generic_ioremap_prot(phys_addr_t phys_addr, size_t size,
				   pgprot_t prot)
{
	unsigned long offset, vaddr;
	phys_addr_t last_addr;
	struct vm_struct *area;

	/* An early platform driver might end up here */
	if (WARN_ON_ONCE(!slab_is_available()))
		return NULL;

	/* Disallow wrap-around or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/* Page-align mappings */
	offset = phys_addr & (~PAGE_MASK);
	phys_addr -= offset;
	size = PAGE_ALIGN(size + offset);

	area = __get_vm_area_caller(size, VM_IOREMAP, IOREMAP_START,
				    IOREMAP_END, __builtin_return_address(0));
	if (!area)
		return NULL;
	vaddr = (unsigned long)area->addr;
	area->phys_addr = phys_addr;

	if (ioremap_page_range(vaddr, vaddr + size, phys_addr, prot)) {
		free_vm_area(area);
		return NULL;
	}

	return (void __iomem *)(vaddr + offset);
}

#ifndef ioremap_prot
void __iomem *ioremap_prot(phys_addr_t phys_addr, size_t size,
			   pgprot_t prot)
{
	return generic_ioremap_prot(phys_addr, size, prot);
}
EXPORT_SYMBOL(ioremap_prot);
#endif

#ifndef ioremap
void __iomem *ioremap(phys_addr_t phys_addr, size_t size)
{
	return ioremap_prot(phys_addr, size, __pgprot(_PAGE_IOREMAP));
}
EXPORT_SYMBOL(ioremap);
#endif

void generic_iounmap(volatile void __iomem *addr)
{
	void *vaddr = (void *)((unsigned long)addr & PAGE_MASK);

	if (is_ioremap_addr(vaddr))
		vunmap(vaddr);
}

#ifndef iounmap
void iounmap(volatile void __iomem *addr)
{
	generic_iounmap(addr);
}
EXPORT_SYMBOL(iounmap);
#endif

#ifdef CONFIG_PCI
/*
 * The PCI specifications (Rev 3.0, 3.2.5 "Transaction Ordering and
 * Posting") mandate non-posted configuration transactions. This default
 * implementation attempts to use the ioremap_np() API to provide this
 * on arches that support it, and falls back to ioremap() on those that
 * don't. Overriding this function is deprecated; arches that properly
 * support non-posted accesses should implement ioremap_np() instead, which
 * this default implementation can then use to return mappings compliant with
 * the PCI specification.
 */
#ifndef pci_remap_cfgspace
void __iomem *pci_remap_cfgspace(phys_addr_t offset, size_t size)
{
	return ioremap_np(offset, size) ?: ioremap(offset, size);
}
EXPORT_SYMBOL_GPL(pci_remap_cfgspace);
#endif
#endif
