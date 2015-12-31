/*
 * gcgcefshdr.c - gcgcef_* translation functions.
 * Copyright (C) 2000 - 2006 Michael Riepe
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

#if __LIBCGCEF64

#ifndef lint
static const char rcsid[] = "@(#) $Id: gcgcefshdr.c,v 1.10 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

#define check_and_copy(type, d, s, name, eret)		\
    do {						\
	if (sizeof((d)->name) < sizeof((s)->name)	\
	 && (type)(s)->name != (s)->name) {		\
	    seterr(ERROR_BADVALUE);			\
	    return (eret);				\
	}						\
	(d)->name = (type)(s)->name;			\
    } while (0)

GCGCEf_Shdr*
gcgcef_getshdr(CGCEf_Scn *scn, GCGCEf_Shdr *dst) {
    GCGCEf_Shdr buf;

    if (!scn) {
	return NULL;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef_assert(scn->s_cgcef);
    cgcef_assert(scn->s_cgcef->e_magic == CGCEF_MAGIC);
    if (!dst) {
	dst = &buf;
    }
    if (scn->s_cgcef->e_class == CGCEFCLASS64) {
	*dst = scn->s_shdr64;
    }
    else if (scn->s_cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Shdr *src = &scn->s_shdr32;

	check_and_copy(GCGCEf_Word,  dst, src, sh_name,      NULL);
	check_and_copy(GCGCEf_Word,  dst, src, sh_type,      NULL);
	check_and_copy(GCGCEf_Xword, dst, src, sh_flags,     NULL);
	check_and_copy(GCGCEf_Addr,  dst, src, sh_addr,      NULL);
	check_and_copy(GCGCEf_Off,   dst, src, sh_offset,    NULL);
	check_and_copy(GCGCEf_Xword, dst, src, sh_size,      NULL);
	check_and_copy(GCGCEf_Word,  dst, src, sh_link,      NULL);
	check_and_copy(GCGCEf_Word,  dst, src, sh_info,      NULL);
	check_and_copy(GCGCEf_Xword, dst, src, sh_addralign, NULL);
	check_and_copy(GCGCEf_Xword, dst, src, sh_entsize,   NULL);
    }
    else {
	if (valid_class(scn->s_cgcef->e_class)) {
	    seterr(ERROR_UNIMPLEMENTED);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Shdr*)malloc(sizeof(GCGCEf_Shdr));
	if (!dst) {
	    seterr(ERROR_MEM_SHDR);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_shdr(CGCEf_Scn *scn, GCGCEf_Shdr *src) {
    if (!scn || !src) {
	return 0;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef_assert(scn->s_cgcef);
    cgcef_assert(scn->s_cgcef->e_magic == CGCEF_MAGIC);
    if (scn->s_cgcef->e_class == CGCEFCLASS64) {
	scn->s_shdr64 = *src;
    }
    else if (scn->s_cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Shdr *dst = &scn->s_shdr32;

	check_and_copy(CGCEf32_Word, dst, src, sh_name,      0);
	check_and_copy(CGCEf32_Word, dst, src, sh_type,      0);
	check_and_copy(CGCEf32_Word, dst, src, sh_flags,     0);
	check_and_copy(CGCEf32_Addr, dst, src, sh_addr,      0);
	check_and_copy(CGCEf32_Off,  dst, src, sh_offset,    0);
	check_and_copy(CGCEf32_Word, dst, src, sh_size,      0);
	check_and_copy(CGCEf32_Word, dst, src, sh_link,      0);
	check_and_copy(CGCEf32_Word, dst, src, sh_info,      0);
	check_and_copy(CGCEf32_Word, dst, src, sh_addralign, 0);
	check_and_copy(CGCEf32_Word, dst, src, sh_entsize,   0);
    }
    else {
	if (valid_class(scn->s_cgcef->e_class)) {
	    seterr(ERROR_UNIMPLEMENTED);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	return 0;
    }
    return 1;
}

#endif /* __LIBCGCEF64 */
