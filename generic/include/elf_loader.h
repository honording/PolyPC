#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

typedef struct {
    unsigned int main_addr;
    unsigned int stack_addr;
    unsigned int thread_size;
} elf_info_t;

#endif