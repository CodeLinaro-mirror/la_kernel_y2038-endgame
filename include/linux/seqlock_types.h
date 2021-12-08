/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SEQLOCK_TYPES_H
#define __LINUX_SEQLOCK_TYPES_H

#include <linux/lockdep_types.h>
#include <linux/spinlock_types.h>

/*
 * Sequence counters (seqcount_t)
 *
 * This is the raw counting mechanism, without any writer protection.
 *
 * Write side critical sections must be serialized and non-preemptible.
 *
 * If readers can be invoked from hardirq or softirq contexts,
 * interrupts or bottom halves must also be respectively disabled before
 * entering the write section.
 *
 * This mechanism can't be used if the protected data contains pointers,
 * as the writer can invalidate a pointer that a reader is following.
 *
 * If the write serialization mechanism is one of the common kernel
 * locking primitives, use a sequence counter with associated lock
 * (seqcount_LOCKNAME_t) instead.
 *
 * If it's desired to automatically handle the sequence counter writer
 * serialization and non-preemptibility requirements, use a sequential
 * lock (seqlock_t) instead.
 *
 * See Documentation/locking/seqlock.rst
 */
typedef struct seqcount {
	unsigned sequence;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} seqcount_t;

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define SEQCOUNT_DEP_MAP_INIT(lockname)				\
		.dep_map = { .name = #lockname }
#else
# define SEQCOUNT_DEP_MAP_INIT(lockname)
#endif


/**
 * SEQCNT_ZERO() - static initializer for seqcount_t
 * @name: Name of the seqcount_t instance
 */
#define SEQCNT_ZERO(name) { .sequence = 0, SEQCOUNT_DEP_MAP_INIT(name) }

/*
 * For PREEMPT_RT, seqcount_LOCKNAME_t write side critical sections cannot
 * disable preemption. It can lead to higher latencies, and the write side
 * sections will not be able to acquire locks which become sleeping locks
 * (e.g. spinlock_t).
 *
 * To remain preemptible while avoiding a possible livelock caused by the
 * reader preempting the writer, use a different technique: let the reader
 * detect if a seqcount_LOCKNAME_t writer is in progress. If that is the
 * case, acquire then release the associated LOCKNAME writer serialization
 * lock. This will allow any possibly-preempted writer to make progress
 * until the end of its writer serialization lock critical section.
 *
 * This lock-unlock technique must be implemented for all of PREEMPT_RT
 * sleeping locks.  See Documentation/locking/locktypes.rst
 */
#if defined(CONFIG_LOCKDEP) || defined(CONFIG_PREEMPT_RT)
#define __SEQ_LOCK(expr)	expr
#else
#define __SEQ_LOCK(expr)
#endif

#define SEQCOUNT_LOCKNAME_TYPE(lockname, locktype)			\
typedef struct {							\
	seqcount_t		seqcount;				\
	__SEQ_LOCK(locktype	*lock);					\
} lockname;

SEQCOUNT_LOCKNAME_TYPE(seqcount_raw_spinlock_t, raw_spinlock_t);
SEQCOUNT_LOCKNAME_TYPE(seqcount_spinlock_t, spinlock_t);
SEQCOUNT_LOCKNAME_TYPE(seqcount_rwlock_t, rwlock_t);
SEQCOUNT_LOCKNAME_TYPE(seqcount_mutex_t, struct mutex);
SEQCOUNT_LOCKNAME_TYPE(seqcount_ww_mutex_t, struct ww_mutex);

/*
 * Sequential locks (seqlock_t)
 *
 * Sequence counters with an embedded spinlock for writer serialization
 * and non-preemptibility.
 *
 * For more info, see:
 *    - Comments on top of seqcount_t
 *    - Documentation/locking/seqlock.rst
 */
typedef struct {
	/*
	 * Make sure that readers don't starve writers on PREEMPT_RT: use
	 * seqcount_spinlock_t instead of seqcount_t. Check __SEQ_LOCK().
	 */
	seqcount_spinlock_t seqcount;
	spinlock_t lock;
} seqlock_t;

#define __SEQLOCK_UNLOCKED(lockname)					\
	{								\
		.seqcount = SEQCNT_SPINLOCK_ZERO(lockname, &(lockname).lock), \
		.lock =	__SPIN_LOCK_UNLOCKED(lockname)			\
	}

/**
 * DEFINE_SEQLOCK(sl) - Define a statically allocated seqlock_t
 * @sl: Name of the seqlock_t instance
 */
#define DEFINE_SEQLOCK(sl) \
		seqlock_t sl = __SEQLOCK_UNLOCKED(sl)

#endif
