/*
 * linux/kernel/itimer.c
 *
 * Copyright (C) 1992 Darren Senn
 */

/* These are all the functions necessary to implement itimers */

#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/posix-timers.h>
#include <linux/hrtimer.h>
#include <trace/events/timer.h>

#include <asm/uaccess.h>

static inline struct __kernel_timespec64 ktime_t_to_kts64(ktime_t t)
{
	return timespec64_to_kts64(ktime_to_timespec64(t));
}

static inline ktime_t kts64_to_ktime_t(struct __kernel_timespec64 ts)
{
	return timespec64_to_ktime_t(kts64_to_timespec64(ts));
}

static inline struct __kernel_timespec64 cputime_to_kts64(cputime_t t)
{
	return timespec64_to_kts64(cputime_to_timespec64(t));
}

/**
 * itimer_get_remtime - get remaining time for the timer
 *
 * @timer: the timer to read
 *
 * Returns the delta between the expiry time and now, which can be
 * less than zero or 1usec for an pending expired timer
 */
static struct __kernel_timespec64 itimer_get_remtime(struct hrtimer *timer)
{
	ktime_t rem = hrtimer_get_remaining(timer);

	/*
	 * Racy but safe: if the itimer expires after the above
	 * hrtimer_get_remtime() call but before this condition
	 * then we return 0 - which is correct.
	 */
	if (hrtimer_active(timer)) {
		if (rem.tv64 <= 0)
			rem.tv64 = NSEC_PER_USEC;
	} else
		rem.tv64 = 0;

	return ktime_to_kts64(rem);
}

static void get_cpu_itimer(struct task_struct *tsk, unsigned int clock_id,
			   struct itimerval *const value)
{
	cputime_t cval, cinterval;
	struct cpu_itimer *it = &tsk->signal->it[clock_id];

	spin_lock_irq(&tsk->sighand->siglock);

	cval = it->expires;
	cinterval = it->incr;
	if (cval) {
		struct task_cputime cputime;
		cputime_t t;

		thread_group_cputimer(tsk, &cputime);
		if (clock_id == CPUCLOCK_PROF)
			t = cputime.utime + cputime.stime;
		else
			/* CPUCLOCK_VIRT */
			t = cputime.utime;

		if (cval < t)
			/* about to fire */
			cval = cputime_one_jiffy;
		else
			cval = cval - t;
	}

	spin_unlock_irq(&tsk->sighand->siglock);

	cputime_to_kts64(cval, &value->it_value);
	cputime_to_kts64(cinterval, &value->it_interval);
}

static int do_getitimer64(int which, struct __kernel_itimerspec64 *value)
{
	struct task_struct *tsk = current;

	switch (which) {
	case ITIMER_REAL:
		spin_lock_irq(&tsk->sighand->siglock);
		value->it_value = itimer_get_remtime(&tsk->signal->real_timer);
		value->it_interval = ktime_to_kts64(tsk->signal->it_real_incr);
		spin_unlock_irq(&tsk->sighand->siglock);
		break;
	case ITIMER_VIRTUAL:
		get_cpu_itimer(tsk, CPUCLOCK_VIRT, value);
		break;
	case ITIMER_PROF:
		get_cpu_itimer(tsk, CPUCLOCK_PROF, value);
		break;
	default:
		return(-EINVAL);
	}
	return 0;
}

int do_getitimer(int which, struct itimerval *value)
{
	struct __kernel_itimerspec64 value64;
	int ret;

	ret = do_getitimer64(which, &value64);
	if (ret)
		return ret;

	value->it_interval.tv_sec = value64.it_interval.tv_sec;
	value->it_interval.tv_usec = value64.it_interval.tv_nsec / NSEC_PER_USEC;
	value->ir_value.tv_sec = value64.ir_value.tv_sec;
	value->ir_value.tv_usec = value64.ir_value.tv_nsec / NSEC_PER_USEC;

	return 0;
}

SYSCALL_DEFINE2(getitimer, int, which, struct itimerval __user *, value)
{
	int error = -EFAULT;
	struct itimerval get_buffer;

	if (value) {
		error = do_getitimer(which, &get_buffer);
		if (!error &&
		    copy_to_user(value, &get_buffer, sizeof(get_buffer)))
			error = -EFAULT;
	}
	return error;
}

