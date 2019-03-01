// SPDX-License-Identifier: GPL-2.0
/*
 * Architecture independent helper functions for memory management
 *
 * Written by Paul Durrant <paul.durrant@citrix.com>
 */
#include <linux/mm.h>
#include <linux/export.h>

struct remap_pfn {
	struct mm_struct *mm;
	struct page **pages;
	pgprot_t prot;
	unsigned long i;
};

static int remap_pfn_fn(pte_t *ptep, pgtable_t token, unsigned long addr,
			void *data)
{
	struct remap_pfn *r = data;
	struct page *page = r->pages[r->i];
	pte_t pte = pte_mkspecial(pfn_pte(page_to_pfn(page), r->prot));

	set_pte_at(r->mm, addr, ptep, pte);
	r->i++;

	return 0;
}

/* Used by the privcmd module, but has to be built-in on ARM */
int xen_remap_vma_range(struct vm_area_struct *vma, unsigned long addr, unsigned long len)
{
	struct remap_pfn r = {
		.mm = vma->vm_mm,
		.pages = vma->vm_private_data,
		.prot = vma->vm_page_prot,
	};

	return apply_to_page_range(vma->vm_mm, addr, len, remap_pfn_fn, &r);
}
EXPORT_SYMBOL_GPL(xen_remap_vma_range);
