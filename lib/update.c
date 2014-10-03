/*
 * update.c - implementation of the cgcef_update(3) function.
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
static const char rcsid[] = "@(#) $Id: update.c,v 1.34 2009/05/22 17:08:09 michael Exp $";
#endif /* lint */

#include <errno.h>

#if HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

static const unsigned short __encoding = CGCEFDATA2LSB + (CGCEFDATA2MSB << 8);
#define native_encoding (*(unsigned char*)&__encoding)

#define rewrite(var,val,f)	\
    do{if((var)!=(val)){(var)=(val);(f)|=CGCEF_F_DIRTY;}}while(0)

#define align(var,val)		\
    do{if((val)>1){(var)+=(val)-1;(var)-=(var)%(val);}}while(0)

#undef max
#define max(a,b)		((a)>(b)?(a):(b))

static off_t
scn_data_layout(CGCEf_Scn *scn, unsigned v, unsigned type, size_t *algn, unsigned *flag) {
    CGCEf *cgcef = scn->s_cgcef;
    CGCEf_Data *data;
    int layout = (cgcef->e_cgcef_flags & CGCEF_F_LAYOUT) == 0;
    size_t scn_align = 1;
    size_t len = 0;
    Scn_Data *sd;
    size_t fsize;

    if (!(sd = scn->s_data_1)) {
	/* no data in section */
	*algn = scn_align;
	return (off_t)len;
    }
    /* load data from file, if any */
    if (!(data = cgcef_getdata(scn, NULL))) {
	return (off_t)-1;
    }
    cgcef_assert(data == &sd->sd_data);
    for (; sd; sd = sd->sd_link) {
	cgcef_assert(sd->sd_magic == DATA_MAGIC);
	cgcef_assert(sd->sd_scn == scn);

	if (!valid_version(sd->sd_data.d_version)) {
	    return (off_t)-1;
	}

	fsize = sd->sd_data.d_size;
	if (fsize && type != SHT_NOBITS && valid_type(sd->sd_data.d_type)) {
	    if (cgcef->e_class == CGCEFCLASS32) {
		fsize = _cgcef32_xltsize(&sd->sd_data, v, CGCEFDATA2LSB, 1);
	    }
#if __LIBCGCEF64
	    else if (cgcef->e_class == CGCEFCLASS64) {
		fsize = _cgcef64_xltsize(&sd->sd_data, v, CGCEFDATA2LSB, 1);
	    }
#endif /* __LIBCGCEF64 */
	    else {
		cgcef_assert(valid_class(cgcef->e_class));
		seterr(ERROR_UNIMPLEMENTED);
		return (off_t)-1;
	    }
	    if (fsize == (size_t)-1) {
		return (off_t)-1;
	    }
	}

	if (layout) {
	    align(len, sd->sd_data.d_align);
	    scn_align = max(scn_align, sd->sd_data.d_align);
	    rewrite(sd->sd_data.d_off, (off_t)len, sd->sd_data_flags);
	    len += fsize;
	}
	else {
	    len = max(len, sd->sd_data.d_off + fsize);
	}

	*flag |= sd->sd_data_flags;
    }
    *algn = scn_align;
    return (off_t)len;
}

static size_t
scn_entsize(const CGCEf *cgcef, unsigned version, unsigned stype) {
    CGCEf_Type type;

    switch ((type = _cgcef_scn_type(stype))) {
	case CGCEF_T_BYTE:
	    return 0;
	case CGCEF_T_VDEF:
	case CGCEF_T_VNEED:
	    return 0;	/* What else can I do?  Thank you, Sun! */
	default:
	    return _fsize(cgcef->e_class, version, type);
    }
}

