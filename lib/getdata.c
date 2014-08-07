/*
getdata.c - implementation of the cgcef_getdata(3) function.
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
static const char rcsid[] = "@(#) $Id: getdata.c,v 1.13 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

static CGCEf_Data*
_cgcef_cook_scn(CGCEf *cgcef, CGCEf_Scn *scn, Scn_Data *sd) {
    CGCEf_Data dst;
    CGCEf_Data src;
    int flag = 0;
    size_t dlen;

    cgcef_assert(cgcef->e_data);

    /*
     * Prepare source
     */
    src = sd->sd_data;
    src.d_version = cgcef->e_version;
    if (cgcef->e_rawdata) {
	src.d_buf = cgcef->e_rawdata + scn->s_offset;
    }
    else {
	src.d_buf = cgcef->e_data + scn->s_offset;
    }

    /*
     * Prepare destination (needs prepared source!)
     */
    dst = sd->sd_data;
    if (cgcef->e_class == CGCEFCLASS32) {
	dlen = _cgcef32_xltsize(&src, dst.d_version, cgcef->e_encoding, 0);
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	dlen = _cgcef64_xltsize(&src, dst.d_version, cgcef->e_encoding, 0);
    }
#endif /* __LIBCGCEF64 */
    else {
	cgcef_assert(valid_class(cgcef->e_class));
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
    }
    if (dlen == (size_t)-1) {
	return NULL;
    }
    dst.d_size = dlen;
    if (cgcef->e_rawdata != cgcef->e_data && dst.d_size <= src.d_size) {
	dst.d_buf = cgcef->e_data + scn->s_offset;
    }
    else if (!(dst.d_buf = malloc(dst.d_size))) {
	seterr(ERROR_MEM_SCNDATA);
	return NULL;
    }
    else {
	flag = 1;
    }

    /*
     * Translate data
     */
    if (_cgcef_xlatetom(cgcef, &dst, &src)) {
	sd->sd_memdata = (char*)dst.d_buf;
	sd->sd_data = dst;
	if (!(sd->sd_free_data = flag)) {
	    cgcef->e_cooked = 1;
	}
	return &sd->sd_data;
    }

    if (flag) {
	free(dst.d_buf);
    }
    return NULL;
}

CGCEf_Data*
cgcef_getdata(CGCEf_Scn *scn, CGCEf_Data *data) {
    Scn_Data *sd;
    CGCEf *cgcef;

    if (!scn) {
	return NULL;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    if (scn->s_index == SHN_UNDEF) {
	seterr(ERROR_NULLSCN);
    }
    else if (data) {
	for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    if (data == &sd->sd_data) {
		/*
		 * sd_link allocated by cgcef_newdata().
		 */
		return &sd->sd_link->sd_data;
	    }
	}
	seterr(ERROR_SCNDATAMISMATCH);
    }
    else if ((sd = scn->s_data_1)) {
	cgcef_assert(sd->sd_magic == DATA_MAGIC);
	cgcef_assert(sd->sd_scn == scn);
	cgcef = scn->s_cgcef;
	cgcef_assert(cgcef);
	cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
	if (sd->sd_freeme) {
	    /* allocated by cgcef_newdata() */
	    return &sd->sd_data;
	}
	else if (scn->s_type == SHT_NULL) {
	    seterr(ERROR_NULLSCN);
	}
	else if (sd->sd_memdata) {
	    /* already cooked */
	    return &sd->sd_data;
	}
	else if (scn->s_offset < 0 || scn->s_offset > cgcef->e_size) {
	    seterr(ERROR_OUTSIDE);
	}
	else if (scn->s_type == SHT_NOBITS || !scn->s_size) {
	    /* no data to read */
	    return &sd->sd_data;
	}
	else if (scn->s_offset + scn->s_size > cgcef->e_size) {
	    seterr(ERROR_TRUNC_SCN);
	}
	else if (valid_class(cgcef->e_class)) {
	    return _cgcef_cook_scn(cgcef, scn, sd);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
    }
    return NULL;
}
