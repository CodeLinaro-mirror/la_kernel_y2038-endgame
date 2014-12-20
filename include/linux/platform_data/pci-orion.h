#ifndef __PLATFORM_DATA_PCI_ORION_H
#define __PLATFORM_DATA_PCI_ORION_H

struct orion_pci_platform_data {
	bool cardbus;
	void (*preinit)(void);
	int (*map_irq)(const struct pci_dev * dev, u8 slot, u8 pin);
};

#endif
