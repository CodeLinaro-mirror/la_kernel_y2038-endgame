#
# Makefile for the Linux kernel device drivers.
#
# 15 Sep 2000, Christoph Hellwig <hch@infradead.org>
# Rewritten to use lists instead of if-statements.
#

drivers-y				+= drivers/irqchip/
drivers-y				+= drivers/bus/

drivers-$(CONFIG_GENERIC_PHY)	+= drivers/phy/

# GPIO must come after pinctrl as gpios may need to mux pins etc
drivers-$(CONFIG_PINCTRL)		+= drivers/pinctrl/
drivers-y				+= drivers/gpio/
drivers-y				+= drivers/pwm/
drivers-$(CONFIG_PCI)		+= drivers/pci/
drivers-$(CONFIG_PARISC)		+= drivers/parisc/
drivers-$(CONFIG_RAPIDIO)		+= drivers/rapidio/
drivers-y				+= drivers/video/
drivers-y				+= drivers/idle/

# IPMI must come before ACPI in order to provide IPMI opregion support
drivers-$(CONFIG_IPMI_HANDLER)	+= drivers/char/ipmi/

drivers-$(CONFIG_ACPI)		+= drivers/acpi/
drivers-$(CONFIG_SFI)		+= drivers/sfi/
# PnP must come after ACPI since it will eventually need to check if acpi
# was used and do nothing if so
drivers-$(CONFIG_PNP)		+= drivers/pnp/
drivers-y				+= drivers/amba/
# Many drivers will want to use DMA so this has to be made available
# really early.
drivers-$(CONFIG_DMADEVICES)	+= drivers/dma/

# SOC specific infrastructure drivers.
drivers-y				+= drivers/soc/

drivers-$(CONFIG_VIRTIO)		+= drivers/virtio/
drivers-$(CONFIG_XEN)		+= drivers/xen/

# regulators early, since some subsystems rely on them to initialize
drivers-$(CONFIG_REGULATOR)		+= drivers/regulator/

# reset controllers early, since gpu drivers might rely on them to initialize
drivers-$(CONFIG_RESET_CONTROLLER)	+= drivers/reset/

# tty/ comes before char/ so that the VT console is the boot-time
# default.
drivers-y				+= drivers/tty/
drivers-y				+= drivers/char/

# iommu/ comes before gpu as gpu are using iommu controllers
drivers-$(CONFIG_IOMMU_SUPPORT)	+= drivers/iommu/

# gpu/ comes after char for AGP vs DRM startup and after iommu
drivers-y				+= drivers/gpu/

drivers-$(CONFIG_CONNECTOR)		+= drivers/connector/

# i810fb and intelfb depend on char/agp/
drivers-$(CONFIG_FB_I810)           += drivers/video/fbdev/i810/
drivers-$(CONFIG_FB_INTEL)          += drivers/video/fbdev/intelfb/

