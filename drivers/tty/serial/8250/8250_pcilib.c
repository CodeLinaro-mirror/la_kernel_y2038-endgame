// SPDX-License-Identifier: GPL-2.0
/*
 * 8250 PCI library.
 *
 * Copyright (C) 2001 Russell King, All Rights Reserved.
 */
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/types.h>

#include "8250.h"
#include "8250_pcilib.h"

int serial_8250_warn_need_ioport(struct pci_dev *dev)
{
	dev_warn(&dev->dev,
		 "Serial port not supported because of missing I/O resource\n");

	return -ENXIO;
}
EXPORT_SYMBOL_NS_GPL(serial_8250_warn_need_ioport, "SERIAL_8250_PCI");

int serial8250_pci_setup_port(struct pci_dev *dev, struct uart_8250_port *port,
		   u8 bar, unsigned int offset, int regshift, void __iomem *iomem)
{
	if (bar >= PCI_STD_NUM_BARS)
		return -EINVAL;

	if (pci_resource_flags(dev, bar) & IORESOURCE_MEM) {
	} else if (IS_ENABLED(CONFIG_HAS_IOPORT)) {
	} else {
		return serial_8250_warn_need_ioport(dev);
	}
	return 0;
}
EXPORT_SYMBOL_NS_GPL(serial8250_pci_setup_port, "SERIAL_8250_PCI");
MODULE_DESCRIPTION("8250 PCI library");
MODULE_LICENSE("GPL");
