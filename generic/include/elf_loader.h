#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_


struct elf_info_struct {
    unsigned int main_addr;
    unsigned int stack_addr;
    unsigned int thread_size;
    unsigned int ddr_addr;
    unsigned int DMA_size;
};

typedef struct elf_info_struct elf_info_t;

#endif