static off_t
_cgcef32_layout(CGCEf *cgcef, unsigned *flag) {
    int layout = (cgcef->e_cgcef_flags & CGCEF_F_LAYOUT) == 0;
    int allow_overlap = (cgcef->e_cgcef_flags & CGCEF_F_LAYOUT_OVERLAP) != 0;
    CGCEf32_Ehdr *ehdr = (CGCEf32_Ehdr*)cgcef->e_ehdr;
    size_t off = 0;
    unsigned version;
    unsigned encoding;
    size_t align_addr;
    size_t entsize;
    unsigned phnum;
    unsigned shnum;
    CGCEf_Scn *scn;

    *flag = cgcef->e_cgcef_flags | cgcef->e_phdr_flags;

    if ((version = ehdr->e_version) == EV_NONE) {
	version = EV_CURRENT;
    }
    if (!valid_version(version)) {
	seterr(ERROR_UNKNOWN_VERSION);
	return -1;
    }
    if ((encoding = ehdr->e_ident[EI_DATA]) == CGCEFDATANONE) {
	encoding = native_encoding;
    }
    if (!valid_encoding(encoding)) {
	seterr(ERROR_UNKNOWN_ENCODING);
	return -1;
    }
    entsize = _fsize(CGCEFCLASS32, version, CGCEF_T_EHDR);
    cgcef_assert(entsize);
    rewrite(ehdr->e_ehsize, entsize, cgcef->e_ehdr_flags);
    off = entsize;

    align_addr = _fsize(CGCEFCLASS32, version, CGCEF_T_ADDR);
    cgcef_assert(align_addr);

    if ((phnum = cgcef->e_phnum)) {
	entsize = _fsize(CGCEFCLASS32, version, CGCEF_T_PHDR);
	cgcef_assert(entsize);
	if (layout) {
	    align(off, align_addr);
	    rewrite(ehdr->e_phoff, off, cgcef->e_ehdr_flags);
	    off += phnum * entsize;
	}
	else {
	    off = max(off, ehdr->e_phoff + phnum * entsize);
	}
    }
    else {
	entsize = 0;
	if (layout) {
	    rewrite(ehdr->e_phoff, 0, cgcef->e_ehdr_flags);
	}
    }
    if (phnum >= PN_XNUM) {
	CGCEf_Scn *scn = cgcef->e_scn_1;
	CGCEf32_Shdr *shdr = &scn->s_shdr32;

	cgcef_assert(scn);
	cgcef_assert(scn->s_index == 0);
	rewrite(shdr->sh_info, phnum, scn->s_shdr_flags);
	*flag |= scn->s_shdr_flags;
	phnum = PN_XNUM;
    }
    rewrite(ehdr->e_phnum, phnum, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_phentsize, entsize, cgcef->e_ehdr_flags);

    for (scn = cgcef->e_scn_1, shnum = 0; scn; scn = scn->s_link, ++shnum) {
	CGCEf32_Shdr *shdr = &scn->s_shdr32;
	size_t scn_align = 1;
	off_t len;

	cgcef_assert(scn->s_index == shnum);

	*flag |= scn->s_scn_flags;

	if (scn->s_index == SHN_UNDEF) {
	    rewrite(shdr->sh_entsize, 0, scn->s_shdr_flags);
	    if (layout) {
		rewrite(shdr->sh_offset, 0, scn->s_shdr_flags);
		rewrite(shdr->sh_size, 0, scn->s_shdr_flags);
		rewrite(shdr->sh_addralign, 0, scn->s_shdr_flags);
	    }
	    *flag |= scn->s_shdr_flags;
	    continue;
	}
	if (shdr->sh_type == SHT_NULL) {
	    *flag |= scn->s_shdr_flags;
	    continue;
	}

	len = scn_data_layout(scn, version, shdr->sh_type, &scn_align, flag);
	if (len == -1) {
	    return -1;
	}

	/*
	 * Never override the program's choice.
	 */
	if (shdr->sh_entsize == 0) {
	    entsize = scn_entsize(cgcef, version, shdr->sh_type);
	    if (entsize > 1) {
		rewrite(shdr->sh_entsize, entsize, scn->s_shdr_flags);
	    }
	}

	if (layout) {
	    align(off, scn_align);
	    rewrite(shdr->sh_offset, off, scn->s_shdr_flags);
	    rewrite(shdr->sh_size, (size_t)len, scn->s_shdr_flags);
	    rewrite(shdr->sh_addralign, scn_align, scn->s_shdr_flags);

	    if (shdr->sh_type != SHT_NOBITS) {
		off += (size_t)len;
	    }
	}
	else if ((size_t)len > shdr->sh_size) {
	    seterr(ERROR_SCN2SMALL);
	    return -1;
	}
	else {
	    CGCEf_Scn *scn2;
	    size_t end1, end2;

	    end1 = shdr->sh_offset;
	    if (shdr->sh_type != SHT_NOBITS) {
		end1 += shdr->sh_size;
	    }
	    if (!allow_overlap && shdr->sh_offset < off) {
		/*
		 * check for overlapping sections
		 */
		for (scn2 = cgcef->e_scn_1; scn2; scn2 = scn2->s_link) {
		    if (scn2 == scn) {
			break;
		    }
		    end2 = scn2->s_shdr32.sh_offset;
		    if (scn2->s_shdr32.sh_type != SHT_NOBITS) {
			end2 += scn2->s_shdr32.sh_size;
		    }
		    if (end1 > scn2->s_shdr32.sh_offset
		     && end2 > shdr->sh_offset) {
			seterr(ERROR_SCN_OVERLAP);
			return -1;
		    }
		}
	    }
	    if (off < end1) {
		off = end1;
	    }
	}
	*flag |= scn->s_shdr_flags;
    }

    if (shnum) {
	entsize = _fsize(CGCEFCLASS32, version, CGCEF_T_SHDR);
	cgcef_assert(entsize);
	if (layout) {
	    align(off, align_addr);
	    rewrite(ehdr->e_shoff, off, cgcef->e_ehdr_flags);
	    off += shnum * entsize;
	}
	else {
	    off = max(off, ehdr->e_shoff + shnum * entsize);
	}
    }
    else {
	entsize = 0;
	if (layout) {
	    rewrite(ehdr->e_shoff, 0, cgcef->e_ehdr_flags);
	}
    }
    if (shnum >= SHN_LORESERVE) {
	CGCEf_Scn *scn = cgcef->e_scn_1;
	CGCEf32_Shdr *shdr = &scn->s_shdr32;

	cgcef_assert(scn->s_index == 0);
	rewrite(shdr->sh_size, shnum, scn->s_shdr_flags);
	*flag |= scn->s_shdr_flags;
	shnum = 0;
    }
    rewrite(ehdr->e_shnum, shnum, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_shentsize, entsize, cgcef->e_ehdr_flags);

    rewrite(ehdr->e_ident[EI_MAG0], CGCEFMAG0, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG1], CGCEFMAG1, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG2], CGCEFMAG2, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG3], CGCEFMAG3, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_CLASS], CGCEFCLASS32, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_DATA], encoding, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_VERSION], version, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_version, version, cgcef->e_ehdr_flags);

    *flag |= cgcef->e_ehdr_flags;

    return off;
}

