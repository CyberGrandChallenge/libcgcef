/*
 * newscn.c - implementation of the cgcef_newscn(3) function.
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
static const char rcsid[] = "@(#) $Id: newscn.c,v 1.13 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

int
_cgcef_update_shnum(CGCEf *cgcef, size_t shnum) {
    size_t extshnum = 0;
    CGCEf_Scn *scn;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_ehdr);
    scn = cgcef->e_scn_1;
    cgcef_assert(scn);
    cgcef_assert(scn->s_index == 0);
    if (shnum >= SHN_LORESERVE) {
	extshnum = shnum;
	shnum = 0;
    }
    if (cgcef->e_class == CGCEFCLASS32) {
	((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shnum = shnum;
	scn->s_shdr32.sh_size = extshnum;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shnum = shnum;
	scn->s_shdr64.sh_size = extshnum;
    }
#endif /* __LIBCGCEF64 */
    else {
	if (valid_class(cgcef->e_class)) {
	    seterr(ERROR_UNIMPLEMENTED);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	return -1;
    }
    cgcef->e_ehdr_flags |= CGCEF_F_DIRTY;
    scn->s_shdr_flags |= CGCEF_F_DIRTY;
    return 0;
}

static CGCEf_Scn*
_makescn(CGCEf *cgcef, size_t index) {
    CGCEf_Scn *scn;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    cgcef_assert(cgcef->e_ehdr);
    cgcef_assert(_cgcef_scn_init.s_magic == SCN_MAGIC);
    if (!(scn = (CGCEf_Scn*)malloc(sizeof(*scn)))) {
	seterr(ERROR_MEM_SCN);
	return NULL;
    }
    *scn = _cgcef_scn_init;
    scn->s_cgcef = cgcef;
    scn->s_scn_flags = CGCEF_F_DIRTY;
    scn->s_shdr_flags = CGCEF_F_DIRTY;
    scn->s_freeme = 1;
    scn->s_index = index;
    return scn;
}

CGCEf_Scn*
_cgcef_first_scn(CGCEf *cgcef) {
    CGCEf_Scn *scn;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if ((scn = cgcef->e_scn_1)) {
	return scn;
    }
    if ((scn = _makescn(cgcef, 0))) {
	cgcef->e_scn_1 = cgcef->e_scn_n = scn;
	if (_cgcef_update_shnum(cgcef, 1)) {
	    free(scn);
	    cgcef->e_scn_1 = cgcef->e_scn_n = scn = NULL;
	}
    }
    return scn;
}

static CGCEf_Scn*
_buildscn(CGCEf *cgcef) {
    CGCEf_Scn *scn;

    if (!_cgcef_first_scn(cgcef)) {
	return NULL;
    }
    scn = cgcef->e_scn_n;
    cgcef_assert(scn);
    if (!(scn = _makescn(cgcef, scn->s_index + 1))) {
	return NULL;
    }
    if (_cgcef_update_shnum(cgcef, scn->s_index + 1)) {
	free(scn);
	return NULL;
    }
    cgcef->e_scn_n = cgcef->e_scn_n->s_link = scn;
    return scn;
}

CGCEf_Scn*
cgcef_newscn(CGCEf *cgcef) {
    CGCEf_Scn *scn;

    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (!cgcef->e_readable && !cgcef->e_ehdr) {
	seterr(ERROR_NOEHDR);
    }
    else if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
    }
    else if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return NULL;
    }
    else if ((scn = _buildscn(cgcef))) {
	return scn;
    }
    return NULL;
}
