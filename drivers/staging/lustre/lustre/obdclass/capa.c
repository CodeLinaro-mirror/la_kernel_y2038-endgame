/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * lustre/obdclass/capa.c
 *
 * Lustre Capability Hash Management
 *
 * Author: Lai Siyao<lsy@clusterfs.com>
 */

#define DEBUG_SUBSYSTEM S_SEC

#include <linux/fs.h>
#include <asm/unistd.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/crypto.h>

#include "../include/obd_class.h"
#include "../include/lustre_debug.h"
#include "../include/lustre/lustre_idl.h"

#include <linux/list.h>
#include "../include/lustre_capa.h"

#define NR_CAPAHASH 32
#define CAPA_HASH_SIZE 3000	      /* for MDS & OSS */

struct kmem_cache *capa_cachep;

/* lock for capa hash/capa_list/fo_capa_keys */
DEFINE_SPINLOCK(capa_lock);

struct list_head capa_list[CAPA_SITE_MAX];

/* capa count */
int capa_count[CAPA_SITE_MAX] = { 0, };

EXPORT_SYMBOL(capa_cachep);
EXPORT_SYMBOL(capa_list);
EXPORT_SYMBOL(capa_lock);
EXPORT_SYMBOL(capa_count);

void capa_cpy(void *capa, struct obd_capa *ocapa)
{
	spin_lock(&ocapa->c_lock);
	*(struct lustre_capa *)capa = ocapa->c_capa;
	spin_unlock(&ocapa->c_lock);
}
EXPORT_SYMBOL(capa_cpy);

void _debug_capa(struct lustre_capa *c,
		 struct libcfs_debug_msg_data *msgdata,
		 const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	libcfs_debug_vmsg2(msgdata, fmt, args,
			   " capability@%p fid " DFID " opc %#llx uid %llu gid %llu flags %u alg %d keyid %u timeout %u expiry %u\n",
			   c, PFID(capa_fid(c)), capa_opc(c),
			   capa_uid(c), capa_gid(c), capa_flags(c),
			   capa_alg(c), capa_keyid(c), capa_timeout(c),
			   capa_expiry(c));
	va_end(args);
}
EXPORT_SYMBOL(_debug_capa);
