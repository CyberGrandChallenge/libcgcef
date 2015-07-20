/*
 * cook.c - read and translate CGCEF files.
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
static const char rcsid[] = "@(#) $Id: cook.c,v 1.29 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

const CGCEf_Scn _cgcef_scn_init = INIT_SCN;
const Scn_Data _cgcef_data_init = INIT_DATA;

CGCEf_Type
_cgcef_scn_type(unsigned t) {
    switch (t) {
	case SHT_DYNAMIC:       return CGCEF_T_DYN;
	case SHT_DYNSYM:        return CGCEF_T_SYM;
	case SHT_HASH:          return CGCEF_T_WORD;
	case SHT_REL:           return CGCEF_T_REL;
	case SHT_RELA:          return CGCEF_T_RELA;
	case SHT_SYMTAB:        return CGCEF_T_SYM;
	case SHT_SYMTAB_SHNDX:	return CGCEF_T_WORD;	/* XXX: really? */
#if __LIBCGCEF_SYMBOL_VERSIONS
#if __LIBCGCEF_SUN_SYMBOL_VERSIONS
	case SHT_SUNW_verdef:   return CGCEF_T_VDEF;
	case SHT_SUNW_verneed:  return CGCEF_T_VNEED;
	case SHT_SUNW_versym:   return CGCEF_T_HALF;
#else /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
	case SHT_GNU_verdef:    return CGCEF_T_VDEF;
	case SHT_GNU_verneed:   return CGCEF_T_VNEED;
	case SHT_GNU_versym:    return CGCEF_T_HALF;
#endif /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
#endif /* __LIBCGCEF_SYMBOL_VERSIONS */
    }
    return CGCEF_T_BYTE;
}

/*
 * Check for overflow on 32-bit systems
 */
#define overflow(a,b,t)	(sizeof(a) < sizeof(t) && (t)(a) != (b))

#define truncerr(t) ((t)==CGCEF_T_EHDR?ERROR_TRUNC_EHDR:	\
		    ((t)==CGCEF_T_PHDR?ERROR_TRUNC_PHDR:	\
		    ERROR_INTERNAL))
#define memerr(t)   ((t)==CGCEF_T_EHDR?ERROR_MEM_EHDR:	\
		    ((t)==CGCEF_T_PHDR?ERROR_MEM_PHDR:	\
		    ERROR_INTERNAL))

CGCEf_Data*
_cgcef_xlatetom(const CGCEf *cgcef, CGCEf_Data *dst, const CGCEf_Data *src) {
    if (cgcef->e_class == CGCEFCLASS32) {
	return cgcef32_xlatetom(dst, src, cgcef->e_encoding);
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	return cgcef64_xlatetom(dst, src, cgcef->e_encoding);
    }
#endif /* __LIBCGCEF64 */
    seterr(ERROR_UNIMPLEMENTED);
    return NULL;
}

static char*
_cgcef_item(void *buf, CGCEf *cgcef, CGCEf_Type type, size_t off) {
    CGCEf_Data src, dst;

    cgcef_assert(valid_type(type));
    if (off < 0 || off > cgcef->e_size) {
	seterr(ERROR_OUTSIDE);
	return NULL;
    }

    src.d_type = type;
    src.d_version = cgcef->e_version;
    src.d_size = _fsize(cgcef->e_class, src.d_version, type);
    cgcef_assert(src.d_size);
    if ((cgcef->e_size - off) < src.d_size) {
	seterr(truncerr(type));
	return NULL;
    }

    dst.d_version = _cgcef_version;
    dst.d_size = _msize(cgcef->e_class, dst.d_version, type);
    cgcef_assert(dst.d_size);

    if (!(dst.d_buf = buf) && !(dst.d_buf = malloc(dst.d_size))) {
	seterr(memerr(type));
	return NULL;
    }

    cgcef_assert(cgcef->e_data);
    if (cgcef->e_rawdata) {
	src.d_buf = cgcef->e_rawdata + off;
    }
    else {
	src.d_buf = cgcef->e_data + off;
    }

    if (_cgcef_xlatetom(cgcef, &dst, &src)) {
	return (char*)dst.d_buf;
    }
    if (dst.d_buf != buf) {
	free(dst.d_buf);
    }
    return NULL;
}

