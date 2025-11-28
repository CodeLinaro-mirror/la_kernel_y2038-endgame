/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ALPHA_STATFS_H
#define _ALPHA_STATFS_H

#include <linux/types.h>

/* Alpha is the only 64-bit platform with 32-bit statfs. And doesn't
   even seem to implement statfs64 */
#define __statfs_word __u32
#define __uapi_arch_pad_statfs64 __uapi_arch_pad32

#include <asm-generic/statfs.h>

#endif
