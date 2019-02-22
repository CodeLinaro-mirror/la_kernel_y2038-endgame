/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VDSO_HELPERS_H
#define __VDSO_HELPERS_H

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

#include <vdso/datapage.h>

static __always_inline notrace u32 vdso_read_begin(const struct vdso_data *vd)
{
	u32 seq;

repeat:
	seq = READ_ONCE(vd->seq);
	if (seq & 1) {
		cpu_relax();
		goto repeat;
	}

	smp_rmb();
	return seq;
}

static __always_inline notrace u32 vdso_read_retry(const struct vdso_data *vd,
						   u32 start)
{
	u32 seq;

	smp_rmb();
	seq = READ_ONCE(vd->seq);
	return seq != start;
}

static __always_inline notrace void vdso_write_begin(struct vdso_data *vd)
{
	++vd->seq;
	smp_wmb();
}

static __always_inline notrace void vdso_write_end(struct vdso_data *vd)
{
	smp_wmb();
	++vd->seq;
}

#endif /* !__ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* __VDSO_HELPERS_H */
