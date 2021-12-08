/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_WAIT_TYPES_H_
#define _LINUX_WAIT_TYPES_H_

#include <linux/spinlock_types.h>

typedef struct wait_queue_entry wait_queue_entry_t;
typedef int (*wait_queue_func_t)(struct wait_queue_entry *wq_entry, unsigned mode, int flags, void *key);

struct wait_queue_head {
	spinlock_t		lock;
	struct list_head	head;
};
typedef struct wait_queue_head wait_queue_head_t;

/* wait_queue_entry::flags */
#define WQ_FLAG_EXCLUSIVE	0x01
#define WQ_FLAG_WOKEN		0x02
#define WQ_FLAG_BOOKMARK	0x04
#define WQ_FLAG_CUSTOM		0x08
#define WQ_FLAG_DONE		0x10
#define WQ_FLAG_PRIORITY	0x20

/*
 * A single wait-queue entry structure:
 */
struct wait_queue_entry {
	unsigned int		flags;
	void			*private;
	wait_queue_func_t	func;
	struct list_head	entry;
};

struct task_struct;

/*
 * Macros for declaration and initialisaton of the datatypes
 */

#define __WAITQUEUE_INITIALIZER(name, tsk) {					\
	.private	= tsk,							\
	.func		= default_wake_function,				\
	.entry		= { NULL, NULL } }

#define DECLARE_WAITQUEUE(name, tsk)						\
	struct wait_queue_entry name = __WAITQUEUE_INITIALIZER(name, tsk)

#define __WAIT_QUEUE_HEAD_INITIALIZER(name) {					\
	.lock		= __SPIN_LOCK_UNLOCKED(name.lock),			\
	.head		= LIST_HEAD_INIT(name.head) }

#define DECLARE_WAIT_QUEUE_HEAD(name) \
	struct wait_queue_head name = __WAIT_QUEUE_HEAD_INITIALIZER(name)


/*
 * rcuwait provides a way of blocking and waking up a single
 * task in an rcu-safe manner.
 *
 * The only time @task is non-nil is when a user is blocked (or
 * checking if it needs to) on a condition, and reset as soon as we
 * know that the condition has succeeded and are awoken.
 */
struct rcuwait {
	struct task_struct __rcu *task;
};

#define __RCUWAIT_INITIALIZER(name)		\
	{ .task = NULL, }

/* Structure to mediate between updaters and fastpath-using readers.  */
struct rcu_sync {
	int			gp_state;
	int			gp_count;
	wait_queue_head_t	gp_wait;

	struct rcu_head		cb_head;
};

#endif
