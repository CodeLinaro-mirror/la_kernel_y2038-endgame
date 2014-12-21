/*
 * PCI functions for Marvell Orion System On Chip
 *
 * Maintainer: Tzachi Perelstein <tzachi@marvell.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_data/pci-orion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mbus.h>
#include <asm/irq.h>
#include <asm/mach/pci.h>

/*****************************************************************************
 * Orion has one PCIe controller and one PCI controller. This driver is only
 * for PCI mode, the other one is pci-mvebu.
 *
 * Note: It is possible for PCI/PCIe agents to access many subsystem's
 * space, by configuring BARs and Address Decode Windows, e.g. flashes on
 * device bus, Orion registers, etc. However this code only enable the
 * access to DDR banks.
 ****************************************************************************/

struct orion5x_pci {
	struct device *dev;
	void __iomem *reg;
	struct resource reg_phys;
	struct resource io_phys;
	struct resource io_io;
	struct resource mem;
	struct resource bus;
	bool cardbus;
};

#define PCI_MODE			0x0d00
#define PCI_CMD				0x0c00
#define PCI_P2P_CONF			0x1d14
#define PCI_CONF_ADDR			0x0c78
#define PCI_CONF_DATA			0x0c7c

/*
 * PCI_MODE bits
 */
#define PCI_MODE_64BIT			(1 << 2)
#define PCI_MODE_PCIX			((1 << 4) | (1 << 5))

/*
 * PCI_CMD bits
 */
#define PCI_CMD_HOST_REORDER		(1 << 29)

/*
 * PCI_P2P_CONF bits
 */
#define PCI_P2P_BUS_OFFS		16
#define PCI_P2P_BUS_MASK		(0xff << PCI_P2P_BUS_OFFS)
#define PCI_P2P_DEV_OFFS		24
#define PCI_P2P_DEV_MASK		(0x1f << PCI_P2P_DEV_OFFS)

/*
 * PCI_CONF_ADDR bits
 */
#define PCI_CONF_REG(reg)		((reg) & 0xfc)
#define PCI_CONF_FUNC(func)		(((func) & 0x3) << 8)
#define PCI_CONF_DEV(dev)		(((dev) & 0x1f) << 11)
#define PCI_CONF_BUS(bus)		(((bus) & 0xff) << 16)
#define PCI_CONF_ADDR_EN		(1 << 31)

/*
 * Internal configuration space
 */
#define PCI_CONF_FUNC_STAT_CMD		0
#define PCI_CONF_REG_STAT_CMD		4
#define PCIX_STAT			0x64
#define PCIX_STAT_BUS_OFFS		8
#define PCIX_STAT_BUS_MASK		(0xff << PCIX_STAT_BUS_OFFS)

/*
 * PCI Address Decode Windows registers
 */
#define PCI_BAR_SIZE_DDR_CS(n)	(((n) == 0) ? 0xc08 : \
				 ((n) == 1) ? 0xd08 : \
				 ((n) == 2) ? 0xc0c : \
				 ((n) == 3) ? 0xd0c : -1)
#define PCI_BAR_REMAP_DDR_CS(n)	(((n) == 0) ? 0xc48 : \
				 ((n) == 1) ? 0xd48 : \
				 ((n) == 2) ? 0xc4c : \
				 ((n) == 3) ? 0xd4c : -1)
#define PCI_BAR_ENABLE		0xc3c
#define PCI_ADDR_DECODE_CTRL	0xd3c

/*
 * PCI configuration helpers for BAR settings
 */
#define PCI_CONF_FUNC_BAR_CS(n)		((n) >> 1)
#define PCI_CONF_REG_BAR_LO_CS(n)	(((n) & 1) ? 0x18 : 0x10)
#define PCI_CONF_REG_BAR_HI_CS(n)	(((n) & 1) ? 0x1c : 0x14)

static int orion5x_pci_hw_rd_conf(struct orion5x_pci *priv,
				  int bus, int dev, u32 func,
				  u32 where, u32 size, u32 *val)
{
	writel(PCI_CONF_BUS(bus) | PCI_CONF_DEV(dev) | PCI_CONF_REG(where) |
		PCI_CONF_FUNC(func) | PCI_CONF_ADDR_EN,
	       priv->reg + PCI_CONF_ADDR);

	*val = readl(priv->reg + PCI_CONF_DATA);

	if (size == 1)
		*val = (*val >> (8*(where & 0x3))) & 0xff;
	else if (size == 2)
		*val = (*val >> (8*(where & 0x3))) & 0xffff;

	return PCIBIOS_SUCCESSFUL;
}

static int orion5x_pci_hw_wr_conf(struct orion5x_pci *priv,
				  int bus, int dev, u32 func,
				  u32 where, u32 size, u32 val)
{
	int ret = PCIBIOS_SUCCESSFUL;

	writel(PCI_CONF_BUS(bus) | PCI_CONF_DEV(dev) | PCI_CONF_REG(where) |
		PCI_CONF_FUNC(func) | PCI_CONF_ADDR_EN,
	       priv->reg + PCI_CONF_ADDR);

	if (size == 4) {
		writel_relaxed(val, priv->reg + PCI_CONF_DATA);
	} else if (size == 2) {
		writew_relaxed(val, priv->reg + PCI_CONF_DATA + (where & 0x3));
	} else if (size == 1) {
		writeb_relaxed(val, priv->reg + PCI_CONF_DATA + (where & 0x3));
	} else {
		ret = PCIBIOS_BAD_REGISTER_NUMBER;
	}

	return ret;
}

