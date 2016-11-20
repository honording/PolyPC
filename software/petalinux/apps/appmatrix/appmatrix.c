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

#define ELF_FILE_NAME   "/mnt/elf_apps/vector_add.elf"
// #define PR_FILE_PATH    "/mnt/pr_files/vector_sub"

#define PR_ROOT_PATH    "/mnt/pr_files/"

#define TRACE_FILE      "/mnt/testscript/trace.txt"

#define ELF_LOAD_ADDR   ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

#define DEVMEM          "/dev/mem"

#define SIZE_PER_PE     2
#define PE_PER_GROUP    4

typedef int d_type;

int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("Input: %s [Number of Groups] [Input Data Size] [Buffer Size] [HW Config]\n", argv[0]);
        printf("HW Config: 1g4s4h\n");
        printf("HW Config: 2g4s8h\n");
        printf("HW Config: 4g2s8h\n");           
        return 0;
    }
    int i;
    reg_clr();
    trace_clr();

    int num_group   = atoi(argv[1]);
    int MEM_SIZE    = atoi(argv[2]);
    int BUF_LEN     = atoi(argv[3]);
    if (BUF_LEN > MEM_SIZE) {
        BUF_LEN = MEM_SIZE;
    }
    int ID_NUM  = MEM_SIZE / num_group / BUF_LEN;
    if (ID_NUM * ID_NUM < 2 * PE_PER_GROUP) {
        printf("%f\n", 0.0);
        int trace_off = trace_alloc_single(num_group * num_group);
        // Read trace information into a file
        FILE *trace_f = fopen(TRACE_FILE, "a");
        unsigned int curr_trace_num = trace_geteachsize(0) / sizeof(unsigned int);
        unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
        trace_gettotalcon(trace_ram);
        fprintf(trace_f, "\n%d %d\n", num_group, MEM_SIZE);
        fprintf(trace_f, "%08X\n", 0.0);
        fprintf(trace_f, "%08X\n", 0.0);
        for (i = 0; i < curr_trace_num; i++) {
            // printf("%08X\n", trace_ram[i]);
            fprintf(trace_f, "%08X\n", trace_ram[i]);
        }

        free(trace_ram);
        fclose(trace_f);
        return 0;
    }
    
    int a_addr = ddr_malloc(MEM_SIZE * MEM_SIZE * sizeof(d_type));
    if (a_addr < 0) {
        // printf("apptest: ddr_malloc error a\n");
        return 0;
    }
    int b_addr = ddr_malloc(MEM_SIZE * MEM_SIZE * sizeof(d_type));
    if (b_addr < 0) {
        // printf("apptest: ddr_malloc error b\n");
        return 0;
    }
    int c_addr = ddr_malloc(MEM_SIZE * MEM_SIZE * sizeof(d_type));
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
    d_type *a = mmap(NULL, MEM_SIZE * MEM_SIZE * sizeof(d_type), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr);
    d_type *b = mmap(NULL, MEM_SIZE * MEM_SIZE * sizeof(d_type), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr);
    d_type *c = mmap(NULL, MEM_SIZE * MEM_SIZE * sizeof(d_type), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr);
    for (i = 0; i < MEM_SIZE * MEM_SIZE; i++) {
        a[i] = i + 2;
        b[i] = i + 1;
        c[i] = 0;
    }


    // printf("Initialize finished.\n");
    struct hapara_thread_struct sp;

    sp.argv[0] = a_addr;
    sp.argv[1] = b_addr;
    sp.argv[2] = c_addr;
    sp.argv[3] = MEM_SIZE;
    sp.argv[4] = BUF_LEN;
    sp.cur_group_id.id0      = 0;
    sp.cur_group_id.id1      = 0;    
    sp.group_size.id0 = ID_NUM;
    sp.group_size.id1 = ID_NUM;
    sp.group_num.id0 = num_group;
    sp.group_num.id1 = num_group;     

    if (BUF_LEN == 1) {
        sp.elf_info.elf_magic = 'x';
    } else {
        sp.elf_info.elf_magic = 'z';
    }
    // Allocate trace space
    int trace_off = trace_alloc(sp.group_num);
    sp.trace_ram_off = trace_off / sizeof(struct hapara_trace_struct);
    // printf("apptest: begin to elf_loader.\n");
    int ret = elf_loader(ELF_FILE_NAME, ELF_START_ADDR, &sp.elf_info);
    if (ret < 0) {
        // printf("apptest: elf_loader error\n");
        return 0;
    }

    // printf("apptest: begin to load PR bitstream into memory.\n");
    char pr_file_path[128];
    if (BUF_LEN == 1) {
        strcpy(pr_file_path, PR_ROOT_PATH);
        strcat(pr_file_path, argv[4]);
        strcat(pr_file_path, "/matrix_mul1buf");
    } else {
        strcpy(pr_file_path, PR_ROOT_PATH);
        strcat(pr_file_path, argv[4]);
        strcat(pr_file_path, "/matrix_mul");
    }
    printf("pr_file_path: %s\n", pr_file_path);
    ret = pr_loader(pr_file_path, &sp.pr_info);
    if (ret < 0) {
        // printf("apptes: pr_loader error\n");
        return 0;
    }

    // if (argc == 2) {
        // printf("Disable MB Pr flow.\n");
        // disable_mb_pr(&sp.pr_info);
    // }


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

    hapara_timer_reset();
    hapara_timer_start();
    // printf("Add htdt struct...\n");
    
    unsigned int timer0;
    unsigned int timer1;

    hapara_timer_gettime(&timer0);

    ret = reg_add(&sp);
    // if (ret == -1) {
    //     // printf("apptest: Reg add error 0.\n");
    //     return 0;
    // }

    // if (htdt[ret].elf_info.elf_magic != 'v') {
    //     // printf("HDDT addr error.\n");
    //     return 0;
    // }
    // print_struct(&htdt[ret]);

    while (htdt[ret].isValid != 0);


    hapara_timer_gettime(&timer1);

    printf("%f\n", (timer1 - timer0) / 100000000.0);
    // sleep(1);

    // Read trace information into a file
    FILE *trace_f = fopen(TRACE_FILE, "a");
    unsigned int curr_trace_num = trace_geteachsize(0) / sizeof(unsigned int);
    unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
    
    trace_gettotalcon(trace_ram);
    while (trace_ram[curr_trace_num - 1] == 0) {
        // printf("Re-read\n");
        trace_gettotalcon(trace_ram);
    }

    fprintf(trace_f, "\n%d %d\n", num_group, MEM_SIZE);
    fprintf(trace_f, "%08X\n", timer0);
    fprintf(trace_f, "%08X\n", timer1);
    for (i = 0; i < curr_trace_num; i++) {
        // printf("%08X\n", trace_ram[i]);
        fprintf(trace_f, "%08X\n", trace_ram[i]);
    }

    free(trace_ram);
    fclose(trace_f);
    
    // printf("Slaves terminated: %f.\n", timeuse);
    // printf("%f\n", timeuse);
    // printf("apptest: add location:%d\n", ret);
    // print_struct(&htdt[ret]);

    ret = reg_del(ret);
    if (ret == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    } 
    int error = 0;
    // int filter[TAP] = {3,2,1,2,3};
    int j, k;
    for (i = 0; i < MEM_SIZE; i++) {
        for (j = 0; j < MEM_SIZE; j++) {
            int sum = 0;
            for (k = 0; k < MEM_SIZE; k++) {
                sum += a[i * MEM_SIZE + k] * b[k * MEM_SIZE + j];
            }
            if (sum != c[i * MEM_SIZE + j]) {
                error++;
            }
            // printf("%d, %d: %d: %d\n", i, j, sum, c[i * MEM_SIZE + j]);
        }
    }
    munmap(htdt, 1024);
    munmap(a, MEM_SIZE * MEM_SIZE * sizeof(d_type));
    munmap(b, MEM_SIZE * MEM_SIZE * sizeof(d_type));
    munmap(c, MEM_SIZE * MEM_SIZE * sizeof(d_type));
    close(devmemfd);

    printf("Number of Error: %d\n", error);
    // ddr_list_print();
    ret = ddr_free(sp.elf_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: elf.\n");
        return 0;
    }
    // ddr_list_print();
    ret = ddr_free(sp.pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: pr.\n");
        return 0;
    }
    // ddr_list_print();
    ret = ddr_free(a_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: a.\n");
        return 0;
    }
    // ddr_list_print();  
    ret = ddr_free(b_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: b.\n");
        return 0;
    }
    // ddr_list_print();
    ret = ddr_free(c_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: c.\n");
        return 0;
    }
    // ddr_list_print();
    // printf("apptest end!\n");
    return 0;
}


