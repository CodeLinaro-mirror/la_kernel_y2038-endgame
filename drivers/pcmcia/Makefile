# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the kernel pcmcia subsystem (c/o David Hinds)
#

pcmcia_core-y					+= cs.o socket_sysfs.o
pcmcia_core-$(CONFIG_CARDBUS)			+= cardbus.o
obj-$(CONFIG_PCCARD)				+= pcmcia_core.o

pcmcia-y					+= ds.o pcmcia_resource.o cistpl.o pcmcia_cis.o
obj-$(CONFIG_PCMCIA)				+= pcmcia.o

pcmcia_rsrc-y					+= rsrc_mgr.o
pcmcia_rsrc-$(CONFIG_PCCARD_NONSTATIC)		+= rsrc_nonstatic.o
pcmcia_rsrc-$(CONFIG_PCCARD_IODYN)		+= rsrc_iodyn.o
obj-$(CONFIG_PCCARD)				+= pcmcia_rsrc.o


# socket drivers

obj-$(CONFIG_YENTA) 				+= yenta_socket.o

obj-$(CONFIG_PD6729)				+= pd6729.o
obj-$(CONFIG_I82365)				+= i82365.o
obj-$(CONFIG_I82092)				+= i82092.o
obj-$(CONFIG_TCIC)				+= tcic.o
obj-$(CONFIG_PCMCIA_SOC_COMMON)			+= soc_common.o
obj-$(CONFIG_OMAP_CF)				+= omap_cf.o
obj-$(CONFIG_ELECTRA_CF)			+= electra_cf.o
obj-$(CONFIG_PCMCIA_ALCHEMY_DEVBOARD)		+= db1xxx_ss.o
obj-$(CONFIG_PCMCIA_MAX1600)			+= max1600.o
obj-$(CONFIG_PCMCIA_XXS1500)			+= xxs1500_ss.o
