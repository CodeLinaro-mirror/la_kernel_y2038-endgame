#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y				+= mfp.o irq.o time.o gpio.o dev.o cpu.o

ifndef CONFIG_COMMON_CLK
obj-y				+= clksel.o clock.o
endif

# W90X900 CPU support files

obj-$(CONFIG_CPU_W90P910)	+= nuc910.o
obj-$(CONFIG_CPU_NUC950)	+= nuc950.o
obj-$(CONFIG_CPU_NUC960)	+= nuc960.o

# machine support

obj-$(CONFIG_MACH_W90P910EVB)	+= mach-nuc910evb.o
obj-$(CONFIG_MACH_W90P950EVB)	+= mach-nuc950evb.o
obj-$(CONFIG_MACH_W90N960EVB)	+= mach-nuc960evb.o
obj-$(CONFIG_SOC_NUC970)	+= nuc900.o
