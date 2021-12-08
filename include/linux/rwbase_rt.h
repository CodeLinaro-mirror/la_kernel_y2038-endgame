// SPDX-License-Identifier: GPL-2.0-only
#ifndef _LINUX_RWBASE_RT_H
#define _LINUX_RWBASE_RT_H

#include <linux/rtmutex.h>
#include <linux/atomic.h>
#include <linux/rwsem_types.h>

#define init_rwbase_rt(rwbase)					\
	do {							\
		rt_mutex_base_init(&(rwbase)->rtmutex);		\
		atomic_set(&(rwbase)->readers, READER_BIAS);	\
	} while (0)


static __always_inline bool rw_base_is_locked(struct rwbase_rt *rwb)
{
	return atomic_read(&rwb->readers) != READER_BIAS;
}

static __always_inline bool rw_base_is_contended(struct rwbase_rt *rwb)
{
	return atomic_read(&rwb->readers) > 0;
}

#endif /* _LINUX_RWBASE_RT_H */
