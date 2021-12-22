// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2009 Arnd Bergmann <arnd@arndb.de>
 * Copyright (C) 2012 Regents of the University of California
 */

#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <asm-generic/syscalls.h>
#include <asm/syscall.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)	asmlinkage long __riscv_##call(const struct pt_regs *);

#define __riscv_sys_mmap  __riscv_sys_riscv_mmap
#define __riscv_sys_mmap2 __riscv_sys_riscv_mmap2

#include <asm/unistd.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)	[nr] = __riscv_##call,

__diag_ignore(GCC, 8, "-Woverride-init", "default to sys_ni_syscall")
__diag_ignore(clang, 9, "-Winitializer-overrides", "default to sys_ni_syscall")

void * const sys_call_table[__NR_syscalls] = {
	[0 ... __NR_syscalls - 1] = __riscv_sys_ni_syscall,

#include <asm/unistd.h>
};
