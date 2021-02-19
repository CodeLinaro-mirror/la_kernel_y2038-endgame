# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the linux ipc.
#

obj-$(CONFIG_SYSVIPC_COMPAT) += compat.o
obj-$(CONFIG_SYSVIPC) += util.o msgutil.o msg.o sem.o shm.o
ifdef CONFIG_SYSVIPC
obj-$(CONFIG_ARCH_WANT_OLD_IPC) += syscall.o
obj-$(CONFIG_ARCH_WANT_OLD_COMPAT_IPC) += syscall.o
endif
obj-$(CONFIG_SYSVIPC_SYSCTL) += ipc_sysctl.o
obj-$(CONFIG_POSIX_MQUEUE) += mqueue.o msgutil.o
obj-$(CONFIG_IPC_NS) += namespace.o
obj-$(CONFIG_POSIX_MQUEUE_SYSCTL) += mq_sysctl.o

