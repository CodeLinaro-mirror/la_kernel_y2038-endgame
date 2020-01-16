/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * linux/arch/unicore32/include/asm/uaccess.h
 *
 * Code specific to PKUnity SoC and UniCore ISA
 *
 * Copyright (C) 2001-2010 GUAN Xue-tao
 */
#ifndef __UNICORE_UACCESS_H__
#define __UNICORE_UACCESS_H__

#include <asm/memory.h>

#define strncpy_from_user	strncpy_from_user
#define strnlen_user		strnlen_user
#define __clear_user		__clear_user

#define __kernel_ok		(uaccess_kernel())
#define __user_ok(addr, size)	(((size) <= TASK_SIZE)			\
				&& ((addr) <= TASK_SIZE - (size)))
#define __access_ok(addr, size)	(__kernel_ok || __user_ok((addr), (size)))

extern unsigned long __must_check
raw_copy_from_user(void *to, const void __user *from, unsigned long n);
extern unsigned long __must_check
raw_copy_to_user(void __user *to, const void *from, unsigned long n);
extern unsigned long __must_check
__clear_user(void __user *addr, unsigned long n);
extern unsigned long __must_check
__strncpy_from_user(char *to, const char __user *from, unsigned long count);
extern unsigned long
__strnlen_user(const char __user *s, long n);

static inline long
strncpy_from_user(char *dst, const char __user *src, long count)
{
	if (!access_ok(src, 1))
		return -EFAULT;
	return __strncpy_from_user(dst, src, count);
}

/*
 * Unlike strnlen, strnlen_user includes the nul terminator in
 * its returned count. Callers should check for a returned value
 * greater than N as an indication the string is too long.
 */
static inline long strnlen_user(const char __user *src, long n)
{
	if (!access_ok(src, 1))
		return 0;
	return __strnlen_user(src, n);
}

#define INLINE_COPY_FROM_USER
#define INLINE_COPY_TO_USER

#include <asm-generic/uaccess.h>

#endif /* __UNICORE_UACCESS_H__ */
