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
#include "../../../generic/include/thread_struct.h"

#define ELF_FILE_NAME	"/mnt/opencl_kernel.elf"

#define	ELF_LOAD_ADDR	ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

#define MEM_SIZE        32768
#define DEVMEM          "/dev/mem"

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

    int a_addr = ddr_malloc(MEM_SIZE);
    if (a_addr < 0) {
        printf("apptest: ddr_malloc error a\n");
        return 0;
    }
    int b_addr = ddr_malloc(MEM_SIZE);
    if (b_addr < 0) {
        printf("apptest: ddr_malloc error b\n");
        return 0;
    }
    int c_addr = ddr_malloc(MEM_SIZE);
    if (c_addr < 0) {
        printf("apptest: ddr_malloc error c\n");
        return 0;
    }
    printf("a addr:0x%08X\n", a_addr);
    printf("b addr:0x%08X\n", b_addr);
    printf("c addr:0x%08X\n", c_addr);
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        printf("apptest: devmem open failed.\n");
        return -1;
    }
    printf("begin to map a, b, and c.\n");
    int *a = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr);
    int *b = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr);
    int *c = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr);
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        a[i] = i;
        b[i] = i + 1;
        c[i] = 0;
    }
    printf("Initialize finished.\n");
    struct hapara_thread_struct sp;

    sp.argv[0] = a_addr;
    sp.argv[1] = b_addr;
    sp.argv[2] = c_addr;
    sp.group_size.id0 = 1;
    sp.group_size.id1 = 512;
    sp.group_num.id0 = 1;
    sp.group_num.id1 = 64;
    sp.elf_info.elf_magic = 'v';
    printf("apptest: begin to elf_loader.\n");
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
    printf("Waiting to terminate...\n");
    devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        printf("apptest: devmem open failed.\n");
        return -1;
    }
    volatile struct hapara_thread_struct *htdt = mmap(NULL, 
                                             1024,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             devmemfd,
                                             ARM_HTDT_BASE);
    if (htdt[ret].elf_info.elf_magic != 'v') {
        printf("HDDT addr error.\n");
        return 0;
    }
    // print_struct(&htdt[ret]);
    while (htdt[ret].isValid != 0);
    printf("Slaves terminated.\n");
    ret = reg_del(ret);
    if (ret == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    } 
    int error = 0;
    for (i = 0; i < MEM_SIZE; i++) {
        if (a[i] + b[i] != c[i]) {
            error = 1;
            break;
        }
    }
    munmap(htdt, 1024);
    munmap(a, MEM_SIZE * sizeof(int));
    munmap(b, MEM_SIZE * sizeof(int));
    munmap(c, MEM_SIZE * sizeof(int));
    close(devmemfd);

    if (error) {
        printf("Solutions are not correct.\n");
    } else {
        printf("Passed!\n");
    }
    ret = ddr_free(sp.elf_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: elf.\n");
        return 0;
    }
    ret = ddr_free(a_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: c.\n");
        return 0;
    }   
    printf("apptest end!\n");
	return 0;
}


