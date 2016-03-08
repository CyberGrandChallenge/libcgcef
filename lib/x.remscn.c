/*
x.remscn.c - implementation of the cgcefx_remscn(3) function.
Copyright (C) 1995 - 2001, 2003 Michael Riepe

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
static const char rcsid[] = "@(#) $Id: x.remscn.c,v 1.15 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

size_t
cgcefx_remscn(CGCEf *cgcef, CGCEf_Scn *scn) {
    CGCEf_Scn *pscn;
    Scn_Data *sd;
    Scn_Data *tmp;
    size_t index;

    if (!cgcef || !scn) {
	return SHN_UNDEF;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
	return SHN_UNDEF;
    }
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
     * Adjust section count in CGCEF header
     */
    if (_cgcef_update_shnum(cgcef, cgcef->e_scn_n->s_index + 1)) {
	return SHN_UNDEF;
    }
    return index;
}
