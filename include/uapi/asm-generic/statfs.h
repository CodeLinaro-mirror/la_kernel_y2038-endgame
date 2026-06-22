/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_GENERIC_STATFS_H
#define _UAPI_GENERIC_STATFS_H

#include <linux/types.h>


/*
 * Most 64-bit platforms use 'long', while most 32-bit platforms use '__u32'.
 * Yes, they differ in signedness as well as size.
 * Special cases can override it for themselves -- except for S390x, which
 * is just a little too special for us. And MIPS, which I'm not touching
 * with a 10' pole.
 */
#ifndef __statfs_word
#if __BITS_PER_LONG == 64
#define __statfs_word __kernel_long_t
#else
#define __statfs_word __u32
#endif
#endif

struct statfs {
	__statfs_word f_type;
	__statfs_word f_bsize;
	__statfs_word f_blocks;
	__statfs_word f_bfree;
	__statfs_word f_bavail;
	__statfs_word f_files;
	__statfs_word f_ffree;
	__kernel_fsid_t f_fsid;
	__statfs_word f_namelen;
	__statfs_word f_frsize;
	__statfs_word f_flags;
	__statfs_word f_spare[4];
};

/*
 * ARM needs to avoid the 32-bit padding at the end, for consistency
 * between EABI and OABI 
 */
#ifndef ARCH_PACK_STATFS64
#define ARCH_PACK_STATFS64
#endif

/*
 * 32-bit architectures and alpha have 32-bit extra padding
 */
#ifndef __uapi_arch_pad_statfs64
#define __uapi_arch_pad_statfs64 __uapi_arch_pad_long_to_u64
#endif

struct statfs64 {
	__statfs_word f_type;
	__statfs_word f_bsize;
	__u64 f_blocks;
	__u64 f_bfree;
	__u64 f_bavail;
	__u64 f_files;
	__u64 f_ffree;
	__kernel_fsid_t f_fsid;
	__statfs_word f_namelen;
	__statfs_word f_frsize;
	__statfs_word f_flags;
	__statfs_word f_spare[4];
	__uapi_arch_pad_statfs64;
} ARCH_PACK_STATFS64;

/* 
 * IA64 and x86_64 need to avoid the 32-bit padding at the end,
 * to be compatible with the i386 ABI
 */
#ifndef ARCH_PACK_COMPAT_STATFS64
#define ARCH_PACK_COMPAT_STATFS64
#define __uapi_arch_pad_compat_statfs64 __uapi_arch_pad32
#else
#define __uapi_arch_pad_compat_statfs64
#endif

struct compat_statfs64 {
	__u32 f_type;
	__u32 f_bsize;
	__u64 f_blocks;
	__u64 f_bfree;
	__u64 f_bavail;
	__u64 f_files;
	__u64 f_ffree;
	__kernel_fsid_t f_fsid;
	__u32 f_namelen;
	__u32 f_frsize;
	__u32 f_flags;
	__u32 f_spare[4];
	__uapi_arch_pad_compat_statfs64;
} ARCH_PACK_COMPAT_STATFS64;

#undef __compat_statfs64_pad

#endif /* _UAPI_GENERIC_STATFS_H */
