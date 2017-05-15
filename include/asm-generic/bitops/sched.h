#ifndef _ASM_GENERIC_BITOPS_SCHED_H_
#define _ASM_GENERIC_BITOPS_SCHED_H_

#include <linux/compiler.h>	/* unlikely() */
#include <asm/types.h>

/*
 * Every architecture must define this function. It's the fastest
 * way of searching a 100-bit bitmap.  It's guaranteed that at least
 * one of the 100 bits is cleared, so we can pass '128' as the length
 * to save one comparison.
 */
#define sched_find_first_bit(b) find_first_bit(b, 128)

#endif /* _ASM_GENERIC_BITOPS_SCHED_H_ */
