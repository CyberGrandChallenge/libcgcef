/*
 * x.cgcefext.c -- handle CGCEF format extensions
 * Copyright (C) 2002 - 2006 Michael Riepe
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
static const char rcsid[] = "@(#) $Id: x.cgcefext.c,v 1.5 2009/07/07 17:57:43 michael Exp $";
#endif /* lint */

int
cgcef_getphdrnum(CGCEf *cgcef, size_t *resultp) {
    if (!cgcef) {
	return -1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return -1;
    }
    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return -1;
    }
    if (resultp) {
	*resultp = cgcef->e_phnum;
    }
    return 0;
}

int
cgcef_getshdrnum(CGCEf *cgcef, size_t *resultp) {
    size_t num = 0;
    CGCEf_Scn *scn;

    if (!cgcef) {
	return -1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return -1;
    }
    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return -1;
    }
    if ((scn = cgcef->e_scn_n)) {
	num = scn->s_index + 1;
    }
    if (resultp) {
	*resultp = num;
    }
    return 0;
}

int
cgcef_getshdrstrndx(CGCEf *cgcef, size_t *resultp) {
    size_t num = 0;
    size_t dummy;
    CGCEf_Scn *scn;

    if (!cgcef) {
	return -1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (resultp == NULL) {
	resultp = &dummy;	/* handle NULL pointer gracefully */
    }
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return -1;
    }
    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return -1;
    }
    if (cgcef->e_class == CGCEFCLASS32) {
	num = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shstrndx;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	num = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shstrndx;
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
    if (num != SHN_XINDEX) {
	*resultp = num;
	return 0;
    }
    /*
     * look at first section header
     */
    if (!(scn = cgcef->e_scn_1)) {
	seterr(ERROR_NOSUCHSCN);
	return -1;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
#if __LIBCGCEF64
    if (cgcef->e_class == CGCEFCLASS64) {
	*resultp = scn->s_shdr64.sh_link;
	return 0;
    }
#endif /* __LIBCGCEF64 */
    *resultp = scn->s_shdr32.sh_link;
    return 0;
}

int
cgcef_getphnum(CGCEf *cgcef, size_t *resultp) {
    return cgcef_getphdrnum(cgcef, resultp) ? LIBCGCEF_FAILURE : LIBCGCEF_SUCCESS;
}

int
cgcef_getshnum(CGCEf *cgcef, size_t *resultp) {
    return cgcef_getshdrnum(cgcef, resultp) ? LIBCGCEF_FAILURE : LIBCGCEF_SUCCESS;
}

int
cgcef_getshstrndx(CGCEf *cgcef, size_t *resultp) {
    return cgcef_getshdrstrndx(cgcef, resultp) ? LIBCGCEF_FAILURE : LIBCGCEF_SUCCESS;
}

int
cgcefx_update_shstrndx(CGCEf *cgcef, size_t value) {
    size_t extvalue = 0;
    CGCEf_Scn *scn;

    if (!cgcef) {
	return LIBCGCEF_FAILURE;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (value >= SHN_LORESERVE) {
	extvalue = value;
	value = SHN_XINDEX;
    }
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return LIBCGCEF_FAILURE;
    }
    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return LIBCGCEF_FAILURE;
    }
    if (!(scn = _cgcef_first_scn(cgcef))) {
	return LIBCGCEF_FAILURE;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    if (cgcef->e_class == CGCEFCLASS32) {
	((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shstrndx = value;
	scn->s_shdr32.sh_link = extvalue;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shstrndx = value;
	scn->s_shdr64.sh_link = extvalue;
    }
#endif /* __LIBCGCEF64 */
    else {
	if (valid_class(cgcef->e_class)) {
	    seterr(ERROR_UNIMPLEMENTED);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	return LIBCGCEF_FAILURE;
    }
    cgcef->e_ehdr_flags |= CGCEF_F_DIRTY;
    scn->s_shdr_flags |= CGCEF_F_DIRTY;
    return LIBCGCEF_SUCCESS;
}
