#ifndef __LIBELFMB_H__
#define __LIBELFMB_H__

#include "../../../generic/include/elf32.h"
#include "../../../generic/include/elf_loader.h"

#include "../libddrmalloc/libddrmalloc.h"

#define DEVMEM  "/dev/mem"


//unsigned int get_main_entry(char *file_name);
//int load_elf(char *file_name, unsigned int addr);
int elf_loader(char *file_name, 
               // unsigned int addr, 
               unsigned int elf_start_addr, 
               elf_info_t *elf_info);

#endif