static int
_cgcef_cook_phdr(CGCEf *cgcef) {
    size_t num, off, entsz;

    if (cgcef->e_class == CGCEFCLASS32) {
	num = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_phnum;
	off = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_phoff;
	entsz = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_phentsize;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	num = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_phnum;
	off = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_phoff;
	entsz = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_phentsize;
	/*
	 * Check for overflow on 32-bit systems
	 */
	if (overflow(off, ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_phoff, CGCEf64_Off)) {
	    seterr(ERROR_OUTSIDE);
	    return 0;
	}
    }
#endif /* __LIBCGCEF64 */
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    if (off) {
	CGCEf_Scn *scn;
	size_t size;
	unsigned i;
	char *p;

	if (num == PN_XNUM) {
	    /*
	     * Overflow in ehdr->e_phnum.
	     * Get real value from first SHDR.
	     */
	    if (!(scn = cgcef->e_scn_1)) {
		seterr(ERROR_NOSUCHSCN);
		return 0;
	    }
	    if (cgcef->e_class == CGCEFCLASS32) {
		num = scn->s_shdr32.sh_info;
	    }
#if __LIBCGCEF64
	    else if (cgcef->e_class == CGCEFCLASS64) {
		num = scn->s_shdr64.sh_info;
	    }
#endif /* __LIBCGCEF64 */
	    /* we already had this
	    else {
		seterr(ERROR_UNIMPLEMENTED);
		return 0;
	    }
	    */
	}

	size = _fsize(cgcef->e_class, cgcef->e_version, CGCEF_T_PHDR);
	cgcef_assert(size);
#if ENABLE_EXTENDED_FORMAT
	if (entsz < size) {
#else /* ENABLE_EXTENDED_FORMAT */
	if (entsz != size) {
#endif /* ENABLE_EXTENDED_FORMAT */
	    seterr(ERROR_EHDR_PHENTSIZE);
	    return 0;
	}
	size = _msize(cgcef->e_class, _cgcef_version, CGCEF_T_PHDR);
	cgcef_assert(size);
	if (!(p = malloc(num * size))) {
	    seterr(memerr(CGCEF_T_PHDR));
	    return 0;
	}
	for (i = 0; i < num; i++) {
	    if (!_cgcef_item(p + i * size, cgcef, CGCEF_T_PHDR, off + i * entsz)) {
		free(p);
		return 0;
	    }
	}
	cgcef->e_phdr = p;
	cgcef->e_phnum = num;
    }
    return 1;
}

static int
_cgcef_cook_shdr(CGCEf *cgcef) {
    size_t num, off, entsz;

    if (cgcef->e_class == CGCEFCLASS32) {
	num = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shnum;
	off = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shoff;
	entsz = ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shentsize;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	num = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shnum;
	off = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shoff;
	entsz = ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shentsize;
	/*
	 * Check for overflow on 32-bit systems
	 */
	if (overflow(off, ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shoff, CGCEf64_Off)) {
	    seterr(ERROR_OUTSIDE);
	    return 0;
	}
    }
#endif /* __LIBCGCEF64 */
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    if (off) {
	struct tmp {
	    CGCEf_Scn	scn;
	    Scn_Data	data;
	} *head;
	CGCEf_Data src, dst;
	CGCEf_Scn *scn;
	Scn_Data *sd;
	unsigned i;

	if (off < 0 || off > cgcef->e_size) {
	    seterr(ERROR_OUTSIDE);
	    return 0;
	}

	src.d_type = CGCEF_T_SHDR;
	src.d_version = cgcef->e_version;
	src.d_size = _fsize(cgcef->e_class, src.d_version, CGCEF_T_SHDR);
	cgcef_assert(src.d_size);
#if ENABLE_EXTENDED_FORMAT
	if (entsz < src.d_size) {
#else /* ENABLE_EXTENDED_FORMAT */
	if (entsz != src.d_size) {
#endif /* ENABLE_EXTENDED_FORMAT */
	    seterr(ERROR_EHDR_SHENTSIZE);
	    return 0;
	}
	dst.d_version = EV_CURRENT;

	if (num == 0) {
	    union {
		CGCEf32_Shdr sh32;
#if __LIBCGCEF64
		CGCEf64_Shdr sh64;
#endif /* __LIBCGCEF64 */
	    } u;

	    /*
	     * Overflow in ehdr->e_shnum.
	     * Get real value from first SHDR.
	     */
	    if (cgcef->e_size - off < entsz) {
		seterr(ERROR_TRUNC_SHDR);
		return 0;
	    }
	    if (cgcef->e_rawdata) {
		src.d_buf = cgcef->e_rawdata + off;
	    }
	    else {
		src.d_buf = cgcef->e_data + off;
	    }
	    dst.d_buf = &u;
	    dst.d_size = sizeof(u);
	    if (!_cgcef_xlatetom(cgcef, &dst, &src)) {
		return 0;
	    }
	    cgcef_assert(dst.d_size == _msize(cgcef->e_class, EV_CURRENT, CGCEF_T_SHDR));
	    cgcef_assert(dst.d_type == CGCEF_T_SHDR);
	    if (cgcef->e_class == CGCEFCLASS32) {
		num = u.sh32.sh_size;
	    }
#if __LIBCGCEF64
	    else if (cgcef->e_class == CGCEFCLASS64) {
		num = u.sh64.sh_size;
		/*
		 * Check for overflow on 32-bit systems
		 */
		if (overflow(num, u.sh64.sh_size, CGCEf64_Xword)) {
		    seterr(ERROR_OUTSIDE);
		    return 0;
		}
	    }
#endif /* __LIBCGCEF64 */
	}

	if ((cgcef->e_size - off) / entsz < num) {
	    seterr(ERROR_TRUNC_SHDR);
	    return 0;
	}

	if (!(head = (struct tmp*)malloc(num * sizeof(struct tmp)))) {
	    seterr(ERROR_MEM_SCN);
	    return 0;
	}
	for (scn = NULL, i = num; i-- > 0; ) {
	    head[i].scn = _cgcef_scn_init;
	    head[i].data = _cgcef_data_init;
	    head[i].scn.s_link = scn;
	    if (!scn) {
		cgcef->e_scn_n = &head[i].scn;
	    }
	    scn = &head[i].scn;
	    sd = &head[i].data;

	    if (cgcef->e_rawdata) {
		src.d_buf = cgcef->e_rawdata + off + i * entsz;
	    }
	    else {
		src.d_buf = cgcef->e_data + off + i * entsz;
	    }
	    dst.d_buf = &scn->s_uhdr;
	    dst.d_size = sizeof(scn->s_uhdr);
	    if (!_cgcef_xlatetom(cgcef, &dst, &src)) {
		cgcef->e_scn_n = NULL;
		free(head);
		return 0;
	    }
	    cgcef_assert(dst.d_size == _msize(cgcef->e_class, EV_CURRENT, CGCEF_T_SHDR));
	    cgcef_assert(dst.d_type == CGCEF_T_SHDR);

	    scn->s_cgcef = cgcef;
	    scn->s_index = i;
	    scn->s_data_1 = sd;
	    scn->s_data_n = sd;

	    sd->sd_scn = scn;

	    if (cgcef->e_class == CGCEFCLASS32) {
		CGCEf32_Shdr *shdr = &scn->s_shdr32;

		scn->s_type = shdr->sh_type;
		scn->s_size = shdr->sh_size;
		scn->s_offset = shdr->sh_offset;
		sd->sd_data.d_align = shdr->sh_addralign;
		sd->sd_data.d_type = _cgcef_scn_type(scn->s_type);
	    }
#if __LIBCGCEF64
	    else if (cgcef->e_class == CGCEFCLASS64) {
		CGCEf64_Shdr *shdr = &scn->s_shdr64;

		scn->s_type = shdr->sh_type;
		scn->s_size = shdr->sh_size;
		scn->s_offset = shdr->sh_offset;
		sd->sd_data.d_align = shdr->sh_addralign;
		/*
		 * Check for overflow on 32-bit systems
		 */
		if (overflow(scn->s_size, shdr->sh_size, CGCEf64_Xword)
		 || overflow(scn->s_offset, shdr->sh_offset, CGCEf64_Off)
		 || overflow(sd->sd_data.d_align, shdr->sh_addralign, CGCEf64_Xword)) {
		    seterr(ERROR_OUTSIDE);
		    return 0;
		}
		sd->sd_data.d_type = _cgcef_scn_type(scn->s_type);
		/*
		 * QUIRKS MODE:
		 *
		 * Some 64-bit architectures use 64-bit entries in the
		 * .hash section. This violates the CGCEF standard, and
		 * should be fixed. It's mostly harmless as long as the
		 * binary and the machine running your program have the
		 * same byte order, but you're in trouble if they don't,
		 * and if the entry size is wrong.
		 *
		 * As a workaround, I let libcgcef guess the right size
		 * for the binary. This relies pretty much on the fact
		 * that the binary provides correct data in the section
		 * headers. If it doesn't, it's probably broken anyway.
		 * Therefore, libcgcef uses a standard conforming value
		 * when it's not absolutely sure.
		 */
		if (scn->s_type == SHT_HASH) {
		    int override = 0;

		    /*
		     * sh_entsize must reflect the entry size
		     */
		    if (shdr->sh_entsize == CGCEF64_FSZ_ADDR) {
			override++;
		    }
		    /*
		     * sh_size must be a multiple of sh_entsize
		     */
		    if (shdr->sh_size % CGCEF64_FSZ_ADDR == 0) {
			override++;
		    }
		    /*
		     * There must be room for at least 2 entries
		     */
		    if (shdr->sh_size >= 2 * CGCEF64_FSZ_ADDR) {
			override++;
		    }
		    /*
		     * sh_addralign must be correctly set
		     */
		    if (shdr->sh_addralign == CGCEF64_FSZ_ADDR) {
			override++;
		    }
		    /*
		     * The section must be properly aligned
		     */
		    if (shdr->sh_offset % CGCEF64_FSZ_ADDR == 0) {
			override++;
		    }
		    /* XXX: also look at the data? */
		    /*
		     * Make a conservative decision...
		     */
		    if (override >= 5) {
			sd->sd_data.d_type = CGCEF_T_ADDR;
		    }
		}
		/*
		 * END QUIRKS MODE.
		 */
	    }
#endif /* __LIBCGCEF64 */
	    /* we already had this
	    else {
		seterr(ERROR_UNIMPLEMENTED);
		return 0;
	    }
	    */

	    sd->sd_data.d_size = scn->s_size;
	    sd->sd_data.d_version = _cgcef_version;
	}
	cgcef_assert(scn == &head[0].scn);
	cgcef->e_scn_1 = &head[0].scn;
	head[0].scn.s_freeme = 1;
    }
    return 1;
}

static int
_cgcef_cook_file(CGCEf *cgcef) {
    cgcef->e_ehdr = _cgcef_item(NULL, cgcef, CGCEF_T_EHDR, 0);
    if (!cgcef->e_ehdr) {
	return 0;
    }
    /*
     * Note: _cgcef_cook_phdr may require the first section header!
     */
    if (!_cgcef_cook_shdr(cgcef)) {
	return 0;
    }
    if (!_cgcef_cook_phdr(cgcef)) {
	return 0;
    }
    return 1;
}

int
_cgcef_cook(CGCEf *cgcef) {
    cgcef_assert(_cgcef_scn_init.s_magic == SCN_MAGIC);
    cgcef_assert(_cgcef_data_init.sd_magic == DATA_MAGIC);
    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    cgcef_assert(cgcef->e_kind == CGCEF_K_CGCEF);
    cgcef_assert(!cgcef->e_ehdr);
    if (!valid_version(cgcef->e_version)) {
	seterr(ERROR_UNKNOWN_VERSION);
    }
    else if (!valid_encoding(cgcef->e_encoding)) {
	seterr(ERROR_UNKNOWN_ENCODING);
    }
    else if (valid_class(cgcef->e_class)) {
	return _cgcef_cook_file(cgcef);
    }
    else {
	seterr(ERROR_UNKNOWN_CLASS);
    }
    return 0;
}
