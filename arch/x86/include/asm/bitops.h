/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_BITOPS_H
#define _ASM_X86_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 *
 * Note: inlines with more than a single statement should be marked
 * __always_inline to avoid problems with older gcc's inlining heuristics.
 */

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/compiler.h>
#include <asm/alternative.h>
#include <asm/rmwcc.h>
#include <asm/barrier.h>

#if BITS_PER_LONG == 32
# define _BITOPS_LONG_SHIFT 5
#elif BITS_PER_LONG == 64
# define _BITOPS_LONG_SHIFT 6
#else
# error "Unexpected BITS_PER_LONG"
#endif

#define BIT_64(n)			(U64_C(1) << (n))

/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. All bit operations return 0 if the bit
 * was cleared before the operation and != 0 if it was not.
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 */

#define RLONG_ADDR(x)			 "m" (*(volatile long *) (x))
#define WBYTE_ADDR(x)			"+m" (*(volatile char *) (x))

#define ADDR				RLONG_ADDR(addr)

/*
 * We do the locked ops that don't return the old value as
 * a mask operation on a byte.
 */
#define CONST_MASK_ADDR(nr, addr)	WBYTE_ADDR((void *)(addr) + ((nr)>>3))
#define CONST_MASK(nr)			(1 << ((nr) & 7))

static __always_inline void
arch_set_bit(long nr, volatile unsigned long *addr)
{
	if (__builtin_constant_p(nr)) {
		asm_inline volatile(LOCK_PREFIX "orb %b1,%0"
			: CONST_MASK_ADDR(nr, addr)
			: "iq" (CONST_MASK(nr))
			: "memory");
	} else {
		asm_inline volatile(LOCK_PREFIX __ASM_SIZE(bts) " %1,%0"
			: : RLONG_ADDR(addr), "Ir" (nr) : "memory");
	}
}

static __always_inline void
arch___set_bit(unsigned long nr, volatile unsigned long *addr)
{
	asm volatile(__ASM_SIZE(bts) " %1,%0" : : ADDR, "Ir" (nr) : "memory");
}

static __always_inline void
arch_clear_bit(long nr, volatile unsigned long *addr)
{
	if (__builtin_constant_p(nr)) {
		asm_inline volatile(LOCK_PREFIX "andb %b1,%0"
			: CONST_MASK_ADDR(nr, addr)
			: "iq" (~CONST_MASK(nr)));
	} else {
		asm_inline volatile(LOCK_PREFIX __ASM_SIZE(btr) " %1,%0"
			: : RLONG_ADDR(addr), "Ir" (nr) : "memory");
	}
}

static __always_inline void
arch_clear_bit_unlock(long nr, volatile unsigned long *addr)
{
	barrier();
	arch_clear_bit(nr, addr);
}

static __always_inline void
arch___clear_bit(unsigned long nr, volatile unsigned long *addr)
{
	asm volatile(__ASM_SIZE(btr) " %1,%0" : : ADDR, "Ir" (nr) : "memory");
}

static __always_inline bool arch_xor_unlock_is_negative_byte(unsigned long mask,
		volatile unsigned long *addr)
{
	bool negative;
	asm_inline volatile(LOCK_PREFIX "xorb %2,%1"
		: "=@ccs" (negative), WBYTE_ADDR(addr)
		: "iq" ((char)mask) : "memory");
	return negative;
}
#define arch_xor_unlock_is_negative_byte arch_xor_unlock_is_negative_byte

static __always_inline void
arch___clear_bit_unlock(long nr, volatile unsigned long *addr)
{
	arch___clear_bit(nr, addr);
}

static __always_inline void
arch___change_bit(unsigned long nr, volatile unsigned long *addr)
{
	asm volatile(__ASM_SIZE(btc) " %1,%0" : : ADDR, "Ir" (nr) : "memory");
}

static __always_inline void
arch_change_bit(long nr, volatile unsigned long *addr)
{
	if (__builtin_constant_p(nr)) {
		asm_inline volatile(LOCK_PREFIX "xorb %b1,%0"
			: CONST_MASK_ADDR(nr, addr)
			: "iq" (CONST_MASK(nr)));
	} else {
		asm_inline volatile(LOCK_PREFIX __ASM_SIZE(btc) " %1,%0"
			: : RLONG_ADDR(addr), "Ir" (nr) : "memory");
	}
}

