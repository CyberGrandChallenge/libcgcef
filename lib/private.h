/*
 * private.h - private definitions for libcgcef.
 * Copyright (C) 1995 - 2007 Michael Riepe
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

/* @(#) $Id: private.h,v 1.40 2009/11/01 13:04:19 michael Exp $ */

#ifndef _PRIVATE_H
#define _PRIVATE_H

#define __LIBCGCEF_INTERNAL__ 1

#if HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

/*
 * Workaround for GLIBC bug:
 * include <stdint.h> before <sys/types.h>
 */
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* STDC_HEADERS */
extern void *malloc(), *realloc();
extern void free(), bcopy(), abort();
extern int strcmp(), strncmp(), memcmp();
extern void *memcpy(), *memmove(), *memset();
#endif /* STDC_HEADERS */

#if defined(_WIN32)
#include <io.h>
#else
#if HAVE_UNISTD_H
# include <unistd.h>
#else /* HAVE_UNISTD_H */
extern int read(), write(), close();
extern off_t lseek();
#if HAVE_FTRUNCATE
extern int ftruncate();
#endif /* HAVE_FTRUNCATE */
#endif /* HAVE_UNISTD_H */
#endif /* defined(_WIN32) */

#ifndef SEEK_SET
#define SEEK_SET	0
#endif /* SEEK_SET */
#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif /* SEEK_CUR */
#ifndef SEEK_END
#define SEEK_END	2
#endif /* SEEK_END */

#if !HAVE_MEMCMP
# define memcmp	strncmp
#endif /* !HAVE_MEMCMP */
#if !HAVE_MEMCPY
# define memcpy(d,s,n)	bcopy(s,d,n)
#endif /* !HAVE_MEMCPY */
#if !HAVE_MEMMOVE
# define memmove(d,s,n)	bcopy(s,d,n)
#endif /* !HAVE_MEMMOVE */

#if !HAVE_MEMSET
# define memset _cgcef_memset
extern void *_cgcef_memset();
#endif /* !HAVE_MEMSET */

#if HAVE_STRUCT_NLIST_DECLARATION
# define nlist __override_nlist_declaration
#endif /* HAVE_STRUCT_NLIST_DECLARATION */

#if __LIBCGCEF_NEED_LINK_H
# include <link.h>
#elif __LIBCGCEF_NEED_SYS_LINK_H
# include <sys/link.h>
#endif /* __LIBCGCEF_NEED_LINK_H */

#if HAVE_AR_H
#include <ar.h>
#else /* HAVE_AR_H */

#define ARMAG	"!<arch>\n"
#define SARMAG	8

struct ar_hdr {
    char    ar_name[16];
    char    ar_date[12];
    char    ar_uid[6];
    char    ar_gid[6];
    char    ar_mode[8];
    char    ar_size[10];
    char    ar_fmag[2];
};

#define ARFMAG	"`\n"

#endif /* HAVE_AR_H */

#include <libcgcef.h>

#if HAVE_STRUCT_NLIST_DECLARATION
# undef nlist
#endif /* HAVE_STRUCT_NLIST_DECLARATION */

#if __LIBCGCEF64
#include <gcgcef.h>
#endif /* __LIBCGCEF64 */

typedef struct Scn_Data Scn_Data;

/*
 * CGCEF descriptor
 */
