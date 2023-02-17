// SPDX-License-Identifier: GPL-2.0-only

#define __SYSCALL_COMPAT

#include <linux/compat.h>
#include <linux/syscalls.h>
#include <asm-generic/mman-common.h>
#include <asm-generic/syscalls.h>
#include <asm/syscall.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)	asmlinkage long __riscv_##call(const struct pt_regs *);
#include <asm/unistd.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)      [nr] = __riscv_##call,

asmlinkage long compat_sys_rt_sigreturn(void);

__diag_ignore(GCC, 5, "-Woverride-init", "default to sys_ni_syscall")
__diag_ignore(clang, 9, "-Winitializer-overrides", "default to sys_ni_syscall")

void * const compat_sys_call_table[__NR_syscalls] = {
	[0 ... __NR_syscalls - 1] = __riscv_sys_ni_syscall,
#include <asm/unistd.h>
};
