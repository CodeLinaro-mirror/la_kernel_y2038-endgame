/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_TYPES_H
#define _UAPI_LINUX_TYPES_H

#include <asm/types.h>

#ifndef __ASSEMBLY__
#ifndef	__KERNEL__
#ifndef __EXPORTED_HEADERS__
#warning "Attempt to use kernel headers from user space, see https://kernelnewbies.org/KernelHeaders"
#endif /* __EXPORTED_HEADERS__ */
#endif

#include <linux/posix_types.h>

#ifdef __SIZEOF_INT128__
typedef __signed__ __int128 __s128 __attribute__((aligned(16)));
typedef unsigned __int128 __u128 __attribute__((aligned(16)));
#endif

/*
 * Below are truly Linux-specific types that should never collide with
 * any application/library that wants linux/types.h.
 */

/* sparse defines __CHECKER__; see Documentation/dev-tools/sparse.rst */
#ifdef __CHECKER__
#define __bitwise	__attribute__((bitwise))
#else
#define __bitwise
#endif

/* The kernel doesn't use this legacy form, but user space does */
#define __bitwise__ __bitwise

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

/*
 * aligned_u64 should be used in defining kernel<->userspace ABIs to avoid
 * common 32/64-bit compat problems.
 * 64-bit values align to 4-byte boundaries on x86_32 (and possibly other
 * architectures) and to 8-byte boundaries on 64-bit architectures.  The new
 * aligned_64 type enforces 8-byte alignment so that structs containing
 * aligned_64 values have the same alignment on 32-bit and 64-bit architectures.
 * No conversions are necessary between 32-bit user-space and a 64-bit kernel.
 */
#define __aligned_u64 __u64 __attribute__((aligned(8)))
#define __aligned_s64 __s64 __attribute__((aligned(8)))
#define __aligned_be64 __be64 __attribute__((aligned(8)))
#define __aligned_le64 __le64 __attribute__((aligned(8)))

typedef unsigned __bitwise __poll_t;

/*
 * Annotations for padding in uapi structures:
 * - all architectures align 16-bit members naturally
 * - all except m68k align 32-bit members
 * - all 64-bit architectures and most 32-bit ones align 64-bit members
 *
 * structures that have holes due to natural alignment should use these
 * helpers to insert anonymous padding on architectures that need it.
 *
 * architectures must override __uapi_arch_pad{16,32} to skip the
 * padding according to their ABI.
 */
#define __uapi_arch_pad8	__u8 :8
#ifndef __uapi_arch_pad16
#define __uapi_arch_pad16	__u16 :16
#endif
#ifndef __uapi_arch_pad32
#define __uapi_arch_pad32	__u32 :32
#endif

/*
 * Padding that is different between 32-bit and 64-bit targets,
 * resulting from sizeof(long):
 *
 * - between a __u32 and a long/size_t/pointer
 * - between a long and a __u64
 * - between a long and an __aligned_u64
 */
#if __BITS_PER_LONG == 64
#define __uapi_arch_pad_long			__uapi_arch_pad32
#define __uapi_arch_pad_long_to_u64
#define __uapi_arch_pad_long_to_aligned_u64
#else
#define __uapi_arch_pad_long
#define __uapi_arch_pad_long_to_u64		__uapi_arch_pad32
#define __uapi_arch_pad_long_to_aligned_u64	__u32 :32
#endif

#ifndef __uapi_arch_align
#if 0
#define __uapi_arch_align __attribute__((aligned(sizeof(__u32))))
#endif
#define __uapi_arch_align
#endif

#endif /*  __ASSEMBLY__ */
#endif /* _UAPI_LINUX_TYPES_H */
