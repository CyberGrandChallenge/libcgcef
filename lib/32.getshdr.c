/*
32.getshdr.c - implementation of the cgcef{32,64}_getshdr(3) functions.
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
static const char rcsid[] = "@(#) $Id: 32.getshdr.c,v 1.10 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

CGCEf32_Shdr*
cgcef32_getshdr(CGCEf_Scn *scn) {
    if (!scn) {
	return NULL;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef_assert(scn->s_cgcef);
    cgcef_assert(scn->s_cgcef->e_magic == CGCEF_MAGIC);
    if (scn->s_cgcef->e_class == CGCEFCLASS32) {
	return &scn->s_shdr32;
    }
    seterr(ERROR_CLASSMISMATCH);
    return NULL;
}

#if __LIBCGCEF64

CGCEf64_Shdr*
cgcef64_getshdr(CGCEf_Scn *scn) {
    if (!scn) {
	return NULL;
    }
    cgcef_assert(scn->s_magic == SCN_MAGIC);
    cgcef_assert(scn->s_cgcef);
    cgcef_assert(scn->s_cgcef->e_magic == CGCEF_MAGIC);
    if (scn->s_cgcef->e_class == CGCEFCLASS64) {
	return &scn->s_shdr64;
    }
    seterr(ERROR_CLASSMISMATCH);
    return NULL;
}

#endif /* __LIBCGCEF64 */
