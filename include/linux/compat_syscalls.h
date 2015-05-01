#ifndef _LINUX_COMPAT_SYSCALL_H
#define _LINUX_COMPAT_SYSCALL_H

#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/compat.h>

#ifndef CONFIG_COMPAT
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
	return (void __user*)(uintptr_t)ptr;
}
#endif

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
asmlinkage long compat_sys_recvmmsg64(int fd, struct compat_mmsghdr __user *mmsg,
				    unsigned vlen, unsigned int flags,
				    struct __kernel_timespec __user *timeout);
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
struct __kernel_timespec;
asmlinkage long compat_sys_rt_sigtimedwait(compat_sigset_t __user *uthese,
		struct compat_siginfo __user *uinfo,
		struct compat_timespec __user *uts, compat_size_t sigsetsize);
asmlinkage long compat_sys_rt_sigtimedwait_time64(compat_sigset_t __user *uthese,
		struct compat_siginfo __user *uinfo,
		struct __kernel_timespec __user *uts, compat_size_t sigsetsize);
asmlinkage long compat_sys_futex(u32 __user *uaddr, int op, u32 val,
		struct compat_timespec __user *utime, u32 __user *uaddr2,
		u32 val3);
asmlinkage long compat_sys_mq_timedsend(mqd_t mqdes,
			const char __user *u_msg_ptr,
			compat_size_t msg_len, unsigned int msg_prio,
			const struct compat_timespec __user *u_abs_timeout);
asmlinkage long compat_sys_mq_timedreceive(mqd_t mqdes,
			char __user *u_msg_ptr,
			compat_size_t msg_len, unsigned int __user *u_msg_prio,
			const struct compat_timespec __user *u_abs_timeout);
asmlinkage long compat_sys_sched_rr_get_interval(compat_pid_t pid,
						 struct compat_timespec __user *interval);

#endif