struct CGCEf {
    /* common */
    size_t	e_size;			/* file/member size */
    size_t	e_dsize;		/* size of memory image */
    CGCEf_Kind	e_kind;			/* kind of file */
    char*	e_data;			/* file/member data */
    char*	e_rawdata;		/* file/member raw data */
    size_t	e_idlen;		/* identifier size */
    int		e_fd;			/* file descriptor */
    unsigned	e_count;		/* activation count */
    /* archive members (still common) */
    CGCEf*	e_parent;		/* NULL if not an archive member */
    size_t	e_next;			/* 0 if not an archive member */
    size_t	e_base;			/* 0 if not an archive member */
    CGCEf*	e_link;			/* next archive member or NULL */
    CGCEf_Arhdr*	e_arhdr;		/* archive member header or NULL */
    /* archives */
    size_t	e_off;			/* current member offset (for cgcef_begin) */
    CGCEf*	e_members;		/* linked list of active archive members */
    char*	e_symtab;		/* archive symbol table */
    size_t	e_symlen;		/* length of archive symbol table */
    char*	e_strtab;		/* archive string table */
    size_t	e_strlen;		/* length of archive string table */
    /* CGCEF files */
    unsigned	e_class;		/* CGCEF class */
    unsigned	e_encoding;		/* CGCEF data encoding */
    unsigned	e_version;		/* CGCEF version */
    char*	e_ehdr;			/* CGCEF header */
    char*	e_phdr;			/* CGCEF program header table */
    size_t	e_phnum;		/* size of program header table */
    CGCEf_Scn*	e_scn_1;		/* first section */
    CGCEf_Scn*	e_scn_n;		/* last section */
    unsigned	e_cgcef_flags;		/* cgcef flags (CGCEF_F_*) */
    unsigned	e_ehdr_flags;		/* ehdr flags (CGCEF_F_*) */
    unsigned	e_phdr_flags;		/* phdr flags (CGCEF_F_*) */
    /* misc flags */
    unsigned	e_readable : 1;		/* file is readable */
    unsigned	e_writable : 1;		/* file is writable */
    unsigned	e_disabled : 1;		/* e_fd has been disabled */
    unsigned	e_cooked : 1;		/* e_data was modified */
    unsigned	e_free_syms : 1;	/* e_symtab is malloc'ed */
    unsigned	e_unmap_data : 1;	/* e_data is mmap'ed */
    unsigned	e_memory : 1;		/* created by cgcef_memory() */
    /* magic number for debugging */
    long	e_magic;
};

#define CGCEF_MAGIC	0x012b649e

#define INIT_CGCEF	{\
    /* e_size */	0,\
    /* e_dsize */	0,\
    /* e_kind */	CGCEF_K_NONE,\
    /* e_data */	NULL,\
    /* e_rawdata */	NULL,\
    /* e_idlen */	0,\
    /* e_fd */		-1,\
    /* e_count */	1,\
    /* e_parent */	NULL,\
    /* e_next */	0,\
    /* e_base */	0,\
    /* e_link */	NULL,\
    /* e_arhdr */	NULL,\
    /* e_off */		0,\
    /* e_members */	NULL,\
    /* e_symtab */	NULL,\
    /* e_symlen */	0,\
    /* e_strtab */	NULL,\
    /* e_strlen */	0,\
    /* e_class */	CGCEFCLASSNONE,\
    /* e_encoding */	CGCEFDATANONE,\
    /* e_version */	EV_NONE,\
    /* e_ehdr */	NULL,\
    /* e_phdr */	NULL,\
    /* e_phnum */	0,\
    /* e_scn_1 */	NULL,\
    /* e_scn_n */	NULL,\
    /* e_cgcef_flags */	0,\
    /* e_ehdr_flags */	0,\
    /* e_phdr_flags */	0,\
    /* e_readable */	0,\
    /* e_writable */	0,\
    /* e_disabled */	0,\
    /* e_cooked */	0,\
    /* e_free_syms */	0,\
    /* e_unmap_data */	0,\
    /* e_memory */	0,\
    /* e_magic */	CGCEF_MAGIC\
}

/*
 * Section descriptor
 */
