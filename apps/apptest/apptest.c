#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../libs/libelfmb/libelfmb.h"
#include "../../libs/libregister/libregister.h"

#include "../../../generic/include/elf_loader.h"
#include "../../../generic/include/base_addr.h"

#define ELF_FILE_NAME	"/mnt/slave_kernel_thread.elf"

#define	ELF_LOAD_ADDR	ARM_DDR_BASE

int main(int argc, char *argv[])
{
	elf_info_t elf_info;
	elf_info.main_addr 		= 0;
	elf_info.stack_addr 	= 0;
	elf_info.thread_size 	= 0;
	printf("return from elf_loader:%x\n", elf_loader(ELF_FILE_NAME, 
                                                     ELF_LOAD_ADDR + 3072 * 4, 
                                                     &elf_info));
	printf("main_addr:  %8X\n", elf_info.main_addr);
	printf("stack_addr: %8X\n", elf_info.stack_addr);
	printf("thread_size:%8X\n", elf_info.thread_size);

	return 0;
}


