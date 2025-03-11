// SPDX-License-Identifier: GPL-1.0+
/* ethernec.c: A variant of ne.c for Atari/ethernec */
/*
    Written 1992-94 by Donald Becker.

    Copyright 1993 United States Government as represented by the
    Director, National Security Agency.

*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>

#include <asm/io.h>

static const char version[] = "NE2000 Ethernec driver";

#define ethernec_isa_read_base	(void __iomem *)0xfffa0000

static inline u8 __iomem *isa_rom_itb(unsigned long addr)
{
	return ethernec_isa_read_base+((((unsigned long)(addr))&0x7F)<<9);
}

static inline u16 __iomem *isa_rom_itw(unsigned long addr)
{
	return ethernec_isa_read_base+((((unsigned long)(addr))&0x7F)<<9);
}

#define isa_rom_inb(port)	rom_in_8(isa_rom_itb(port))
#define isa_rom_inw(port)	rom_in_le16(isa_rom_itw(port))
#define isa_rom_outb(val, port)	rom_out_8(isa_rom_itb(port), (val))
#define isa_rom_outw(val, port)	rom_out_le16(isa_rom_itw(port), (val))
#define isa_rom_insb(port, buf, nr) raw_rom_insb(isa_rom_itb(port), (u8 *)(buf), (nr))
#define isa_rom_insw(port, buf, nr) raw_rom_insw_swapw(isa_rom_itw(port), (u16 *)(buf), (nr))
#define isa_rom_outsb(port, buf, nr) raw_rom_outsb(isa_rom_itb(port), (u8 *)(buf), (nr))
#define isa_rom_outsw(port, buf, nr) raw_rom_outsw_swapw(isa_rom_itw(port), (u16 *)(buf), (nr))

#define ei_inb(port)      	isa_rom_inb(port)
#define ei_inb_p(port)    	isa_rom_inb(port)
#define ei_inw(port)      	isa_rom_inw(port)
#define ei_inw_p(port)    	isa_rom_inw(port)
#define ei_outb(val, port)	isa_rom_outb((val), (port))
#define ei_outb_p(val, port)	isa_rom_outb((val), (port))
#define ei_outw(val, port)     	isa_rom_outw((val), (port))
#define ei_outw_p(val, port) 	isa_rom_outw((val), (port))
#define ei_insb(port, buf, nr)	isa_rom_insb((port), (buf), (nr))
#define ei_insw(port, buf, nr)	isa_rom_insw((port), (buf), (nr))
#define ei_outsb(port, buf, nr)	isa_rom_outsb((port), (buf), (nr))
#define ei_outsw(port, buf, nr)	isa_rom_outsw((port), (buf), (nr))

#include "8390.h"
#include "lib8390.c"

/* Do we support clones that don't adhere to 14,15 of the SAprom ? */
#define SUPPORT_NE_BAD_CLONES
/* 0xbad = bad sig or no reset ack */
#define BAD 0xbad

MODULE_DESCRIPTION("NE2000 Ethernec driver");
MODULE_LICENSE("GPL");

/* ---- No user-serviceable parts below ---- */

#define NE_BASE	 (dev->base_addr)
#define NE_CMD	 	0x00
#define NE_DATAPORT	0x10	/* NatSemi-defined port window offset. */
#define NE_RESET	0x1f	/* Issue a read to reset, a write to clear. */
#define NE_IO_EXTENT	0x20

#define NE1SM_START_PG	0x20	/* First page of TX buffer */
#define NE1SM_STOP_PG 	0x40	/* Last page +1 of RX ring */
#define NESM_START_PG	0x40	/* First page of TX buffer */
#define NESM_STOP_PG	0x80	/* Last page +1 of RX ring */

#define DCR_VAL		0x48 	/* 8-bit mode on Atari */

static int ne_probe1(struct net_device *dev, unsigned long ioaddr);

static void ne_reset_8390(struct net_device *dev);
static void ne_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr,
			  int ring_page);
static void ne_block_input(struct net_device *dev, int count,
			  struct sk_buff *skb, int ring_offset);
static void ne_block_output(struct net_device *dev, const int count,
		const unsigned char *buf, const int start_page);

