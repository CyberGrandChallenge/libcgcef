/*
32.getphdr.c - implementation of the cgcef{32,64}_getphdr(3) functions.
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
static const char rcsid[] = "@(#) $Id: 32.getphdr.c,v 1.11 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

char*
_cgcef_getphdr(CGCEf *cgcef, unsigned cls) {
    if (!cgcef) {
	return NULL;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	seterr(ERROR_NOTCGCEF);
    }
    else if (cgcef->e_class != cls) {
	seterr(ERROR_CLASSMISMATCH);
    }
    else if (cgcef->e_ehdr || _cgcef_cook(cgcef)) {
	return cgcef->e_phdr;
    }
    return NULL;
}

CGCEf32_Phdr*
cgcef32_getphdr(CGCEf *cgcef) {
    return (CGCEf32_Phdr*)_cgcef_getphdr(cgcef, CGCEFCLASS32);
}

#if __LIBCGCEF64

CGCEf64_Phdr*
cgcef64_getphdr(CGCEf *cgcef) {
    return (CGCEf64_Phdr*)_cgcef_getphdr(cgcef, CGCEFCLASS64);
}

#endif /* __LIBCGCEF64 */