struct CGCEf_Scn {
    CGCEf_Scn*	s_link;			/* pointer to next CGCEf_Scn */
    CGCEf*	s_cgcef;			/* pointer to cgcef descriptor */
    size_t	s_index;		/* number of this section */
    unsigned	s_scn_flags;		/* section flags (CGCEF_F_*) */
    unsigned	s_shdr_flags;		/* shdr flags (CGCEF_F_*) */
    Scn_Data*	s_data_1;		/* first data buffer */
    Scn_Data*	s_data_n;		/* last data buffer */
    Scn_Data*	s_rawdata;		/* raw data buffer */
    /* data copied from shdr */
    unsigned	s_type;			/* section type */
    size_t	s_offset;		/* section offset */
    size_t	s_size;			/* section size */
    /* misc flags */
    unsigned	s_freeme : 1;		/* this CGCEf_Scn was malloc'ed */
    /* section header */
    union {
#if __LIBCGCEF64
	CGCEf64_Shdr	u_shdr64;
#endif /* __LIBCGCEF64 */
	CGCEf32_Shdr	u_shdr32;
    }		s_uhdr;
    /* magic number for debugging */
    long	s_magic;
};
#define s_shdr32	s_uhdr.u_shdr32
#define s_shdr64	s_uhdr.u_shdr64

#define SCN_MAGIC	0x012c747d

#define INIT_SCN	{\
    /* s_link */	NULL,\
    /* s_cgcef */		NULL,\
    /* s_index */	0,\
    /* s_scn_flags */	0,\
    /* s_shdr_flags */	0,\
    /* s_data_1 */	NULL,\
    /* s_data_n */	NULL,\
    /* s_rawdata */	NULL,\
    /* s_type */	SHT_NULL,\
    /* s_offset */	0,\
    /* s_size */	0,\
    /* s_freeme */	0,\
    /* s_uhdr */	{{0,}},\
    /* s_magic */	SCN_MAGIC\
}

/*
 * Data descriptor
 */
struct Scn_Data {
    CGCEf_Data	sd_data;		/* must be first! */
    Scn_Data*	sd_link;		/* pointer to next Scn_Data */
    CGCEf_Scn*	sd_scn;			/* pointer to section */
    char*	sd_memdata;		/* memory image of section */
    unsigned	sd_data_flags;		/* data flags (CGCEF_F_*) */
    /* misc flags */
    unsigned	sd_freeme : 1;		/* this Scn_Data was malloc'ed */
    unsigned	sd_free_data : 1;	/* sd_memdata is malloc'ed */
    /* magic number for debugging */
    long	sd_magic;
};

#define DATA_MAGIC	0x01072639

#define INIT_DATA	{\
    {\
    /* d_buf */		NULL,\
    /* d_type */	CGCEF_T_BYTE,\
    /* d_size */	0,\
    /* d_off */		0,\
    /* d_align */	0,\
    /* d_version */	EV_NONE\
    },\
    /* sd_link */	NULL,\
    /* sd_scn */	NULL,\
    /* sd_memdata */	NULL,\
    /* sd_data_flags */	0,\
    /* sd_freeme */	0,\
    /* sd_free_data */	0,\
    /* sd_magic */	DATA_MAGIC\
}

/*
 * Private status variables
 */
extern unsigned _cgcef_version;
extern int _cgcef_errno;
extern int _cgcef_fill;
extern int _cgcef_sanity_checks;
#define SANITY_CHECK_STRPTR	(1u << 0)

/*
 * Private functions
 */
extern void *_cgcef_read __P((CGCEf*, void*, size_t, size_t));
extern void *_cgcef_mmap __P((CGCEf*));
extern int _cgcef_cook __P((CGCEf*));
extern char *_cgcef_getehdr __P((CGCEf*, unsigned));
extern char *_cgcef_getphdr __P((CGCEf*, unsigned));
extern CGCEf_Data *_cgcef_xlatetom __P((const CGCEf*, CGCEf_Data*, const CGCEf_Data*));
extern CGCEf_Type _cgcef_scn_type __P((unsigned));
extern size_t _cgcef32_xltsize __P((const CGCEf_Data *__src, unsigned __dv, unsigned __encode, int __tof));
extern size_t _cgcef64_xltsize __P((const CGCEf_Data *__src, unsigned __dv, unsigned __encode, int __tof));
extern int _cgcef_update_shnum(CGCEf *__cgcef, size_t __shnum);
extern CGCEf_Scn *_cgcef_first_scn(CGCEf *__cgcef);

/*
 * Special translators
 */