static const struct net_device_ops ne_netdev_ops = {
	.ndo_open		= __ei_open,
	.ndo_stop		= __ei_close,
	.ndo_start_xmit		= __ei_start_xmit,
	.ndo_tx_timeout		= __ei_tx_timeout,
	.ndo_get_stats		= __ei_get_stats,
	.ndo_set_rx_mode	= __ei_set_multicast_list,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= eth_mac_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= __ei_poll,
#endif
};

static int ne_probe1(struct net_device *dev, unsigned long ioaddr)
{
	int i;
	unsigned char SA_prom[32];
	int wordlength = 2;
	const char *name = NULL;
	int start_page, stop_page;
	int neX000, ctron, copam, bad_card;
	int reg0, ret;
	struct ei_device *ei_local = netdev_priv(dev);

	if (!request_region(ioaddr, NE_IO_EXTENT, "ethernec"))
		return -EBUSY;

	reg0 = ei_inb(ioaddr);
	if (reg0 == 0xFF) {
		ret = -ENODEV;
		goto err_out;
	}

	/* Do a preliminary verification that we have a 8390. */
	{
		int regd;
		ei_outb(E8390_NODMA+E8390_PAGE1+E8390_STOP, ioaddr + E8390_CMD);
		regd = ei_inb(ioaddr + 0x0d);
		ei_outb(0xff, ioaddr + 0x0d);
		ei_outb(E8390_NODMA+E8390_PAGE0, ioaddr + E8390_CMD);
		ei_inb(ioaddr + EN0_COUNTER0); /* Clear the counter by reading. */
		if (ei_inb(ioaddr + EN0_COUNTER0) != 0) {
			ei_outb(reg0, ioaddr);
			ei_outb(regd, ioaddr + 0x0d);	/* Restore the old values. */
			ret = -ENODEV;
			goto err_out;
		}
	}

	netdev_info(dev, "NE*000 ethercard probe at %#3lx:", ioaddr);

	/* A user with a poor card that fails to ack the reset, or that
	   does not have a valid 0x57,0x57 signature can still use this
	   without having to recompile. Specifying an i/o address along
	   with an otherwise unused dev->mem_end value of "0xBAD" will
	   cause the driver to skip these parts of the probe. */

	bad_card = ((dev->base_addr != 0) && (dev->mem_end == BAD));

	/* Reset card. Who knows what dain-bramaged state it was left in. */

	{
		unsigned long reset_start_time = jiffies;

		/* DON'T change these to inb_p/outb_p or reset will fail on clones. */
		ei_outb(ei_inb(ioaddr + NE_RESET), ioaddr + NE_RESET);

		while ((ei_inb(ioaddr + EN0_ISR) & ENISR_RESET) == 0)
		if (time_after(jiffies, reset_start_time + 2*HZ/100)) {
			if (bad_card) {
				pr_cont(" (warning: no reset ack)");
				break;
			} else {
				pr_cont(" not found (no reset ack).\n");
				ret = -ENODEV;
				goto err_out;
			}
		}

		ei_outb(0xff, ioaddr + EN0_ISR);		/* Ack all intr. */
	}

	/* Read the 16 bytes of station address PROM.
	   We must first initialize registers, similar to __NS8390_init(eifdev, 0).
	   We can't reliably read the SAPROM address without this.
	   (I learned the hard way!). */
	{
		struct {unsigned char value, offset; } program_seq[] =
		{
			{E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD}, /* Select page 0*/
			{0x48,	EN0_DCFG},	/* Set byte-wide (0x48) access. */
			{0x00,	EN0_RCNTLO},	/* Clear the count regs. */
			{0x00,	EN0_RCNTHI},
			{0x00,	EN0_IMR},	/* Mask completion irq. */
			{0xFF,	EN0_ISR},
			{E8390_RXOFF, EN0_RXCR},	/* 0x20  Set to monitor */
			{E8390_TXOFF, EN0_TXCR},	/* 0x02  and loopback mode. */
			{32,	EN0_RCNTLO},
			{0x00,	EN0_RCNTHI},
			{0x00,	EN0_RSARLO},	/* DMA starting at 0x0000. */
			{0x00,	EN0_RSARHI},
			{E8390_RREAD+E8390_START, E8390_CMD},
		};

		for (i = 0; i < ARRAY_SIZE(program_seq); i++)
			ei_outb(program_seq[i].value, ioaddr + program_seq[i].offset);

	}
	for(i = 0; i < 32 /*sizeof(SA_prom)*/; i+=2) {
		SA_prom[i] = ei_inb(ioaddr + NE_DATAPORT);
		SA_prom[i+1] = ei_inb(ioaddr + NE_DATAPORT);
		if (SA_prom[i] != SA_prom[i+1])
			wordlength = 1;
	}

	if (wordlength == 2)
	{
		for (i = 0; i < 16; i++)
			SA_prom[i] = SA_prom[i+i];
		/* We must set the 8390 for word mode. */
		ei_outb(DCR_VAL, ioaddr + EN0_DCFG);
		start_page = NESM_START_PG;

		/*
		 * Realtek RTL8019AS datasheet says that the PSTOP register
		 * shouldn't exceed 0x60 in 8-bit mode.
		 * This chip can be identified by reading the signature from
		 * the  remote byte count registers (otherwise write-only)...
		 */
		if ((DCR_VAL & 0x01) == 0 &&		/* 8-bit mode */
		    ei_inb(ioaddr + EN0_RCNTLO) == 0x50 &&
		    ei_inb(ioaddr + EN0_RCNTHI) == 0x70)
			stop_page = 0x60;
		else
			stop_page = NESM_STOP_PG;
	} else {
		start_page = NE1SM_START_PG;
		stop_page  = NE1SM_STOP_PG;
	}

	neX000 = (SA_prom[14] == 0x57  &&  SA_prom[15] == 0x57);
	ctron =  (SA_prom[0] == 0x00 && SA_prom[1] == 0x00 && SA_prom[2] == 0x1d);
	copam =  (SA_prom[14] == 0x49 && SA_prom[15] == 0x00);

	/* Set up the rest of the parameters. */
	if (neX000 || bad_card || copam) {
		name = (wordlength == 2) ? "NE2000" : "NE1000";
	}
	else if (ctron)
	{
		name = (wordlength == 2) ? "Ctron-8" : "Ctron-16";
		start_page = 0x01;
		stop_page = (wordlength == 2) ? 0x40 : 0x20;
	}

	if (dev->irq < 2)
	{
		unsigned long cookie = probe_irq_on();
		ei_outb(0x50, ioaddr + EN0_IMR);	/* Enable one interrupt. */
		ei_outb(0x00, ioaddr + EN0_RCNTLO);
		ei_outb(0x00, ioaddr + EN0_RCNTHI);
		ei_outb(E8390_RREAD+E8390_START, ioaddr); /* Trigger it... */
		mdelay(10);		/* wait 10ms for interrupt to propagate */
		ei_outb(0x00, ioaddr + EN0_IMR); 		/* Mask it again. */
		dev->irq = probe_irq_off(cookie);
		if (msg_enable & NETIF_MSG_PROBE)
			pr_cont(" autoirq is %d", dev->irq);
	} else if (dev->irq == 2)
		/* Fixup for users that don't know that IRQ 2 is really IRQ 9,
		   or don't know which one to set. */
		dev->irq = 9;

	if (! dev->irq) {
		pr_cont(" failed to detect IRQ line.\n");
		ret = -EAGAIN;
		goto err_out;
	}

	/* Snarf the interrupt now.  There's no point in waiting since we cannot
	   share and the board will usually be enabled. */
	ret = request_irq(dev->irq, __ei_interrupt, 0, name, dev);
	if (ret) {
		pr_cont(" unable to get IRQ %d (errno=%d).\n", dev->irq, ret);
		goto err_out;
	}

	dev->base_addr = ioaddr;

	eth_hw_addr_set(dev, SA_prom);

	pr_cont("%pM\n", dev->dev_addr);

	ei_status.name = name;
	ei_status.tx_start_page = start_page;
	ei_status.stop_page = stop_page;

	/* no 16-bit mode on ethernec */
	ei_status.word16 = 0;

	ei_status.rx_start_page = start_page + TX_PAGES;

	ei_status.reset_8390 = &ne_reset_8390;
	ei_status.block_input = &ne_block_input;
	ei_status.block_output = &ne_block_output;
	ei_status.get_8390_hdr = &ne_get_8390_hdr;

	__NS8390_init(dev, 0);

	ei_local->msg_enable = msg_enable;
	ret = register_netdev(dev);
	if (ret)
		goto out_irq;
	netdev_info(dev, "%s found at %#lx, using IRQ %d.\n",
		    name, ioaddr, dev->irq);
	return 0;

out_irq:
	free_irq(dev->irq, dev);
err_out:
	release_region(ioaddr, NE_IO_EXTENT);
	return ret;
}