#if __LIBCGCEF64

static off_t
_cgcef64_layout(CGCEf *cgcef, unsigned *flag) {
    int layout = (cgcef->e_cgcef_flags & CGCEF_F_LAYOUT) == 0;
    int allow_overlap = (cgcef->e_cgcef_flags & CGCEF_F_LAYOUT_OVERLAP) != 0;
    CGCEf64_Ehdr *ehdr = (CGCEf64_Ehdr*)cgcef->e_ehdr;
    size_t off = 0;
    unsigned version;
    unsigned encoding;
    size_t align_addr;
    size_t entsize;
    unsigned phnum;
    unsigned shnum;
    CGCEf_Scn *scn;

    *flag = cgcef->e_cgcef_flags | cgcef->e_phdr_flags;

    if ((version = ehdr->e_version) == EV_NONE) {
	version = EV_CURRENT;
    }
    if (!valid_version(version)) {
	seterr(ERROR_UNKNOWN_VERSION);
	return -1;
    }
    if ((encoding = ehdr->e_ident[EI_DATA]) == CGCEFDATANONE) {
	encoding = native_encoding;
    }
    if (!valid_encoding(encoding)) {
	seterr(ERROR_UNKNOWN_ENCODING);
	return -1;
    }
    entsize = _fsize(CGCEFCLASS64, version, CGCEF_T_EHDR);
    cgcef_assert(entsize);
    rewrite(ehdr->e_ehsize, entsize, cgcef->e_ehdr_flags);
    off = entsize;

    align_addr = _fsize(CGCEFCLASS64, version, CGCEF_T_ADDR);
    cgcef_assert(align_addr);

    if ((phnum = cgcef->e_phnum)) {
	entsize = _fsize(CGCEFCLASS64, version, CGCEF_T_PHDR);
	cgcef_assert(entsize);
	if (layout) {
	    align(off, align_addr);
	    rewrite(ehdr->e_phoff, off, cgcef->e_ehdr_flags);
	    off += phnum * entsize;
	}
	else {
	    off = max(off, ehdr->e_phoff + phnum * entsize);
	}
    }
    else {
	entsize = 0;
	if (layout) {
	    rewrite(ehdr->e_phoff, 0, cgcef->e_ehdr_flags);
	}
    }
    if (phnum >= PN_XNUM) {
	CGCEf_Scn *scn = cgcef->e_scn_1;
	CGCEf32_Shdr *shdr = &scn->s_shdr32;

	/* modify first section header, too! */
	cgcef_assert(scn);
	cgcef_assert(scn->s_index == 0);
	rewrite(shdr->sh_info, phnum, scn->s_shdr_flags);
	*flag |= scn->s_shdr_flags;
	phnum = PN_XNUM;
    }
    rewrite(ehdr->e_phnum, phnum, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_phentsize, entsize, cgcef->e_ehdr_flags);

    for (scn = cgcef->e_scn_1, shnum = 0; scn; scn = scn->s_link, ++shnum) {
	CGCEf64_Shdr *shdr = &scn->s_shdr64;
	size_t scn_align = 1;
	off_t len;

	cgcef_assert(scn->s_index == shnum);

	*flag |= scn->s_scn_flags;

	if (scn->s_index == SHN_UNDEF) {
	    rewrite(shdr->sh_entsize, 0, scn->s_shdr_flags);
	    if (layout) {
		rewrite(shdr->sh_offset, 0, scn->s_shdr_flags);
		rewrite(shdr->sh_size, 0, scn->s_shdr_flags);
		rewrite(shdr->sh_addralign, 0, scn->s_shdr_flags);
	    }
	    *flag |= scn->s_shdr_flags;
	    continue;
	}
	if (shdr->sh_type == SHT_NULL) {
	    *flag |= scn->s_shdr_flags;
	    continue;
	}

	len = scn_data_layout(scn, version, shdr->sh_type, &scn_align, flag);
	if (len == -1) {
	    return -1;
	}

	/*
	 * Never override the program's choice.
	 */
	if (shdr->sh_entsize == 0) {
	    entsize = scn_entsize(cgcef, version, shdr->sh_type);
	    if (entsize > 1) {
		rewrite(shdr->sh_entsize, entsize, scn->s_shdr_flags);
	    }
	}

	if (layout) {
	    align(off, scn_align);
	    rewrite(shdr->sh_offset, off, scn->s_shdr_flags);
	    rewrite(shdr->sh_size, (size_t)len, scn->s_shdr_flags);
	    rewrite(shdr->sh_addralign, scn_align, scn->s_shdr_flags);

	    if (shdr->sh_type != SHT_NOBITS) {
		off += (size_t)len;
	    }
	}
	else if ((size_t)len > shdr->sh_size) {
	    seterr(ERROR_SCN2SMALL);
	    return -1;
	}
	else {
	    CGCEf_Scn *scn2;
	    size_t end1, end2;

	    end1 = shdr->sh_offset;
	    if (shdr->sh_type != SHT_NOBITS) {
		end1 += shdr->sh_size;
	    }
	    if (!allow_overlap && shdr->sh_offset < off) {
		/*
		 * check for overlapping sections
		 */
		for (scn2 = cgcef->e_scn_1; scn2; scn2 = scn2->s_link) {
		    if (scn2 == scn) {
			break;
		    }
		    end2 = scn2->s_shdr64.sh_offset;
		    if (scn2->s_shdr64.sh_type != SHT_NOBITS) {
			end2 += scn2->s_shdr64.sh_size;
		    }
		    if (end1 > scn2->s_shdr64.sh_offset
		     && end2 > shdr->sh_offset) {
			seterr(ERROR_SCN_OVERLAP);
			return -1;
		    }
		}
	    }
	    if (off < end1) {
		off = end1;
	    }
	}
	*flag |= scn->s_shdr_flags;
    }

    if (shnum) {
	entsize = _fsize(CGCEFCLASS64, version, CGCEF_T_SHDR);
	cgcef_assert(entsize);
	if (layout) {
	    align(off, align_addr);
	    rewrite(ehdr->e_shoff, off, cgcef->e_ehdr_flags);
	    off += shnum * entsize;
	}
	else {
	    off = max(off, ehdr->e_shoff + shnum * entsize);
	}
    }
    else {
	entsize = 0;
	if (layout) {
	    rewrite(ehdr->e_shoff, 0, cgcef->e_ehdr_flags);
	}
    }
    if (shnum >= SHN_LORESERVE) {
	CGCEf_Scn *scn = cgcef->e_scn_1;
	CGCEf64_Shdr *shdr = &scn->s_shdr64;

	cgcef_assert(scn->s_index == 0);
	rewrite(shdr->sh_size, shnum, scn->s_shdr_flags);
	*flag |= scn->s_shdr_flags;
	shnum = 0;
    }
    rewrite(ehdr->e_shnum, shnum, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_shentsize, entsize, cgcef->e_ehdr_flags);

    rewrite(ehdr->e_ident[EI_MAG0], CGCEFMAG0, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG1], CGCEFMAG1, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG2], CGCEFMAG2, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_MAG3], CGCEFMAG3, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_CLASS], CGCEFCLASS64, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_DATA], encoding, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_ident[EI_VERSION], version, cgcef->e_ehdr_flags);
    rewrite(ehdr->e_version, version, cgcef->e_ehdr_flags);

    *flag |= cgcef->e_ehdr_flags;

    return off;
}

