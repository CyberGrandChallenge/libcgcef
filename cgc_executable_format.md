% CGC Executable Format Manuals

# NAME
CGCEF - CGC Executable Format

# SYNOPSIS
\#include \<libcgcef.h\>

# DESCRIPTION
This document describes the on-disk format of CGC Challenge Binaries; also known as the executable format and the linker format.
The CGC Executable Format (CGCEF) has been designed with two goals in mind:

- guaranteeing that vulnerable CGC binaries will not run outside of captivity
- simplifying the cost of retargeting existing tools to the CGC platform by mimicking the ELF format

# FILE COMPONENTS

There are three parts to a CGCEF file:

- the CGCEF File Header
- optional section headers
- program headers

# CGCEF FILE HEADER
The CGCEF file header is located at the beginning of the CGCEF file and describes the content of the file.

    /* The CGC Executable File Header */
    #define CI_NIDENT   16
    typedef struct{
        unsigned char	e_ident[EI_NIDENT];
    #define C_IDENT	"\x7fCGC\x01\x01\x01\x43\x01\x00\x00\x00\x00\x00\x00"
        /* ELF vs CGC identification 
         * ELF          CGC
         *  0x7f        0x7f
         *  'E'         'C'
         *  'L'         'G'
         *  'F'         'C'
         *  class       1       : '1' translates to 32bit ELF
         *  data        1       : '1' translates to little endian ELF
         *  version     1       : '1' translates to little endian ELF
         *  osabi       \x43    : '1' CGC
         *  abiversion  1       : '1' translates to version 1
         *  padding     random values
         */
        uint16_t	e_type;         /* Must be 2 for executable */
        uint16_t	e_machine;      /* Must be 3 for i386 */
        uint32_t	e_version;      /* Must be 1 */
        uint32_t	e_entry;        /* Virtual address entry point */
        uint32_t	e_phoff;        /* Program Header offset */
        uint32_t	e_shoff;        /* Section Header offset */
        uint32_t	e_flags;        /* Must be 0 */
        uint16_t	e_ehsize;       /* CGC header's size */
        uint16_t	e_phentsize;    /* Program header entry size */
        uint16_t	e_phnum;        /* # program header entries */
        uint16_t	e_shentsize;    /* Section header entry size */
        uint16_t	e_shnum;        /* # section header entries */
        uint16_t	e_shstrndx;     /* sect header # of str table */
    } CGC32_hdr;


# CGCEF PROGRAM HEADERS
The CGC Executable File format program headers dictate how sections of the file are mapped into process memory prior to execution.
Note that there are only four section types.
These do NOT include 'interpreter', 'dynamic loading', or 'thread-local-storage' section types.

    /* The CGC Executable Program Header */
    typedef struct{
        uint32_t        p_type;         /* Section type */
    #define PT_NULL     0               /* Unused header */
    #define PT_LOAD     1               /* Segment loaded into mem */
    #define PT_PHDR     6               /* Program hdr tbl itself */
    #define PT_CGCPOV2  0x6ccccccc      /* CFE Type 2 PoV flag sect */
        uint32_t        p_offset;       /* Offset into the file */
        uint32_t        p_vaddr;        /* Virtual program address */
        uint32_t        p_paddr;        /* Set to zero */
        uint32_t        p_filesz;       /* Section bytes in file */
        uint32_t        p_memsz;        /* Section bytes in memory */
        uint32_t        p_flags;        /* section flags */
    #define PF_X        (1<<0)          /* Mapped executable */
    #define PF_W        (1<<1)          /* Mapped writable */
    #define PF_R        (1<<2)          /* Mapped readable */
        /* Acceptable flag combinations are:
         *        PF_R
         *        PF_R|PF_W
         *        PF_R|PF_X
         *        PF_R|PF_W|PF_X
         */
        uint32_t        p_align;        /* Only used by core dumps */
    } CGC32_Phdr;


# CGCEF SECTION HEADERS
The CGC Executable File format support section headers in purely an
optional informational capacity.
They may be utilized for debugging but are ignored completely by the loader.
Teams are discouraged from relying on them in any capacity.
They may be stripped out prior to deployment, distribution or consensus evaluation.

    /* The CGC Executable Section Header */
    typedef struct {
        uint32_t        sh_name;        /* Name (index into strtab) */
        uint32_t        sh_type;        /* Section type */
    #define SHT_SYMTAB  2               /* Symbol table */
    #define SHT_STRTAB  3               /* String Table */
        uint32_t        sh_flags;
    #define SHT_WRITE   (1<<0)          /* Section is writable */
    #define SHT_ALLOC   (1<<1)          /* Section is in memory */
    #define SHT_EXECINSTR (1<<2)        /* Section contains code */
        uint32_t        sh_addr;        /* Address of section */
        uint32_t        sh_offset;      /* Offset into file */
        uint32_t        sh_size;        /* Section size in file */
        uint32_t        sh_link;
                /* When sh_type is SHT_SYMTAB, sh_link is the index of
                 * the associated SHT_STRTAB section
                 */
        uint32_t        sh_info;
                /* When sh_type is SHT_SYMTAB, info is one greater
                 * than the symbol table index of the last local
                 * symbol.
                 */
        uint32_t        sh_addralign;   /* Alignment constraints */
        uint32_t        sh_entsize;
                /* Size in bytes of each entry pointed to by this
                 * section table */
    } CGC32_Shdr;


# SEE ALSO

Several CGCEF libraries and utilities have been provided:

- binutils - common binary utilities including the linker
- cgcef-verify - a utility to verify CGCEF binaries
- libcgcef - a libelf port for reading and manipulating CGCEF files
- readcgcef - a readelf port for examining CGCEF binaries
