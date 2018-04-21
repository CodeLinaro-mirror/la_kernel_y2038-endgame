/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_GENERIC_COMPAT_H
#define __ASM_GENERIC_COMPAT_H

/* These types are common across all compat ABIs */
typedef u32 compat_size_t;
typedef s32 compat_ssize_t;
typedef s32 compat_clock_t;
typedef s32 compat_pid_t;
typedef u32 compat_ino_t;
typedef s32 compat_off_t;
typedef s64 compat_loff_t;
typedef s32 compat_daddr_t;
typedef s32 compat_timer_t;
typedef s32 compat_key_t;
typedef s16 compat_short_t;
typedef s32 compat_int_t;
typedef s32 compat_long_t;
typedef u16 compat_ushort_t;
typedef u32 compat_uint_t;
typedef u32 compat_ulong_t;
typedef u32 compat_uptr_t;
typedef u32 compat_aio_context_t;

/*
 * A pointer passed in from user mode. This should not
 * be used for syscall parameters, just declare them
 * as pointers because the syscall entry code will have
 * appropriately converted them already.
 */
#ifndef compat_ptr
static inline void __user *compat_ptr(compat_uptr_t uptr)
{
	return (void __user *)(unsigned long)uptr;
}
#endif

#ifndef ptr_to_compat
static inline compat_uptr_t ptr_to_compat(void __user *uptr)
{
	return (u32)(unsigned long)uptr;
}
#endif

#endif