static int orion5x_pci_valid_config(struct orion5x_pci *priv, int bus, u32 devfn)
{
	if (bus == priv->bus.start) {
		/*
		 * Don't go out for local device
		 */
		if (PCI_SLOT(devfn) == 0 && PCI_FUNC(devfn) != 0)
			return 0;

		/*
		 * When the PCI signals are directly connected to a
		 * Cardbus slot, ignore all but device IDs 0 and 1.
		 */
		if (priv->cardbus && PCI_SLOT(devfn) > 1)
			return 0;
	}

	return 1;
}

static int orion5x_pci_rd_conf(struct pci_bus *bus, u32 devfn,
			       int where, int size, u32 *val)
{
	struct pci_sys_data *sysdata = bus->sysdata;
	struct orion5x_pci *priv = sysdata->private_data;

	if (!orion5x_pci_valid_config(priv, bus->number, devfn)) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	return orion5x_pci_hw_rd_conf(priv, bus->number, PCI_SLOT(devfn),
					PCI_FUNC(devfn), where, size, val);
}

static int orion5x_pci_wr_conf(struct pci_bus *bus, u32 devfn,
			       int where, int size, u32 val)
{
	struct pci_sys_data *sysdata = bus->sysdata;
	struct orion5x_pci *priv = sysdata->private_data;

	if (!orion5x_pci_valid_config(priv, bus->number, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	return orion5x_pci_hw_wr_conf(priv, bus->number, PCI_SLOT(devfn),
					PCI_FUNC(devfn), where, size, val);
}

static struct pci_ops orion5x_pci_ops = {
	.read = orion5x_pci_rd_conf,
	.write = orion5x_pci_wr_conf,
};

static void orion5x_pci_set_bus_nr(struct orion5x_pci *priv, int nr)
{
	u32 p2p = readl(priv->reg + PCI_P2P_CONF);

	if (readl(priv->reg + PCI_MODE) & PCI_MODE_PCIX) {
		/*
		 * PCI-X mode
		 */
		u32 pcix_status, bus, dev;
		bus = (p2p & PCI_P2P_BUS_MASK) >> PCI_P2P_BUS_OFFS;
		dev = (p2p & PCI_P2P_DEV_MASK) >> PCI_P2P_DEV_OFFS;
		orion5x_pci_hw_rd_conf(priv, bus, dev, 0, PCIX_STAT, 4, &pcix_status);
		pcix_status &= ~PCIX_STAT_BUS_MASK;
		pcix_status |= (nr << PCIX_STAT_BUS_OFFS);
		orion5x_pci_hw_wr_conf(priv, bus, dev, 0, PCIX_STAT, 4, pcix_status);
	} else {
		/*
		 * PCI Conventional mode
		 */
		p2p &= ~PCI_P2P_BUS_MASK;
		p2p |= (nr << PCI_P2P_BUS_OFFS);
		writel(p2p, priv->reg + PCI_P2P_CONF);
	}
}

static void orion5x_pci_master_slave_enable(struct orion5x_pci *priv)
{
	int bus_nr, func, reg;
	u32 val;

	bus_nr = priv->bus.start;
	func = PCI_CONF_FUNC_STAT_CMD;
	reg = PCI_CONF_REG_STAT_CMD;
	orion5x_pci_hw_rd_conf(priv, bus_nr, 0, func, reg, 4, &val);
	val |= (PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	orion5x_pci_hw_wr_conf(priv, bus_nr, 0, func, reg, 4, val | 0x7);
}

static void orion5x_setup_pci_wins(struct orion5x_pci *priv)
{
	const struct mbus_dram_target_info *dram = mv_mbus_dram_info();
	u32 win_enable;
	int bus;
	int i;

	/*
	 * First, disable windows.
	 */
	win_enable = 0xffffffff;
	writel(win_enable, priv->reg + PCI_BAR_ENABLE);

	/*
	 * Setup windows for DDR banks.
	 */
	bus = priv->bus.start;

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;
		u32 func = PCI_CONF_FUNC_BAR_CS(cs->cs_index);
		u32 reg;
		u32 val;

		/*
		 * Write DRAM bank base address register.
		 */
		reg = PCI_CONF_REG_BAR_LO_CS(cs->cs_index);
		orion5x_pci_hw_rd_conf(priv, bus, 0, func, reg, 4, &val);
		val = (cs->base & 0xfffff000) | (val & 0xfff);
		orion5x_pci_hw_wr_conf(priv, bus, 0, func, reg, 4, val);

		/*
		 * Write DRAM bank size register.
		 */
		reg = PCI_CONF_REG_BAR_HI_CS(cs->cs_index);
		orion5x_pci_hw_wr_conf(priv, bus, 0, func, reg, 4, 0);
		writel((cs->size - 1) & 0xfffff000,
			priv->reg + PCI_BAR_SIZE_DDR_CS(cs->cs_index));
		writel(cs->base & 0xfffff000,
			priv->reg + PCI_BAR_REMAP_DDR_CS(cs->cs_index));

		/*
		 * Enable decode window for this chip select.
		 */
		win_enable &= ~(1 << cs->cs_index);
	}

	/*
	 * Re-enable decode windows.
	 */
	writel(win_enable, priv->reg + PCI_BAR_ENABLE);

	/*
	 * Disable automatic update of address remapping when writing to BARs.
	 */
	writel(readl(priv->reg + PCI_ADDR_DECODE_CTRL) | 1, priv->reg + PCI_ADDR_DECODE_CTRL);
}

static int orion5x_pci_setup(int nr, struct pci_sys_data *sys)
{
	struct orion5x_pci *priv = sys->private_data;

	/*
	 * map mmio registers
	 */
	priv->reg = devm_ioremap_resource(priv->dev, &priv->reg_phys);

	/*
	 * set up the bus number
	 */
	orion5x_pci_set_bus_nr(priv, priv->bus.start);

	/*
	 * Point PCI unit MBUS decode windows to DRAM space.
	 */
	orion5x_setup_pci_wins(priv);

	/*
	 * Master + Slave enable
	 */
	orion5x_pci_master_slave_enable(priv);

	/*
	 * Force ordering
	 */
	writel(readl(priv->reg + PCI_CMD) | PCI_CMD_HOST_REORDER,
	       priv->reg + PCI_CMD);

	/*
	 * Request resources
	 */
	if (request_resource(&iomem_resource, &priv->mem) == 0)
		pci_add_resource_offset(&sys->resources, &priv->mem,
					sys->mem_offset);

	if (request_resource(&iomem_resource, &priv->io_phys) == 0 &&
	    request_resource(&ioport_resource, &priv->io_io) == 0) {
		sys->io_offset = priv->io_io.start;
		pci_ioremap_io(priv->io_io.start, priv->io_phys.start);
		pci_add_resource_offset(&sys->resources, &priv->io_io,
					sys->io_offset);
	}

	return 1;
}

/* hardcoded MMIO locations when booting without DT */
#define __ORION5X_PCI_REG_PHYS_BASE	(0xf1000000 + 0x30000)
#define __ORION5X_PCI_REG_SIZE		0x10000
#define __ORION5X_PCI_IO_PHYS_BASE	0xf2100000
#define __ORION5X_PCI_IO_SIZE		SZ_64K
#define __ORION5X_PCI_MEM_PHYS_BASE	0xe8000000
#define __ORION5X_PCI_MEM_SIZE		SZ_128M
static int orion5x_pci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct orion5x_pci *priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	void *hw_priv[1] = { priv };
	struct orion_pci_platform_data *pdata = dev_get_platdata(dev);
	/*
	 * we always use domain 1 here and domain 0 for pcie, but after
	 * "arm: pcibios: remove pci_sys_data domain" is merged, it will
	 * work automatically
	 */
	struct hw_pci hwpci = {
		.domain		= 1,
		.nr_controllers	= ARRAY_SIZE(hw_priv),
		.ops		= &orion5x_pci_ops,
		.setup		= orion5x_pci_setup,
		.map_irq	= pdata->map_irq,
		.private_data	= hw_priv,
	};

	if (!priv)
		return -ENOMEM;

	priv->cardbus = pdata->cardbus;
	priv->reg_phys = (struct resource)
		DEFINE_RES_MEM_NAMED(__ORION5X_PCI_REG_PHYS_BASE,
				     __ORION5X_PCI_REG_SIZE, "PCI host");
	priv->io_phys = (struct resource)
		DEFINE_RES_MEM_NAMED(__ORION5X_PCI_IO_PHYS_BASE,
				     __ORION5X_PCI_IO_SIZE, "PCI I/O window");
	/* start at offset 64K to get out of the way of PCIe */
	priv->io_io = (struct resource)
		DEFINE_RES_IO_NAMED(SZ_64K,
				     __ORION5X_PCI_IO_SIZE, "PCI");
	priv->mem = (struct resource)
		DEFINE_RES_MEM_NAMED(__ORION5X_PCI_MEM_PHYS_BASE,
				     __ORION5X_PCI_MEM_SIZE, "PCI MMIO");
	priv->bus = (struct resource)
		DEFINE_RES_NAMED(0, 255, "PCI bus", IORESOURCE_BUS);

	if (!priv->reg)
		return -ENXIO;

	if (pdata->preinit)
		pdata->preinit();

	pci_common_init_dev(dev, &hwpci);

	return 0;
}

static struct platform_driver orion5x_pci_driver = {
	.driver = {
		.name = "orion-pci",
		.suppress_bind_attrs = true,
	},
	.probe = orion5x_pci_probe,
};

static int orion5x_pci_init(void)
{
	return platform_driver_register(&orion5x_pci_driver);
}
module_init(orion5x_pci_init);

MODULE_LICENSE("GPL v2");
