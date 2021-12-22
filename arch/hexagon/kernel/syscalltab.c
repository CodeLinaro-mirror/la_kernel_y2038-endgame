// SPDX-License-Identifier: GPL-2.0-only
/*
 * System call table for Hexagon
 *
 * Copyright (c) 2010-2011, The Linux Foundation. All rights reserved.
 */

#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>

#include <asm/syscall.h>

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

/* hexagon libc passes PAGE_SIZE units rather than the usual 4K */
#define sys_mmap2 sys_mmap_pgoff
SYSCALL_DEFINE6(mmap_pgoff, unsigned long, addr, unsigned long, len,
		long, prot, unsigned long, flags,
		long, fd, unsigned long, pgoff)
{
      return ksys_mmap_pgoff(addr, len, prot, flags, fd, pgoff);
}

void *sys_call_table[__NR_syscalls] = {
#include <asm/unistd.h>
};
