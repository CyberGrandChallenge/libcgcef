/*
gcgceftrans.c - gcgcef_* translation functions.
Copyright (C) 2000 - 2001 Michael Riepe

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

#include <private.h>

#if __LIBCGCEF64

#ifndef lint
static const char rcsid[] = "@(#) $Id: gcgceftrans.c,v 1.10 2008/05/23 08:15:34 michael Exp $";
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

/*
 * These macros are missing on some Linux systems
 */
#if !defined(CGCEF32_R_SYM) || !defined(CGCEF32_R_TYPE) || !defined(CGCEF32_R_INFO)
# undef CGCEF32_R_SYM
# undef CGCEF32_R_TYPE
# undef CGCEF32_R_INFO
# define CGCEF32_R_SYM(i)		((i)>>8)
# define CGCEF32_R_TYPE(i)	((unsigned char)(i))
# define CGCEF32_R_INFO(s,t)	(((s)<<8)+(unsigned char)(t))
#endif /* !defined(...) */

#if !defined(CGCEF64_R_SYM) || !defined(CGCEF64_R_TYPE) || !defined(CGCEF64_R_INFO)
# undef CGCEF64_R_SYM
# undef CGCEF64_R_TYPE
# undef CGCEF64_R_INFO
# define CGCEF64_R_SYM(i)		((i)>>32)
# define CGCEF64_R_TYPE(i)	((i)&0xffffffffL)
# define CGCEF64_R_INFO(s,t)	(((CGCEf64_Xword)(s)<<32)+((t)&0xffffffffL))
#endif /* !defined(...) */

static char*
get_addr_and_class(const CGCEf_Data *data, int ndx, CGCEf_Type type, unsigned *cls) {
    Scn_Data *sd = (Scn_Data*)data;
    CGCEf_Scn *scn;
    CGCEf *cgcef;
    size_t n;

    if (!sd) {
	return NULL;
    }
    cgcef_assert(sd->sd_magic == DATA_MAGIC);
    scn = sd->sd_scn;
    cgcef_assert(scn);
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef = scn->s_cgcef;
    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return NULL;
    }
    if (!valid_class(cgcef->e_class)) {
	seterr(ERROR_UNKNOWN_CLASS);
	return NULL;
    }
    if (data->d_type != type) {
	seterr(ERROR_BADTYPE);
	return NULL;
    }
    n = _msize(cgcef->e_class, data->d_version, type);
    if (n == 0) {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (ndx < 0 || data->d_size < (ndx + 1) * n) {
	seterr(ERROR_BADINDEX);
	return NULL;
    }
    if (!data->d_buf) {
	seterr(ERROR_NULLBUF);
	return NULL;
    }
    if (cls) {
	*cls = cgcef->e_class;
    }
    return (char*)data->d_buf + n * ndx;
}

