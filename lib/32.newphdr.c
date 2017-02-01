/*
 * 32.newphdr.c - implementation of the cgcef{32,64}_newphdr(3) functions.
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
static const char rcsid[] = "@(#) $Id: 32.newphdr.c,v 1.16 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

static char*
_cgcef_newphdr(CGCEf *cgcef, size_t count, unsigned cls) {
    size_t extcount = 0;
    CGCEf_Scn *scn = NULL;
    char *phdr = NULL;
    size_t size;

    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (!cgcef->e_ehdr && !cgcef->e_readable) {
	seterr(ERROR_NOEHDR);
    }
    else if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
    }
    else if (cgcef->e_class != cls) {
	seterr(ERROR_CLASSMISMATCH);
    }
    else if (cgcef->e_ehdr || _cgcef_cook(cgcef)) {
	size = _msize(cls, _cgcef_version, CGCEF_T_PHDR);
	cgcef_assert(size);
	if (!(scn = _cgcef_first_scn(cgcef))) {
	    return NULL;
	}
	if (count) {
	    if (!(phdr = (char*)malloc(count * size))) {
		seterr(ERROR_MEM_PHDR);
		return NULL;
	    }
	    memset(phdr, 0, count * size);
	}
	cgcef_assert(cgcef->e_ehdr);
	cgcef->e_phnum = count;
	if (count >= PN_XNUM) {
	    /*
	     * get NULL section (create it if necessary)
	     */
	    extcount = count;
	    count = PN_XNUM;
	}
	if (cls == CGCEFCLASS32) {
	    ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_phnum = count;
	    scn->s_shdr32.sh_info = extcount;
	}
#if __LIBCGCEF64
	else if (cls == CGCEFCLASS64) {
	    ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_phnum = count;
	    scn->s_shdr64.sh_info = extcount;
	}
#endif /* __LIBCGCEF64 */
	else {
	    seterr(ERROR_UNIMPLEMENTED);
	    if (phdr) {
		free(phdr);
	    }
	    return NULL;
	}
	if (cgcef->e_phdr) {
	    free(cgcef->e_phdr);
	}
	cgcef->e_phdr = phdr;
	cgcef->e_phdr_flags |= CGCEF_F_DIRTY;
	cgcef->e_ehdr_flags |= CGCEF_F_DIRTY;
	scn->s_scn_flags |= CGCEF_F_DIRTY;
	return phdr;
    }
    return NULL;
}

CGCEf32_Phdr*
cgcef32_newphdr(CGCEf *cgcef, size_t count) {
    return (CGCEf32_Phdr*)_cgcef_newphdr(cgcef, count, CGCEFCLASS32);
}

#if __LIBCGCEF64

CGCEf64_Phdr*
cgcef64_newphdr(CGCEf *cgcef, size_t count) {
    return (CGCEf64_Phdr*)_cgcef_newphdr(cgcef, count, CGCEFCLASS64);
}

unsigned long
gcgcef_newphdr(CGCEf *cgcef, size_t phnum) {
    if (!valid_class(cgcef->e_class)) {
	seterr(ERROR_UNKNOWN_CLASS);
	return 0;
    }
    return (unsigned long)_cgcef_newphdr(cgcef, phnum, cgcef->e_class);
}

#endif /* __LIBCGCEF64 */
