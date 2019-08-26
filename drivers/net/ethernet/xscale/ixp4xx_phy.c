// SPDX-License-Identifier: GPL-2.0-only
/*
 * Intel IXP4xx Ethernet phy driver for Linux
 *
 * Copyright (C) 2007 Krzysztof Halasa <khc@pm.waw.pl>
 */
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/soc/ixp4xx/cpu.h>

#include "ixp4xx_eth.h"
 
#define DEBUG_MDIO		0
#define MDIO_INTERVAL		(3 * HZ)
#define MAX_MDIO_RETRIES	100 /* microseconds, typically 30 cycles */

static spinlock_t mdio_lock;
static struct eth_regs __iomem *mdio_regs; /* mdio command and status only */
static struct mii_bus *mdio_bus;

static int ixp4xx_mdio_cmd(struct mii_bus *bus, int phy_id, int location,
			   int write, u16 cmd)
{
	int cycles = 0;

	if (__raw_readl(&mdio_regs->mdio_command[3]) & 0x80) {
		printk(KERN_ERR "%s: MII not ready to transmit\n", bus->name);
		return -1;
	}

	if (write) {
		__raw_writel(cmd & 0xFF, &mdio_regs->mdio_command[0]);
		__raw_writel(cmd >> 8, &mdio_regs->mdio_command[1]);
	}
	__raw_writel(((phy_id << 5) | location) & 0xFF,
		     &mdio_regs->mdio_command[2]);
	__raw_writel((phy_id >> 3) | (write << 2) | 0x80 /* GO */,
		     &mdio_regs->mdio_command[3]);

	while ((cycles < MAX_MDIO_RETRIES) &&
	       (__raw_readl(&mdio_regs->mdio_command[3]) & 0x80)) {
		udelay(1);
		cycles++;
	}

	if (cycles == MAX_MDIO_RETRIES) {
		printk(KERN_ERR "%s #%i: MII write failed\n", bus->name,
		       phy_id);
		return -1;
	}

#if DEBUG_MDIO
	printk(KERN_DEBUG "%s #%i: mdio_%s() took %i cycles\n", bus->name,
	       phy_id, write ? "write" : "read", cycles);
#endif

	if (write)
		return 0;

	if (__raw_readl(&mdio_regs->mdio_status[3]) & 0x80) {
#if DEBUG_MDIO
		printk(KERN_DEBUG "%s #%i: MII read failed\n", bus->name,
		       phy_id);
#endif
		return 0xFFFF; /* don't return error */
	}

	return (__raw_readl(&mdio_regs->mdio_status[0]) & 0xFF) |
		((__raw_readl(&mdio_regs->mdio_status[1]) & 0xFF) << 8);
}

static int ixp4xx_mdio_read(struct mii_bus *bus, int phy_id, int location)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&mdio_lock, flags);
	ret = ixp4xx_mdio_cmd(bus, phy_id, location, 0, 0);
	spin_unlock_irqrestore(&mdio_lock, flags);
#if DEBUG_MDIO
	printk(KERN_DEBUG "%s #%i: MII read [%i] -> 0x%X\n", bus->name,
	       phy_id, location, ret);
#endif
	return ret;
}

static int ixp4xx_mdio_write(struct mii_bus *bus, int phy_id, int location,
			     u16 val)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&mdio_lock, flags);
	ret = ixp4xx_mdio_cmd(bus, phy_id, location, 1, val);
	spin_unlock_irqrestore(&mdio_lock, flags);
#if DEBUG_MDIO
	printk(KERN_DEBUG "%s #%i: MII write [%i] <- 0x%X, err = %i\n",
	       bus->name, phy_id, location, val, ret);
#endif
	return ret;
}

static int ixp4xx_mdio_register(void)
{
	int err;
	/*
	 * FIXME: we bail out on device tree boot but this really needs
	 * to be fixed in a nicer way: this registers the MDIO bus before
	 * even matching the driver infrastructure, we should only probe
	 * detected hardware.
	 */
	if (of_have_populated_dt())
		return -ENODEV;

	if (!(mdio_bus = mdiobus_alloc()))
		return -ENOMEM;

	if (cpu_is_ixp43x()) {
		/* IXP43x lacks NPE-B and uses NPE-C for MII PHY access */
		if (!(ixp4xx_read_feature_bits() & IXP4XX_FEATURE_NPEC_ETH))
			return -ENODEV;
		mdio_regs = (struct eth_regs __iomem *)IXP4XX_EthC_BASE_VIRT;
	} else {
		/* All MII PHY accesses use NPE-B Ethernet registers */
		if (!(ixp4xx_read_feature_bits() & IXP4XX_FEATURE_NPEB_ETH0))
			return -ENODEV;
		mdio_regs = (struct eth_regs __iomem *)IXP4XX_EthB_BASE_VIRT;
	}

	__raw_writel(DEFAULT_CORE_CNTRL, &mdio_regs->core_control);
	spin_lock_init(&mdio_lock);
	mdio_bus->name = "IXP4xx MII Bus";
	mdio_bus->read = &ixp4xx_mdio_read;
	mdio_bus->write = &ixp4xx_mdio_write;
	snprintf(mdio_bus->id, MII_BUS_ID_SIZE, IXP4XX_MDIO_BUS_ID);

	if ((err = mdiobus_register(mdio_bus)))
		mdiobus_free(mdio_bus);
	return err;
}
module_init(ixp4xx_mdio_register);

static void ixp4xx_mdio_remove(void)
{
	mdiobus_unregister(mdio_bus);
	mdiobus_free(mdio_bus);
}
module_exit(ixp4xx_mdio_remove);

MODULE_AUTHOR("Krzysztof Halasa");
MODULE_DESCRIPTION("Intel IXP4xx Ethernet PHY driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:ixp4xx_phy");
