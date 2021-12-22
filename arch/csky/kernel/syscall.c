// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Hangzhou C-SKY Microsystems co.,ltd.

#include <linux/syscalls.h>

SYSCALL_DEFINE1(set_thread_area, unsigned long, addr)
{
	struct thread_info *ti = task_thread_info(current);
	struct pt_regs *reg = current_pt_regs();

	reg->tls = addr;
	ti->tp_value = addr;

	return 0;
}

/*
 * for abiv1 the 64bits args should be even th, So we need mov the advice
 * forward.
 */
SYSCALL_DEFINE4(csky_fadvise64_64,
	int, fd,
	int, advice,
	loff_t, offset,
	loff_t, len)
{
	return ksys_fadvise64_64(fd, offset, len, advice);
}
