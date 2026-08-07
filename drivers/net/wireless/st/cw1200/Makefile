# SPDX-License-Identifier: GPL-2.0
cw1200_wlan_sdio-y := cw1200_sdio.o \
		fwio.o \
		txrx.o \
		main.o \
		queue.o \
		hwio.o \
		bh.o \
		wsm.o \
		sta.o \
		scan.o \
		debug.o
cw1200_wlan_sdio-$(CONFIG_PM)	+= pm.o

# CFLAGS_sta.o += -DDEBUG


obj-$(CONFIG_CW1200) += cw1200_wlan_sdio.o