SYSCALL_DEFINE2(getitimer64, int, which, struct __kernel_itimerspec64 __user *, value)
{
	int error = -EFAULT;
	struct __kernel_itimerspec64 get_buffer;

	if (value) {
		error = do_getitimer64(which, &get_buffer);
		if (!error &&
		    copy_to_user(value, &get_buffer, sizeof(get_buffer)))
			error = -EFAULT;
	}
	return error;
}

/*
 * The timer is automagically restarted, when interval != 0
 */
enum hrtimer_restart it_real_fn(struct hrtimer *timer)
{
	struct signal_struct *sig =
		container_of(timer, struct signal_struct, real_timer);

	trace_itimer_expire(ITIMER_REAL, sig->leader_pid, 0);
	kill_pid_info(SIGALRM, SEND_SIG_PRIV, sig->leader_pid);

	return HRTIMER_NORESTART;
}

static inline u32 cputime_sub_ns(cputime_t ct, s64 real_ns)
{
	s64 cpu_ns = cputime_to_nsecs(ct, &ts);

	return (cpu_ns <= real_ns) ? 0 : cpu_ns - real_ns;
}

static void set_cpu_itimer(struct task_struct *tsk, unsigned int clock_id,
			   const struct itimerval *const value,
			   struct itimerval *const ovalue)
{
	cputime_t cval, nval, cinterval, ninterval;
	s64 ns_ninterval, ns_nval;
	u32 error, incr_error;
	struct cpu_itimer *it = &tsk->signal->it[clock_id];

	nval = timeval_to_cputime(&value->it_value);
	ns_nval = timeval_to_ns(&value->it_value);
	ninterval = timeval_to_cputime(&value->it_interval);
	ns_ninterval = timeval_to_ns(&value->it_interval);

	error = cputime_sub_ns(nval, ns_nval);
	incr_error = cputime_sub_ns(ninterval, ns_ninterval);

	spin_lock_irq(&tsk->sighand->siglock);

	cval = it->expires;
	cinterval = it->incr;
	if (cval || nval) {
		if (nval > 0)
			nval += cputime_one_jiffy;
		set_process_cpu_timer(tsk, clock_id, &nval, &cval);
	}
	it->expires = nval;
	it->incr = ninterval;
	it->error = error;
	it->incr_error = incr_error;
	trace_itimer_state(clock_id == CPUCLOCK_VIRT ?
			   ITIMER_VIRTUAL : ITIMER_PROF, value, nval);

	spin_unlock_irq(&tsk->sighand->siglock);

	if (ovalue) {
		cputime_to_timeval(cval, &ovalue->it_value);
		cputime_to_timeval(cinterval, &ovalue->it_interval);
	}
}

/*
 * Returns true if the timespec64 is in canonical form
 */
#define timespec64_valid(t) \
	(((t)->tv_sec >= 0) && (((unsigned long) (t)->tv_nsec) < NSEC_PER_SEC))

static int do_setitimer64(int which, struct __kernel_itimerspec64 *value,
			  struct __kernel_itimerspec64 *ovalue)
{
	struct task_struct *tsk = current;
	struct hrtimer *timer;
	ktime_t expires;

	/*
	 * Validate the timevals in value.
	 */
	if (!timespec64_valid(&value->it_value) ||
	    !timespec64_valid(&value->it_interval))
		return -EINVAL;