/* Hard reset the card.  This used to pause for the same period that a
   8390 reset command required, but that shouldn't be necessary. */

static void ne_reset_8390(struct net_device *dev)
{
	unsigned long reset_start_time = jiffies;
	struct ei_device *ei_local = netdev_priv(dev);

	netif_dbg(ei_local, hw, dev, "resetting the 8390 t=%ld...\n", jiffies);

	/* DON'T change these to inb_p/outb_p or reset will fail on clones. */
	ei_outb(ei_inb(NE_BASE + NE_RESET), NE_BASE + NE_RESET);

	ei_status.txing = 0;
	ei_status.dmaing = 0;

	/* This check _should_not_ be necessary, omit eventually. */
	while ((ei_inb(NE_BASE+EN0_ISR) & ENISR_RESET) == 0)
		if (time_after(jiffies, reset_start_time + 2*HZ/100)) {
			netdev_err(dev, "ne_reset_8390() did not complete.\n");
			break;
		}
	ei_outb(ENISR_RESET, NE_BASE + EN0_ISR);	/* Ack intr. */
}

/* Grab the 8390 specific header. Similar to the block_input routine, but
   we don't need to be concerned with ring wrap as the header will be at
   the start of a page, so we optimize accordingly. */

static void ne_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr, int ring_page)
{
	int nic_base = dev->base_addr;

	/* This *shouldn't* happen. If it does, it's the last thing you'll see */

	if (ei_status.dmaing)
	{
		netdev_err(dev, "DMAing conflict in ne_get_8390_hdr "
			   "[DMAstat:%d][irqlock:%d].\n",
			   ei_status.dmaing, ei_status.irqlock);
		return;
	}

	ei_status.dmaing |= 0x01;
	ei_outb(E8390_NODMA+E8390_PAGE0+E8390_START, nic_base+ NE_CMD);
	ei_outb(sizeof(struct e8390_pkt_hdr), nic_base + EN0_RCNTLO);
	ei_outb(0, nic_base + EN0_RCNTHI);
	ei_outb(0, nic_base + EN0_RSARLO);		/* On page boundary */
	ei_outb(ring_page, nic_base + EN0_RSARHI);
	ei_outb(E8390_RREAD+E8390_START, nic_base + NE_CMD);

	ei_insb(NE_BASE + NE_DATAPORT, hdr, sizeof(struct e8390_pkt_hdr));

	ei_outb(ENISR_RDC, nic_base + EN0_ISR);	/* Ack intr. */
	ei_status.dmaing &= ~0x01;

	le16_to_cpus(&hdr->count);
}

