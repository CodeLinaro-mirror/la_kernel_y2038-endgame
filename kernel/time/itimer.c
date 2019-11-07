// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 1992 Darren Senn
 */

/* These are all the functions necessary to implement itimers */

#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/sched/signal.h>
#include <linux/sched/cputime.h>
#include <linux/posix-timers.h>
#include <linux/hrtimer.h>
#include <trace/events/timer.h>
#include <linux/compat.h>

#include <linux/uaccess.h>

/**
 * itimer_get_remtime - get remaining time for the timer
 *
 * @timer: the timer to read
 *
 * Returns the delta between the expiry time and now, which can be
 * less than zero or 1usec for an pending expired timer
 */
static ktime_t itimer_get_remtime(struct hrtimer *timer)
{
	ktime_t rem = __hrtimer_get_remaining(timer, true);

	/*
	 * Racy but safe: if the itimer expires after the above
	 * hrtimer_get_remtime() call but before this condition
	 * then we return 0 - which is correct.
	 */
	if (hrtimer_active(timer)) {
		if (rem <= 0)
			rem = NSEC_PER_USEC;
	} else
		rem = 0;

	return rem;
}

static void get_cpu_itimer(struct task_struct *tsk, unsigned int clock_id,
			   ktime_t *ovalue, ktime_t *ointerval)
{
	u64 val, interval;
	struct cpu_itimer *it = &tsk->signal->it[clock_id];

	spin_lock_irq(&tsk->sighand->siglock);

	val = it->expires;
	interval = it->incr;
	if (val) {
		u64 t, samples[CPUCLOCK_MAX];

		thread_group_sample_cputime(tsk, samples);
		t = samples[clock_id];

		if (val < t)
			/* about to fire */
			val = TICK_NSEC;
		else
			val -= t;
	}

	spin_unlock_irq(&tsk->sighand->siglock);

	*ovalue = ns_to_ktime(val);
	*ointerval = ns_to_ktime(interval);
}

static int do_getitimer(int which, ktime_t *value, ktime_t *interval)
{
	struct task_struct *tsk = current;

	switch (which) {
	case ITIMER_REAL:
		spin_lock_irq(&tsk->sighand->siglock);
		*value = itimer_get_remtime(&tsk->signal->real_timer);
		*interval = tsk->signal->it_real_incr;
		spin_unlock_irq(&tsk->sighand->siglock);
		break;
	case ITIMER_VIRTUAL:
		get_cpu_itimer(tsk, CPUCLOCK_VIRT, value, interval);
		break;
	case ITIMER_PROF:
		get_cpu_itimer(tsk, CPUCLOCK_PROF, value, interval);
		break;
	default:
		return(-EINVAL);
	}
	return 0;
}

static int put_itimerval(struct itimerval __user *o,
			 ktime_t value, ktime_t interval)
{
	struct itimerval v;
	struct timespec64 ts;

	ts = ktime_to_timespec64(interval);
	v.it_interval.tv_sec = ts.tv_sec;
	v.it_interval.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	ts = ktime_to_timespec64(value);
	v.it_value.tv_sec = ts.tv_sec;
	v.it_value.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	return copy_to_user(o, &v, sizeof(struct itimerval)) ? -EFAULT : 0;
}


SYSCALL_DEFINE2(getitimer, int, which, struct itimerval __user *, it)
{
	ktime_t value, interval;
	int error;

	error = do_getitimer(which, &value, &interval);
	if (error)
		return error;

	return put_itimerval(it, value, interval);
}

#if defined(CONFIG_COMPAT) || defined(CONFIG_ALPHA)
struct old_itimerval32 {
	struct old_timeval32	it_interval;
	struct old_timeval32	it_value;
};

