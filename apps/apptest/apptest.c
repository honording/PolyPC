#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../libs/libelfmb/libelfmb.h"
#include "../../libs/libregister/libregister.h"
#include "../../libs/libddrmalloc/libddrmalloc.h"

#include "../../../generic/include/elf_loader.h"
#include "../../../generic/include/base_addr.h"

#define ELF_FILE_NAME	"/mnt/slave_kernel_thread.elf"

#define	ELF_LOAD_ADDR	ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

int main(int argc, char *argv[])
{
    /*
	elf_info_t elf_info;
	elf_info.main_addr 		= 0;
	elf_info.stack_addr 	= 0;
	elf_info.thread_size 	= 0;
	printf("return from elf_loader:%x\n", elf_loader(ELF_FILE_NAME, 
                                                     ELF_LOAD_ADDR + 3072 * 4, 
                                                     ELF_START_ADDR,
                                                     &elf_info));
	printf("main_addr:  %8X\n", elf_info.main_addr);
	printf("stack_addr: %8X\n", elf_info.stack_addr);
	printf("thread_size:%8X\n", elf_info.thread_size);

    reg_clr();
    int ret = 0;
    struct hapara_thread_struct *sp;
    sp = malloc(sizeof(struct hapara_thread_struct));
    if (sp == NULL) {
        printf("Malloc error.@main\n");
        return 0;
    }

    set_struct(sp, 1, 10, 1, 0, 100, 0, 0, elf_info.main_addr, 
                                           elf_info.stack_addr, 
                                           elf_info.thread_size);
    
    ret = reg_add(sp);
    */
    // ddr_list_print();
    // int a0 = ddr_malloc(34);
    // printf("a0 = 0x%8X\n", a0);
    // int a1 = ddr_malloc(8987);
    // printf("a1 = 0x%8X\n", a1);
    // int a2 = ddr_malloc(343);
    // printf("a2 = 0x%8X\n", a2);
    // int a3 = ddr_malloc(1203);
    // printf("a3 = 0x%8X\n", a3);
    // ddr_list_print();
    // int f0 = ddr_free(a0);
    // // printf("f0 = %d\n", f0);
    // int f1 = ddr_free(a1);
    // // printf("f1 = %d\n", f1);
    // // ddr_list_print();
    // int f2 = ddr_free(a2);
    // // printf("f2 = %d\n", f2);
    // int f3 = ddr_free(a3);
    // ddr_list_print();
    reg_clr();
    struct hapara_thread_struct sp = {
        .isValid = 1,
        .priority = 1,
        .next = 0,
    };

    int ret = elf_loader(ELF_FILE_NAME, ELF_START_ADDR, &sp.elf_info);
    if (ret < 0) {
        printf("apptest: elf_loader error\n");
        return 0;
    }
    print_struct(&sp);
    ret = reg_add(&sp);
    if (ret == -1) {
        printf("apptest: Reg add error 0.\n");
        return 0;
    }
    printf("apptest: add location:%d\n", ret);
    print_list();
    ret = reg_del(ret);
    if (ret == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }   
    print_list();
    ret = ddr_free(sp.elf_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_addr error.\n");
        return 0;
    }
	return 0;
}