/* Block input and output, similar to the Crynwr packet driver.  If you
   are porting to a new ethercard, look at the packet driver source for hints.
   The NEx000 doesn't share the on-board packet memory -- you have to put
   the packet out through the "remote DMA" dataport using outb. */

static void ne_block_input(struct net_device *dev, int count, struct sk_buff *skb, int ring_offset)
{
	int nic_base = dev->base_addr;
	char *buf = skb->data;

	/* This *shouldn't* happen. If it does, it's the last thing you'll see */
	if (ei_status.dmaing)
	{
		netdev_err(dev, "DMAing conflict in ne_block_input "
			   "[DMAstat:%d][irqlock:%d].\n",
			   ei_status.dmaing, ei_status.irqlock);
		return;
	}
	ei_status.dmaing |= 0x01;
	ei_outb(E8390_NODMA+E8390_PAGE0+E8390_START, nic_base+ NE_CMD);
	ei_outb(count & 0xff, nic_base + EN0_RCNTLO);
	ei_outb(count >> 8, nic_base + EN0_RCNTHI);
	ei_outb(ring_offset & 0xff, nic_base + EN0_RSARLO);
	ei_outb(ring_offset >> 8, nic_base + EN0_RSARHI);
	ei_outb(E8390_RREAD+E8390_START, nic_base + NE_CMD);
	ei_insb(NE_BASE + NE_DATAPORT, buf, count);
	ei_outb(ENISR_RDC, nic_base + EN0_ISR);	/* Ack intr. */
	ei_status.dmaing &= ~0x01;
}