GCGCEf_Sym*
gcgcef_getsym(CGCEf_Data *src, int ndx, GCGCEf_Sym *dst) {
    GCGCEf_Sym buf;
    unsigned cls;
    char *tmp;

    if (!dst) {
	dst = &buf;
    }
    tmp = get_addr_and_class(src, ndx, CGCEF_T_SYM, &cls);
    if (!tmp) {
	return NULL;
    }
    if (cls == CGCEFCLASS64) {
	*dst = *(CGCEf64_Sym*)tmp;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Sym *src = (CGCEf32_Sym*)tmp;

	check_and_copy(GCGCEf_Word,     dst, src, st_name,  NULL);
	check_and_copy(unsigned char, dst, src, st_info,  NULL);
	check_and_copy(unsigned char, dst, src, st_other, NULL);
	check_and_copy(GCGCEf_Half,     dst, src, st_shndx, NULL);
	check_and_copy(GCGCEf_Addr,     dst, src, st_value, NULL);
	check_and_copy(GCGCEf_Xword,    dst, src, st_size,  NULL);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Sym*)malloc(sizeof(GCGCEf_Sym));
	if (!dst) {
	    seterr(ERROR_MEM_SYM);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_sym(CGCEf_Data *dst, int ndx, GCGCEf_Sym *src) {
    unsigned cls;
    char *tmp;

    tmp = get_addr_and_class(dst, ndx, CGCEF_T_SYM, &cls);
    if (!tmp) {
	return 0;
    }
    if (cls == CGCEFCLASS64) {
	*(CGCEf64_Sym*)tmp = *src;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Sym *dst = (CGCEf32_Sym*)tmp;

	check_and_copy(CGCEf32_Word,    dst, src, st_name,  0);
	check_and_copy(CGCEf32_Addr,    dst, src, st_value, 0);
	check_and_copy(CGCEf32_Word,    dst, src, st_size,  0);
	check_and_copy(unsigned char, dst, src, st_info,  0);
	check_and_copy(unsigned char, dst, src, st_other, 0);
	check_and_copy(CGCEf32_Half,    dst, src, st_shndx, 0);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    return 1;
}

GCGCEf_Dyn*
gcgcef_getdyn(CGCEf_Data *src, int ndx, GCGCEf_Dyn *dst) {
    GCGCEf_Dyn buf;
    unsigned cls;
    char *tmp;

    if (!dst) {
	dst = &buf;
    }
    tmp = get_addr_and_class(src, ndx, CGCEF_T_DYN, &cls);
    if (!tmp) {
	return NULL;
    }
    if (cls == CGCEFCLASS64) {
	*dst = *(CGCEf64_Dyn*)tmp;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Dyn *src = (CGCEf32_Dyn*)tmp;

	check_and_copy(GCGCEf_Sxword, dst, src, d_tag,      NULL);
	check_and_copy(GCGCEf_Xword,  dst, src, d_un.d_val, NULL);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Dyn*)malloc(sizeof(GCGCEf_Dyn));
	if (!dst) {
	    seterr(ERROR_MEM_DYN);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_dyn(CGCEf_Data *dst, int ndx, GCGCEf_Dyn *src) {
    unsigned cls;
    char *tmp;

    tmp = get_addr_and_class(dst, ndx, CGCEF_T_DYN, &cls);
    if (!tmp) {
	return 0;
    }
    if (cls == CGCEFCLASS64) {
	*(CGCEf64_Dyn*)tmp = *src;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Dyn *dst = (CGCEf32_Dyn*)tmp;

	check_and_copy(CGCEf32_Sword, dst, src, d_tag,      0);
	check_and_copy(CGCEf32_Word,  dst, src, d_un.d_val, 0);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    return 1;
}

GCGCEf_Rela*
gcgcef_getrela(CGCEf_Data *src, int ndx, GCGCEf_Rela *dst) {
    GCGCEf_Rela buf;
    unsigned cls;
    char *tmp;

    if (!dst) {
	dst = &buf;
    }
    tmp = get_addr_and_class(src, ndx, CGCEF_T_RELA, &cls);
    if (!tmp) {
	return NULL;
    }
    if (cls == CGCEFCLASS64) {
	*dst = *(CGCEf64_Rela*)tmp;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Rela *src = (CGCEf32_Rela*)tmp;

	check_and_copy(GCGCEf_Addr,   dst, src, r_offset, NULL);
	dst->r_info = CGCEF64_R_INFO((CGCEf64_Xword)CGCEF32_R_SYM(src->r_info),
				   (CGCEf64_Xword)CGCEF32_R_TYPE(src->r_info));
	check_and_copy(GCGCEf_Sxword, dst, src, r_addend, NULL);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Rela*)malloc(sizeof(GCGCEf_Rela));
	if (!dst) {
	    seterr(ERROR_MEM_RELA);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_rela(CGCEf_Data *dst, int ndx, GCGCEf_Rela *src) {
    unsigned cls;
    char *tmp;

    tmp = get_addr_and_class(dst, ndx, CGCEF_T_RELA, &cls);
    if (!tmp) {
	return 0;
    }
    if (cls == CGCEFCLASS64) {
	*(CGCEf64_Rela*)tmp = *src;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Rela *dst = (CGCEf32_Rela*)tmp;

	check_and_copy(CGCEf32_Addr,  dst, src, r_offset, 0);
	if (CGCEF64_R_SYM(src->r_info) > 0xffffffUL
	 || CGCEF64_R_TYPE(src->r_info) > 0xffUL) {
	    seterr(ERROR_BADVALUE);
	    return 0;
	}
	dst->r_info = CGCEF32_R_INFO((CGCEf32_Word)CGCEF64_R_SYM(src->r_info),
				  (CGCEf32_Word)CGCEF64_R_TYPE(src->r_info));
	check_and_copy(CGCEf32_Sword, dst, src, r_addend, 0);
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    return 1;
}

GCGCEf_Rel*
gcgcef_getrel(CGCEf_Data *src, int ndx, GCGCEf_Rel *dst) {
    GCGCEf_Rel buf;
    unsigned cls;
    char *tmp;

    if (!dst) {
	dst = &buf;
    }
    tmp = get_addr_and_class(src, ndx, CGCEF_T_REL, &cls);
    if (!tmp) {
	return NULL;
    }
    if (cls == CGCEFCLASS64) {
	*dst = *(CGCEf64_Rel*)tmp;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Rel *src = (CGCEf32_Rel*)tmp;

	check_and_copy(GCGCEf_Addr, dst, src, r_offset, NULL);
	dst->r_info = CGCEF64_R_INFO((CGCEf64_Xword)CGCEF32_R_SYM(src->r_info),
				   (CGCEf64_Xword)CGCEF32_R_TYPE(src->r_info));
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (dst == &buf) {
	dst = (GCGCEf_Rel*)malloc(sizeof(GCGCEf_Rel));
	if (!dst) {
	    seterr(ERROR_MEM_REL);
	    return NULL;
	}
	*dst = buf;
    }
    return dst;
}

int
gcgcef_update_rel(CGCEf_Data *dst, int ndx, GCGCEf_Rel *src) {
    unsigned cls;
    char *tmp;

    tmp = get_addr_and_class(dst, ndx, CGCEF_T_REL, &cls);
    if (!tmp) {
	return 0;
    }
    if (cls == CGCEFCLASS64) {
	*(CGCEf64_Rel*)tmp = *src;
    }
    else if (cls == CGCEFCLASS32) {
	CGCEf32_Rel *dst = (CGCEf32_Rel*)tmp;

	check_and_copy(CGCEf32_Addr, dst, src, r_offset, 0);
	if (CGCEF64_R_SYM(src->r_info) > 0xffffffUL
	 || CGCEF64_R_TYPE(src->r_info) > 0xffUL) {
	    seterr(ERROR_BADVALUE);
	    return 0;
	}
	dst->r_info = CGCEF32_R_INFO((CGCEf32_Word)CGCEF64_R_SYM(src->r_info),
				   (CGCEf32_Word)CGCEF64_R_TYPE(src->r_info));
    }
    else {
	seterr(ERROR_UNIMPLEMENTED);
	return 0;
    }
    return 1;
}

#if 0

GCGCEf_Syminfo*
gcgcef_getsyminfo(CGCEf_Data *src, int ndx, GCGCEf_Syminfo *dst) {
    seterr(ERROR_UNIMPLEMENTED);
    return NULL;
}

int
gcgcef_update_syminfo(CGCEf_Data *dst, int ndx, GCGCEf_Syminfo *src) {
    seterr(ERROR_UNIMPLEMENTED);
    return 0;
}

GCGCEf_Move*
gcgcef_getmove(CGCEf_Data *src, int ndx, GCGCEf_Move *src) {
    seterr(ERROR_UNIMPLEMENTED);
    return NULL;
}

int
gcgcef_update_move(CGCEf_Data *dst, int ndx, GCGCEf_Move *src) {
    seterr(ERROR_UNIMPLEMENTED);
    return 0;
}

#endif

#endif /* __LIBCGCEF64 */
