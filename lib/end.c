/*
 * end.c - implementation of the cgcef_end(3) function.
 * Copyright (C) 1995 - 2004 Michael Riepe
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <private.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: end.c,v 1.12 2008/05/23 08:15:34 michael Exp $";
#endif /* lint */

#if HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

static void
_cgcef_free(void *ptr) {
    if (ptr) {
	free(ptr);
    }
}

static void
_cgcef_free_scns(CGCEf *cgcef, CGCEf_Scn *scn) {
    Scn_Data *sd, *tmp;
    CGCEf_Scn *freescn;

    for (freescn = NULL; scn; scn = scn->s_link) {
	cgcef_assert(scn->s_magic == SCN_MAGIC);
	cgcef_assert(scn->s_cgcef == cgcef);
	for (sd = scn->s_data_1; sd; sd = tmp) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    tmp = sd->sd_link;
	    if (sd->sd_free_data) {
		_cgcef_free(sd->sd_memdata);
	    }
	    if (sd->sd_freeme) {
		free(sd);
	    }
	}
	if ((sd = scn->s_rawdata)) {
	    cgcef_assert(sd->sd_magic == DATA_MAGIC);
	    cgcef_assert(sd->sd_scn == scn);
	    if (sd->sd_free_data) {
		_cgcef_free(sd->sd_memdata);
	    }
	    if (sd->sd_freeme) {
		free(sd);
	    }
	}
	if (scn->s_freeme) {
	    _cgcef_free(freescn);
	    freescn = scn;
	}
    }
    _cgcef_free(freescn);
}

int
cgcef_end(CGCEf *cgcef) {
    CGCEf **siblings;

    if (!cgcef) {
	return 0;
    }
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    if (--cgcef->e_count) {
	return cgcef->e_count;
    }
    if (cgcef->e_parent) {
	cgcef_assert(cgcef->e_parent->e_magic == CGCEF_MAGIC);
	cgcef_assert(cgcef->e_parent->e_kind == CGCEF_K_AR);
	siblings = &cgcef->e_parent->e_members;
	while (*siblings) {
	    if (*siblings == cgcef) {
		*siblings = cgcef->e_link;
		break;
	    }
	    siblings = &(*siblings)->e_link;
	}
	cgcef_end(cgcef->e_parent);
	_cgcef_free(cgcef->e_arhdr);
    }
#if HAVE_MMAP
    else if (cgcef->e_unmap_data) {
	munmap(cgcef->e_data, cgcef->e_size);
    }
#endif /* HAVE_MMAP */
    else if (!cgcef->e_memory) {
	_cgcef_free(cgcef->e_data);
    }
    _cgcef_free_scns(cgcef, cgcef->e_scn_1);
    if (cgcef->e_rawdata != cgcef->e_data) {
	_cgcef_free(cgcef->e_rawdata);
    }
    if (cgcef->e_free_syms) {
	_cgcef_free(cgcef->e_symtab);
    }
    _cgcef_free(cgcef->e_ehdr);
    _cgcef_free(cgcef->e_phdr);
    free(cgcef);
    return 0;
}
