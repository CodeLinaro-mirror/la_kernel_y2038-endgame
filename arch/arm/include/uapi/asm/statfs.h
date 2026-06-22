/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASMARM_STATFS_H
#define _ASMARM_STATFS_H

#include <linux/types.h>

/*
 * With EABI there is 4 bytes of padding added to this structure.
 * Let's pack it so the padding goes away to simplify dual ABI support.
 * Note that user space does NOT have to pack this structure.
 */
#define ARCH_PACK_STATFS64 __attribute__((packed,aligned(4)))
#define __uapi_arch_pad_statfs64

#include <asm-generic/statfs.h>
#endif