static void ne_block_output(struct net_device *dev, int count,
		const unsigned char *buf, const int start_page)
{
	int nic_base = NE_BASE;
	unsigned long dma_start;

	/* This *shouldn't* happen. If it does, it's the last thing you'll see */
	if (ei_status.dmaing)
	{
		netdev_err(dev, "DMAing conflict in ne_block_output."
			   "[DMAstat:%d][irqlock:%d]\n",
			   ei_status.dmaing, ei_status.irqlock);
		return;
	}
	ei_status.dmaing |= 0x01;
	/* We should already be in page 0, but to be safe... */
	ei_outb(E8390_PAGE0+E8390_START+E8390_NODMA, nic_base + NE_CMD);

	ei_outb(ENISR_RDC, nic_base + EN0_ISR);

	/* Now the normal output. */
	ei_outb(count & 0xff, nic_base + EN0_RCNTLO);
	ei_outb(count >> 8,   nic_base + EN0_RCNTHI);
	ei_outb(0x00, nic_base + EN0_RSARLO);
	ei_outb(start_page, nic_base + EN0_RSARHI);

	ei_outb(E8390_RWRITE+E8390_START, nic_base + NE_CMD);
	ei_outsb(NE_BASE + NE_DATAPORT, buf, count);

	dma_start = jiffies;

	while ((ei_inb(nic_base + EN0_ISR) & ENISR_RDC) == 0)
		if (time_after(jiffies, dma_start + 2*HZ/100)) {		/* 20ms */
			netdev_warn(dev, "timeout waiting for Tx RDC.\n");
			ne_reset_8390(dev);
			__NS8390_init(dev, 1);
			break;
		}

	ei_outb(ENISR_RDC, nic_base + EN0_ISR);	/* Ack intr. */
	ei_status.dmaing &= ~0x01;
}

static int ne_drv_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	int err;
	struct resource *res;

	dev = ____alloc_ei_netdev(0);
	if (!dev)
		return -ENOMEM;
	dev->netdev_ops = &ne_netdev_ops;

	/* ne.c doesn't populate resources in platform_device, but
	 * rbtx4927_ne_init and rbtx4938_ne_init do register devices
	 * with resources.
	 */
	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	if (res) {
		dev->base_addr = res->start;
		dev->irq = platform_get_irq(pdev, 0);
	}
	SET_NETDEV_DEV(dev, &pdev->dev);
	err = ne_probe1(dev, dev->base_addr);
	if (err) {
		free_netdev(dev);
		return err;
	}
	platform_set_drvdata(pdev, dev);

	return 0;
}

static void ne_drv_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	if (dev) {
		netif_device_detach(dev);
		unregister_netdev(dev);
		/* Careful ne_drv_remove can be called twice, once from
		 * the platform_driver.remove and again when the
		 * platform_device is being removed.
		 */
		free_irq(dev->irq, dev);
		release_region(dev->base_addr, NE_IO_EXTENT);
		free_netdev(dev);
	}
}

#ifdef CONFIG_PM
static int ne_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	if (netif_running(dev)) {
		netif_device_detach(dev);
	}
	return 0;
}

static int ne_drv_resume(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	if (netif_running(dev)) {
		ne_reset_8390(dev);
		__NS8390_init(dev, 1);
		netif_device_attach(dev);
	}
	return 0;
}
#else
#define ne_drv_suspend NULL
#define ne_drv_resume NULL
#endif

static struct platform_driver ethernec_driver = {
	.probe		= ne_drv_probe,
	.remove		= ne_drv_remove,
	.suspend	= ne_drv_suspend,
	.resume		= ne_drv_resume,
	.driver		= {
		.name	= "ethernec",
	},
};
module_platform_driver(ethernec_driver);
