/*
 * acconfig.h - Special definitions for libcgcef, processed by autoheader.
 * Copyright (C) 1995 - 2001, 2004, 2006 Michael Riepe
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

/* @(#) $Id: acconfig.h,v 1.16 2008/05/23 08:17:56 michael Exp $ */

/* Define if you want to include extra debugging code */
#undef ENABLE_DEBUG

/* Define if you want to support extended CGCEF formats */
#undef ENABLE_EXTENDED_FORMAT

/* Define if you want CGCEF format sanity checks by default */
#undef ENABLE_SANITY_CHECKS

/* Define if memmove() does not copy overlapping arrays correctly */
#undef HAVE_BROKEN_MEMMOVE

/* Define if you have the catgets function. */
#undef HAVE_CATGETS

/* Define if you have the dgettext function. */
#undef HAVE_DGETTEXT

/* Define if you have the memset function.  */
#undef HAVE_MEMSET

/* Define if struct nlist is declared in <cgcef.h> or <sys/cgcef.h> */
#undef HAVE_STRUCT_NLIST_DECLARATION

/* Define if Elf32_Dyn is declared in <link.h> */
#undef __LIBCGCEF_NEED_LINK_H

/* Define if Elf32_Dyn is declared in <sys/link.h> */
#undef __LIBCGCEF_NEED_SYS_LINK_H

/* Define to `<cgcef.h>' or `<sys/cgcef.h>' if one of them is present */
#undef __LIBCGCEF_HEADER_CGCEF_H

/* Define if you want 64-bit support (and your system supports it) */
#undef __LIBCGCEF64

/* Define if you want 64-bit support, and are running IRIX */
#undef __LIBCGCEF64_IRIX

/* Define if you want 64-bit support, and are running Linux */
#undef __LIBCGCEF64_LINUX

/* Define if you want symbol versioning (and your system supports it) */
#undef __LIBCGCEF_SYMBOL_VERSIONS

/* Define if symbol versioning uses Sun section type (SHT_SUNW_*) */
#undef __LIBCGCEF_SUN_SYMBOL_VERSIONS

/* Define if symbol versioning uses GNU section types (SHT_GNU_*) */
#undef __LIBCGCEF_GNU_SYMBOL_VERSIONS

/* Define to a 64-bit signed integer type if one exists */
#undef __libcgcef_i64_t

/* Define to a 64-bit unsigned integer type if one exists */
#undef __libcgcef_u64_t

/* Define to a 32-bit signed integer type if one exists */
#undef __libcgcef_i32_t

/* Define to a 32-bit unsigned integer type if one exists */
#undef __libcgcef_u32_t

/* Define to a 16-bit signed integer type if one exists */
#undef __libcgcef_i16_t

/* Define to a 16-bit unsigned integer type if one exists */
#undef __libcgcef_u16_t
