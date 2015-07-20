/*
ext_types.h - external representation of CGCEF data types.
Copyright (C) 1995 - 1998 Michael Riepe

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

/* @(#) $Id: ext_types.h,v 1.9 2008/05/23 08:15:34 michael Exp $ */

#ifndef _EXT_TYPES_H
#define _EXT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Scalar data types
 */
typedef unsigned char __ext_CGCEf32_Addr  [CGCEF32_FSZ_ADDR];
typedef unsigned char __ext_CGCEf32_Half  [CGCEF32_FSZ_HALF];
typedef unsigned char __ext_CGCEf32_Off   [CGCEF32_FSZ_OFF];
typedef unsigned char __ext_CGCEf32_Sword [CGCEF32_FSZ_SWORD];
typedef unsigned char __ext_CGCEf32_Word  [CGCEF32_FSZ_WORD];

#if __LIBCGCEF64

typedef unsigned char __ext_CGCEf32_Lword [8];

typedef unsigned char __ext_CGCEf64_Addr  [CGCEF64_FSZ_ADDR];
typedef unsigned char __ext_CGCEf64_Half  [CGCEF64_FSZ_HALF];
typedef unsigned char __ext_CGCEf64_Off   [CGCEF64_FSZ_OFF];
typedef unsigned char __ext_CGCEf64_Sword [CGCEF64_FSZ_SWORD];
typedef unsigned char __ext_CGCEf64_Word  [CGCEF64_FSZ_WORD];
typedef unsigned char __ext_CGCEf64_Sxword[CGCEF64_FSZ_SXWORD];
typedef unsigned char __ext_CGCEf64_Xword [CGCEF64_FSZ_XWORD];

typedef unsigned char __ext_CGCEf64_Lword [8];

#endif /* __LIBCGCEF64 */

/*
 * CGCEF header
 */
typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    __ext_CGCEf32_Half	e_type;
    __ext_CGCEf32_Half	e_machine;
    __ext_CGCEf32_Word	e_version;
    __ext_CGCEf32_Addr	e_entry;
    __ext_CGCEf32_Off	e_phoff;
    __ext_CGCEf32_Off	e_shoff;
    __ext_CGCEf32_Word	e_flags;
    __ext_CGCEf32_Half	e_ehsize;
    __ext_CGCEf32_Half	e_phentsize;
    __ext_CGCEf32_Half	e_phnum;
    __ext_CGCEf32_Half	e_shentsize;
    __ext_CGCEf32_Half	e_shnum;
    __ext_CGCEf32_Half	e_shstrndx;
} __ext_CGCEf32_Ehdr;

#if __LIBCGCEF64
typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    __ext_CGCEf64_Half	e_type;
    __ext_CGCEf64_Half	e_machine;
    __ext_CGCEf64_Word	e_version;
    __ext_CGCEf64_Addr	e_entry;
    __ext_CGCEf64_Off	e_phoff;
    __ext_CGCEf64_Off	e_shoff;
    __ext_CGCEf64_Word	e_flags;
    __ext_CGCEf64_Half	e_ehsize;
    __ext_CGCEf64_Half	e_phentsize;
    __ext_CGCEf64_Half	e_phnum;
    __ext_CGCEf64_Half	e_shentsize;
    __ext_CGCEf64_Half	e_shnum;
    __ext_CGCEf64_Half	e_shstrndx;
} __ext_CGCEf64_Ehdr;
#endif /* __LIBCGCEF64 */

/*
 * Section header
 */
typedef struct {
    __ext_CGCEf32_Word	sh_name;
    __ext_CGCEf32_Word	sh_type;
    __ext_CGCEf32_Word	sh_flags;
    __ext_CGCEf32_Addr	sh_addr;
    __ext_CGCEf32_Off	sh_offset;
    __ext_CGCEf32_Word	sh_size;
    __ext_CGCEf32_Word	sh_link;
    __ext_CGCEf32_Word	sh_info;
    __ext_CGCEf32_Word	sh_addralign;
    __ext_CGCEf32_Word	sh_entsize;
} __ext_CGCEf32_Shdr;

