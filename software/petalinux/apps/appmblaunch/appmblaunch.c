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
#include <sys/time.h>

#include "../../libs/libelfmb/libelfmb.h"
#include "../../libs/libpr/libpr.h"
#include "../../libs/libregister/libregister.h"
#include "../../libs/libddrmalloc/libddrmalloc.h"
#include "../../libs/libtrace/libtrace.h"

#include "../../../generic/include/elf_loader.h"
#include "../../../generic/include/pr_loader.h"
#include "../../../generic/include/base_addr.h"
#include "../../../generic/include/thread_struct.h"

#define ELF_FILE_NAME   "/mnt/elf_apps/"

#define TRACE_FILE      "/mnt/testscript/trace.txt"

#define	ELF_LOAD_ADDR	ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

#define DEVMEM          "/dev/mem"

#define TAP         5

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printf("Input: %s [Number of Groups] [Input Data Size] [Buffer Size] [Benchmark]\n", argv[0]);
        printf("Available Benchmarks:\n");
        printf("mem_r, mem_w, mem_rw\n");
        printf("vector_add, vector_sub, vector_mul\n");
        printf("fir\n");
        return 0;
    }
    int i;
    reg_clr();
    trace_clr();

    int MEM_SIZE = atoi(argv[2]) * 1024;        // Input Data Size
    int BUF_LEN = atoi(argv[3]);                // Buffer Size
    int num_group = atoi(argv[1]);              // Number of Groups

    int a_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (a_addr < 0) {
        // printf("apptest: ddr_malloc error a\n");
        return 0;
    }
    int b_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (b_addr < 0) {
        // printf("apptest: ddr_malloc error b\n");
        return 0;
    }
    int c_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (c_addr < 0) {
        // printf("apptest: ddr_malloc error c\n");
        return 0;
    }
    // printf("a addr:0x%08X\n", a_addr);
    // printf("b addr:0x%08X\n", b_addr);
    // printf("c addr:0x%08X\n", c_addr);
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        // printf("apptest: devmem open failed.\n");
        return -1;
    }
    // printf("begin to map a, b, and c.\n");
    int *a = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr);
    int *b = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr);
    int *c = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr);

    for (i = 0; i < MEM_SIZE; i++) {
        a[i] = i + 3;
        b[i] = i;
        c[i] = 0;
    }
    // printf("Initialize finished.\n");
    struct hapara_thread_struct sp;

    int ID_NUM  = MEM_SIZE / BUF_LEN / num_group;

    if (strcmp(argv[4], "mem_r") == 0 ||
        strcmp(argv[4], "mem_w") == 0) {
        sp.argv[0] = a_addr;
        sp.argv[1] = BUF_LEN;        
    } else if (strcmp(argv[4], "mem_rw") == 0 ||
               strcmp(argv[4], "fir") == 0) {
        sp.argv[0] = a_addr;
        sp.argv[1] = b_addr;
        sp.argv[2] = BUF_LEN;        
    } else {
        sp.argv[0] = a_addr;
        sp.argv[1] = b_addr;
        sp.argv[2] = c_addr;
        sp.argv[3] = BUF_LEN;        
    }
    sp.cur_group_id.id0      = 0;
    sp.cur_group_id.id1      = 0;    
    sp.group_size.id0        = 1;
    sp.group_size.id1        = ID_NUM;
    sp.group_num.id0         = 1;
    sp.group_num.id1         = num_group;

    if (strcmp(argv[4], "vector_add") == 0) {
        sp.elf_info.elf_magic = 'a';
    } else if (strcmp(argv[4], "vector_sub") == 0) {
        sp.elf_info.elf_magic = 's';
    } else if (strcmp(argv[4], "vector_mul") == 0) {
        sp.elf_info.elf_magic = 'm';
    } else if (strcmp(argv[4], "mem_r") == 0) {
        sp.elf_info.elf_magic = 'r';
    } else if (strcmp(argv[4], "mem_w") == 0) {
        sp.elf_info.elf_magic = 'w';
    } else if (strcmp(argv[4], "mem_rw") == 0) {
        sp.elf_info.elf_magic = 't';
    } else if (strcmp(argv[4], "fir") == 0) {
        sp.elf_info.elf_magic = 'f';
    } else {
        printf("Acc Name Error: %s\n", argv[4]);
        return 0;
    }
    // Allocate trace space
    int trace_off = trace_alloc(sp.group_num);
    sp.trace_ram_off = trace_off / sizeof(struct hapara_trace_struct);
    // printf("apptest: begin to elf_loader.\n");
    char elf_file_path[128];
    strcpy(elf_file_path, ELF_FILE_NAME);
    strcat(pr_file_path, argv[4]);
    strcat(pr_file_path, ".elf");
    printf("pr_file_path: %s\n", pr_file_path);

    int ret = elf_loader(elf_file_path, ELF_START_ADDR, &sp.elf_info);
    if (ret < 0) {
        // printf("apptest: elf_loader error\n");
        return 0;
    }

    print_struct(&sp);
    devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        // printf("apptest: devmem open failed.\n");
        return -1;
    }    
    volatile struct hapara_thread_struct *htdt = mmap(NULL, 
                                             1024,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             devmemfd,
                                             ARM_HTDT_BASE);    



    struct timeval t1, t2;
    double timeuse;

    hapara_timer_reset();
    hapara_timer_start();
    // printf("Add htdt struct...\n");
    
    unsigned int timer0;
    unsigned int timer1;

    hapara_timer_gettime(&timer0);

    ret = reg_add(&sp);

    // goto go_exit;

    // if (ret == -1) {
    //     // printf("apptest: Reg add error 0.\n");
    //     return 0;
    // }

    // if (htdt[ret].elf_info.elf_magic != 'v') {
    //     // printf("HDDT addr error.\n");
    //     return 0;
    // }
    // print_struct(&htdt[ret]);

    // gettimeofday(&t1, NULL);
    while (htdt[ret].isValid != 0);
    // gettimeofday(&t2, NULL);
    hapara_timer_gettime(&timer1);

    // timeuse = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;


    printf("%f\n", (timer1 - timer0) / 100000000.0);
    // sleep(1);

    // Read trace information into a file
    printf("Waiting...\n");
    hapara_dump_trace(0, TRACE_FILE);
    // printf("Slaves terminated: %f.\n", timeuse);
    // printf("%f\n", timeuse);
    // printf("apptest: add location:%d\n", ret);
    // print_struct(&htdt[ret]);

    ret = reg_del(ret);
    if (ret == -1) {
        // printf("apptest: Reg del error 0.\n");
        return 0;
    } 
    int error = 0;
    int filter[TAP] = {3,2,1,2,3};
    int j;

    if (strcmp(argv[4], "vector_add") == 0) {
        printf("Vector Add:\n");
        for (i = 0; i < MEM_SIZE; i++) {
            if (a[i] + b[i] != c[i]) {
                error++;
            }
        }        
    } else if (strcmp(argv[4], "vector_sub") == 0) {
        printf("Vector Sub:\n");
        for (i = 0; i < MEM_SIZE; i++) {
            if (a[i] - b[i] != c[i]) {
                error++;
            }
        }          
    } else if (strcmp(argv[4], "vector_mul") == 0) {
        printf("Vector Mul.\n");
        for (i = 0; i < MEM_SIZE; i++) {
            if (a[i] * b[i] != c[i]) {
                error++;
            }
        }
    } else if (strcmp(argv[4], "fir") == 0) {
        printf("Fir.\n");
        for (i = TAP - 1; i < MEM_SIZE; i++) {
            int sum = 0;
            for (j = 0; j < TAP; j++) {
                sum += filter[j] * a[i + (j - TAP + 1)];
            }
            if (b[i] != sum) {
                error++;
            }
        }        
    }

go_exit:
    munmap(htdt, 1024);
    munmap(a, MEM_SIZE * sizeof(int));
    munmap(b, MEM_SIZE * sizeof(int));
    munmap(c, MEM_SIZE * sizeof(int));
    close(devmemfd);

    printf("Number of Error: %d\n", error);
    ret = ddr_free(sp.elf_info.ddr_addr);
    if (ret < 0) {
        // printf("apptest: ddr_free error: elf.\n");
        return 0;
    }
    ret = ddr_free(sp.pr_info.ddr_addr);
    if (ret < 0) {
        // printf("apptest: ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(a_addr);
    if (ret < 0) {
        // printf("apptest: ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr);
    if (ret < 0) {
        // printf("apptest: ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr);
    if (ret < 0) {
        // printf("apptest: ddr_free error: c.\n");
        return 0;
    }   
    // printf("apptest end!\n");
	return 0;
}


