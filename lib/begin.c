/*
 * begin.c - implementation of the cgcef_begin(3) and cgcef_memory(3) functions.
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
static const char rcsid[] = "@(#) $Id: begin.c,v 1.22 2009/11/01 13:04:19 michael Exp $";
#endif /* lint */

static const CGCEf _cgcef_init = INIT_CGCEF;
static const char fmag[] = ARFMAG;

static unsigned long
getnum(const char *str, size_t len, int base, size_t *err) {
    unsigned long result = 0;

    while (len && *str == ' ') {
	str++; len--;
    }
    while (len && *str >= '0' && (*str - '0') < base) {
	result = base * result + *str++ - '0'; len--;
    }
    while (len && *str == ' ') {
	str++; len--;
    }
    if (len) {
	*err = len;
    }
    return result;
}

static void
_cgcef_init_ar(CGCEf *cgcef) {
    struct ar_hdr *hdr;
    size_t offset;
    size_t size;
    size_t err = 0;

    cgcef->e_kind = CGCEF_K_AR;
    cgcef->e_idlen = SARMAG;
    cgcef->e_off = SARMAG;

    /* process special members */
    offset = SARMAG;
    while (!cgcef->e_strtab && offset + sizeof(*hdr) <= cgcef->e_size) {
	hdr = (struct ar_hdr*)(cgcef->e_data + offset);
	if (memcmp(hdr->ar_fmag, fmag, sizeof(fmag) - 1)) {
	    break;
	}
	if (hdr->ar_name[0] != '/') {
	    break;
	}
	size = getnum(hdr->ar_size, sizeof(hdr->ar_size), 10, &err);
	if (err || !size) {
	    break;
	}
	offset += sizeof(*hdr);
	if (offset + size > cgcef->e_size) {
	    break;
	}
	if (hdr->ar_name[1] == '/' && hdr->ar_name[2] == ' ') {
	    cgcef->e_strtab = cgcef->e_data + offset;
	    cgcef->e_strlen = size;
	    break;
	}
	if (hdr->ar_name[1] != ' ') {
	    break;
	}
	/*
	 * Windows (.lib) archives provide two symbol tables
	 * The first one is the one we want.
	 */
	if (!cgcef->e_symtab) {
	    cgcef->e_symtab = cgcef->e_data + offset;
	    cgcef->e_symlen = size;
	}
	offset += size + (size & 1);
    }
}

static CGCEf_Arhdr*
_cgcef_arhdr(CGCEf *arf) {
    struct ar_hdr *hdr;
    CGCEf_Arhdr *arhdr;
    size_t namelen;
    size_t tmp;
    char *name;
    size_t err = 0;

    if (arf->e_off == arf->e_size) {
	/* no error! */
	return NULL;
    }
    if (arf->e_off < 0 || arf->e_off > arf->e_size) {
	seterr(ERROR_OUTSIDE);
	return NULL;
    }
    if (arf->e_off + sizeof(*hdr) > arf->e_size) {
	seterr(ERROR_TRUNC_ARHDR);
	return NULL;
    }
    cgcef_assert(arf->e_data != NULL);
    hdr = (struct ar_hdr*)(arf->e_data + arf->e_off);
    if (memcmp(hdr->ar_fmag, fmag, sizeof(fmag) - 1)) {
	seterr(ERROR_ARFMAG);
	return NULL;
    }

    name = hdr->ar_name;
    for (namelen = sizeof(hdr->ar_name); namelen > 0; namelen--) {
	if (name[namelen - 1] != ' ') {
	    break;
	}
    }
    if (name[0] == '/') {
	if (name[1] >= '0' && name[1] <= '9') {
	    if (!arf->e_strtab) {
		seterr(ERROR_ARSTRTAB);
		return NULL;
	    }
	    tmp = getnum(&name[1], namelen - 1, 10, &err);
	    if (err) {
		seterr(ERROR_ARSPECIAL);
		return NULL;
	    }
	    if (tmp < 0 || tmp >= arf->e_strlen) {
		seterr(ERROR_ARSTRTAB);
		return NULL;
	    }
	    for (namelen = tmp; namelen < arf->e_strlen; namelen++) {
		if (arf->e_strtab[namelen] == '/') {
		    break;
		}
	    }
	    if (namelen == arf->e_strlen) {
		seterr(ERROR_ARSTRTAB);
		return NULL;
	    }
	    name = arf->e_strtab + tmp;
	    namelen -= tmp;
	}
	else if (namelen != 1 && !(namelen == 2 && name[1] == '/')) {
	    seterr(ERROR_ARSPECIAL);
	    return NULL;
	}
    }
    else if (namelen > 0 && name[namelen - 1] == '/') {
	namelen--;
    }
    /* XXX some broken software omits the trailing slash
    else {
	namelen = 0;
    }
    */

    if (!(arhdr = (CGCEf_Arhdr*)malloc(sizeof(*arhdr) +
		     sizeof(hdr->ar_name) + namelen + 2))) {
	seterr(ERROR_MEM_ARHDR);
	return NULL;
    }

    arhdr->ar_name = NULL;
    arhdr->ar_rawname = (char*)(arhdr + 1);
    arhdr->ar_date = getnum(hdr->ar_date, sizeof(hdr->ar_date), 10, &err);
    arhdr->ar_uid = getnum(hdr->ar_uid, sizeof(hdr->ar_uid), 10, &err);
    arhdr->ar_gid = getnum(hdr->ar_gid, sizeof(hdr->ar_gid), 10, &err);
    arhdr->ar_mode = getnum(hdr->ar_mode, sizeof(hdr->ar_mode), 8, &err);
    arhdr->ar_size = getnum(hdr->ar_size, sizeof(hdr->ar_size), 10, &err);
    if (err) {
	free(arhdr);
	seterr(ERROR_ARHDR);
	return NULL;
    }
    if (arf->e_off + sizeof(struct ar_hdr) + arhdr->ar_size > arf->e_size) {
	free(arhdr);
	seterr(ERROR_TRUNC_MEMBER);
	return NULL;
    }

    memcpy(arhdr->ar_rawname, hdr->ar_name, sizeof(hdr->ar_name));
    arhdr->ar_rawname[sizeof(hdr->ar_name)] = '\0';

    if (namelen) {
	arhdr->ar_name = arhdr->ar_rawname + sizeof(hdr->ar_name) + 1;
	memcpy(arhdr->ar_name, name, namelen);
	arhdr->ar_name[namelen] = '\0';
    }

    return arhdr;
}

