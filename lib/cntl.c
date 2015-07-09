/*
cntl.c - implementation of the cgcef_cntl(3) function.
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

#include <private.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: cntl.c,v 1.7 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

int
cgcef_cntl(CGCEf *cgcef, CGCEf_Cmd cmd) {
    CGCEf_Scn *scn;
    CGCEf *child;

    if (!cgcef) {
	return -1;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cmd == CGCEF_C_FDREAD) {
	if (!cgcef->e_readable) {
	    seterr(ERROR_WRONLY);
	    return -1;
	}
    }
    else if (cmd != CGCEF_C_FDDONE) {
	seterr(ERROR_INVALID_CMD);
	return -1;
    }
    if (cgcef->e_disabled) {
	return 0;
    }
    if (cgcef->e_kind == CGCEF_K_AR) {
	for (child = cgcef->e_members; child; child = child->e_link) {
	    cgcef_assert(cgcef == child->e_parent);
	    if (cgcef_cntl(child, cmd)) {
		return -1;
	    }
	}
    }
    else if (cgcef->e_kind == CGCEF_K_CGCEF && cmd == CGCEF_C_FDREAD) {
	if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	    return -1;
	}
	for (scn = cgcef->e_scn_1; scn; scn = scn->s_link) {
	    if (scn->s_index == SHN_UNDEF || scn->s_type == SHT_NULL) {
		continue;
	    }
	    else if (!cgcef_getdata(scn, NULL)) {
		return -1;
	    }
	}
    }
    cgcef->e_disabled = 1;
    return 0;
}
