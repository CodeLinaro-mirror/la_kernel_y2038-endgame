#ifndef _LINUX_COMPAT_TIME_H
#define _LINUX_COMPAT_TIME_H
/*
 * These are the type definitions for the architecture specific
 * syscall compatibility layer for 32-bit time_t.
 *
 * The emulation is used both on 32-bit architectures that have
 * been converted to 64-bit __kernel_time_t as well as on 64-bit
 * architectures with 32-bit user space.
 */
#include <linux/resource.h>
#include <linux/signal.h>

#ifndef COMPAT_USE_64BIT_TIME
#define COMPAT_USE_64BIT_TIME 0
#endif

typedef s32	compat_int_t;
typedef u32	compat_uint_t;
typedef s32	compat_long_t;
typedef u32	compat_ulong_t;
typedef u32	compat_size_t;
typedef s32	compat_ssize_t;
typedef s32	compat_time_t;
typedef	u32	compat_aio_context_t;
typedef u32	compat_uptr_t;
typedef __kernel_pid_t compat_pid_t;

struct compat_timespec {
	compat_time_t	tv_sec;
	s32		tv_nsec;
};

struct compat_timeval {
	compat_time_t	tv_sec;
	s32		tv_usec;
};

struct compat_itimerspec {
	struct compat_timespec it_interval;
	struct compat_timespec it_value;
};

struct compat_utimbuf {
	compat_time_t		actime;
	compat_time_t		modtime;
};

struct compat_itimerval {
	struct compat_timeval	it_interval;
	struct compat_timeval	it_value;
};

struct compat_timex {
	compat_uint_t modes;
	compat_long_t offset;
	compat_long_t freq;
	compat_long_t maxerror;
	compat_long_t esterror;
	compat_int_t status;
	compat_long_t constant;
	compat_long_t precision;
	compat_long_t tolerance;
	struct compat_timeval time;
	compat_long_t tick;
	compat_long_t ppsfreq;
	compat_long_t jitter;
	compat_int_t shift;
	compat_long_t stabil;
	compat_long_t jitcnt;
	compat_long_t calcnt;
	compat_long_t errcnt;
	compat_long_t stbcnt;
	compat_int_t tai;

	compat_int_t:32; compat_int_t:32; compat_int_t:32; compat_int_t:32;
	compat_int_t:32; compat_int_t:32; compat_int_t:32; compat_int_t:32;
	compat_int_t:32; compat_int_t:32; compat_int_t:32;
};

#ifdef CONFIG_COMPAT
#include <asm/compat.h>

#define _COMPAT_NSIG_WORDS     (_COMPAT_NSIG / _COMPAT_NSIG_BPW)
typedef struct {
       compat_sigset_word      sig[_COMPAT_NSIG_WORDS];
} compat_sigset_t;

#else

#define compat_mmsghdr mmsghdr
#define compat_stat stat
#define compat_siginfo siginfo
#define compat_sigevent sigevent
#define compat_sigset_t sigset_t
#define __compat_uid_t __kernel_uid_t
#define __compat_gid_t __kernel_gid_t
#define compat_mode_t __kernel_mode_t
#define copy_siginfo_to_user32(uinfo, info) copy_siginfo_to_user(uinfo, info)
static inline void __user *compat_ptr(compat_uptr_t ptr)
{
	return (void __user*)ptr;
}
#endif

/*
 * These functions operate on 32- or 64-bit specs depending on
 * COMPAT_USE_64BIT_TIME, hence the void user pointer arguments.
 */
extern int compat_get_timespec(struct timespec *, const void __user *);
extern int compat_put_timespec(const struct timespec *, void __user *);
extern int compat_get_timeval(struct timeval *, const void __user *);
extern int compat_put_timeval(const struct timeval *, void __user *);

/*
 * This function convert a timespec if necessary and returns a *user
 * space* pointer.  If no conversion is necessary, it returns the
 * initial pointer.  NULL is a legitimate argument and will always
 * output NULL.
 */
extern int compat_convert_timespec(struct timespec __user **,
				   const void __user *);

struct compat_rusage {
	struct compat_timeval ru_utime;
	struct compat_timeval ru_stime;
	compat_long_t	ru_maxrss;
	compat_long_t	ru_ixrss;
	compat_long_t	ru_idrss;
	compat_long_t	ru_isrss;
	compat_long_t	ru_minflt;
	compat_long_t	ru_majflt;
	compat_long_t	ru_nswap;
	compat_long_t	ru_inblock;
	compat_long_t	ru_oublock;
	compat_long_t	ru_msgsnd;
	compat_long_t	ru_msgrcv;
	compat_long_t	ru_nsignals;
	compat_long_t	ru_nvcsw;
	compat_long_t	ru_nivcsw;
};

struct rusage;
extern int put_compat_rusage(const struct rusage *,
			     struct compat_rusage __user *);

static inline int compat_timeval_compare(struct compat_timeval *lhs,
					struct compat_timeval *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_usec - rhs->tv_usec;
}

static inline int compat_timespec_compare(struct compat_timespec *lhs,
					struct compat_timespec *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_nsec - rhs->tv_nsec;
}

extern int get_compat_itimerspec(struct itimerspec *dst,
				 const struct compat_itimerspec __user *src);
extern int put_compat_itimerspec(struct compat_itimerspec __user *dst,
				 const struct itimerspec *src);

struct sembuf;
asmlinkage long compat_sys_semtimedop(int semid, struct sembuf __user *tsems,
		unsigned nsems, const struct compat_timespec __user *timeout);