static int put_old_itimerval32(struct old_itimerval32 __user *o,
			       ktime_t value, ktime_t interval)
{
	struct old_itimerval32 v32;
	struct timespec64 ts;

	ts = ktime_to_timespec64(interval);
	v32.it_interval.tv_sec = ts.tv_sec;
	v32.it_interval.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	ts = ktime_to_timespec64(value);
	v32.it_value.tv_sec = ts.tv_sec;
	v32.it_value.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	return copy_to_user(o, &v32, sizeof(struct old_itimerval32)) ? -EFAULT : 0;
}

COMPAT_SYSCALL_DEFINE2(getitimer, int, which,
		       struct old_itimerval32 __user *, it)
{
	ktime_t value, interval;
	int error;

	error = do_getitimer(which, &value, &interval);
	if (error)
		return error;

	return put_old_itimerval32(it, value, interval);
}
#endif

/*
 * The timer is automagically restarted, when interval != 0
 */
enum hrtimer_restart it_real_fn(struct hrtimer *timer)
{
	struct signal_struct *sig =
		container_of(timer, struct signal_struct, real_timer);
	struct pid *leader_pid = sig->pids[PIDTYPE_TGID];

	trace_itimer_expire(ITIMER_REAL, leader_pid, 0);
	kill_pid_info(SIGALRM, SEND_SIG_PRIV, leader_pid);

	return HRTIMER_NORESTART;
}

static void set_cpu_itimer(struct task_struct *tsk, unsigned int clock_id,
			   ktime_t value, ktime_t interval,
			   ktime_t *ovalue, ktime_t *ointerval)
{
	u64 oval, nval;
	struct cpu_itimer *it = &tsk->signal->it[clock_id];

	nval = value;

	spin_lock_irq(&tsk->sighand->siglock);

	oval = it->expires;
	if (ointerval)
		*ointerval = it->incr;

	if (oval || nval) {
		if (nval > 0)
			nval += TICK_NSEC;
		set_process_cpu_timer(tsk, clock_id, &nval, &oval);
	}

	it->expires = nval;
	it->incr = interval;

	trace_itimer_state(clock_id == CPUCLOCK_VIRT ?
			   ITIMER_VIRTUAL : ITIMER_PROF, value, interval, nval);

	spin_unlock_irq(&tsk->sighand->siglock);

	if (ovalue)
		*ovalue = oval;
}

#define timeval_valid(t) \
	(((t)->tv_sec >= 0) && (((unsigned long) (t)->tv_usec) < USEC_PER_SEC))

