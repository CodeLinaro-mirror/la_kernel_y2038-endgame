#ifndef __LINUX_CPUTIME_H
#define __LINUX_CPUTIME_H

#include <asm/cputime.h>

#ifndef cputime_to_nsecs
# define cputime_to_nsecs(__ct)	\
	(cputime_to_usecs(__ct) * NSEC_PER_USEC)
#endif

#ifndef nsecs_to_cputime
# define nsecs_to_cputime(__nsecs)	\
	usecs_to_cputime((__nsecs) / NSEC_PER_USEC)
#endif

#ifdef CONFIG_Y2038_UNSAFE
static inline cputime_t timespec_to_cputime(const struct timespec *ts)
{
	struct timespec64 ts64 = timespec_to_timespec64(*ts);

	return timespec64_to_cputime(&ts64);
}

static inline void cputime_to_timespec(const cputime_t cputime,
				       struct timespec *value)
{
	struct timespec64 ts64;

	cputime_to_timespec64(cputime, &ts64);
	*value = timespec64_to_timespec(ts64);
}
#endif

#endif /* __LINUX_CPUTIME_H */