asmlinkage long compat_sys_select(int n, compat_ulong_t __user *inp,
		compat_ulong_t __user *outp, compat_ulong_t __user *exp,
		struct compat_timeval __user *tvp);
struct compat_sel_arg_struct;
asmlinkage long compat_sys_old_select(struct compat_sel_arg_struct __user *arg);
asmlinkage long compat_sys_pselect6(int n, compat_ulong_t __user *inp,
				    compat_ulong_t __user *outp,
				    compat_ulong_t __user *exp,
				    struct compat_timespec __user *tsp,
				    void __user *sig);
asmlinkage long compat_sys_gettimeofday(struct compat_timeval __user *tv,
		struct timezone __user *tz);
asmlinkage long compat_sys_settimeofday(struct compat_timeval __user *tv,
		struct timezone __user *tz);

asmlinkage long compat_sys_adjtimex(struct compat_timex __user *utp);

asmlinkage long compat_sys_utime(const char __user *filename,
				 struct compat_utimbuf __user *t);
asmlinkage long compat_sys_utimensat(unsigned int dfd,
				     const char __user *filename,
				     struct compat_timespec __user *t,
				     int flags);

asmlinkage long compat_sys_time(compat_time_t __user *tloc);
asmlinkage long compat_sys_stime(compat_time_t __user *tptr);

asmlinkage long compat_sys_timerfd_settime(int ufd, int flags,
				   const struct compat_itimerspec __user *utmr,
				   struct compat_itimerspec __user *otmr);
asmlinkage long compat_sys_timerfd_gettime(int ufd,
				   struct compat_itimerspec __user *otmr);

asmlinkage long compat_sys_futimesat(unsigned int dfd,
				     const char __user *filename,
				     struct compat_timeval __user *t);
asmlinkage long compat_sys_utimes(const char __user *filename,
				  struct compat_timeval __user *t);
struct compat_stat;
asmlinkage long compat_sys_newstat(const char __user *filename,
				   struct compat_stat __user *statbuf);
asmlinkage long compat_sys_newlstat(const char __user *filename,
				    struct compat_stat __user *statbuf);
asmlinkage long compat_sys_newfstatat(unsigned int dfd,
				      const char __user *filename,
				      struct compat_stat __user *statbuf,
				      int flag);
asmlinkage long compat_sys_newfstat(unsigned int fd,
				    struct compat_stat __user *statbuf);
struct io_event;
asmlinkage long compat_sys_io_getevents(compat_aio_context_t ctx_id,
					compat_long_t min_nr,
					compat_long_t nr,
					struct io_event __user *events,
					struct compat_timespec __user *timeout);
asmlinkage long compat_sys_ppoll(struct pollfd __user *ufds,
				 unsigned int nfds,
				 struct compat_timespec __user *tsp,
				 const compat_sigset_t __user *sigmask,
				 compat_size_t sigsetsize);
struct compat_mmsghdr;
asmlinkage long compat_sys_recvmmsg(int fd, struct compat_mmsghdr __user *mmsg,
				    unsigned vlen, unsigned int flags,
				    struct compat_timespec __user *timeout);
asmlinkage long compat_sys_nanosleep(struct compat_timespec __user *rqtp,
				     struct compat_timespec __user *rmtp);
asmlinkage long compat_sys_getitimer(int which,
				     struct compat_itimerval __user *it);
asmlinkage long compat_sys_setitimer(int which,
				     struct compat_itimerval __user *in,
				     struct compat_itimerval __user *out);
asmlinkage long compat_sys_timer_settime(timer_t timer_id, int flags,
					 struct compat_itimerspec __user *new,
					 struct compat_itimerspec __user *old);
asmlinkage long compat_sys_timer_gettime(timer_t timer_id,
				 struct compat_itimerspec __user *setting);
asmlinkage long compat_sys_clock_settime(clockid_t which_clock,
					 struct compat_timespec __user *tp);
asmlinkage long compat_sys_clock_gettime(clockid_t which_clock,
					 struct compat_timespec __user *tp);
asmlinkage long compat_sys_clock_adjtime(clockid_t which_clock,
					 struct compat_timex __user *tp);
asmlinkage long compat_sys_clock_getres(clockid_t which_clock,
					struct compat_timespec __user *tp);
asmlinkage long compat_sys_clock_nanosleep(clockid_t which_clock, int flags,
					   struct compat_timespec __user *rqtp,
					   struct compat_timespec __user *rmtp);
struct compat_siginfo;
asmlinkage long compat_sys_rt_sigtimedwait(compat_sigset_t __user *uthese,
		struct compat_siginfo __user *uinfo,
		struct compat_timespec __user *uts, compat_size_t sigsetsize);
asmlinkage long compat_sys_futex(u32 __user *uaddr, int op, u32 val,
		struct compat_timespec __user *utime, u32 __user *uaddr2,
		u32 val3);
asmlinkage long compat_sys_mq_timedsend(mqd_t mqdes,
			const char __user *u_msg_ptr,
			compat_size_t msg_len, unsigned int msg_prio,
			const struct compat_timespec __user *u_abs_timeout);
asmlinkage ssize_t compat_sys_mq_timedreceive(mqd_t mqdes,
			char __user *u_msg_ptr,
			compat_size_t msg_len, unsigned int __user *u_msg_prio,
			const struct compat_timespec __user *u_abs_timeout);
asmlinkage long compat_sys_sched_rr_get_interval(compat_pid_t pid,
						 struct compat_timespec __user *interval);
#endif
