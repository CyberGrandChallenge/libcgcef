/*
 * input.c - low-level input for libcgcef.
 * Copyright (C) 1995 - 2001, 2005 Michael Riepe
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
static const char rcsid[] = "@(#) $Id: input.c,v 1.11 2008/05/23 08:15:35 michael Exp $";
#endif /* lint */

#include <errno.h>

#if HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

static int
xread(int fd, char *buffer, size_t len) {
    size_t done = 0;
    size_t n;

    while (done < len) {
	n = read(fd, buffer + done, len - done);
	if (n == 0) {
	    /* premature end of file */
	    return -1;
	}
	else if (n != (size_t)-1) {
	    /* some bytes read, continue */
	    done += n;
	}
	else if (errno != EAGAIN && errno != EINTR) {
	    /* real error */
	    return -1;
	}
    }
    return 0;
}

void*
_cgcef_read(CGCEf *cgcef, void *buffer, size_t off, size_t len) {
    void *tmp;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    cgcef_assert(off >= 0 && off + len <= cgcef->e_size);
    if (cgcef->e_disabled) {
	seterr(ERROR_FDDISABLED);
    }
    else if (len) {
	off += cgcef->e_base;
	if (lseek(cgcef->e_fd, (off_t)off, SEEK_SET) != (off_t)off) {
	    seterr(ERROR_IO_SEEK);
	}
	else if (!(tmp = buffer) && !(tmp = malloc(len))) {
	    seterr(ERROR_IO_2BIG);
	}
	else if (xread(cgcef->e_fd, tmp, len)) {
	    seterr(ERROR_IO_READ);
	    if (tmp != buffer) {
		free(tmp);
	    }
	}
	else {
	    return tmp;
	}
    }
    return NULL;
}

void*
_cgcef_mmap(CGCEf *cgcef) {
#if HAVE_MMAP
    void *tmp;

    cgcef_assert(cgcef);
    cgcef_assert(cgcef->e_magic == CGCEF_MAGIC);
    cgcef_assert(cgcef->e_base == 0);
    if (cgcef->e_disabled) {
	seterr(ERROR_FDDISABLED);
    }
    else if (cgcef->e_size) {
	tmp = (void*)mmap(0, cgcef->e_size, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE, cgcef->e_fd, 0);
	if (tmp != (void*)-1) {
	    return tmp;
	}
    }
#endif /* HAVE_MMAP */
    return NULL;
}