static void
_cgcef_check_type(CGCEf *cgcef, size_t size) {
    cgcef->e_idlen = size;
    if (size >= EI_NIDENT && !memcmp(cgcef->e_data, CGCEFMAG, SCGCEFMAG)) {
	cgcef->e_kind = CGCEF_K_CGCEF;
	cgcef->e_idlen = EI_NIDENT;
	cgcef->e_class = cgcef->e_data[EI_CLASS];
	cgcef->e_encoding = cgcef->e_data[EI_DATA];
	cgcef->e_version = cgcef->e_data[EI_VERSION];
    }
    else if (size >= SARMAG && !memcmp(cgcef->e_data, ARMAG, SARMAG)) {
	_cgcef_init_ar(cgcef);
    } else {
	cgcef->e_kind = CGCEF_K_CGCEF;
	cgcef->e_idlen = EI_NIDENT;
	cgcef->e_class = CGCEFCLASS32;
	cgcef->e_encoding = CGCEFDATA2LSB;
	cgcef->e_version = EV_CURRENT;
    }
}

CGCEf*
cgcef_begin(int fd, CGCEf_Cmd cmd, CGCEf *ref) {
    CGCEf_Arhdr *arhdr = NULL;
    size_t size = 0;
    off_t off;
    CGCEf *cgcef;

    cgcef_assert(_cgcef_init.e_magic == CGCEF_MAGIC);
    if (_cgcef_version == EV_NONE) {
	seterr(ERROR_VERSION_UNSET);
	return NULL;
    }
    else if (cmd == CGCEF_C_NULL) {
	return NULL;
    }
    else if (cmd == CGCEF_C_WRITE) {
	ref = NULL;
    }
    else if (cmd != CGCEF_C_READ && cmd != CGCEF_C_RDWR) {
	seterr(ERROR_INVALID_CMD);
	return NULL;
    }
    else if (ref) {
	cgcef_assert(ref->e_magic == CGCEF_MAGIC);
	if (!ref->e_readable || (cmd == CGCEF_C_RDWR && !ref->e_writable)) {
	    seterr(ERROR_CMDMISMATCH);
	    return NULL;
	}
	if (ref->e_kind != CGCEF_K_AR) {
	    ref->e_count++;
	    return ref;
	}
	if (cmd == CGCEF_C_RDWR) {
	    seterr(ERROR_MEMBERWRITE);
	    return NULL;
	}
	if (ref->e_memory) {
	    fd = ref->e_fd;
	}
	else if (fd != ref->e_fd) {
	    seterr(ERROR_FDMISMATCH);
	    return NULL;
	}
	if (!(arhdr = _cgcef_arhdr(ref))) {
	    return NULL;
	}
	size = arhdr->ar_size;
    }
    else if ((off = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1
	  || (off_t)(size = off) != off) {
	seterr(ERROR_IO_GETSIZE);
	return NULL;
    }

    if (!(cgcef = (CGCEf*)malloc(sizeof(CGCEf)))) {
	seterr(ERROR_MEM_CGCEF);
	return NULL;
    }
    *cgcef = _cgcef_init;
    cgcef->e_fd = fd;
    cgcef->e_parent = ref;
    cgcef->e_size = cgcef->e_dsize = size;

    if (cmd != CGCEF_C_READ) {
	cgcef->e_writable = 1;
    }
    if (cmd != CGCEF_C_WRITE) {
	cgcef->e_readable = 1;
    }
    else {
	return cgcef;
    }

    if (ref) {
	size_t offset = ref->e_off + sizeof(struct ar_hdr);
	CGCEf *xcgcef;

	cgcef_assert(arhdr);
	cgcef->e_arhdr = arhdr;
	cgcef->e_base = ref->e_base + offset;
	/*
	 * Share the archive's memory image. To avoid
	 * multiple independent cgcef descriptors if the
	 * same member is requested twice, scan the list
	 * of open members for duplicates.
	 *
	 * I don't know how SVR4 handles this case. Don't rely on it.
	 */
	for (xcgcef = ref->e_members; xcgcef; xcgcef = xcgcef->e_link) {
	    cgcef_assert(xcgcef->e_parent == ref);
	    if (xcgcef->e_base == cgcef->e_base) {
		free(arhdr);
		free(cgcef);
		xcgcef->e_count++;
		return xcgcef;
	    }
	}
	if (size == 0) {
	    cgcef->e_data = NULL;
	}
#if 1
	else {
	    /*
	     * Archive members may be misaligned.  Freezing them will
	     * cause libcgcef to allocate buffers for translated data,
	     * which should be properly aligned in all cases.
	     */
	    cgcef_assert(!ref->e_cooked);
	    cgcef->e_data = cgcef->e_rawdata = ref->e_data + offset;
	}
#else
	else if (ref->e_data == ref->e_rawdata) {
	    cgcef_assert(!ref->e_cooked);
	    /*
	     * archive is frozen - freeze member, too
	     */
	    cgcef->e_data = cgcef->e_rawdata = ref->e_data + offset;
	}
	else {
	    cgcef_assert(!ref->e_memory);
	    cgcef->e_data = ref->e_data + offset;
	    /*
	     * The member's memory image may have been modified if
	     * the member has been processed before. Since we need the
	     * original image, we have to re-read the archive file.
	     * Will fail if the archive's file descriptor is disabled.
	     */
	    if (!ref->e_cooked) {
		ref->e_cooked = 1;
	    }
	    else if (!_cgcef_read(ref, cgcef->e_data, offset, size)) {
		free(arhdr);
		free(cgcef);
		return NULL;
	    }
	}
#endif
	cgcef->e_next = offset + size + (size & 1);
	cgcef->e_disabled = ref->e_disabled;
	cgcef->e_memory = ref->e_memory;
	/* parent/child linking */
	cgcef->e_link = ref->e_members;
	ref->e_members = cgcef;
	ref->e_count++;
	/* Slowaris compatibility - do not rely on this! */
	ref->e_off = cgcef->e_next;
    }
    else if (size) {
#if HAVE_MMAP
	/*
	 * Using mmap on writable files will interfere with cgcef_update
	 */
	if (!cgcef->e_writable && (cgcef->e_data = _cgcef_mmap(cgcef))) {
	    cgcef->e_unmap_data = 1;
	}
	else
#endif /* HAVE_MMAP */
	if (!(cgcef->e_data = _cgcef_read(cgcef, NULL, 0, size))) {
	    free(cgcef);
	    return NULL;
	}
    }

    _cgcef_check_type(cgcef, size);
    return cgcef;
}

CGCEf*
cgcef_memory(char *image, size_t size) {
    CGCEf *cgcef;

    cgcef_assert(_cgcef_init.e_magic == CGCEF_MAGIC);
    if (_cgcef_version == EV_NONE) {
	seterr(ERROR_VERSION_UNSET);
	return NULL;
    }
    else if (size == 0 || image == NULL) {
	/* TODO: set error code? */
	return NULL;
    }

    if (!(cgcef = (CGCEf*)malloc(sizeof(CGCEf)))) {
	seterr(ERROR_MEM_CGCEF);
	return NULL;
    }
    *cgcef = _cgcef_init;
    cgcef->e_size = cgcef->e_dsize = size;
    cgcef->e_data = cgcef->e_rawdata = image;
    cgcef->e_readable = 1;
    cgcef->e_disabled = 1;
    cgcef->e_memory = 1;

    _cgcef_check_type(cgcef, size);
    return cgcef;
}

#if __LIBCGCEF64

int
gcgcef_getclass(CGCEf *cgcef) {
    if (cgcef && cgcef->e_kind == CGCEF_K_CGCEF && valid_class(cgcef->e_class)) {
	return cgcef->e_class;
    }
    return CGCEFCLASSNONE;
}

#endif /* __LIBCGCEF64 */
