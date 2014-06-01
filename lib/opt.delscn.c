/*
opt.delscn.c - implementation of the cgcef_delscn(3) function.
Copyright (C) 1995 - 2001 Michael Riepe

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

#ifndef lint
static const char rcsid[] = "@(#) $Id: opt.delscn.c,v 1.12 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

static size_t
_newindex(size_t old, size_t index) {
    return old == index ? SHN_UNDEF : (old > index ? old - 1 : old);
}

static void
_cgcef32_update_shdr(CGCEf *cgcef, size_t index) {
    CGCEf32_Shdr *shdr;
    CGCEf_Scn *scn;

    ((CGCEf32_Ehdr*)cgcef->e_ehdr)->e_shnum = cgcef->e_scn_n->s_index + 1;
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	shdr = &scn->s_shdr32;
	switch (shdr->sh_type) {
	    case SHT_REL:
	    case SHT_RELA:
		shdr->sh_info = _newindex(shdr->sh_info, index);
		/* fall through */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
	    case SHT_HASH:
	    case SHT_SYMTAB:
#if __LIBCGCEF_SYMBOL_VERSIONS
#if __LIBCGCEF_SUN_SYMBOL_VERSIONS
	    case SHT_SUNW_verdef:
	    case SHT_SUNW_verneed:
	    case SHT_SUNW_versym:
#else /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
	    case SHT_GNU_verdef:
	    case SHT_GNU_verneed:
	    case SHT_GNU_versym:
#endif /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
#endif /* __LIBCGCEF_SYMBOL_VERSIONS */
		shdr->sh_link = _newindex(shdr->sh_link, index);
		/* fall through */
	    default:
		break;
	}
    }
}

#if __LIBCGCEF64

static void
_cgcef64_update_shdr(CGCEf *cgcef, size_t index) {
    CGCEf64_Shdr *shdr;
    CGCEf_Scn *scn;

    ((CGCEf64_Ehdr*)cgcef->e_ehdr)->e_shnum = cgcef->e_scn_n->s_index + 1;
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	shdr = &scn->s_shdr64;
	switch (shdr->sh_type) {
	    case SHT_REL:
	    case SHT_RELA:
		shdr->sh_info = _newindex(shdr->sh_info, index);
		/* fall through */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
	    case SHT_HASH:
	    case SHT_SYMTAB:
#if __LIBCGCEF_SYMBOL_VERSIONS
#if __LIBCGCEF_SUN_SYMBOL_VERSIONS
	    case SHT_SUNW_verdef:
	    case SHT_SUNW_verneed:
	    case SHT_SUNW_versym:
#else /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
	    case SHT_GNU_verdef:
	    case SHT_GNU_verneed:
	    case SHT_GNU_versym:
#endif /* __LIBCGCEF_SUN_SYMBOL_VERSIONS */
#endif /* __LIBCGCEF_SYMBOL_VERSIONS */
		shdr->sh_link = _newindex(shdr->sh_link, index);
		/* fall through */
	    default:
		break;
	}
    }
}

#endif /* __LIBCGCEF64 */

size_t
cgcef_delscn(CGCEf *cgcef, CGCEf_Scn *scn) {
    CGCEf_Scn *pscn;
    Scn_Data *sd;
    Scn_Data *tmp;
    size_t index;

    if (!cgcef || !scn) {
	return SHN_UNDEF;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef_assert(cgcef->e_ehdr);
    if (scn->s_cgcef != cgcef) {
	seterr(ERROR_CGCEFSCNMISMATCH);
	return SHN_UNDEF;
    }
    cgcef_assert(cgcef->e_scn_1);
    if (scn == cgcef->e_scn_1) {
	seterr(ERROR_NULLSCN);
	return SHN_UNDEF;
    }

    /*
     * Find previous section.
     */
    for (pscn = cgcef->e_scn_1; pscn->s_link; pscn = pscn->s_link) {
	if (pscn->s_link == scn) {
	    break;
	}
    }
    if (pscn->s_link != scn) {
	seterr(ERROR_CGCEFSCNMISMATCH);
	return SHN_UNDEF;
    }
    /*
     * Unlink section.
     */
    if (cgcef->e_scn_n == scn) {
	cgcef->e_scn_n = pscn;
    }
    pscn->s_link = scn->s_link;
    index = scn->s_index;
    /*
     * Free section descriptor and data.
     */
    for (sd = scn->s_data_1; sd; sd = tmp) {
	cgcef_assert(sd->sd_magic == DATA_MAGIC);
	cgcef_assert(sd->sd_scn == scn);
	tmp = sd->sd_link;
	if (sd->sd_free_data && sd->sd_memdata) {
	    free(sd->sd_memdata);
	}
	if (sd->sd_freeme) {
	    free(sd);
	}
    }
    if ((sd = scn->s_rawdata)) {
	cgcef_assert(sd->sd_magic == DATA_MAGIC);
	cgcef_assert(sd->sd_scn == scn);
	if (sd->sd_free_data && sd->sd_memdata) {
	    free(sd->sd_memdata);
	}
	if (sd->sd_freeme) {
	    free(sd);
	}
    }
    if (scn->s_freeme) {
	cgcef_assert(scn->s_index > 0);
	free(scn);
    }
    /*
     * Adjust section indices.
     */
    for (scn = pscn->s_link; scn; scn = scn->s_link) {
	cgcef_assert(scn->s_index > index);
	scn->s_index--;
    }
    /*
     * Adjust CGCEF header and well-known section headers.
     */
    if (cgcef->e_class == CGCEFCLASS32) {
	_cgcef32_update_shdr(cgcef, index);
	return index;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	_cgcef64_update_shdr(cgcef, index);
	return index;
    }
#endif /* __LIBCGCEF64 */
    else if (valid_class(cgcef->e_class)) {
	seterr(ERROR_UNIMPLEMENTED);
    }
    else {
	seterr(ERROR_UNKNOWN_CLASS);
    }
    return SHN_UNDEF;
}
