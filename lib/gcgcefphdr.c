/*
 * gcgcefphdr.c - gcgcef_* translation functions.
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
static const char rcsid[] = "@(#) $Id: gcgcefphdr.c,v 1.9 2008/05/23 08:15:34 michael Exp $";
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

GCGCEf_Phdr*
gcgcef_getphdr(CGCEf *cgcef, int ndx, GCGCEf_Phdr *dst) {
    GCGCEf_Phdr buf;
    char *tmp;
    size_t n;

    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    tmp = _cgcef_getphdr(cgcef, cgcef->e_class);
    if (!tmp) {
	return NULL;
    }
    if (ndx < 0 || ndx >= cgcef->e_phnum) {
	seterr(ERROR_BADINDEX);
	return NULL;
    }
    n = _msize(cgcef->e_class, _cgcef_version, CGCEF_T_PHDR);
    if (n == 0) {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (!dst) {
	dst = &buf;
    }
    if (cgcef->e_class == CGCEFCLASS64) {
	*dst = *(CGCEf64_Phdr*)(tmp + ndx * n);
    }
    else if (cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Phdr *src = (CGCEf32_Phdr*)(tmp + ndx * n);

	check_and_copy(GCGCEf_Word,  dst, src, p_type,   NULL);
	check_and_copy(GCGCEf_Word,  dst, src, p_flags,  NULL);
	check_and_copy(GCGCEf_Off,   dst, src, p_offset, NULL);
	check_and_copy(GCGCEf_Addr,  dst, src, p_vaddr,  NULL);
	check_and_copy(GCGCEf_Addr,  dst, src, p_paddr,  NULL);
	check_and_copy(GCGCEf_Xword, dst, src, p_filesz, NULL);
	check_and_copy(GCGCEf_Xword, dst, src, p_memsz,  NULL);
	check_and_copy(GCGCEf_Xword, dst, src, p_align,  NULL);
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
	dst = (GCGCEf_Phdr*)malloc(sizeof(GCGCEf_Phdr));
	if (!dst) {
	    seterr(ERROR_MEM_PHDR);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_phdr(CGCEf *cgcef, int ndx, GCGCEf_Phdr *src) {
    char *tmp;
    size_t n;

    if (!cgcef || !src) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    tmp = _cgcef_getphdr(cgcef, cgcef->e_class);
    if (!tmp) {
	return 0;
    }
    if (ndx < 0 || ndx >= cgcef->e_phnum) {
	seterr(ERROR_BADINDEX);
	return 0;
    }
    n = _msize(cgcef->e_class, _cgcef_version, CGCEF_T_PHDR);
    if (n == 0) {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    if (cgcef->e_class == CGCEFCLASS64) {
	*(CGCEf64_Phdr*)(tmp + ndx * n) = *src;
    }
    else if (cgcef->e_class == CGCEFCLASS32) {
	CGCEf32_Phdr *dst = (CGCEf32_Phdr*)(tmp + ndx * n);

	check_and_copy(CGCEf32_Word, dst, src, p_type,   0);
	check_and_copy(CGCEf32_Off,  dst, src, p_offset, 0);
	check_and_copy(CGCEf32_Addr, dst, src, p_vaddr,  0);
	check_and_copy(CGCEf32_Addr, dst, src, p_paddr,  0);
	check_and_copy(CGCEf32_Word, dst, src, p_filesz, 0);
	check_and_copy(CGCEf32_Word, dst, src, p_memsz,  0);
	check_and_copy(CGCEf32_Word, dst, src, p_flags,  0);
	check_and_copy(CGCEf32_Word, dst, src, p_align,  0);
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
