/*
 * nlist.c - implementation of the nlist(3) function.
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
#include <nlist.h>

#ifndef lint
static const char rcsid[] = "@(#) $Id: nlist.c,v 1.15 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

#if !defined(_WIN32)
#if HAVE_FCNTL_H
#include <fcntl.h>
#else
extern int open();
#endif /* HAVE_FCNTL_H */
#endif /* defined(_WIN32) */

#ifndef O_RDONLY
#define O_RDONLY	0
#endif /* O_RDONLY */

#ifndef O_BINARY
#define O_BINARY	0
#endif /* O_BINARY */

#define FILE_OPEN_MODE	(O_RDONLY | O_BINARY)

#define PRIME	217

struct hash {
    const char*		name;
    unsigned long	hash;
    unsigned		next;
};

static const char*
symbol_name(CGCEf *cgcef, const void *syms, const char *names, size_t nlimit, size_t index) {
    size_t off;

    if (cgcef->e_class == CGCEFCLASS32) {
	off = ((CGCEf32_Sym*)syms)[index].st_name;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	off = ((CGCEf64_Sym*)syms)[index].st_name;
    }
#endif /* __LIBCGCEF64 */
    else {
	return NULL;
    }
    if (off >= 0 && off < nlimit) {
	return &names[off];
    }
    return NULL;
}

static void
copy_symbol(CGCEf *cgcef, struct nlist *np, const void *syms, size_t index) {
    if (cgcef->e_class == CGCEFCLASS32) {
	np->n_value = ((CGCEf32_Sym*)syms)[index].st_value;
	np->n_scnum = ((CGCEf32_Sym*)syms)[index].st_shndx;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	np->n_value = ((CGCEf64_Sym*)syms)[index].st_value;
	np->n_scnum = ((CGCEf64_Sym*)syms)[index].st_shndx;
    }
#endif /* __LIBCGCEF64 */
    /*
     * this needs more work
     */
    np->n_type = 0;
    np->n_sclass = 0;
    np->n_numaux = 0;
}

static int
_cgcef_nlist(CGCEf *cgcef, struct nlist *nl) {
    unsigned first[PRIME];
    CGCEf_Scn *symtab = NULL;
    CGCEf_Scn *strtab = NULL;
    CGCEf_Data *symdata;
    CGCEf_Data *strdata;
    size_t symsize;
    size_t nsymbols;
    const char *name;
    struct hash *table;
    unsigned long hash;
    unsigned i;
    struct nlist *np;

    /*
     * Get and translate CGCEF header, section table and so on.
     * Must be class independent, so don't use cgcef32_get*().
     */
    if (cgcef->e_kind != CGCEF_K_CGCEF) {
	return -1;
    }
    if (!cgcef->e_ehdr && !_cgcef_cook(cgcef)) {
	return -1;
    }

    /*
     * Find symbol table. If there is none, try dynamic symbols.
     */
    for (symtab = cgcef->e_scn_1; symtab; symtab = symtab->s_link) {
	if (symtab->s_type == SHT_SYMTAB) {
	    break;
	}
	if (symtab->s_type == SHT_DYNSYM) {
	    strtab = symtab;
	}
    }
    if (!symtab && !(symtab = strtab)) {
	return -1;
    }

    /*
     * Get associated string table.
     */
    i = 0;
    if (cgcef->e_class == CGCEFCLASS32) {
	i = symtab->s_shdr32.sh_link;
    }
#if __LIBCGCEF64
    else if (cgcef->e_class == CGCEFCLASS64) {
	i = symtab->s_shdr64.sh_link;
    }
#endif /* __LIBCGCEF64 */
    if (i == 0) {
	return -1;
    }
    for (strtab = cgcef->e_scn_1; strtab; strtab = strtab->s_link) {
	if (strtab->s_index == i) {
	    break;
	}
    }
    if (!strtab || strtab->s_type != SHT_STRTAB) {
	return -1;
    }

    /*
     * Get and translate section data.
     */
    symdata = cgcef_getdata(symtab, NULL);
    strdata = cgcef_getdata(strtab, NULL);
    if (!symdata || !strdata) {
	return -1;
    }
    symsize = _msize(cgcef->e_class, _cgcef_version, CGCEF_T_SYM);
    cgcef_assert(symsize);
    nsymbols = symdata->d_size / symsize;
    if (!symdata->d_buf || !strdata->d_buf || !nsymbols || !strdata->d_size) {
	return -1;
    }

    /*
     * Build a simple hash table.
     */
    if (!(table = (struct hash*)malloc(nsymbols * sizeof(*table)))) {
	return -1;
    }
    for (i = 0; i < PRIME; i++) {
	first[i] = 0;
    }
    for (i = 0; i < nsymbols; i++) {
	table[i].name = NULL;
	table[i].hash = 0;
	table[i].next = 0;
    }
    for (i = 1; i < nsymbols; i++) {
	name = symbol_name(cgcef, symdata->d_buf, strdata->d_buf,
			   strdata->d_size, i);
	if (name == NULL) {
	    free(table);
	    return -1;
	}
	if (*name != '\0') {
	    table[i].name = name;
	    table[i].hash = cgcef_hash((unsigned char*)name);
	    hash = table[i].hash % PRIME;
	    table[i].next = first[hash];
	    first[hash] = i;
	}
    }

    /*
     * Lookup symbols, one by one.
     */
    for (np = nl; (name = np->n_name) && *name; np++) {
	hash = cgcef_hash((unsigned char*)name);
	for (i = first[hash % PRIME]; i; i = table[i].next) {
	    if (table[i].hash == hash && !strcmp(table[i].name, name)) {
		break;
	    }
	}
	if (i) {
	    copy_symbol(cgcef, np, symdata->d_buf, i);
	}
	else {
	    np->n_value = 0;
	    np->n_scnum = 0;
	    np->n_type = 0;
	    np->n_sclass = 0;
	    np->n_numaux = 0;
	}
    }
    free(table);
    return 0;
}

int
nlist(const char *filename, struct nlist *nl) {
    int result = -1;
    unsigned oldver;
    CGCEf *cgcef;
    int fd;

    if ((oldver = cgcef_version(EV_CURRENT)) != EV_NONE) {
	if ((fd = open(filename, FILE_OPEN_MODE)) != -1) {
	    if ((cgcef = cgcef_begin(fd, CGCEF_C_READ, NULL))) {
		result = _cgcef_nlist(cgcef, nl);
		cgcef_end(cgcef);
	    }
	    close(fd);
	}
	cgcef_version(oldver);
    }
    if (result) {
	while (nl->n_name && *nl->n_name) {
	    nl->n_value = 0;
	    nl++;
	}
    }
    return result;
}
