/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_OSQ_LOCK_H
#define __LINUX_OSQ_LOCK_H

#include <linux/mutex_types.h>

static inline void osq_lock_init(struct optimistic_spin_queue *lock)
{
	atomic_set(&lock->tail, OSQ_UNLOCKED_VAL);
}

extern bool osq_lock(struct optimistic_spin_queue *lock);
extern void osq_unlock(struct optimistic_spin_queue *lock);

static inline bool osq_is_locked(struct optimistic_spin_queue *lock)
{
	return atomic_read(&lock->tail) != OSQ_UNLOCKED_VAL;
}

#endif
