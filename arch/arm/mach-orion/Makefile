# SPDX-License-Identifier: GPL-2.0-only

orion-gpio-$(CONFIG_GPIOLIB)		+= gpio.o
obj-$(CONFIG_PLAT_ORION_LEGACY)		+= irq.o time.o common.o mpp.o
obj-$(CONFIG_PLAT_ORION_LEGACY)		+= $(orion-gpio-y)

ifdef CONFIG_PCI
obj-$(CONFIG_PLAT_ORION_LEGACY)		+= pcie.o
obj-$(CONFIG_ARCH_DOVE)			+= dove-pcie.o
obj-$(CONFIG_ARCH_MV78XX0)		+= mv78xx0-pcie.o
obj-$(CONFIG_ARCH_ORION5X)		+= orion5x-pci.o
endif

obj-$(CONFIG_ARCH_DOVE)			+= dove-common.o dove-irq.o dove-mpp.o
obj-$(CONFIG_MACH_DOVE_DB)		+= dove-db-setup.o
obj-$(CONFIG_MACH_CM_A510)		+= cm-a510.o

obj-$(CONFIG_ARCH_MV78XX0)		+= mv78xx0-common.o mv78xx0-mpp.o mv78xx0-irq.o
obj-$(CONFIG_MACH_DB78X00_BP)		+= db78x00-bp-setup.o
obj-$(CONFIG_MACH_RD78X00_MASA)		+= rd78x00-masa-setup.o
obj-$(CONFIG_MACH_TERASTATION_WXL)	+= buffalo-wxl-setup.o

obj-$(CONFIG_ARCH_ORION5X)		+= orion5x.o orion5x-irq.o orion5x-mpp.o
obj-$(CONFIG_MACH_DB88F5281)		+= db88f5281-setup.o
obj-$(CONFIG_MACH_RD88F5182)		+= rd88f5182-setup.o
obj-$(CONFIG_MACH_KUROBOX_PRO)		+= kurobox_pro-setup.o
obj-$(CONFIG_MACH_TERASTATION_PRO2)	+= terastation_pro2-setup.o
obj-$(CONFIG_MACH_LINKSTATION_PRO)	+= kurobox_pro-setup.o
obj-$(CONFIG_MACH_LINKSTATION_LS_HGL)	+= ls_hgl-setup.o
obj-$(CONFIG_MACH_DNS323)		+= dns323-setup.o
obj-$(CONFIG_MACH_TS209)		+= ts209-setup.o tsx09-common.o
obj-$(CONFIG_MACH_TS409)		+= ts409-setup.o tsx09-common.o
obj-$(CONFIG_MACH_WRT350N_V2)		+= wrt350n-v2-setup.o
obj-$(CONFIG_MACH_TS78XX)		+= ts78xx-setup.o
obj-$(CONFIG_MACH_MV2120)		+= mv2120-setup.o
obj-$(CONFIG_MACH_NET2BIG)		+= net2big-setup.o
obj-$(CONFIG_MACH_WNR854T)		+= wnr854t-setup.o
obj-$(CONFIG_MACH_RD88F5181L_GE)	+= rd88f5181l-ge-setup.o
obj-$(CONFIG_MACH_RD88F5181L_FXO)	+= rd88f5181l-fxo-setup.o
obj-$(CONFIG_MACH_RD88F6183AP_GE)	+= rd88f6183ap-ge-setup.o

obj-$(CONFIG_ARCH_ORION5X_DT)		+= board-dt.o
obj-$(CONFIG_MACH_D2NET_DT)		+= board-d2net.o
obj-$(CONFIG_MACH_MSS2_DT)		+= board-mss2.o
obj-$(CONFIG_MACH_RD88F5182_DT)		+= board-rd88f5182.o
