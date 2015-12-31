/*
flag.c - implementation of the cgcef_flag*(3) functions.
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
static const char rcsid[] = "@(#) $Id: flag.c,v 1.7 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

static unsigned
_cgcef_flag(unsigned *f, CGCEf_Cmd cmd, unsigned flags) {
    if (cmd == CGCEF_C_SET) {
	return *f |= flags;
    }
    if (cmd == CGCEF_C_CLR) {
	return *f &= ~flags;
    }
    seterr(ERROR_INVALID_CMD);
    return 0;
}

unsigned
cgcef_flagdata(CGCEf_Data *data, CGCEf_Cmd cmd, unsigned flags) {
    Scn_Data *sd = (Scn_Data*)data;

    if (!sd) {
	return 0;
    }
    cgcef_assert(sd->sd_magic == DATA_MAGIC);
    return _cgcef_flag(&sd->sd_data_flags, cmd, flags);
}

unsigned
cgcef_flagehdr(CGCEf *cgcef, CGCEf_Cmd cmd, unsigned flags) {
    if (!cgcef) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    return _cgcef_flag(&cgcef->e_ehdr_flags, cmd, flags);
}

unsigned
cgcef_flagcgcef(CGCEf *cgcef, CGCEf_Cmd cmd, unsigned flags) {
    if (!cgcef) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    return _cgcef_flag(&cgcef->e_cgcef_flags, cmd, flags);
}

unsigned
cgcef_flagphdr(CGCEf *cgcef, CGCEf_Cmd cmd, unsigned flags) {
    if (!cgcef) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    return _cgcef_flag(&cgcef->e_phdr_flags, cmd, flags);
}

unsigned
cgcef_flagscn(CGCEf_Scn *scn, CGCEf_Cmd cmd, unsigned flags) {
    if (!scn) {
	return 0;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    return _cgcef_flag(&scn->s_scn_flags, cmd, flags);
}

unsigned
cgcef_flagshdr(CGCEf_Scn *scn, CGCEf_Cmd cmd, unsigned flags) {
    if (!scn) {
	return 0;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    return _cgcef_flag(&scn->s_shdr_flags, cmd, flags);
}
