# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_DAMON)		:= core.o
obj-$(CONFIG_DAMON_VADDR)	+= vaddr.o vaddr-test.o
CFLAGS_vaddr-test.o		+= $(DISABLE_STRUCTLEAK_PLUGIN)
obj-$(CONFIG_DAMON_DBGFS)	+= dbgfs.o
