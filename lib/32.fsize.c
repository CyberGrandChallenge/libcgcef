/*
32.fsize.c - implementation of the cgcef{32,64}_fsize(3) functions.
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
#include <ext_types.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: 32.fsize.c,v 1.13 2008/05/23 08:15:33 michael Exp $";
#endif /* lint */

const size_t
_cgcef_fmsize[2][EV_CURRENT - EV_NONE][CGCEF_T_NUM][2] = {
    /* CGCEFCLASS32 */
    {
	/* version 1 */
	{
	    { sizeof(unsigned char),  sizeof(unsigned char)      },
	    { sizeof(CGCEf32_Addr),     sizeof(__ext_CGCEf32_Addr)   },
	    { sizeof(CGCEf32_Dyn),      sizeof(__ext_CGCEf32_Dyn)    },
	    { sizeof(CGCEf32_Ehdr),     sizeof(__ext_CGCEf32_Ehdr)   },
	    { sizeof(CGCEf32_Half),     sizeof(__ext_CGCEf32_Half)   },
	    { sizeof(CGCEf32_Off),      sizeof(__ext_CGCEf32_Off)    },
	    { sizeof(CGCEf32_Phdr),     sizeof(__ext_CGCEf32_Phdr)   },
	    { sizeof(CGCEf32_Rela),     sizeof(__ext_CGCEf32_Rela)   },
	    { sizeof(CGCEf32_Rel),      sizeof(__ext_CGCEf32_Rel)    },
	    { sizeof(CGCEf32_Shdr),     sizeof(__ext_CGCEf32_Shdr)   },
	    { sizeof(CGCEf32_Sword),    sizeof(__ext_CGCEf32_Sword)  },
	    { sizeof(CGCEf32_Sym),      sizeof(__ext_CGCEf32_Sym)    },
	    { sizeof(CGCEf32_Word),     sizeof(__ext_CGCEf32_Word)   },
	    { 0, 0 },	/* there is no CGCEf32_Sxword */
	    { 0, 0 },	/* there is no CGCEf32_Xword */
	    /* XXX: check Solaris values */
	    { 0, 0 },	/* CGCEf32_Verdef/Verdaux size varies */
	    { 0, 0 },	/* CGCEf32_Verneed/Vernaux size varies */
	},
    },
#if __LIBCGCEF64
    /* CGCEFCLASS64 */
    {
	/* version 1 */
	{
	    { sizeof(unsigned char),  sizeof(unsigned char)      },
	    { sizeof(CGCEf64_Addr),     sizeof(__ext_CGCEf64_Addr)   },
	    { sizeof(CGCEf64_Dyn),      sizeof(__ext_CGCEf64_Dyn)    },
	    { sizeof(CGCEf64_Ehdr),     sizeof(__ext_CGCEf64_Ehdr)   },
	    { sizeof(CGCEf64_Half),     sizeof(__ext_CGCEf64_Half)   },
	    { sizeof(CGCEf64_Off),      sizeof(__ext_CGCEf64_Off)    },
	    { sizeof(CGCEf64_Phdr),     sizeof(__ext_CGCEf64_Phdr)   },
	    { sizeof(CGCEf64_Rela),     sizeof(__ext_CGCEf64_Rela)   },
	    { sizeof(CGCEf64_Rel),      sizeof(__ext_CGCEf64_Rel)    },
	    { sizeof(CGCEf64_Shdr),     sizeof(__ext_CGCEf64_Shdr)   },
	    { sizeof(CGCEf64_Sword),    sizeof(__ext_CGCEf64_Sword)  },
	    { sizeof(CGCEf64_Sym),      sizeof(__ext_CGCEf64_Sym)    },
	    { sizeof(CGCEf64_Word),     sizeof(__ext_CGCEf64_Word)   },
	    { sizeof(CGCEf64_Sxword),   sizeof(__ext_CGCEf64_Sxword) },
	    { sizeof(CGCEf64_Xword),    sizeof(__ext_CGCEf64_Xword)  },
	    /* XXX: check Solaris values */
	    { 0, 0 },	/* CGCEf64_Verdef/Verdaux size varies */
	    { 0, 0 },	/* CGCEf64_Verneed/Vernaux size varies */
	},
    },
#endif /* __LIBCGCEF64 */
};

static size_t
_cgcef_fsize(unsigned cls, CGCEf_Type type, unsigned ver) {
    size_t n = 0;

    if (!valid_version(ver)) {
	seterr(ERROR_UNKNOWN_VERSION);
    }
    else if (!valid_type(type)) {
	seterr(ERROR_UNKNOWN_TYPE);
    }
    else if (!(n = _fsize(cls, ver, type))) {
	seterr(ERROR_UNKNOWN_TYPE);
    }
    return n;
}

size_t
cgcef32_fsize(CGCEf_Type type, size_t count, unsigned ver) {
    return count * _cgcef_fsize(CGCEFCLASS32, type, ver);
}

#if __LIBCGCEF64

size_t
cgcef64_fsize(CGCEf_Type type, size_t count, unsigned ver) {
    return count * _cgcef_fsize(CGCEFCLASS64, type, ver);
}

size_t
gcgcef_fsize(CGCEf *cgcef, CGCEf_Type type, size_t count, unsigned ver) {
    if (cgcef) {
	if (cgcef->e_kind != CGCEF_K_CGCEF) {
	    seterr(ERROR_NOTCGCEF);
	}
	else if (valid_class(cgcef->e_class)) {
	    return count * _cgcef_fsize(cgcef->e_class, type, ver);
	}
	else {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
    }
    return 0;
}

/*
 * Extension: report memory size
 */
size_t
gcgcef_msize(CGCEf *cgcef, CGCEf_Type type, size_t count, unsigned ver) {
    size_t n;

    if (cgcef) {
	if (cgcef->e_kind != CGCEF_K_CGCEF) {
	    seterr(ERROR_NOTCGCEF);
	}
	else if (!valid_class(cgcef->e_class)) {
	    seterr(ERROR_UNKNOWN_CLASS);
	}
	else if (!valid_version(ver)) {
	    seterr(ERROR_UNKNOWN_VERSION);
	}
	else if (!valid_type(type)) {
	    seterr(ERROR_UNKNOWN_TYPE);
	}
	else if (!(n = _msize(cgcef->e_class, ver, type))) {
	    seterr(ERROR_UNKNOWN_TYPE);
	}
	else {
	    return count * n;
	}
    }
    return 0;
}

#endif /* __LIBCGCEF64 */
