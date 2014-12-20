/*
 * PCIe functions for Marvell Orion System On Chip
 *
 * Maintainer: Tzachi Perelstein <tzachi@marvell.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/mbus.h>
#include <video/vga.h>
#include <asm/irq.h>
#include <asm/mach/pci.h>
#include <plat/pcie.h>
#include <plat/addr-map.h>
#include <mach/orion5x.h>
#include "common.h"

/*****************************************************************************
 * Orion has one PCIe controller and one PCI controller.
 * Orion has one PCIe controller and one PCI controller. This driver is only
 * for PCIe mode, and will eventually be replaced by pci-mvebu.
 *
 * Note: It is possible for PCI/PCIe agents to access many subsystem's
 * space, by configuring BARs and Address Decode Windows, e.g. flashes on
 * device bus, Orion registers, etc. However this code only enable the
 * access to DDR banks.
 ****************************************************************************/


/*****************************************************************************
 * PCIe controller
 ****************************************************************************/
#define PCIE_BASE	(ORION5X_PCIE_VIRT_BASE)

void __init orion5x_pcie_id(u32 *dev, u32 *rev)
{
	*dev = orion_pcie_dev_id(PCIE_BASE);
	*rev = orion_pcie_rev(PCIE_BASE);
}

static int pcie_valid_config(int bus, int dev)
{
	/*
	 * Don't go out when trying to access --
	 * 1. nonexisting device on local bus
	 * 2. where there's no device connected (no link)
	 */
	if (bus == 0 && dev == 0)
		return 1;

	if (!orion_pcie_link_up(PCIE_BASE))
		return 0;

	if (bus == 0 && dev != 1)
		return 0;

	return 1;
}


/*
 * PCIe config cycles are done by programming the PCIE_CONF_ADDR register
 * and then reading the PCIE_CONF_DATA register. Need to make sure these
 * transactions are atomic.
 */
static DEFINE_SPINLOCK(orion5x_pcie_lock);

static int pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			int size, u32 *val)
{
	unsigned long flags;
	int ret;

	if (pcie_valid_config(bus->number, PCI_SLOT(devfn)) == 0) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_lock_irqsave(&orion5x_pcie_lock, flags);
	ret = orion_pcie_rd_conf(PCIE_BASE, bus, devfn, where, size, val);
	spin_unlock_irqrestore(&orion5x_pcie_lock, flags);

	return ret;
}

static int pcie_rd_conf_wa(struct pci_bus *bus, u32 devfn,
			   int where, int size, u32 *val)
{
	int ret;

	if (pcie_valid_config(bus->number, PCI_SLOT(devfn)) == 0) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	/*
	 * We only support access to the non-extended configuration
	 * space when using the WA access method (or we would have to
	 * sacrifice 256M of CPU virtual address space.)
	 */
	if (where >= 0x100) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	ret = orion_pcie_rd_conf_wa(ORION5X_PCIE_WA_VIRT_BASE,
				    bus, devfn, where, size, val);

	return ret;
}

static int pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			int where, int size, u32 val)
{
	unsigned long flags;
	int ret;

	if (pcie_valid_config(bus->number, PCI_SLOT(devfn)) == 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	spin_lock_irqsave(&orion5x_pcie_lock, flags);
	ret = orion_pcie_wr_conf(PCIE_BASE, bus, devfn, where, size, val);
	spin_unlock_irqrestore(&orion5x_pcie_lock, flags);

	return ret;
}

static struct pci_ops pcie_ops = {
	.read = pcie_rd_conf,
	.write = pcie_wr_conf,
};


static int __init pcie_setup(struct pci_sys_data *sys)
{
	struct resource *res;
	int dev;

	/*
	 * Generic PCIe unit setup.
	 */
	orion_pcie_setup(PCIE_BASE);

	/*
	 * Check whether to apply Orion-1/Orion-NAS PCIe config
	 * read transaction workaround.
	 */
	dev = orion_pcie_dev_id(PCIE_BASE);
	if (dev == MV88F5181_DEV_ID || dev == MV88F5182_DEV_ID) {
		printk(KERN_NOTICE "Applying Orion-1/Orion-NAS PCIe config "
				   "read transaction workaround\n");
		mvebu_mbus_add_window_by_id(ORION_MBUS_PCIE_WA_TARGET,
					    ORION_MBUS_PCIE_WA_ATTR,
					    ORION5X_PCIE_WA_PHYS_BASE,
					    ORION5X_PCIE_WA_SIZE);
		pcie_ops.read = pcie_rd_conf_wa;
	}

	pci_ioremap_io(sys->busnr * SZ_64K, ORION5X_PCIE_IO_PHYS_BASE);

	/*
	 * Request resources.
	 */
	res = kzalloc(sizeof(struct resource), GFP_KERNEL);
	if (!res)
		panic("pcie_setup unable to alloc resources");

	/*
	 * IORESOURCE_MEM
	 */
	res->name = "PCIe Memory Space";
	res->flags = IORESOURCE_MEM;
	res->start = ORION5X_PCIE_MEM_PHYS_BASE;
	res->end = res->start + ORION5X_PCIE_MEM_SIZE - 1;
	if (request_resource(&iomem_resource, res))
		panic("Request PCIe Memory resource failed\n");
	pci_add_resource_offset(&sys->resources, res, sys->mem_offset);

	return 1;
}

/*****************************************************************************
 * General PCIe + PCI
 ****************************************************************************/
static void rc_pci_fixup(struct pci_dev *dev)
{
	/*
	 * FIXME: is this still required? do we need it for both drivers?
	 * Prevent enumeration of root complex.
	 */
	if (dev->bus->parent == NULL && dev->devfn == 0) {
		int i;

		for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
			dev->resource[i].start = 0;
			dev->resource[i].end   = 0;
			dev->resource[i].flags = 0;
		}
	}
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_MARVELL, PCI_ANY_ID, rc_pci_fixup);

int __init orion5x_pci_sys_setup(int nr, struct pci_sys_data *sys)
{
	vga_base = ORION5X_PCIE_MEM_PHYS_BASE;

	orion_pcie_set_local_bus_nr(PCIE_BASE, sys->busnr);
	return pcie_setup(sys);
}

struct pci_bus __init *orion5x_pci_sys_scan_bus(int nr, struct pci_sys_data *sys)
{
	return pci_scan_root_bus(NULL, sys->busnr, &pcie_ops, sys,
					&sys->resources);
}

int __init orion5x_pci_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	return IRQ_ORION5X_PCIE0_INT;
}