#endif /* __LIBCGCEF64 */

#define ptrinside(p,a,l)	((p)>=(a)&&(p)<(a)+(l))
#define newptr(p,o,n)		((p)=((p)-(o))+(n))

static int
_cgcef_update_pointers(CGCEf *cgcef, char *outbuf, size_t len) {
    CGCEf_Scn *scn;
    Scn_Data *sd;
    char *data, *rawdata;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_data);
    cgcef_assert(!cgcef->e_parent);
    cgcef_assert(!cgcef->e_unmap_data);
    cgcef_assert(cgcef->e_kind == CGCEF_K_CGCEF);
    cgcef_assert(len >= EI_NIDENT);

    /* resize memory images */
    if (len <= cgcef->e_dsize) {
	/* don't shorten the memory image */
	data = cgcef->e_data;
    }
    else if ((data = (char*)realloc(cgcef->e_data, len))) {
	cgcef->e_dsize = len;
    }
    else {
	seterr(ERROR_IO_2BIG);
	return -1;
    }
    if (cgcef->e_rawdata == cgcef->e_data) {
	/* update frozen raw image */
	memcpy(data, outbuf, len);
	cgcef->e_data = cgcef->e_rawdata = data;
	/* cooked data is stored outside the raw image */
	return 0;
    }
    if (cgcef->e_rawdata) {
	/* update raw image */
	if (!(rawdata = (char*)realloc(cgcef->e_rawdata, len))) {
	    seterr(ERROR_IO_2BIG);
	    return -1;
	}
	memcpy(rawdata, outbuf, len);
	cgcef->e_rawdata = rawdata;
    }
    if (data == cgcef->e_data) {
	/* nothing more to do */
	return 0;
    }
    /* adjust internal pointers */
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	cgcef_assert(scn->s_magic == SCN_MAGIC);
	cgcef_assert(scn->s_cgcef == cgcef);
	if ((sd = scn->s_data_1)) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    if (sd->sd_memdata && !sd->sd_free_data) {
		cgcef_assert(ptrinside(sd->sd_memdata, cgcef->e_data, cgcef->e_dsize));
		if (sd->sd_data.d_buf == sd->sd_memdata) {
		    newptr(sd->sd_memdata, cgcef->e_data, data);
		    sd->sd_data.d_buf = sd->sd_memdata;
		}
		else {
		    newptr(sd->sd_memdata, cgcef->e_data, data);
		}
	    }
	}
	if ((sd = scn->s_rawdata)) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    if (sd->sd_memdata && sd->sd_free_data) {
		size_t off, len;

		if (cgcef->e_class == CGCEFCLASS32) {
		    off = scn->s_shdr32.sh_offset;
		    len = scn->s_shdr32.sh_size;
		}
#if __LIBCGCEF64
		else if (cgcef->e_class == CGCEFCLASS64) {
		    off = scn->s_shdr64.sh_offset;
		    len = scn->s_shdr64.sh_size;
		}
#endif /* __LIBCGCEF64 */
		else {
		    seterr(ERROR_UNIMPLEMENTED);
		    return -1;
		}
		if (!(rawdata = (char*)realloc(sd->sd_memdata, len))) {
		    seterr(ERROR_IO_2BIG);
		    return -1;
		}
		memcpy(rawdata, outbuf + off, len);
		if (sd->sd_data.d_buf == sd->sd_memdata) {
		    sd->sd_data.d_buf = rawdata;
		}
		sd->sd_memdata = rawdata;
	    }
	}
    }
    cgcef->e_data = data;
    return 0;
}

