/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_SH_SYSCALLS_H
#define __ASM_SH_SYSCALLS_H

asmlinkage int old_mmap(unsigned long addr, unsigned long len,
			unsigned long prot, unsigned long flags,
			int fd, unsigned long off);

#include <asm/syscalls_32.h>

#endif /* __ASM_SH_SYSCALLS_H */