#if __LIBCGCEF64
typedef struct {
    __ext_CGCEf64_Word	sh_name;
    __ext_CGCEf64_Word	sh_type;
    __ext_CGCEf64_Xword	sh_flags;
    __ext_CGCEf64_Addr	sh_addr;
    __ext_CGCEf64_Off	sh_offset;
    __ext_CGCEf64_Xword	sh_size;
    __ext_CGCEf64_Word	sh_link;
    __ext_CGCEf64_Word	sh_info;
    __ext_CGCEf64_Xword	sh_addralign;
    __ext_CGCEf64_Xword	sh_entsize;
} __ext_CGCEf64_Shdr;
#endif /* __LIBCGCEF64 */

/*
 * Symbol table
 */
typedef struct {
    __ext_CGCEf32_Word	st_name;
    __ext_CGCEf32_Addr	st_value;
    __ext_CGCEf32_Word	st_size;
    unsigned char	st_info;
    unsigned char	st_other;
    __ext_CGCEf32_Half	st_shndx;
} __ext_CGCEf32_Sym;

#if __LIBCGCEF64
typedef struct {
    __ext_CGCEf64_Word	st_name;
    unsigned char	st_info;
    unsigned char	st_other;
    __ext_CGCEf64_Half	st_shndx;
    __ext_CGCEf64_Addr	st_value;
    __ext_CGCEf64_Xword	st_size;
} __ext_CGCEf64_Sym;
#endif /* __LIBCGCEF64 */

/*
 * Relocation
 */
typedef struct {
    __ext_CGCEf32_Addr	r_offset;
    __ext_CGCEf32_Word	r_info;
} __ext_CGCEf32_Rel;

typedef struct {
    __ext_CGCEf32_Addr	r_offset;
    __ext_CGCEf32_Word	r_info;
    __ext_CGCEf32_Sword	r_addend;
} __ext_CGCEf32_Rela;

#if __LIBCGCEF64
typedef struct {
    __ext_CGCEf64_Addr	r_offset;
#if __LIBCGCEF64_IRIX
    __ext_CGCEf64_Word	r_sym;
    unsigned char	r_ssym;
    unsigned char	r_type3;
    unsigned char	r_type2;
    unsigned char	r_type;
#else /* __LIBCGCEF64_IRIX */
    __ext_CGCEf64_Xword	r_info;
#endif /* __LIBCGCEF64_IRIX */
} __ext_CGCEf64_Rel;

typedef struct {
    __ext_CGCEf64_Addr	r_offset;
#if __LIBCGCEF64_IRIX
    __ext_CGCEf64_Word	r_sym;
    unsigned char	r_ssym;
    unsigned char	r_type3;
    unsigned char	r_type2;
    unsigned char	r_type;
#else /* __LIBCGCEF64_IRIX */
    __ext_CGCEf64_Xword	r_info;
#endif /* __LIBCGCEF64_IRIX */
    __ext_CGCEf64_Sxword	r_addend;
} __ext_CGCEf64_Rela;
#endif /* __LIBCGCEF64 */

/*
 * Program header
 */
typedef struct {
    __ext_CGCEf32_Word	p_type;
    __ext_CGCEf32_Off	p_offset;
    __ext_CGCEf32_Addr	p_vaddr;
    __ext_CGCEf32_Addr	p_paddr;
    __ext_CGCEf32_Word	p_filesz;
    __ext_CGCEf32_Word	p_memsz;
    __ext_CGCEf32_Word	p_flags;
    __ext_CGCEf32_Word	p_align;
} __ext_CGCEf32_Phdr;

#if __LIBCGCEF64
typedef struct {
    __ext_CGCEf64_Word	p_type;
    __ext_CGCEf64_Word	p_flags;
    __ext_CGCEf64_Off	p_offset;
    __ext_CGCEf64_Addr	p_vaddr;
    __ext_CGCEf64_Addr	p_paddr;
    __ext_CGCEf64_Xword	p_filesz;
    __ext_CGCEf64_Xword	p_memsz;
    __ext_CGCEf64_Xword	p_align;
} __ext_CGCEf64_Phdr;
#endif /* __LIBCGCEF64 */