#undef ptrinside
#undef newptr

static off_t
_cgcef32_write(CGCEf *cgcef, char *outbuf, size_t len) {
    CGCEf32_Ehdr *ehdr;
    CGCEf32_Shdr *shdr;
    CGCEf_Scn *scn;
    Scn_Data *sd;
    CGCEf_Data src;
    CGCEf_Data dst;
    unsigned encode;

    cgcef_assert(len);
    cgcef_assert(cgcef->e_ehdr);
    ehdr = (CGCEf32_Ehdr*)cgcef->e_ehdr;
    encode = ehdr->e_ident[EI_DATA];

    src.d_buf = ehdr;
    src.d_type = CGCEF_T_EHDR;
    src.d_size = _msize(CGCEFCLASS32, _cgcef_version, CGCEF_T_EHDR);
    src.d_version = _cgcef_version;
    dst.d_buf = outbuf;
    dst.d_size = ehdr->e_ehsize;
    dst.d_version = ehdr->e_version;
    if (!cgcef32_xlatetof(&dst, &src, encode)) {
	return -1;
    }

    if (cgcef->e_phnum) {
	src.d_buf = cgcef->e_phdr;
	src.d_type = CGCEF_T_PHDR;
	src.d_size = cgcef->e_phnum * _msize(CGCEFCLASS32, _cgcef_version, CGCEF_T_PHDR);
	src.d_version = _cgcef_version;
	dst.d_buf = outbuf + ehdr->e_phoff;
	dst.d_size = cgcef->e_phnum * ehdr->e_phentsize;
	dst.d_version = ehdr->e_version;
	if (!cgcef32_xlatetof(&dst, &src, encode)) {
	    return -1;
	}
    }

    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	cgcef_assert(scn->s_magic == SCN_MAGIC);
	cgcef_assert(scn->s_cgcef == cgcef);

	src.d_buf = &scn->s_uhdr;
	src.d_type = CGCEF_T_SHDR;
	src.d_size = _msize(CGCEFCLASS32, EV_CURRENT, CGCEF_T_SHDR);
	src.d_version = EV_CURRENT;
	dst.d_buf = outbuf + ehdr->e_shoff + scn->s_index * ehdr->e_shentsize;
	dst.d_size = ehdr->e_shentsize;
	dst.d_version = ehdr->e_version;
	if (!cgcef32_xlatetof(&dst, &src, encode)) {
	    return -1;
	}

	if (scn->s_index == SHN_UNDEF) {
	    continue;
	}
	shdr = &scn->s_shdr32;
	if (shdr->sh_type == SHT_NULL || shdr->sh_type == SHT_NOBITS) {
	    continue;
	}
	/* XXX: this is probably no longer necessary */
	if (scn->s_data_1 && !cgcef_getdata(scn, NULL)) {
	    return -1;
	}
	for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    src = sd->sd_data;
	    if (!src.d_size) {
		continue;
	    }
	    if (!src.d_buf) {
		seterr(ERROR_NULLBUF);
		return -1;
	    }
	    dst.d_buf = outbuf + shdr->sh_offset + src.d_off;
	    dst.d_size = src.d_size;
	    dst.d_version = ehdr->e_version;
	    if (valid_type(src.d_type)) {
		size_t tmp;

		tmp = _cgcef32_xltsize(&src, dst.d_version, CGCEFDATA2LSB, 1);
		if (tmp == (size_t)-1) {
		    return -1;
		}
		dst.d_size = tmp;
	    }
	    else {
		src.d_type = CGCEF_T_BYTE;
	    }
	    if (!cgcef32_xlatetof(&dst, &src, encode)) {
		return -1;
	    }
	}
    }

    /* cleanup */
    if (cgcef->e_readable && _cgcef_update_pointers(cgcef, outbuf, len)) {
	return -1;
    }
    /* NOTE: ehdr is no longer valid! */
    ehdr = (CGCEf32_Ehdr*)cgcef->e_ehdr; cgcef_assert(ehdr);
    cgcef->e_encoding = ehdr->e_ident[EI_DATA];
    cgcef->e_version = ehdr->e_ident[EI_VERSION];
    cgcef->e_cgcef_flags &= ~CGCEF_F_DIRTY;
    cgcef->e_ehdr_flags &= ~CGCEF_F_DIRTY;
    cgcef->e_phdr_flags &= ~CGCEF_F_DIRTY;
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	scn->s_scn_flags &= ~CGCEF_F_DIRTY;
	scn->s_shdr_flags &= ~CGCEF_F_DIRTY;
	for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
	    sd->sd_data_flags &= ~CGCEF_F_DIRTY;
	}
	if (cgcef->e_readable) {
	    shdr = &scn->s_shdr32;
	    scn->s_type = shdr->sh_type;
	    scn->s_size = shdr->sh_size;
	    scn->s_offset = shdr->sh_offset;
	}
    }
    cgcef->e_size = len;
    return len;
}

