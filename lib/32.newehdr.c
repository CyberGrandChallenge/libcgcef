/*
 * 32.newehdr.c - implementation of the cgcef{32,64}_newehdr(3) functions.
 * Copyright (C) 1995 - 2006 Michael Riepe
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
static const char rcsid[] = "@(#) $Id: 32.newehdr.c,v 1.16 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

static char*
_cgcef_newehdr(CGCEf *cgcef, unsigned cls) {
    size_t size;

    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_readable) {
	return _cgcef_getehdr(cgcef, cls);
    }
    else if (!cgcef->e_ehdr) {
	size = _msize(cls, _cgcef_version, CGCEF_T_EHDR);
	cgcef_assert(size);
	if ((cgcef->e_ehdr = (char*)malloc(size))) {
	    memset(cgcef->e_ehdr, 0, size);
	    cgcef->e_ehdr_flags |= CGCEF_F_DIRTY;
	    cgcef->e_kind = CGCEF_K_CGCEF;
	    cgcef->e_class = cls;
	    return cgcef->e_ehdr;
	}
	seterr(ERROR_MEM_EHDR);
    }
    else if (cgcef->e_class != cls) {
	seterr(ERROR_CLASSMISMATCH);
    }
    else {
	cgcef_assert(cgcef->e_kind == CGCEF_K_CGCEF);
	return cgcef->e_ehdr;
    }
    return NULL;
}

CGCEf32_Ehdr*
cgcef32_newehdr(CGCEf *cgcef) {
    return (CGCEf32_Ehdr*)_cgcef_newehdr(cgcef, CGCEFCLASS32);
}

#if __LIBCGCEF64

CGCEf64_Ehdr*
cgcef64_newehdr(CGCEf *cgcef) {
    return (CGCEf64_Ehdr*)_cgcef_newehdr(cgcef, CGCEFCLASS64);
}

unsigned long
gcgcef_newehdr(CGCEf *cgcef, int cls) {
    if (!valid_class(cls) || !_msize(cls, _cgcef_version, CGCEF_T_EHDR)) {
	seterr(ERROR_UNKNOWN_CLASS);
	return 0;
    }
    return (unsigned long)_cgcef_newehdr(cgcef, cls);
}

#endif /* __LIBCGCEF64 */
