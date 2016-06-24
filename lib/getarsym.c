/*
 * getarsym.c - implementation of the cgcef_getarsym(3) function.
 * Copyright (C) 1995 - 1998, 2004 Michael Riepe
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
#include <byteswap.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: getarsym.c,v 1.9 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

CGCEf_Arsym*
cgcef_getarsym(CGCEf *cgcef, size_t *ptr) {
    CGCEf_Arsym *syms;
    size_t count;
    size_t tmp;
    size_t i;
    char *s;
    char *e;

    if (!ptr) {
	ptr = &tmp;
    }
    *ptr = 0;
    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_AR) {
	seterr(ERROR_NOTARCHIVE);
	return NULL;
    }
    if (cgcef->e_symtab && !cgcef->e_free_syms) {
	if (cgcef->e_symlen < 4) {
	    seterr(ERROR_SIZE_ARSYMTAB);
	    return NULL;
	}
	count = __load_u32M(cgcef->e_symtab);
	if (cgcef->e_symlen < 4 * (count + 1)) {
	    seterr(ERROR_SIZE_ARSYMTAB);
	    return NULL;
	}
	if (!(syms = (CGCEf_Arsym*)malloc((count + 1) * sizeof(*syms)))) {
	    seterr(ERROR_MEM_ARSYMTAB);
	    return NULL;
	}
	s = cgcef->e_symtab + 4 * (count + 1);
	e = cgcef->e_symtab + cgcef->e_symlen;
	for (i = 0; i < count; i++, s++) {
	    syms[i].as_name = s;
	    while (s < e && *s) {
		s++;
	    }
	    if (s >= e) {
		seterr(ERROR_SIZE_ARSYMTAB);
		free(syms);
		return NULL;
	    }
	    cgcef_assert(!*s);
	    syms[i].as_hash = cgcef_hash((unsigned char*)syms[i].as_name);
	    syms[i].as_off = __load_u32M(cgcef->e_symtab + 4 * (i + 1));
	}
	syms[count].as_name = NULL;
	syms[count].as_hash = ~0UL;
	syms[count].as_off = 0;
	cgcef->e_symtab = (char*)syms;
	cgcef->e_symlen = count + 1;
	cgcef->e_free_syms = 1;
    }
    *ptr = cgcef->e_symlen;
    return (CGCEf_Arsym*)cgcef->e_symtab;
}
