/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_XT_HASHLIMIT_H
#define _UAPI_XT_HASHLIMIT_H

#include <linux/types.h>
#include <linux/limits.h>
#include <linux/if.h>

/* timings are in milliseconds. */
#define XT_HASHLIMIT_SCALE 10000
#define XT_HASHLIMIT_SCALE_v2 1000000llu
/* 1/10,000 sec period => max of 10,000/sec.  Min rate is then 429490
 * seconds, or one packet every 59 hours.
 */

/* packet length accounting is done in 16-byte steps */
#define XT_HASHLIMIT_BYTE_SHIFT 4

/* details of this structure hidden by the implementation */
struct xt_hashlimit_htable;

enum {
	XT_HASHLIMIT_HASH_DIP		= 1 << 0,
	XT_HASHLIMIT_HASH_DPT		= 1 << 1,
	XT_HASHLIMIT_HASH_SIP		= 1 << 2,
	XT_HASHLIMIT_HASH_SPT		= 1 << 3,
	XT_HASHLIMIT_INVERT		= 1 << 4,
	XT_HASHLIMIT_BYTES		= 1 << 5,
	XT_HASHLIMIT_RATE_MATCH		= 1 << 6,
};

struct hashlimit_cfg {
	__u32 mode;	  /* bitmask of XT_HASHLIMIT_HASH_* */
	__u32 avg;    /* Average secs between packets * scale */
	__u32 burst;  /* Period multiplier for upper limit. */

	/* user specified */
	__u32 size;		/* how many buckets */
	__u32 max;		/* max number of entries */
	__u32 gc_interval;	/* gc interval */
	__u32 expire;	/* when do entries expire? */
};

struct xt_hashlimit_info {
	char name [IFNAMSIZ];		/* name */
	struct hashlimit_cfg cfg;
	__uapi_arch_pad_long;

	/* Used internally by the kernel */
	struct xt_hashlimit_htable *hinfo;
	union {
		void *ptr;
		struct xt_hashlimit_info *master;
	} u;
};

struct hashlimit_cfg1 {
	__u32 mode;	  /* bitmask of XT_HASHLIMIT_HASH_* */
	__u32 avg;    /* Average secs between packets * scale */
	__u32 burst;  /* Period multiplier for upper limit. */

	/* user specified */
	__u32 size;		/* how many buckets */
	__u32 max;		/* max number of entries */
	__u32 gc_interval;	/* gc interval */
	__u32 expire;	/* when do entries expire? */

	__u8 srcmask, dstmask;
	__uapi_arch_pad16;
} __uapi_arch_align;

struct hashlimit_cfg2 {
	__u64 avg;		/* Average secs between packets * scale */
	__u64 burst;		/* Period multiplier for upper limit. */
	__u32 mode;		/* bitmask of XT_HASHLIMIT_HASH_* */

	/* user specified */
	__u32 size;		/* how many buckets */
	__u32 max;		/* max number of entries */
	__u32 gc_interval;	/* gc interval */
	__u32 expire;		/* when do entries expire? */

	__u8 srcmask, dstmask;
	__uapi_arch_pad16;
} __uapi_arch_align;

struct hashlimit_cfg3 {
	__u64 avg;		/* Average secs between packets * scale */
	__u64 burst;		/* Period multiplier for upper limit. */
	__u32 mode;		/* bitmask of XT_HASHLIMIT_HASH_* */

	/* user specified */
	__u32 size;		/* how many buckets */
	__u32 max;		/* max number of entries */
	__u32 gc_interval;	/* gc interval */
	__u32 expire;		/* when do entries expire? */

	__u32 interval;
	__u8 srcmask, dstmask;
	__uapi_arch_pad16;
	__uapi_arch_pad32;
} __uapi_arch_align;

struct xt_hashlimit_mtinfo1 {
	char name[IFNAMSIZ];
	struct hashlimit_cfg1 cfg;
#ifdef __m68k__
	__u16 :16;
#endif

	/* Used internally by the kernel */
	struct xt_hashlimit_htable *hinfo __attribute__((aligned(8)));
	__uapi_arch_pad_long_to_aligned_u64;
};

struct xt_hashlimit_mtinfo2 {
	char name[NAME_MAX];
	__uapi_arch_pad8;
	struct hashlimit_cfg2 cfg;
#ifdef __m68k__
	__u16 :16;
#endif

	/* Used internally by the kernel */
	struct xt_hashlimit_htable *hinfo __attribute__((aligned(8)));
	__uapi_arch_pad_long_to_aligned_u64;
};

struct xt_hashlimit_mtinfo3 {
	char name[NAME_MAX];
	__uapi_arch_pad8;
	struct hashlimit_cfg3 cfg;
#ifdef __m68k__
	__u16 :16;
#endif
#if defined(__i386__) || defined(__csky__) || defined(__m68k__) || defined(__microblaze__) || defined(__arc__) || defined(__nios2__) || defined(__or1k__) || defined(__sh__)
	__u32 :32;
#endif
	/* Used internally by the kernel */
	struct xt_hashlimit_htable *hinfo __attribute__((aligned(8)));
	__uapi_arch_pad_long_to_aligned_u64;
};

#endif /* _UAPI_XT_HASHLIMIT_H */
