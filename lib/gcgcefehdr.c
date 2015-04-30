/*
 * gcgcefehdr.c - gcgcef_* translation functions.
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
static const char rcsid[] = "@(#) $Id: gcgcefehdr.c,v 1.9 2008/05/23 08:15:34 michael Exp $";
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

GCGCEf_Ehdr*
gcgcef_getehdr(CGCEf *cgcef, GCGCEf_Ehdr *dst) {
    GCGCEf_Ehdr buf;
    char *tmp;

    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    tmp = _cgcef_getehdr(cgcef, cgcef->e_class);
    if (!tmp) {
	return NULL;
    }
    if (!dst) {
	dst = &buf;
    }
    if (cgcef->e_class == CGCEFCLASS64) {
	*dst = *(CGCEf64_Ehdr*)tmp;
    }
    else if (cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Ehdr *src = (CGCEf32_Ehdr*)tmp;

	memcpy(dst->e_ident, src->e_ident, EI_NIDENT);
	check_and_copy(GCGCEf_Half, dst, src, e_type,      NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_machine,   NULL);
	check_and_copy(GCGCEf_Word, dst, src, e_version,   NULL);
	check_and_copy(GCGCEf_Addr, dst, src, e_entry,     NULL);
	check_and_copy(GCGCEf_Off,  dst, src, e_phoff,     NULL);
	check_and_copy(GCGCEf_Off,  dst, src, e_shoff,     NULL);
	check_and_copy(GCGCEf_Word, dst, src, e_flags,     NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_ehsize,    NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_phentsize, NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_phnum,     NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_shentsize, NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_shnum,     NULL);
	check_and_copy(GCGCEf_Half, dst, src, e_shstrndx,  NULL);
    }
    else {
	if (valid_class(cgcef->e_class)) {
	    seterr(ERROR_UNIMPLEMENTED);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Ehdr*)malloc(sizeof(GCGCEf_Ehdr));
	if (!dst) {
	    seterr(ERROR_MEM_EHDR);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_ehdr(CGCEf *cgcef, GCGCEf_Ehdr *src) {
    char *tmp;

    if (!cgcef || !src) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    tmp = _cgcef_getehdr(cgcef, cgcef->e_class);
    if (!tmp) {
	return 0;
    }
    if (cgcef->e_class == CGCEFCLASS64) {
	*(CGCEf64_Ehdr*)tmp = *src;
    }
    else if (cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Ehdr *dst = (CGCEf32_Ehdr*)tmp;

	memcpy(dst->e_ident, src->e_ident, EI_NIDENT);
	check_and_copy(CGCEf32_Half, dst, src, e_type,      0);
	check_and_copy(CGCEf32_Half, dst, src, e_machine,   0);
	check_and_copy(CGCEf32_Word, dst, src, e_version,   0);
	check_and_copy(CGCEf32_Addr, dst, src, e_entry,     0);
	check_and_copy(CGCEf32_Off,  dst, src, e_phoff,     0);
	check_and_copy(CGCEf32_Off,  dst, src, e_shoff,     0);
	check_and_copy(CGCEf32_Word, dst, src, e_flags,     0);
	check_and_copy(CGCEf32_Half, dst, src, e_ehsize,    0);
	check_and_copy(CGCEf32_Half, dst, src, e_phentsize, 0);
	check_and_copy(CGCEf32_Half, dst, src, e_phnum,     0);
	check_and_copy(CGCEf32_Half, dst, src, e_shentsize, 0);
	check_and_copy(CGCEf32_Half, dst, src, e_shnum,     0);
	check_and_copy(CGCEf32_Half, dst, src, e_shstrndx,  0);
    }
    else {
	if (valid_class(cgcef->e_class)) {
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
