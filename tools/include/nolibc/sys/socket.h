#include "../nolibc.h"

#ifndef _NOLIBC_SYS_SOCKET_H
#define _NOLIBC_SYS_SOCKET_H

#include <linux/socket.h>

typedef __kernel_sa_family_t	sa_family_t;

/*
 *	1003.1g requires sa_family_t and that sa_data is char.
 */

struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
#ifdef __clang__
	char sa_data[14];		/* prevent -Wgnu-variable-sized-type-not-at-end warning */
#else
	union {
		char sa_data_min[14];		/* Minimum 14 bytes of protocol address	*/
		__DECLARE_FLEX_ARRAY(char, sa_data);
	};
#endif
};

#endif
