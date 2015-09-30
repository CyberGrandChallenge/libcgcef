/*
 * getaroff.c - implementation of the cgcef_getaroff(3) function.
 * Copyright (C) 2009 Michael Riepe
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <private.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: getaroff.c,v 1.1 2009/11/01 13:04:19 michael Exp $";
#endif /* lint */

off_t
cgcef_getaroff(CGCEf *cgcef) {
    CGCEf *ref;

    if (!cgcef) {
	return (off_t)-1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (!(ref = cgcef->e_parent)) {
	return (off_t)-1;
    }
    cgcef_assert(ref->e_magic == CGCEF_MAGIC);
    cgcef_assert(cgcef->e_base >= ref->e_base + SARMAG + sizeof(struct ar_hdr));
    return (off_t)(cgcef->e_base - ref->e_base - sizeof(struct ar_hdr));
}