extern size_t _cgcef_verdef_32L11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_32L11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_32M11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_32M11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_64L11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_64L11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_64M11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verdef_64M11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_32L11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_32L11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_32M11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_32M11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_64L11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_64L11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_64M11_tof __P((unsigned char *dst, const unsigned char *src, size_t n));
extern size_t _cgcef_verneed_64M11_tom __P((unsigned char *dst, const unsigned char *src, size_t n));

/*
 * Private data
 */
extern const CGCEf_Scn _cgcef_scn_init;
extern const Scn_Data _cgcef_data_init;
extern const size_t _cgcef_fmsize[2][EV_CURRENT - EV_NONE][CGCEF_T_NUM][2];

/*
 * Access macros for _cgcef_fmsize[]
 */
#define _fmsize(c,v,t,w)	\
	(_cgcef_fmsize[(c)-CGCEFCLASS32][(v)-EV_NONE-1][(t)-CGCEF_T_BYTE][(w)])
#define _fsize(c,v,t)		_fmsize((c),(v),(t),1)
#define _msize(c,v,t)		_fmsize((c),(v),(t),0)

/*
 * Various checks
 */
#define valid_class(c)		((c) >= CGCEFCLASS32 && (c) <= CGCEFCLASS64)
#define valid_encoding(e)	((e) >= CGCEFDATA2LSB && (e) <= CGCEFDATA2MSB)
#define valid_version(v)	((v) > EV_NONE && (v) <= EV_CURRENT)
#define valid_type(t)		((unsigned)(t) < CGCEF_T_NUM)

/*
 * Error codes
 */
enum {
#define __err__(a,b)	a,
#include <errors.h>		/* include constants from errors.h */
#undef __err__
ERROR_NUM
};

#define seterr(err)	(_cgcef_errno = (err))

/*
 * Sizes of data types (external representation)
 * These definitions should be in <cgcef.h>, but...
 */
#ifndef CGCEF32_FSZ_ADDR
# define CGCEF32_FSZ_ADDR		4
# define CGCEF32_FSZ_HALF		2
# define CGCEF32_FSZ_OFF		4
# define CGCEF32_FSZ_SWORD	4
# define CGCEF32_FSZ_WORD		4
#endif /* CGCEF32_FSZ_ADDR */
#ifndef CGCEF64_FSZ_ADDR
# define CGCEF64_FSZ_ADDR		8
# define CGCEF64_FSZ_HALF		2
# define CGCEF64_FSZ_OFF		8
# define CGCEF64_FSZ_SWORD	4
# define CGCEF64_FSZ_SXWORD	8
# define CGCEF64_FSZ_WORD		4
# define CGCEF64_FSZ_XWORD	8
#endif /* CGCEF64_FSZ_ADDR */

/*
 * More missing pieces, in no particular order
 */
#ifndef SHT_SYMTAB_SHNDX
#define SHT_SYMTAB_SHNDX	18
#endif /* SHT_SYMTAB_SHNDX */

#ifndef SHN_XINDEX
#define SHN_XINDEX		0xffff
#endif /* SHN_XINDEX */

#ifndef PN_XNUM
#define PN_XNUM			0xffff
#endif /* PN_XNUM */

/*
 * Debugging
 */
#if ENABLE_DEBUG
extern void __cgcef_assert __P((const char*, unsigned, const char*));
# if (__STDC__ + 0)
#  define cgcef_assert(x)	do{if(!(x))__cgcef_assert(__FILE__,__LINE__,#x);}while(0)
# else /* __STDC__ */
#  define cgcef_assert(x)	do{if(!(x))__cgcef_assert(__FILE__,__LINE__,"x");}while(0)
# endif /* __STDC__ */
#else /* ENABLE_DEBUG */
# define cgcef_assert(x)	do{}while(0)
#endif /* ENABLE_DEBUG */

/*
 * Return values for certain functions
 */
#define LIBCGCEF_SUCCESS	1
#define LIBCGCEF_FAILURE	0

#endif /* _PRIVATE_H */
