#ifndef __LIBELFMB_H__
#define __LIBELFMB_H__

#include "../../../generic/include/elf32.h"

#define DEVMEM  "/dev/mem"

//unsigned int get_main_entry(char *file_name);
//int load_elf(char *file_name, unsigned int addr);
int elf_loader(char *file_name, unsigned int addr);

#endif
