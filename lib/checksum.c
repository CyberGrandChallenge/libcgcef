/*
checksum.c - implementation of the cgcef{32,64}_checksum(3) functions.
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
static const char rcsid[] = "@(#) $Id: checksum.c,v 1.7 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

/*
 * Compatibility note:
 *
 * The algorithm used in {cgcef32,cgcef64,gcgcef}_checksum() does not seem to
 * be documented.  I hope I got it right.  My implementation does the
 * following:
 *
 *   - skip sections that do not have the SHF_ALLOC flag set
 *   - skip sections of type SHT_NULL, SHT_NOBITS, SHT_DYNSYM and
 *     SHT_DYNAMIC
 *   - add all data bytes from the remaining sections, modulo 2**32
 *   - add upper and lower half of the result
 *   - subtract 0xffff if the result is > 0xffff
 *   - if any error occurs, return 0L
 */

static int
skip_section(CGCEf_Scn *scn, unsigned cls) {
    if (cls == CGCEFCLASS32) {
	CGCEf32_Shdr *shdr = &scn->s_shdr32;

	if (!(shdr->sh_flags & SHF_ALLOC)) {
	    return 1;
	}
	switch (shdr->sh_type) {
	    case SHT_NULL:
	    case SHT_NOBITS:
	    /* Solaris seems to ignore these, too */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
		return 1;
	}
    }
#if __LIBCGCEF64
    else if (cls == CGCEFCLASS64) {
	CGCEf64_Shdr *shdr = &scn->s_shdr64;

	if (!(shdr->sh_flags & SHF_ALLOC)) {
	    return 1;
	}
	switch (shdr->sh_type) {
	    case SHT_NULL:
	    case SHT_NOBITS:
	    /* Solaris seems to ignore these, too */
	    case SHT_DYNSYM:
	    case SHT_DYNAMIC:
		return 1;
	}
    }
#endif /* __LIBCGCEF64 */
    else {
	seterr(ERROR_UNIMPLEMENTED);
    }
    return 0;
}

static long
add_bytes(unsigned char *ptr, size_t len) {
    long csum = 0;

    while (len--) {
	csum += *ptr++;
    }
    return csum;
}

static long
_cgcef_csum(CGCEf *cgcef) {
    long csum = 0;
    CGCEf_Data *data;
    CGCEf_Scn *scn;

    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	/* propagate errors from _cgcef_cook */
	return 0L;
    }
    seterr(0);
    for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	if (scn->s_index == SHN_UNDEF || skip_section(scn, cgcef->e_class)) {
	    continue;
	}
	data = NULL;
	while ((data = cgcef_getdata(scn, data))) {
	    if (data->d_size) {
		if (data->d_buf == NULL) {
		    seterr(ERROR_NULLBUF);
		    return 0L;
		}
		csum += add_bytes(data->d_buf, data->d_size);
	    }
	}
    }
    if (_cgcef_errno) {
	return 0L;
    }
    csum = (csum & 0xffff) + ((csum >> 16) & 0xffff);
    if (csum > 0xffff) {
	csum -= 0xffff;
    }
    return csum;
}

long
cgcef32_checksum(CGCEf *cgcef) {
    if (cgcef) {
	if (cgcef->e_kind != CGCEF_K_CGCEF) {
	    seterr(ERROR_NOTCGCEF);
	}
	else if (cgcef->e_class != CGCEFCLASS32) {
	    seterr(ERROR_CLASSMISMATCH);
	}
	else {
	    return _cgcef_csum(cgcef);
	}
    }
    return 0L;
}

#if __LIBCGCEF64

long
cgcef64_checksum(CGCEf *cgcef) {
    if (cgcef) {
	if (cgcef->e_kind != CGCEF_K_CGCEF) {
	    seterr(ERROR_NOTCGCEF);
	}
	else if (cgcef->e_class != CGCEFCLASS64) {
	    seterr(ERROR_CLASSMISMATCH);
	}
	else {
	    return _cgcef_csum(cgcef);
	}
    }
    return 0L;
}

long
gcgcef_checksum(CGCEf *cgcef) {
    if (cgcef) {
	if (cgcef->e_kind != CGCEF_K_CGCEF) {
	    seterr(ERROR_NOTCGCEF);
	}
	else if (!valid_class(cgcef->e_class)) {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	else {
	    return _cgcef_csum(cgcef);
	}
    }
    return 0L;
}

#endif /* __LIBCGCEF64 */
