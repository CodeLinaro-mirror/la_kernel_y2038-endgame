/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/cmpxchg.h
 *
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_CMPXCHG_H
#define __ASM_CMPXCHG_H

#include <linux/build_bug.h>
#include <linux/compiler.h>

#include <asm/barrier.h>
#include <asm/lse.h>

/*
 * We need separate acquire parameters for ll/sc and lse, since the full
 * barrier case is generated as release+dmb for the former and
 * acquire+release for the latter.
 */
#define __XCHG_CASE(w, sfx, name, sz, mb, nop_lse, acq, acq_lse, rel, cl)	\
static inline u##sz arch_xchg ## sz ## name(volatile u##sz *ptr, u##sz x)	\
{										\
	u##sz ret;								\
	unsigned long tmp;							\
										\
	asm volatile(ARM64_LSE_ATOMIC_INSN(					\
	/* LL/SC */								\
	"	prfm	pstl1strm, %2\n"					\
	"1:	ld" #acq "xr" #sfx "\t%" #w "0, %2\n"				\
	"	st" #rel "xr" #sfx "\t%w1, %" #w "3, %2\n"			\
	"	cbnz	%w1, 1b\n"						\
	"	" #mb,								\
	/* LSE atomics */							\
	"	swp" #acq_lse #rel #sfx "\t%" #w "3, %" #w "0, %2\n"		\
		__nops(3)							\
	"	" #nop_lse)							\
	: "=&r" (ret), "=&r" (tmp), "+Q" (*(u##sz *)ptr)			\
	: "r" (x)								\
	: cl);									\
										\
	return ret;								\
}

__XCHG_CASE(w, b, _relaxed,  8,        ,    ,  ,  ,  ,         )
__XCHG_CASE(w, h, _relaxed, 16,        ,    ,  ,  ,  ,         )
__XCHG_CASE(w,  , _relaxed, 32,        ,    ,  ,  ,  ,         )
__XCHG_CASE( ,  , _relaxed, 64,        ,    ,  ,  ,  ,         )
__XCHG_CASE(w, b, _acquire,  8,        ,    , a, a,  , "memory")
__XCHG_CASE(w, h, _acquire, 16,        ,    , a, a,  , "memory")
__XCHG_CASE(w,  , _acquire, 32,        ,    , a, a,  , "memory")
__XCHG_CASE( ,  , _acquire, 64,        ,    , a, a,  , "memory")
__XCHG_CASE(w, b, _release,  8,        ,    ,  ,  , l, "memory")
__XCHG_CASE(w, h, _release, 16,        ,    ,  ,  , l, "memory")
__XCHG_CASE(w,  , _release, 32,        ,    ,  ,  , l, "memory")
__XCHG_CASE( ,  , _release, 64,        ,    ,  ,  , l, "memory")
__XCHG_CASE(w, b,         ,  8, dmb ish, nop,  , a, l, "memory")
__XCHG_CASE(w, h,         , 16, dmb ish, nop,  , a, l, "memory")
__XCHG_CASE(w,  ,         , 32, dmb ish, nop,  , a, l, "memory")
__XCHG_CASE( ,  ,         , 64, dmb ish, nop,  , a, l, "memory")

#undef __XCHG_CASE

#define __xchg_wrapper(sfx, ptr, x)					\
({									\
 	__auto_type __ptr = (ptr);					\
	BUILD_BUG_ON(sizeof(*__ptr) != 8);				\
	(typeof(*__ptr))arch_xchg64 ## sfx((void *)(__ptr), (unsigned long)(x));\
})

/* xchg */
#define arch_xchg_relaxed(...)	 __xchg_wrapper(_relaxed, __VA_ARGS__)
#define arch_xchg_acquire(...)	 __xchg_wrapper(_acquire, __VA_ARGS__)
#define arch_xchg_release(...)	 __xchg_wrapper(_release, __VA_ARGS__)
#define arch_xchg(...)		 __xchg_wrapper(        , __VA_ARGS__)

#define xchg32_relaxed(...) arch_xchg32_relaxed(__VA_ARGS__)
#define xchg32_acquire(...) arch_xchg32_acquire( __VA_ARGS__)
#define xchg32_release(...) arch_xchg32_release( __VA_ARGS__)
#define xchg32(...)	    arch_xchg32( __VA_ARGS__)

#define xchg16_relaxed(...) arch_xchg16_relaxed(__VA_ARGS__)
#define xchg16_acquire(...) arch_xchg16_acquire( __VA_ARGS__)
#define xchg16_release(...) arch_xchg16_release( __VA_ARGS__)
#define xchg16(...)	    arch_xchg16( __VA_ARGS__)

#define xchg8_relaxed(...)  arch_xchg8_relaxed(__VA_ARGS__)
#define xchg8_acquire(...)  arch_xchg8_acquire( __VA_ARGS__)
#define xchg8_release(...)  arch_xchg8_release( __VA_ARGS__)
#define xchg8(...)	    arch_xchg8( __VA_ARGS__)

#define __CMPXCHG_CASE(name, sz)			\
static inline u##sz arch_cmpxchg ##sz ##name(volatile u##sz *ptr,	\
					      u##sz old,		\
					      u##sz new)		\
{									\
	return __lse_ll_sc_body(_cmpxchg ## sz ##name,			\
				ptr, old, new);				\
}

__CMPXCHG_CASE(_relaxed,  8)
__CMPXCHG_CASE(_relaxed, 16)
__CMPXCHG_CASE(_relaxed, 32)
__CMPXCHG_CASE(_relaxed, 64)
__CMPXCHG_CASE(_acquire,  8)
__CMPXCHG_CASE(_acquire, 16)
__CMPXCHG_CASE(_acquire, 32)
__CMPXCHG_CASE(_acquire, 64)
__CMPXCHG_CASE(_release,  8)
__CMPXCHG_CASE(_release, 16)
__CMPXCHG_CASE(_release, 32)
__CMPXCHG_CASE(_release, 64)
__CMPXCHG_CASE(        ,  8)
__CMPXCHG_CASE(        , 16)
__CMPXCHG_CASE(        , 32)
__CMPXCHG_CASE(        , 64)

#undef __CMPXCHG_CASE

#define __CMPXCHG_DBL(name)						\
static inline long __cmpxchg_double##name(unsigned long old1,		\
					 unsigned long old2,		\
					 unsigned long new1,		\
					 unsigned long new2,		\
					 volatile void *ptr)		\
{									\
	return __lse_ll_sc_body(_cmpxchg_double##name, 			\
				old1, old2, new1, new2, ptr);		\
}

__CMPXCHG_DBL(   )
__CMPXCHG_DBL(_mb)

#undef __CMPXCHG_DBL

#define __cmpxchg_wrapper(sfx, ptr, o, n)				\
({									\
 	__auto_type __ptr = (ptr);					\
	BUILD_BUG_ON(sizeof(*__ptr) != 8);				\
	(__typeof__(*(__ptr)))						\
		arch_cmpxchg64##sfx((volatile void *)__ptr,		\
				(unsigned long)(o),			\
				(unsigned long)(n));			\
})

/* cmpxchg */
#define arch_cmpxchg_relaxed(...)	__cmpxchg_wrapper(_relaxed, __VA_ARGS__)
#define arch_cmpxchg_acquire(...)	__cmpxchg_wrapper(_acquire, __VA_ARGS__)
#define arch_cmpxchg_release(...)	__cmpxchg_wrapper(_release, __VA_ARGS__)
#define arch_cmpxchg(...)		__cmpxchg_wrapper(        , __VA_ARGS__)
#define arch_cmpxchg_local		arch_cmpxchg_relaxed

/* cmpxchg64 */
#define arch_cmpxchg64_relaxed	arch_cmpxchg64_relaxed
#define arch_cmpxchg64_acquire	arch_cmpxchg64_acquire
#define arch_cmpxchg64_release	arch_cmpxchg64_release
#define arch_cmpxchg64		arch_cmpxchg64
#define arch_cmpxchg64_local	arch_cmpxchg64_local

/* cmpxchg32 */
#define cmpxchg32_relaxed(...)	arch_cmpxchg32_relaxed(__VA_ARGS__)
#define cmpxchg32_acquire(...)	arch_cmpxchg32_acquire(__VA_ARGS__)
#define cmpxchg32_release(...)	arch_cmpxchg32_release(__VA_ARGS__)
#define cmpxchg32(...)		arch_cmpxchg32(__VA_ARGS__)
#define cmpxchg32_local(...)	arch_cmpxchg32_relaxed(__VA_ARGS__)

/* cmpxchg16 */
#define cmpxchg16_relaxed(...)	arch_cmpxchg16_relaxed(__VA_ARGS__)
#define cmpxchg16_acquire(...)	arch_cmpxchg16_acquire(__VA_ARGS__)
#define cmpxchg16_release(...)	arch_cmpxchg16_release(__VA_ARGS__)
#define cmpxchg16(...)		arch_cmpxchg16(__VA_ARGS__)
#define cmpxchg16_local(...)	arch_cmpxchg16_relaxed(__VA_ARGS__)

/* cmpxchg8 */
#define cmpxchg8_relaxed(...)	arch_cmpxchg8_relaxed(__VA_ARGS__)
#define cmpxchg8_acquire(...)	arch_cmpxchg8_acquire(__VA_ARGS__)
#define cmpxchg8_release(...)	arch_cmpxchg8_release(__VA_ARGS__)
#define cmpxchg8(...)		arch_cmpxchg8(__VA_ARGS__)
#define cmpxchg8_local		arch_cmpxchg8_relaxed

/* cmpxchg_double */
#define system_has_cmpxchg_double()     1

#define __cmpxchg_double_check(ptr1, ptr2)					\
({										\
	if (sizeof(*(ptr1)) != 8)						\
		BUILD_BUG();							\
	VM_BUG_ON((unsigned long *)(ptr2) - (unsigned long *)(ptr1) != 1);	\
})

#define arch_cmpxchg_double(ptr1, ptr2, o1, o2, n1, n2)				\
({										\
	int __ret;								\
	__cmpxchg_double_check(ptr1, ptr2);					\
	__ret = !__cmpxchg_double_mb((unsigned long)(o1), (unsigned long)(o2),	\
				     (unsigned long)(n1), (unsigned long)(n2),	\
				     ptr1);					\
	__ret;									\
})

#define arch_cmpxchg_double_local(ptr1, ptr2, o1, o2, n1, n2)			\
({										\
	int __ret;								\
	__cmpxchg_double_check(ptr1, ptr2);					\
	__ret = !__cmpxchg_double((unsigned long)(o1), (unsigned long)(o2),	\
				  (unsigned long)(n1), (unsigned long)(n2),	\
				  ptr1);					\
	__ret;									\
})

#define __CMPWAIT_CASE(w, sfx, sz)					\
static inline void __cmpwait_case_##sz(volatile void *ptr,		\
				       unsigned long val)		\
{									\
	unsigned long tmp;						\
									\
	asm volatile(							\
	"	sevl\n"							\
	"	wfe\n"							\
	"	ldxr" #sfx "\t%" #w "[tmp], %[v]\n"			\
	"	eor	%" #w "[tmp], %" #w "[tmp], %" #w "[val]\n"	\
	"	cbnz	%" #w "[tmp], 1f\n"				\
	"	wfe\n"							\
	"1:"								\
	: [tmp] "=&r" (tmp), [v] "+Q" (*(unsigned long *)ptr)		\
	: [val] "r" (val));						\
}

__CMPWAIT_CASE(w, b, 8);
__CMPWAIT_CASE(w, h, 16);
__CMPWAIT_CASE(w,  , 32);
__CMPWAIT_CASE( ,  , 64);

#undef __CMPWAIT_CASE

#define __CMPWAIT_GEN(sfx)						\
static __always_inline void __cmpwait##sfx(volatile void *ptr,		\
				  unsigned long val,			\
				  int size)				\
{									\
	switch (size) {							\
	case 4:								\
		return __cmpwait_case##sfx##_32(ptr, val);		\
	case 8:								\
		return __cmpwait_case##sfx##_64(ptr, val);		\
	default:							\
		BUILD_BUG();						\
	}								\
									\
	unreachable();							\
}

__CMPWAIT_GEN()

#undef __CMPWAIT_GEN

#define __cmpwait_relaxed(ptr, val) \
	__cmpwait((ptr), (unsigned long)(val), sizeof(*(ptr)))

#endif	/* __ASM_CMPXCHG_H */
