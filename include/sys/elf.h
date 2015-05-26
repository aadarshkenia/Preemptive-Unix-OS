#ifndef _ELF_H
#define _ELF_H
#include <sys/defs.h>
#include <sys/structs.h>

//AADY:17th APR

//Permissions for section headers
#define SH_ALLOC 0x2
#define SH_EXEC 0x4
#define SH_WRITE 0x1

//Permissions for VMA s
#define VM_READ 0x1
#define VM_WRITE 0x2
#define VM_EXEC 0x4


struct ELF_hdr {
	unsigned char	e_ident[16];
	uint16_t	e_type;
	uint16_t	e_machine;
	uint32_t	e_version;
	uint64_t	e_entry;
	uint64_t	e_phoff;
	uint64_t	e_shoff;
	uint32_t	e_flags;
	uint16_t	e_ehsize;
	uint16_t	e_phentsize;
	uint16_t	e_phnum;
	uint16_t	e_shentsize;
	uint16_t	e_shnum;
	uint16_t	e_shtrndx;
};

struct ELF_P_hdr {
	uint32_t	p_type;
	uint32_t	p_flags;
	uint64_t	p_offset;
	uint64_t	p_vaddr;
	uint64_t	p_paddr;
	uint64_t	p_filesz;
	uint64_t	p_memsz;
	uint64_t	p_align;
};

struct ELF_S_hdr {
	uint32_t	sh_name;
	uint32_t	sh_type;
	uint64_t	sh_flags;
	uint64_t	sh_addr;
	uint64_t	sh_offset;
	uint64_t	sh_size;
	uint32_t	sh_link;
	uint32_t	sh_info;
	uint64_t	sh_addralign;
	uint64_t	sh_entsize;
};

struct ELF_Str {
	uint32_t	st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	uint16_t	st_shndx;
	uint64_t	st_value;
	uint64_t	st_size;
};

uint64_t getELF(char *fileName, struct PCB* proc);
int checkELF(char *fileName);
#endif
