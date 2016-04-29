/*
 * Generic PCI host driver common code
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2014 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 */

#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/pci-ecam.h>
#include <linux/platform_device.h>

static int gen_pci_parse_request_of_pci_ranges(struct pci_host_bridge *bridge)
{
	int err, res_valid = 0;
	struct device *dev = bridge->dev.parent;
	struct device_node *np = bridge->dev.parent->of_node;
	resource_size_t iobase;
	struct resource_entry *win, *tmp;

	err = of_pci_get_host_bridge_resources(np, 0, 0xff, &bridge->windows, &iobase);
	if (err)
		return err;

	err = devm_request_pci_bus_resources(dev, &bridge->windows);
	if (err)
		return err;

	resource_list_for_each_entry_safe(win, tmp, &bridge->windows) {
		struct resource *res = win->res;

		switch (resource_type(res)) {
		case IORESOURCE_IO:
			err = pci_remap_iospace(res, iobase);
			if (err) {
				dev_warn(dev, "error %d: failed to map resource %pR\n",
					 err, res);
				resource_list_destroy_entry(win);
			}
			break;
		case IORESOURCE_MEM:
			res_valid |= !(res->flags & IORESOURCE_PREFETCH);
			break;
		case IORESOURCE_BUS:
			bridge->busnr = res->start;
		}
	}

	if (res_valid)
		return 0;

	dev_err(dev, "non-prefetchable memory resource required\n");
	return -EINVAL;
}

static struct pci_config_window *gen_pci_init(struct pci_host_bridge *bridge,
					      struct pci_ecam_ops *ops)
{
	int err;
	struct resource cfgres;
	struct device *dev = bridge->dev.parent;
	struct pci_config_window *cfg;
	struct resource *bus_res;

	err = of_address_to_resource(dev->of_node, 0, &cfgres);
	if (err) {
		dev_err(dev, "missing \"reg\" property\n");
		goto err_out;
	}

	bus_res = __pci_find_resource(&bridge->windows, IORESOURCE_BUS);

	/* Parse our PCI ranges and request their resources */
	err = gen_pci_parse_request_of_pci_ranges(bridge);
	if (err)
		return ERR_PTR(err);

	cfg = pci_ecam_create(dev, &cfgres, bus_res, ops);
	if (IS_ERR(cfg)) {
		err = PTR_ERR(cfg);
		goto err_out;
	}

	bridge->sysdata = cfg;

	return cfg;

err_out:
	pci_free_resource_list(&bridge->windows);
	return ERR_PTR(err);
}

static void gen_pci_release(struct device *dev)
{
	struct pci_host_bridge *bridge = container_of(dev, struct pci_host_bridge, dev);

	pci_ecam_free(bridge->sysdata);
	pci_free_resource_list(&bridge->windows);
	kfree(bridge);
}

int pci_host_common_probe(struct platform_device *pdev,
			  struct pci_ecam_ops *ops)
{
	const char *type;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct pci_host_bridge *bridge;
	struct pci_config_window *cfg;
	struct pci_bus *child;
	struct resource *bus_res;
	int err;

	type = of_get_property(np, "device_type", NULL);
	if (!type || strcmp(type, "pci")) {
		dev_err(dev, "invalid \"device_type\" %s\n", type);
		return -EINVAL;
	}

	bridge = kzalloc(sizeof(struct pci_host_bridge), GFP_KERNEL);

	of_pci_check_probe_only();

	bridge->dev.parent = dev;
	bridge->dev.release = gen_pci_release;
	INIT_LIST_HEAD(&bridge->windows);

	/* Parse and map our Configuration Space windows */
	cfg = gen_pci_init(bridge, ops);
	if (IS_ERR(cfg))
		return PTR_ERR(cfg);

	/* Do not reassign resources if probe only */
	if (!pci_has_flag(PCI_PROBE_ONLY))
		pci_add_flags(PCI_REASSIGN_ALL_RSRC | PCI_REASSIGN_ALL_BUS);

	bridge->ops = &cfg->ops->pci_ops;
	err = pci_register_host_bridge(bridge);
	if (!err) {
		dev_err(dev, "registering host failed");
		return err;
	}
	bus_res = __pci_find_resource(&bridge->windows, IORESOURCE_BUS);
	bus_res->end = pci_scan_child_bus(bridge->bus);

	pci_fixup_irqs(pci_common_swizzle, of_irq_parse_and_map_pci);

	/*
	 * We insert PCI resources into the iomem_resource and
	 * ioport_resource trees in either pci_bus_claim_resources()
	 * or pci_bus_assign_resources().
	 */
	if (pci_has_flag(PCI_PROBE_ONLY)) {
		pci_bus_claim_resources(bridge->bus);
	} else {
		pci_bus_size_bridges(bridge->bus);
		pci_bus_assign_resources(bridge->bus);

		list_for_each_entry(child, &bridge->bus->children, node)
			pcie_bus_configure_settings(child);
	}

	pci_bus_add_devices(bridge->bus);
	return 0;
}
