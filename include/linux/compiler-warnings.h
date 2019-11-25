#ifndef __LINUX_COMPILER_WARNINGS_H
#define __LINUX_COMPILER_WARNINGS_H

/* building blocks: support for "make W=0123" and "E=0123" */
#ifdef KBUILD_ERR0
#define KBUILD_WARN_LEVEL0 error
#elif defined(KBUILD_WARN0)
#define KBUILD_WARN_LEVEL0 warning
#else
#define KBUILD_WARN_LEVEL0 ignored
#endif

#ifdef KBUILD_ERR1
#define KBUILD_WARN_LEVEL1 error
#elif defined(KBUILD_EXTRA_WARN1)
#define KBUILD_WARN_LEVEL1 warning
#else
#define KBUILD_WARN_LEVEL1 ignored
#endif

#ifdef KBUILD_ERR2
#define KBUILD_WARN_LEVEL2 error
#elif defined(KBUILD_EXTRA_WARN2)
#define KBUILD_WARN_LEVEL2 warning
#else
#define KBUILD_WARN_LEVEL2 ignored
#endif

#ifdef KBUILD_ERR3
#define KBUILD_WARN_LEVEL3 error
#elif defined(KBUILD_EXTRA_WARN3)
#define KBUILD_WARN_LEVEL3 warning
#else
#define KBUILD_WARN_LEVEL3 ignored
#endif

#ifdef KBUILD_ERR4
#define KBUILD_WARN_LEVEL4 error
#elif defined(KBUILD_EXTRA_WARN4)
#define KBUILD_WARN_LEVEL4 warning
#else
#define KBUILD_WARN_LEVEL4 ignored
#endif

/* building blocks: support for compiler versions */
#if defined __CHECKER__
#define __KBUILD_WARN_LEVEL(arg) /* empty */
#else
#define __KBUILD_WARN_LEVEL(arg) _Pragma(#arg)
#endif

#if defined(GCC_VERSION)
#define KBUILD_WARN_LEVEL(level, warning)  \
	__KBUILD_WARN_LEVEL(GCC diagnostic level warning)

#define KBUILD_WARN_LEVEL_GCC_4_6(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_4_6(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 40700
#define KBUILD_WARN_LEVEL_GCC_4_7(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_4_7(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 40800
#define KBUILD_WARN_LEVEL_GCC_4_8(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_4_8(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 40900
#define KBUILD_WARN_LEVEL_GCC_4_9(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_4_9(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 50000
#define KBUILD_WARN_LEVEL_GCC_5(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_5(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 60000
#define KBUILD_WARN_LEVEL_GCC_6(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_6(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 70000
#define KBUILD_WARN_LEVEL_GCC_7(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_7(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 80000
#define KBUILD_WARN_LEVEL_GCC_8(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_8(level, warning)
#endif

#if defined(GCC_VERSION) && GCC_VERSION >= 90000
#define KBUILD_WARN_LEVEL_GCC_9(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_GCC_9(level, warning)
#endif

#if defined(__clang__)
#define KBUILD_WARN_LEVEL(level, warning)  \
	__KBUILD_WARN_LEVEL(clang diagnostic level warning)

#define KBUILD_WARN_LEVEL_CLANG_8(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_CLANG_8(level, warning)
#endif

#if defined(CONFIG_CLANG_VERSION) && CONFIG_CLANG_VERSION >= 90000
#define KBUILD_WARN_LEVEL_CLANG_9(level, warning) \
	KBUILD_WARN_LEVEL(level, warning)
#else
#define KBUILD_WARN_LEVEL_CLANG_9(level, warning)
#endif

#define KBUILD_WARN(level, ver, warning) \
	KBUILD_WARN_LEVEL_ ## ver(KBUILD_WARN_LEVEL ## level, warning)

#include <linux/warnings.h>

#endif /* __LINUX_COMPILER_WARNINGS_H */