drivers-$(CONFIG_PARPORT)		+= drivers/parport/
drivers-$(CONFIG_NVM)			+= drivers/lightnvm/
drivers-y				+= drivers/base/
drivers-y				+= drivers/block/
drivers-y				+= drivers/misc/
drivers-y				+= drivers/mfd/
drivers-y				+= drivers/nfc/
drivers-$(CONFIG_LIBNVDIMM)		+= drivers/nvdimm/
drivers-$(CONFIG_DMA_SHARED_BUFFER) += drivers/dma-buf/
drivers-$(CONFIG_NUBUS)		+= drivers/nubus/
drivers-y				+= drivers/macintosh/
drivers-$(CONFIG_IDE)		+= drivers/ide/
drivers-$(CONFIG_SCSI)		+= drivers/scsi/
drivers-y				+= drivers/nvme/
drivers-$(CONFIG_ATA)		+= drivers/ata/
drivers-$(CONFIG_TARGET_CORE)	+= drivers/target/
drivers-$(CONFIG_MTD)		+= drivers/mtd/
drivers-$(CONFIG_SPI)		+= drivers/spi/
drivers-$(CONFIG_SPMI)		+= drivers/spmi/
drivers-y				+= drivers/hsi/
drivers-y				+= drivers/net/
drivers-$(CONFIG_ATM)		+= drivers/atm/
drivers-$(CONFIG_FUSION)		+= drivers/message/
drivers-y				+= drivers/firewire/
drivers-$(CONFIG_UIO)		+= drivers/uio/
drivers-$(CONFIG_VFIO)		+= drivers/vfio/
drivers-y				+= drivers/cdrom/
drivers-y				+= drivers/auxdisplay/
drivers-$(CONFIG_PCCARD)		+= drivers/pcmcia/
drivers-$(CONFIG_DIO)		+= drivers/dio/
drivers-$(CONFIG_SBUS)		+= drivers/sbus/
drivers-$(CONFIG_ZORRO)		+= drivers/zorro/
drivers-$(CONFIG_ATA_OVER_ETH)	+= drivers/block/aoe/
drivers-$(CONFIG_PARIDE) 		+= drivers/block/paride/
drivers-$(CONFIG_TC)		+= drivers/tc/
drivers-$(CONFIG_UWB)		+= drivers/uwb/
drivers-y			+= drivers/usb/
drivers-$(CONFIG_SERIO)		+= drivers/input/serio/
drivers-$(CONFIG_GAMEPORT)		+= drivers/input/gameport/
drivers-$(CONFIG_INPUT)		+= drivers/input/
drivers-$(CONFIG_RTC_LIB)		+= drivers/rtc/
drivers-y				+= drivers/i2c/
drivers-y				+= drivers/media/
drivers-$(CONFIG_PPS)		+= drivers/pps/
drivers-$(CONFIG_PTP_1588_CLOCK)	+= drivers/ptp/
drivers-$(CONFIG_W1)		+= drivers/w1/
drivers-y				+= drivers/power/
drivers-$(CONFIG_HWMON)		+= drivers/hwmon/
drivers-$(CONFIG_THERMAL)		+= drivers/thermal/
drivers-$(CONFIG_WATCHDOG)		+= drivers/watchdog/
drivers-$(CONFIG_MD)		+= drivers/md/
drivers-$(CONFIG_BT)		+= drivers/bluetooth/
drivers-$(CONFIG_ACCESSIBILITY)	+= drivers/accessibility/
drivers-$(CONFIG_ISDN)		+= drivers/isdn/
drivers-$(CONFIG_EDAC)		+= drivers/edac/
drivers-$(CONFIG_EISA)		+= drivers/eisa/
drivers-y				+= drivers/lguest/
drivers-$(CONFIG_CPU_FREQ)		+= drivers/cpufreq/
drivers-$(CONFIG_CPU_IDLE)		+= drivers/cpuidle/
drivers-y				+= drivers/mmc/
drivers-$(CONFIG_MEMSTICK)		+= drivers/memstick/
drivers-y				+= drivers/leds/
drivers-$(CONFIG_INFINIBAND)	+= drivers/infiniband/
drivers-$(CONFIG_SGI_SN)		+= drivers/sn/
drivers-y				+= drivers/firmware/
drivers-$(CONFIG_CRYPTO)		+= drivers/crypto/
drivers-$(CONFIG_SUPERH)		+= drivers/sh/
drivers-$(CONFIG_ARCH_SHMOBILE)	+= drivers/sh/
ifndef CONFIG_ARCH_USES_GETTIMEOFFSET
drivers-y				+= drivers/clocksource/
endif
drivers-$(CONFIG_DCA)		+= drivers/dca/
drivers-$(CONFIG_HID)		+= drivers/hid/
drivers-$(CONFIG_PPC_PS3)		+= drivers/ps3/
drivers-$(CONFIG_OF)		+= drivers/of/
drivers-$(CONFIG_SSB)		+= drivers/ssb/
drivers-$(CONFIG_BCMA)		+= drivers/bcma/
drivers-$(CONFIG_VHOST_RING)	+= drivers/vhost/
drivers-$(CONFIG_VLYNQ)		+= drivers/vlynq/
drivers-$(CONFIG_STAGING)		+= drivers/staging/
drivers-y				+= drivers/platform/
#common clk code
drivers-y				+= drivers/clk/

drivers-$(CONFIG_MAILBOX)		+= drivers/mailbox/
drivers-$(CONFIG_HWSPINLOCK)	+= drivers/hwspinlock/
drivers-$(CONFIG_REMOTEPROC)	+= drivers/remoteproc/
drivers-$(CONFIG_RPMSG)		+= drivers/rpmsg/

# Virtualization drivers
drivers-$(CONFIG_VIRT_DRIVERS)	+= drivers/virt/
drivers-$(CONFIG_HYPERV)		+= drivers/hv/

drivers-$(CONFIG_PM_DEVFREQ)	+= drivers/devfreq/
drivers-$(CONFIG_EXTCON)		+= drivers/extcon/
drivers-$(CONFIG_MEMORY)		+= drivers/memory/
drivers-$(CONFIG_IIO)		+= drivers/iio/
drivers-$(CONFIG_VME_BUS)		+= drivers/vme/
drivers-$(CONFIG_IPACK_BUS)		+= drivers/ipack/
drivers-$(CONFIG_NTB)		+= drivers/ntb/
drivers-$(CONFIG_FMC)		+= drivers/fmc/
drivers-$(CONFIG_POWERCAP)		+= drivers/powercap/
drivers-$(CONFIG_MCB)		+= drivers/mcb/
drivers-$(CONFIG_PERF_EVENTS)	+= drivers/perf/
drivers-$(CONFIG_RAS)		+= drivers/ras/
drivers-$(CONFIG_THUNDERBOLT)	+= drivers/thunderbolt/
drivers-$(CONFIG_CORESIGHT)		+= drivers/hwtracing/coresight/
drivers-y				+= drivers/hwtracing/intel_th/
drivers-$(CONFIG_STM)		+= drivers/hwtracing/stm/
drivers-$(CONFIG_ANDROID)		+= drivers/android/
drivers-$(CONFIG_NVMEM)		+= drivers/nvmem/
drivers-$(CONFIG_FPGA)		+= drivers/fpga/
