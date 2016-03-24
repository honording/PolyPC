#ifndef __LIBELFMB_H__
#define __LIBELFMB_H__

#include "../../../generic/include/elf32.h"
#include "../../../generic/include/elf_loader.h"

#define DEVMEM  "/dev/mem"

// typedef struct {
//     unsigned int main_addr;
//     unsigned int stack_addr;
//     unsigned int thread_size;
// } elf_info_t;

//unsigned int get_main_entry(char *file_name);
//int load_elf(char *file_name, unsigned int addr);
int elf_loader(char *file_name, unsigned int addr, elf_info_t *elf_info);

#endif