static __always_inline bool
arch_test_and_set_bit(long nr, volatile unsigned long *addr)
{
	return GEN_BINARY_RMWcc(LOCK_PREFIX __ASM_SIZE(bts), *addr, c, "Ir", nr);
}

static __always_inline bool
arch_test_and_set_bit_lock(long nr, volatile unsigned long *addr)
{
	return arch_test_and_set_bit(nr, addr);
}

static __always_inline bool
arch___test_and_set_bit(unsigned long nr, volatile unsigned long *addr)
{
	bool oldbit;

	asm(__ASM_SIZE(bts) " %2,%1"
	    : "=@ccc" (oldbit)
	    : ADDR, "Ir" (nr) : "memory");
	return oldbit;
}

static __always_inline bool
arch_test_and_clear_bit(long nr, volatile unsigned long *addr)
{
	return GEN_BINARY_RMWcc(LOCK_PREFIX __ASM_SIZE(btr), *addr, c, "Ir", nr);
}

/*
 * Note: the operation is performed atomically with respect to
 * the local CPU, but not other CPUs. Portable code should not
 * rely on this behaviour.
 * KVM relies on this behaviour on x86 for modifying memory that is also
 * accessed from a hypervisor on the same CPU if running in a VM: don't change
 * this without also updating arch/x86/kernel/kvm.c
 */
static __always_inline bool
arch___test_and_clear_bit(unsigned long nr, volatile unsigned long *addr)
{
	bool oldbit;

	asm volatile(__ASM_SIZE(btr) " %2,%1"
		     : "=@ccc" (oldbit)
		     : ADDR, "Ir" (nr) : "memory");
	return oldbit;
}

static __always_inline bool
arch___test_and_change_bit(unsigned long nr, volatile unsigned long *addr)
{
	bool oldbit;

	asm volatile(__ASM_SIZE(btc) " %2,%1"
		     : "=@ccc" (oldbit)
		     : ADDR, "Ir" (nr) : "memory");

	return oldbit;
}

static __always_inline bool
arch_test_and_change_bit(long nr, volatile unsigned long *addr)
{
	return GEN_BINARY_RMWcc(LOCK_PREFIX __ASM_SIZE(btc), *addr, c, "Ir", nr);
}

static __always_inline bool constant_test_bit(long nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr & (BITS_PER_LONG-1))) &
		(addr[nr >> _BITOPS_LONG_SHIFT])) != 0;
}

static __always_inline bool constant_test_bit_acquire(long nr, const volatile unsigned long *addr)
{
	bool oldbit;

	asm volatile("testb %2,%1"
		     : "=@ccnz" (oldbit)
		     : "m" (((unsigned char *)addr)[nr >> 3]),
		       "i" (1 << (nr & 7))
		     :"memory");

	return oldbit;
}

static __always_inline bool variable_test_bit(long nr, volatile const unsigned long *addr)
{
	bool oldbit;

	asm volatile(__ASM_SIZE(bt) " %2,%1"
		     : "=@ccc" (oldbit)
		     : "m" (*(unsigned long *)addr), "Ir" (nr) : "memory");

	return oldbit;
}

static __always_inline bool
arch_test_bit(unsigned long nr, const volatile unsigned long *addr)
{
	return __builtin_constant_p(nr) ? constant_test_bit(nr, addr) :
					  variable_test_bit(nr, addr);
}

static __always_inline bool
arch_test_bit_acquire(unsigned long nr, const volatile unsigned long *addr)
{
	return __builtin_constant_p(nr) ? constant_test_bit_acquire(nr, addr) :
					  variable_test_bit(nr, addr);
}

#undef ADDR

#include <asm-generic/bitops/__ffs.h>
#include <asm-generic/bitops/ffz.h>
#include <asm-generic/bitops/builtin-fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>


#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/builtin-ffs.h>
#include <asm/arch_hweight.h>
#include <asm-generic/bitops/const_hweight.h>

#include <asm-generic/bitops/instrumented-atomic.h>
#include <asm-generic/bitops/instrumented-non-atomic.h>
#include <asm-generic/bitops/instrumented-lock.h>

#include <asm-generic/bitops/le.h>

#include <asm-generic/bitops/ext2-atomic-setbit.h>

#endif /* _ASM_X86_BITOPS_H */
