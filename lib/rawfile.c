/*
 * rawfile.c - implementation of the cgcef_rawfile(3) function.
 * Copyright (C) 1995 - 2009 Michael Riepe
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
static const char rcsid[] = "@(#) $Id: rawfile.c,v 1.8 2009/05/22 17:07:46 michael Exp $";
#endif /* lint */

char*
cgcef_rawfile(CGCEf *cgcef, size_t *ptr) {
    size_t tmp;

    if (!ptr) {
	ptr = &tmp;
    }
    *ptr = 0;
    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (!cgcef->e_readable) {
	return NULL;
    }
    else if (cgcef->e_size) {
	if (!cgcef->e_rawdata) {
	    cgcef_assert(cgcef->e_data);
	    if (!cgcef->e_cooked) {
		cgcef->e_rawdata = cgcef->e_data;
	    }
	    else if (!(cgcef->e_rawdata = _cgcef_read(cgcef, NULL, 0, cgcef->e_size))) {
		return NULL;
	    }
	}
	*ptr = cgcef->e_size;
    }
    return cgcef->e_rawdata;
}
