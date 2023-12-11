/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_STDBOOL_H
#define _LINUX_STDBOOL_H

#if __STDC_VERSION__ <= 201710L || \
 (!defined(__clang__) && __GNUC__ < 13) || \
  (defined(__clang__) && __clang_major__ < 15)
typedef _Bool	bool;

enum {
	false	= 0,
	true	= 1
};
#endif

#endif /* _LINUX_STDBOOL_H */
