#
# Makefile for code common across different PXA processor families
#
ccflags-$(CONFIG_ARCH_MMP) := -I$(srctree)/$(src)/include \
	-I$(srctree)/arch/arm/mach-mmp/include

obj-y	:= dma.o

obj-$(CONFIG_PXA3xx)		+= mfp.o
obj-$(CONFIG_ARCH_MMP)		+= mfp.o

obj-$(CONFIG_PXA_SSP)		+= ssp.o
