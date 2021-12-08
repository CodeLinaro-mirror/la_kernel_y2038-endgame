/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mutexes: blocking mutual exclusion locks
 * RT Mutexes: blocking mutual exclusion locks with PI support
 *
 * started by Ingo Molnar:
 *
 *  Copyright (C) 2004-2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *  Copyright (C) 2006, Timesys Corp., Thomas Gleixner <tglx@timesys.com>
 *
 * This file contains the main data structures, with the minumum set
 * of indirect includes.
 */
#ifndef __LINUX_MUTEX_TYPES_H
#define __LINUX_MUTEX_TYPES_H

#include <linux/rbtree_types.h>
#include <linux/spinlock_types_raw.h>

/*
 * An MCS like lock especially tailored for optimistic spinning for sleeping
 * lock implementations (mutex, rwsem, etc).
 */
struct optimistic_spin_node {
	struct optimistic_spin_node *next, *prev;
	int locked; /* 1 if lock acquired */
	int cpu; /* encoded CPU # + 1 value */
};

struct optimistic_spin_queue {
	/*
	 * Stores an encoded value of the CPU # of the tail node in the queue.
	 * If the queue is empty, then it's set to OSQ_UNLOCKED_VAL.
	 */
	atomic_t tail;
};

#define OSQ_UNLOCKED_VAL (0)
#define OSQ_LOCK_UNLOCKED { ATOMIC_INIT(OSQ_UNLOCKED_VAL) }

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define __DEP_MAP_MUTEX_INITIALIZER(lockname)			\
		, .dep_map = {					\
			.name = #lockname,			\
			.wait_type_inner = LD_WAIT_SLEEP,	\
		}
#else
# define __DEP_MAP_MUTEX_INITIALIZER(lockname)
#endif

struct rt_mutex_base {
	raw_spinlock_t		wait_lock;
	struct rb_root_cached   waiters;
	struct task_struct	*owner;
};

#define __RT_MUTEX_BASE_INITIALIZER(rtbasename)				\
{									\
	.wait_lock = __RAW_SPIN_LOCK_UNLOCKED(rtbasename.wait_lock),	\
	.waiters = RB_ROOT_CACHED,					\
	.owner = NULL							\
}

/**
 * The rt_mutex structure
 *
 * @wait_lock:	spinlock to protect the structure
 * @waiters:	rbtree root to enqueue waiters in priority order;
 *              caches top-waiter (leftmost node).
 * @owner:	the mutex owner
 */
struct rt_mutex {
	struct rt_mutex_base	rtmutex;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define __DEP_MAP_RT_MUTEX_INITIALIZER(mutexname)	\
	.dep_map = {					\
		.name = #mutexname,			\
		.wait_type_inner = LD_WAIT_SLEEP,	\
	}
#else
#define __DEP_MAP_RT_MUTEX_INITIALIZER(mutexname)
#endif

#define __RT_MUTEX_INITIALIZER(mutexname)				\
{									\
	.rtmutex = __RT_MUTEX_BASE_INITIALIZER(mutexname.rtmutex),	\
	__DEP_MAP_RT_MUTEX_INITIALIZER(mutexname)			\
}

#define DEFINE_RT_MUTEX(mutexname) \
	struct rt_mutex mutexname = __RT_MUTEX_INITIALIZER(mutexname)

#ifndef CONFIG_PREEMPT_RT

/*
 * Simple, straightforward mutexes with strict semantics:
 *
 * - only one task can hold the mutex at a time
 * - only the owner can unlock the mutex
 * - multiple unlocks are not permitted
 * - recursive locking is not permitted
 * - a mutex object must be initialized via the API
 * - a mutex object must not be initialized via memset or copying
 * - task may not exit with mutex held
 * - memory areas where held locks reside must not be freed
 * - held mutexes must not be reinitialized
 * - mutexes may not be used in hardware or software interrupt
 *   contexts such as tasklets and timers
 *
 * These semantics are fully enforced when DEBUG_MUTEXES is
 * enabled. Furthermore, besides enforcing the above rules, the mutex
 * debugging code also implements a number of additional features
 * that make lock debugging easier and faster:
 *
 * - uses symbolic names of mutexes, whenever they are printed in debug output
 * - point-of-acquire tracking, symbolic lookup of function names
 * - list of all locks held in the system, printout of them
 * - owner tracking
 * - detects self-recursing locks and prints out all relevant info
 * - detects multi-task circular deadlocks and prints out all affected
 *   locks and tasks (and only those tasks)
 */
struct mutex {
	atomic_long_t		owner;
	raw_spinlock_t		wait_lock;
#ifdef CONFIG_MUTEX_SPIN_ON_OWNER
	struct optimistic_spin_queue osq; /* Spinner MCS lock */
#endif
	struct list_head	wait_list;
#ifdef CONFIG_DEBUG_MUTEXES
	void			*magic;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

#ifdef CONFIG_DEBUG_MUTEXES
#define __DEBUG_MUTEX_INITIALIZER(lockname)				\
	, .magic = &lockname
#else
# define __DEBUG_MUTEX_INITIALIZER(lockname)
#endif

#define __MUTEX_INITIALIZER(lockname) \
		{ .owner = ATOMIC_LONG_INIT(0) \
		, .wait_lock = __RAW_SPIN_LOCK_UNLOCKED(lockname.wait_lock) \
		, .wait_list = LIST_HEAD_INIT(lockname.wait_list) \
		__DEBUG_MUTEX_INITIALIZER(lockname) \
		__DEP_MAP_MUTEX_INITIALIZER(lockname) }

#define DEFINE_MUTEX(mutexname) \
	struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#else /* !CONFIG_PREEMPT_RT */
/*
 * Preempt-RT variant based on rtmutexes.
 */
struct mutex {
	struct rt_mutex_base	rtmutex;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

#define __MUTEX_INITIALIZER(mutexname)					\
{									\
	.rtmutex = __RT_MUTEX_BASE_INITIALIZER(mutexname.rtmutex)	\
	__DEP_MAP_MUTEX_INITIALIZER(mutexname)				\
}

#define DEFINE_MUTEX(mutexname)						\
	struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#endif /* CONFIG_PREEMPT_RT */

#endif /* __LINUX_MUTEX_TYPES_H */