#if __LIBCGCEF64

static off_t
_cgcef64_write(CGCEf *cgcef, char *outbuf, size_t len) {
    CGCEf64_Ehdr *ehdr;
    CGCEf64_Shdr *shdr;
    CGCEf_Scn *scn;
    Scn_Data *sd;
    CGCEf_Data src;
    CGCEf_Data dst;
    unsigned encode;

    cgcef_assert(len);
    cgcef_assert(cgcef->e_ehdr);
    ehdr = (CGCEf64_Ehdr*)cgcef->e_ehdr;
    encode = ehdr->e_ident[EI_DATA];

    src.d_buf = ehdr;
    src.d_type = CGCEF_T_EHDR;
    src.d_size = _msize(CGCEFCLASS64, _cgcef_version, CGCEF_T_EHDR);
    src.d_version = _cgcef_version;
    dst.d_buf = outbuf;
    dst.d_size = ehdr->e_ehsize;
    dst.d_version = ehdr->e_version;
    if (!cgcef64_xlatetof(&dst, &src, encode)) {
	return -1;
    }

    if (cgcef->e_phnum) {
	src.d_buf = cgcef->e_phdr;
	src.d_type = CGCEF_T_PHDR;
	src.d_size = cgcef->e_phnum * _msize(CGCEFCLASS64, _cgcef_version, CGCEF_T_PHDR);
	src.d_version = _cgcef_version;
	dst.d_buf = outbuf + ehdr->e_phoff;
	dst.d_size = cgcef->e_phnum * ehdr->e_phentsize;
	dst.d_version = ehdr->e_version;
	if (!cgcef64_xlatetof(&dst, &src, encode)) {
	    return -1;
	}
    }

    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	cgcef_assert(scn->s_magic == SCN_MAGIC);
	cgcef_assert(scn->s_cgcef == cgcef);

	src.d_buf = &scn->s_uhdr;
	src.d_type = CGCEF_T_SHDR;
	src.d_size = _msize(CGCEFCLASS64, EV_CURRENT, CGCEF_T_SHDR);
	src.d_version = EV_CURRENT;
	dst.d_buf = outbuf + ehdr->e_shoff + scn->s_index * ehdr->e_shentsize;
	dst.d_size = ehdr->e_shentsize;
	dst.d_version = ehdr->e_version;
	if (!cgcef64_xlatetof(&dst, &src, encode)) {
	    return -1;
	}

	if (scn->s_index == SHN_UNDEF) {
	    continue;
	}
	shdr = &scn->s_shdr64;
	if (shdr->sh_type == SHT_NULL || shdr->sh_type == SHT_NOBITS) {
	    continue;
	}
	/* XXX: this is probably no longer necessary */
	if (scn->s_data_1 && !cgcef_getdata(scn, NULL)) {
	    return -1;
	}
	for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    src = sd->sd_data;
	    if (!src.d_size) {
		continue;
	    }
	    if (!src.d_buf) {
		seterr(ERROR_NULLBUF);
		return -1;
	    }
	    dst.d_buf = outbuf + shdr->sh_offset + src.d_off;
	    dst.d_size = src.d_size;
	    dst.d_version = ehdr->e_version;
	    if (valid_type(src.d_type)) {
		size_t tmp;

		tmp = _cgcef64_xltsize(&src, dst.d_version, CGCEFDATA2LSB, 1);
		if (tmp == (size_t)-1) {
		    return -1;
		}
		dst.d_size = tmp;
	    }
	    else {
		src.d_type = CGCEF_T_BYTE;
	    }
	    if (!cgcef64_xlatetof(&dst, &src, encode)) {
		return -1;
	    }
	}
    }

    /* cleanup */
    if (cgcef->e_readable && _cgcef_update_pointers(cgcef, outbuf, len)) {
	return -1;
    }
    /* NOTE: ehdr is no longer valid! */
    ehdr = (CGCEf64_Ehdr*)cgcef->e_ehdr; cgcef_assert(ehdr);
    cgcef->e_encoding = ehdr->e_ident[EI_DATA];
    cgcef->e_version = ehdr->e_ident[EI_VERSION];
    cgcef->e_cgcef_flags &= ~CGCEF_F_DIRTY;
    cgcef->e_ehdr_flags &= ~CGCEF_F_DIRTY;
    cgcef->e_phdr_flags &= ~CGCEF_F_DIRTY;
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	scn->s_scn_flags &= ~CGCEF_F_DIRTY;
	scn->s_shdr_flags &= ~CGCEF_F_DIRTY;
	for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
	    sd->sd_data_flags &= ~CGCEF_F_DIRTY;
	}
	if (cgcef->e_readable) {
	    shdr = &scn->s_shdr64;
	    scn->s_type = shdr->sh_type;
	    scn->s_size = shdr->sh_size;
	    scn->s_offset = shdr->sh_offset;
	}
    }
    cgcef->e_size = len;
    return len;
}

