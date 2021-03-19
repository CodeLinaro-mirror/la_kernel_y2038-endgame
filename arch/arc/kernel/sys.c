// SPDX-License-Identifier: GPL-2.0

#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>

#include <asm/syscalls.h>

#define sys_clone	sys_clone_wrapper
#define sys_clone3	sys_clone3_wrapper

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

__diag_ignore(GCC, 5, "-Woverride-init", "default to sys_ni_syscall")
__diag_ignore(clang, 9, "-Winitializer-overrides", "default to sys_ni_syscall")

void *sys_call_table[NR_syscalls] = {
	[0 ... NR_syscalls-1] = sys_ni_syscall,
#include <asm/unistd.h>
};
