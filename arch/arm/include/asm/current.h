/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021 Keith Packard <keithp@keithp.com>
 * Copyright (c) 2021 Google, LLC <ardb@kernel.org>
 */

#ifndef _ASM_ARM_CURRENT_H
#define _ASM_ARM_CURRENT_H

#ifndef __ASSEMBLY__
#include <asm/insn.h>

struct task_struct;

extern struct task_struct *__current;

static __always_inline __attribute_const__ struct task_struct *get_current(void)
{
	struct task_struct *cur;

#if __has_builtin(__builtin_thread_pointer) && defined(CONFIG_CURRENT_POINTER_IN_TPIDRURO)
	/*
	 * Use the __builtin helper when available - this results in better
	 * code, especially when using GCC in combination with the per-task
	 * stack protector, as the compiler will recognize that it needs to
	 * load the TLS register only once in every function.
	 */
	cur = __builtin_thread_pointer();
#elif defined(CONFIG_CURRENT_POINTER_IN_TPIDRURO) || defined(CONFIG_SMP)
	asm("0:	mrc p15, 0, %0, c13, c0, 3			\n\t"
	    : "=r"(cur));
#elif __LINUX_ARM_ARCH__>= 7 || \
      !defined(CONFIG_ARM_HAS_GROUP_RELOCS) || \
      (defined(MODULE) && defined(CONFIG_ARM_MODULE_PLTS))
	cur = __current;
#else
	asm(LOAD_SYM_ARMV6(%0, __current) : "=r"(cur));
#endif
	return cur;
}

#define current get_current()

#endif /* __ASSEMBLY__ */

#endif /* _ASM_ARM_CURRENT_H */
