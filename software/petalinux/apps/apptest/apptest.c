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
#include "../../libs/libpr/libpr.h"
#include "../../libs/libregister/libregister.h"
#include "../../libs/libddrmalloc/libddrmalloc.h"

#include "../../../generic/include/elf_loader.h"
#include "../../../generic/include/pr_loader.h"
#include "../../../generic/include/base_addr.h"
#include "../../../generic/include/thread_struct.h"

#define ELF_FILE_NAME   "/mnt/elf_apps/vector_add.elf"
#define PR_FILE_PATH    "/mnt/pr_files/vector_sub"

#define	ELF_LOAD_ADDR	ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

#define MEM_SIZE        16
#define DEVMEM          "/dev/mem"

int main(int argc, char *argv[])
{
    reg_clr();

    int a_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (a_addr < 0) {
        printf("apptest: ddr_malloc error a\n");
        return 0;
    }
    int b_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (b_addr < 0) {
        printf("apptest: ddr_malloc error b\n");
        return 0;
    }
    int c_addr = ddr_malloc(MEM_SIZE * sizeof(int));
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
        a[i] = i + i;
        b[i] = i;
        c[i] = 0;
    }
    printf("Initialize finished.\n");
    struct hapara_thread_struct sp;

    sp.argv[0] = a_addr;
    sp.argv[1] = b_addr;
    sp.argv[2] = c_addr;
    sp.group_size.id0 = 1;
    sp.group_size.id1 = 4;
    sp.group_num.id0 = 1;
    sp.group_num.id1 = 4;
    sp.elf_info.elf_magic = 'v';
    printf("apptest: begin to elf_loader.\n");
    int ret = elf_loader(ELF_FILE_NAME, ELF_START_ADDR, &sp.elf_info);
    if (ret < 0) {
        printf("apptest: elf_loader error\n");
        return 0;
    }

    printf("apptest: begin to load PR bitstream into memory.\n");
    ret = pr_loader(PR_FILE_PATH, &sp.pr_info);
    if (ret < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }

    if (argc == 2) {
        printf("Disable MB Pr flow.\n");
        disable_mb_pr(&sp.pr_info);
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
        printf("%d + %d = %d.", a[i], b[i], c[i]);
        if (a[i] + b[i] != c[i]) {
            printf("Error.\n");
        } else {
            printf("\n");
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
    ret = ddr_free(sp.pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: pr.\n");
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