	switch (which) {
	case ITIMER_REAL:
again:
		spin_lock_irq(&tsk->sighand->siglock);
		timer = &tsk->signal->real_timer;
		if (ovalue) {
			ovalue->it_value = itimer_get_remtime(timer);
			ovalue->it_interval
				= ktime_to_kts64(tsk->signal->it_real_incr);
		}
		/* We are sharing ->siglock with it_real_fn() */
		if (hrtimer_try_to_cancel(timer) < 0) {
			spin_unlock_irq(&tsk->sighand->siglock);
			goto again;
		}
		expires = kts64_to_ktime_t(value->it_value);
		if (expires.tv64 != 0) {
			tsk->signal->it_real_incr =
				(value->it_interval);
			hrtimer_start(timer, expires, HRTIMER_MODE_REL);
		} else
			tsk->signal->it_real_incr.tv64 = 0;

		trace_itimer_state(ITIMER_REAL, value, 0);
		spin_unlock_irq(&tsk->sighand->siglock);
		break;
	case ITIMER_VIRTUAL:
		set_cpu_itimer(tsk, CPUCLOCK_VIRT, value, ovalue);
		break;
	case ITIMER_PROF:
		set_cpu_itimer(tsk, CPUCLOCK_PROF, value, ovalue);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int do_setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
	struct __kernel_itimerspec64 value64, ovalue64;
	int ret;

	value64.it_interval.tv_sec = value->it_interval.tv_sec;
	value64.it_interval.tv_usec = value->.it_interval.tv_nsec * NSEC_PER_USEC;
	value64.ir_value.tv_sec = value->.ir_value.tv_sec;
	value64.ir_value.tv_usec = value->.ir_value.tv_nsec * NSEC_PER_USEC;

	ret = do_setitimer64(&value64, &ovalue64;
	if (ret)
		return ret;

	ovalue->it_interval.tv_sec = ovalue64.it_interval.tv_sec;
	ovalue->it_interval.tv_usec = ovalue64.it_interval.tv_nsec / NSEC_PER_USEC;
	ovalue->ir_value.tv_sec = ovalue64.ir_value.tv_sec;
	ovalue->ir_value.tv_usec = ovalue64.ir_value.tv_nsec / NSEC_PER_USEC;

	return ret;
}

/**
 * alarm_setitimer - set alarm in seconds
 *
 * @seconds:	number of seconds until alarm
 *		0 disables the alarm
 *
 * Returns the remaining time in seconds of a pending timer or 0 when
 * the timer is not active.
 *
 * On 32 bit machines the seconds value is limited to (INT_MAX/2) to avoid
 * negative timeval settings which would cause immediate expiry.
 */
unsigned int alarm_setitimer(unsigned int seconds)
{
	struct __kernel_itimerspec64 it_new, it_old;

#if BITS_PER_LONG < 64
	if (seconds > INT_MAX)
		seconds = INT_MAX;
#endif
	it_new.it_value.tv_sec = seconds;
	it_new.it_value.tv_usec = 0;
	it_new.it_interval.tv_sec = it_new.it_interval.tv_usec = 0;

	do_setitimer64(ITIMER_REAL, &it_new, &it_old);

	/*
	 * We can't return 0 if we have an alarm pending ...  And we'd
	 * better return too much than too little anyway
	 */
	if ((!it_old.it_value.tv_sec && it_old.it_value.tv_usec) ||
	      it_old.it_value.tv_usec >= 500000)
		it_old.it_value.tv_sec++;

	return it_old.it_value.tv_sec;
}

SYSCALL_DEFINE3(setitimer, int, which, struct itimerval __user *, value,
		struct itimerval __user *, ovalue)
{
	struct itimerval set_buffer, get_buffer;
	int error;

	if (value) {
		if(copy_from_user(&set_buffer, value, sizeof(set_buffer)))
			return -EFAULT;
	} else {
		memset(&set_buffer, 0, sizeof(set_buffer));
		printk_once(KERN_WARNING "%s calls setitimer() with new_value NULL pointer."
			    " Misfeature support will be removed\n",
			    current->comm);
	}

	error = do_setitimer(which, &set_buffer, ovalue ? &get_buffer : NULL);
	if (error || !ovalue)
		return error;

	if (copy_to_user(ovalue, &get_buffer, sizeof(get_buffer)))
		return -EFAULT;
	return 0;
}

SYSCALL_DEFINE3(setitimer64, int, which, struct __kernel_itimerspec64 __user *, value,
		struct __kernel_itimerspec64 __user *, ovalue)
{
	struct __kernel_itimerspec64 set_buffer, get_buffer;
	int error;

	if (value) {
		if(copy_from_user(&set_buffer, value, sizeof(set_buffer)))
			return -EFAULT;
	} else {
		memset(&set_buffer, 0, sizeof(set_buffer));
		printk_once(KERN_WARNING "%s calls setitimer() with new_value NULL pointer."
			    " Misfeature support will be removed\n",
			    current->comm);
	}

	error = do_setitimer64(which, &set_buffer, ovalue ? &get_buffer : NULL);
	if (error || !ovalue)
		return error;

	if (copy_to_user(ovalue, &get_buffer, sizeof(get_buffer)))
		return -EFAULT;
	return 0;
}