#endif /* __LIBCGCEF64 */

static int
xwrite(int fd, char *buffer, size_t len) {
    size_t done = 0;
    size_t n;

    while (done < len) {
	n = write(fd, buffer + done, len - done);
	if (n == 0) {
	    /* file system full */
	    return -1;
	}
	else if (n != (size_t)-1) {
	    /* some bytes written, continue */
	    done += n;
	}
	else if (errno != EAGAIN && errno != EINTR) {
	    /* real error */
	    return -1;
	}
    }
    return 0;
}

static off_t
_cgcef_output(CGCEf *cgcef, int fd, size_t len, off_t (*_cgcef_write)(CGCEf*, char*, size_t)) {
    char *buf;
    off_t err;

    cgcef_assert(len);
#if HAVE_FTRUNCATE
    ftruncate(fd, 0);
#endif /* HAVE_FTRUNCATE */
#if HAVE_MMAP
    /*
     * Make sure the file is (at least) len bytes long
     */
#if HAVE_FTRUNCATE
    lseek(fd, (off_t)len, SEEK_SET);
    if (ftruncate(fd, len)) {
#else /* HAVE_FTRUNCATE */
    {
#endif /* HAVE_FTRUNCATE */
	if (lseek(fd, (off_t)len - 1, SEEK_SET) != (off_t)len - 1) {
	    seterr(ERROR_IO_SEEK);
	    return -1;
	}
	if (xwrite(fd, "", 1)) {
	    seterr(ERROR_IO_WRITE);
	    return -1;
	}
    }
    buf = (void*)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buf != (char*)-1) {
	if ((char)_cgcef_fill && !(cgcef->e_cgcef_flags & CGCEF_F_LAYOUT)) {
	    memset(buf, _cgcef_fill, len);
	}
	err = _cgcef_write(cgcef, buf, len);
	munmap(buf, len);
	return err;
    }
#endif /* HAVE_MMAP */
    if (!(buf = (char*)malloc(len))) {
	seterr(ERROR_MEM_OUTBUF);
	return -1;
    }
    memset(buf, _cgcef_fill, len);
    err = _cgcef_write(cgcef, buf, len);
    if (err != -1 && (size_t)err == len) {
	if (lseek(fd, (off_t)0, SEEK_SET)) {
	    seterr(ERROR_IO_SEEK);
	    err = -1;
	}
	else if (xwrite(fd, buf, len)) {
	    seterr(ERROR_IO_WRITE);
	    err = -1;
	}
    }
    free(buf);
    return err;
}