/*
 * Dynamic structure
 */
typedef struct {
    __ext_CGCEf32_Sword	d_tag;
    union {
	__ext_CGCEf32_Word	d_val;
	__ext_CGCEf32_Addr	d_ptr;
    } d_un;
} __ext_CGCEf32_Dyn;

#if __LIBCGCEF64
typedef struct {
    __ext_CGCEf64_Sxword	d_tag;
    union {
	__ext_CGCEf64_Xword	d_val;
	__ext_CGCEf64_Addr	d_ptr;
    } d_un;
} __ext_CGCEf64_Dyn;
#endif /* __LIBCGCEF64 */

/*
 * Version definitions
 */
typedef struct {
    __ext_CGCEf32_Half	vd_version;
    __ext_CGCEf32_Half	vd_flags;
    __ext_CGCEf32_Half	vd_ndx;
    __ext_CGCEf32_Half	vd_cnt;
    __ext_CGCEf32_Word	vd_hash;
    __ext_CGCEf32_Word	vd_aux;
    __ext_CGCEf32_Word	vd_next;
} __ext_CGCEf32_Verdef;

typedef struct {
    __ext_CGCEf32_Word	vda_name;
    __ext_CGCEf32_Word	vda_next;
} __ext_CGCEf32_Verdaux;

typedef struct {
    __ext_CGCEf32_Half	vn_version;
    __ext_CGCEf32_Half	vn_cnt;
    __ext_CGCEf32_Word	vn_file;
    __ext_CGCEf32_Word	vn_aux;
    __ext_CGCEf32_Word	vn_next;
} __ext_CGCEf32_Verneed;

typedef struct {
    __ext_CGCEf32_Word	vna_hash;
    __ext_CGCEf32_Half	vna_flags;
    __ext_CGCEf32_Half	vna_other;
    __ext_CGCEf32_Word	vna_name;
    __ext_CGCEf32_Word	vna_next;
} __ext_CGCEf32_Vernaux;

#if __LIBCGCEF64

typedef struct {
    __ext_CGCEf64_Half	vd_version;
    __ext_CGCEf64_Half	vd_flags;
    __ext_CGCEf64_Half	vd_ndx;
    __ext_CGCEf64_Half	vd_cnt;
    __ext_CGCEf64_Word	vd_hash;
    __ext_CGCEf64_Word	vd_aux;
    __ext_CGCEf64_Word	vd_next;
} __ext_CGCEf64_Verdef;

typedef struct {
    __ext_CGCEf64_Word	vda_name;
    __ext_CGCEf64_Word	vda_next;
} __ext_CGCEf64_Verdaux;

typedef struct {
    __ext_CGCEf64_Half	vn_version;
    __ext_CGCEf64_Half	vn_cnt;
    __ext_CGCEf64_Word	vn_file;
    __ext_CGCEf64_Word	vn_aux;
    __ext_CGCEf64_Word	vn_next;
} __ext_CGCEf64_Verneed;

typedef struct {
    __ext_CGCEf64_Word	vna_hash;
    __ext_CGCEf64_Half	vna_flags;
    __ext_CGCEf64_Half	vna_other;
    __ext_CGCEf64_Word	vna_name;
    __ext_CGCEf64_Word	vna_next;
} __ext_CGCEf64_Vernaux;

#endif /* __LIBCGCEF64 */

/*
 * Move section
 */
#if __LIBCGCEF64

typedef struct {
    __ext_CGCEf32_Lword	m_value;
    __ext_CGCEf32_Word	m_info;
    __ext_CGCEf32_Word	m_poffset;
    __ext_CGCEf32_Half	m_repeat;
    __ext_CGCEf32_Half	m_stride;
} __ext_CGCEf32_Move;

typedef struct {
    __ext_CGCEf64_Lword	m_value;
    __ext_CGCEf64_Xword	m_info;
    __ext_CGCEf64_Xword	m_poffset;
    __ext_CGCEf64_Half	m_repeat;
    __ext_CGCEf64_Half	m_stride;
} __ext_CGCEf64_Move;

#endif /* __LIBCGCEF64 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _EXT_TYPES_H */
