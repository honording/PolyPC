#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../libs/libelfmb/libelfmb.h"
#define ELF_FILE_NAME	"/mnt/kernel_thread.elf"

#define	SIG_ADDR		0x40000000
#define	ELF_LOAD_ADDR	0x42000000

int main(int argc, char *argv[])
{
	//unsigned int addr = get_main_entry(ELF_FILE_NAME);
	//printf("main addr: %x\n", addr);
	printf("main addr:%x\n", elf_loader(ELF_FILE_NAME, ELF_LOAD_ADDR));
	return 0;
}


