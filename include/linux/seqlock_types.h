/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SEQLOCK_TYPES_H
#define __LINUX_SEQLOCK_TYPES_H

#include <linux/spinlock_types.h>
#include <linux/lockdep.h>

/*
 * Version using sequence counter only.
 * This can be used when code has its own mutex protecting the
 * updating starting before the write_seqcountbeqin() and ending
 * after the write_seqcount_end().
 */
typedef struct seqcount {
	unsigned sequence;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} seqcount_t;

typedef struct {
	struct seqcount seqcount;
	spinlock_t lock;
} seqlock_t;

#endif /* __LINUX_SEQLOCK_TYPES_H */
