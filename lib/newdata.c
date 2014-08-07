/*
newdata.c - implementation of the cgcef_newdata(3) function.
Copyright (C) 1995 - 2000 Michael Riepe

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
static const char rcsid[] = "@(#) $Id: newdata.c,v 1.10 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

CGCEf_Data*
cgcef_newdata(CGCEf_Scn *scn) {
    Scn_Data *sd;

    if (!scn) {
	return NULL;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    if (scn->s_index == SHN_UNDEF) {
	seterr(ERROR_NULLSCN);
    }
    else if (!(sd = (Scn_Data*)malloc(sizeof(*sd)))) {
	seterr(ERROR_MEM_SCNDATA);
    }
    else {
	*sd = _cgcef_data_init;
	sd->sd_scn = scn;
	sd->sd_data_flags = CGCEF_F_DIRTY;
	sd->sd_freeme = 1;
	sd->sd_data.d_version = _cgcef_version;
	if (scn->s_data_n) {
	    scn->s_data_n->sd_link = sd;
	}
	else {
	    scn->s_data_1 = sd;
	}
	scn->s_data_n = sd;
	return &sd->sd_data;
    }
    return NULL;
}
