# SPDX-License-Identifier: GPL-2.0
ifdef CONFIG_PROC_FS
fdinfo-$(CONFIG_INOTIFY_USER)	+= fdinfo.o
fdinfo-$(CONFIG_FANOTIFY)	+= fdinfo.o
endif

obj-$(CONFIG_FSNOTIFY)		+= fsnotify.o notification.o group.o mark.o \
				   $(fdinfo-y)

obj-y			+= dnotify/
obj-y			+= inotify/
obj-y			+= fanotify/
