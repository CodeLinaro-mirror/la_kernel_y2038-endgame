#
# Makefile for ATA over Ethernet
#

obj-$(CONFIG_ATA_OVER_ETH)	+= aoe.o
aoe-y := aoeblk.o aoechr.o aoecmd.o aoedev.o aoemain.o aoenet.o
ifdef CONFIG_CPU_32v3
CFLAGS_aoechr.o += -O0
endif