off_t
cgcef_update(CGCEf *cgcef, CGCEf_Cmd cmd) {
    unsigned flag;
    off_t len;

    if (!cgcef) {
	return -1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cmd == CGCEF_C_WRITE) {
	if (!cgcef->e_writable) {
	    seterr(ERROR_RDONLY);
	    return -1;
	}
	if (cgcef->e_disabled) {
	    seterr(ERROR_FDDISABLED);
	    return -1;
	}
    }
    else if (cmd != CGCEF_C_NULL) {
	seterr(ERROR_INVALID_CMD);
	return -1;
    }

    if (!cgcef->e_ehdr) {
	seterr(ERROR_NOEHDR);
    }
    else if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
    }
    else if (cgcef->e_class == CGCEFCLASS32) {
	len = _cgcef32_layout(cgcef, &flag);
	if (len != -1 && cmd == CGCEF_C_WRITE && (flag & CGCEF_F_DIRTY)) {
	    len = _cgcef_output(cgcef, cgcef->e_fd, (size_t)len, _cgcef32_write);
	}
	return len;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	len = _cgcef64_layout(cgcef, &flag);
	if (len != -1 && cmd == CGCEF_C_WRITE && (flag & CGCEF_F_DIRTY)) {
	    len = _cgcef_output(cgcef, cgcef->e_fd, (size_t)len, _cgcef64_write);
	}
	return len;
    }
#endif /* __LIBCGCEF64 */
    else if (valid_class(cgcef->e_class)) {
	seterr(ERROR_UNIMPLEMENTED);
    }
    else {
	seterr(ERROR_UNKNOWN_CLASS);
    }
    return -1;
}
