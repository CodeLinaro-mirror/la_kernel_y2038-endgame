/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *  arch/arm/include/asm/mach/pci.h
 *
 *  Copyright (C) 2000 Russell King
 */

#ifndef __ASM_MACH_PCI_H
#define __ASM_MACH_PCI_H

#include <linux/ioport.h>

struct pci_sys_data;
struct pci_ops;
struct pci_bus;
struct pci_host_bridge;
struct device;

struct hw_pci {
	struct pci_ops	*ops;
	void		*private_data;
	int		(*setup)(struct pci_host_bridge *bridge);
	void		(*preinit)(void);
	void		(*postinit)(void);
	u8		(*swizzle)(struct pci_dev *dev, u8 *pin);
	int		(*map_irq)(const struct pci_dev *dev, u8 slot, u8 pin);
};

/*
 * Per-controller structure
 */
struct pci_sys_data {
	u64		mem_offset;	/* bus->cpu memory mapping offset	*/
	unsigned long	io_offset;	/* bus->cpu IO mapping offset		*/
	struct resource io_res;
	char		io_res_name[12];
	void		*private_data;	/* platform controller private data	*/
};

/*
 * Call this with your hw_pci struct to initialise the PCI system.
 */
int pci_common_init_dev(struct device *, struct hw_pci *);

/*
 * Compatibility wrapper for older platforms that do not care about
 * passing the parent device.
 */
static inline int pci_common_init(struct hw_pci *hw)
{
	return pci_common_init_dev(NULL, hw);
}

/*
 * Setup early fixed I/O mapping.
 */
#if defined(CONFIG_PCI)
extern void pci_map_io_early(unsigned long pfn);
#else
static inline void pci_map_io_early(unsigned long pfn) {}
#endif

/*
 * PCI controllers
 */
extern struct pci_ops iop3xx_ops;
extern int iop3xx_pci_setup(struct pci_host_bridge *bridge);
extern void iop3xx_pci_preinit(void);
extern void iop3xx_pci_preinit_cond(void);

extern struct pci_ops dc21285_ops;
extern int dc21285_setup(struct pci_host_bridge *bridge);
extern void dc21285_preinit(void);
extern void dc21285_postinit(void);

#endif /* __ASM_MACH_PCI_H */