static int do_setitimer(int which, ktime_t value, ktime_t interval,
			ktime_t *ovalue, ktime_t *ointerval)
{
	struct task_struct *tsk = current;
	struct hrtimer *timer;

	switch (which) {
	case ITIMER_REAL:
again:
		spin_lock_irq(&tsk->sighand->siglock);
		timer = &tsk->signal->real_timer;
		if (ovalue)
			*ovalue = itimer_get_remtime(timer);
		if (ointerval)
			*ointerval = tsk->signal->it_real_incr;
		/* We are sharing ->siglock with it_real_fn() */
		if (hrtimer_try_to_cancel(timer) < 0) {
			spin_unlock_irq(&tsk->sighand->siglock);
			hrtimer_cancel_wait_running(timer);
			goto again;
		}
		if (value != 0) {
			tsk->signal->it_real_incr = interval;
			hrtimer_start(timer, value, HRTIMER_MODE_REL);
		} else
			tsk->signal->it_real_incr = 0;

		trace_itimer_state(ITIMER_REAL, value, interval, 0);
		spin_unlock_irq(&tsk->sighand->siglock);
		break;
	case ITIMER_VIRTUAL:
		set_cpu_itimer(tsk, CPUCLOCK_VIRT, value, interval,
			       ovalue, ointerval);
		break;
	case ITIMER_PROF:
		set_cpu_itimer(tsk, CPUCLOCK_PROF, value, interval,
			       ovalue, ointerval);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#ifdef CONFIG_SECURITY_SELINUX
void clear_itimer(void)
{
	int i;

	for (i = 0; i < 3; i++)
		do_setitimer(i, 0, 0, NULL, NULL);
}
#endif

#ifdef __ARCH_WANT_SYS_ALARM

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
static unsigned int alarm_setitimer(unsigned int seconds)
{
	ktime_t old;

#if BITS_PER_LONG < 64
	if (seconds > INT_MAX)
		seconds = INT_MAX;
#endif

	do_setitimer(ITIMER_REAL, ktime_set(seconds, 0), 0, &old, NULL);

	/*
	 * We can't return 0 if we have an alarm pending ...  And we'd
	 * better return too much than too little anyway
	 */
	if (old > 0 && old < NSEC_PER_SEC)
		return 1;

	return div_u64(old + (NSEC_PER_SEC / 2), NSEC_PER_SEC);
}

/*
 * For backwards compatibility?  This can be done in libc so Alpha
 * and all newer ports shouldn't need it.
 */
SYSCALL_DEFINE1(alarm, unsigned int, seconds)
{
	return alarm_setitimer(seconds);
}

#endif

static int get_itimerval(ktime_t *value, ktime_t *interval,
				const struct itimerval __user *i)
{
	struct itimerval v;

	if (copy_from_user(&v, i, sizeof(struct itimerval)))
		return -EFAULT;

	/* Validate the timevals in value. */
	if (!timeval_valid(&v.it_value) ||
	    !timeval_valid(&v.it_interval))
		return -EINVAL;

	*interval = ktime_set(v.it_interval.tv_sec,
			      v.it_interval.tv_usec * NSEC_PER_USEC);
	*value = ktime_set(v.it_value.tv_sec,
			   v.it_value.tv_usec * NSEC_PER_USEC);

	return 0;
}


SYSCALL_DEFINE3(setitimer, int, which, struct itimerval __user *, in,
		struct itimerval __user *, out)
{
	ktime_t value = 0, interval = 0;
	ktime_t ovalue, ointerval;
	int error;

	if (in) {
		error = get_itimerval(&value, &interval, in);
		if (error)
			return error;
	} else {
		printk_once(KERN_WARNING "%s calls setitimer() with new_value NULL pointer."
			    " Misfeature support will be removed\n",
			    current->comm);
	}

	if (!out)
		return do_setitimer(which, value, interval, NULL, NULL);

	error = do_setitimer(which, value, interval, &ovalue, &ointerval);
	if (error)
		return error;

	return put_itimerval(out, ovalue, ointerval);
}

#if defined(CONFIG_COMPAT) || defined(CONFIG_ALPHA)
static int get_old_itimerval32(ktime_t *value, ktime_t *interval,
				const struct old_itimerval32 __user *i)
{
	struct old_itimerval32 v32;

	if (copy_from_user(&v32, i, sizeof(struct old_itimerval32)))
		return -EFAULT;

	/* Validate the timevals in value.  */
	if (!timeval_valid(&v32.it_value) ||
	    !timeval_valid(&v32.it_interval))
		return -EINVAL;

	*interval = ktime_set(v32.it_interval.tv_sec,
			      v32.it_interval.tv_usec * NSEC_PER_USEC);
	*value = ktime_set(v32.it_value.tv_sec,
			   v32.it_value.tv_usec * NSEC_PER_USEC);

	return 0;
}

COMPAT_SYSCALL_DEFINE3(setitimer, int, which,
		       struct old_itimerval32 __user *, in,
		       struct old_itimerval32 __user *, out)
{
	ktime_t value = 0, interval = 0;
	ktime_t ovalue, ointerval;
	int error;

	if (in) {
		error = get_old_itimerval32(&value, &interval, in);
		if (error)
			return error;
	} else {
		printk_once(KERN_WARNING "%s calls setitimer() with new_value NULL pointer."
			    " Misfeature support will be removed\n",
			    current->comm);
	}

	if (!out)
		return do_setitimer(which, value, interval, NULL, NULL);

	error = do_setitimer(which, value, interval, &ovalue, &ointerval);
	if (error)
		return error;
	return put_old_itimerval32(out, ovalue, ointerval);
}
#endif
