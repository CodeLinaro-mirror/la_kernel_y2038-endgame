.. SPDX-License-Identifier: GPL-2.0

===============================
TI EMIF SDRAM Controller Driver
===============================

Author
======
Aneesh V <aneesh@ti.com>

Location
========
driver/memory/emif.c

Supported SoCs:
===============
TI OMAP44xx
TI OMAP54xx

Menuconfig option:
==================
Device Drivers
	Memory devices
		Texas Instruments EMIF driver

Description
===========
This driver is for the EMIF module available in Texas Instruments
SoCs. EMIF is an SDRAM controller that, based on its revision,
supports one or more of DDR2, DDR3, and LPDDR2 SDRAM protocols.
This driver takes care of only LPDDR2 memories presently. The
functions of the driver includes re-configuring AC timing
parameters and other settings during frequency, voltage and
temperature changes

Interface to the external world
===============================
EMIF driver registers notifiers for voltage and frequency changes
affecting EMIF and takes appropriate actions when these are invoked.

- freq_pre_notify_handling()
- freq_post_notify_handling()
- volt_notify_handling()

Debugfs
=======
The driver creates two debugfs entries per device.

- regcache_dump : dump of register values calculated and saved for all
  frequencies used so far.
- mr4 : last polled value of MR4 register in the LPDDR2 device. MR4
  indicates the current temperature level of the device.
