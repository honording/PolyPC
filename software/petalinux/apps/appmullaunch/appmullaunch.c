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
#include <time.h>

#include "../../libs/libelfmb/libelfmb.h"
#include "../../libs/libpr/libpr.h"
#include "../../libs/libregister/libregister.h"
#include "../../libs/libddrmalloc/libddrmalloc.h"
#include "../../libs/libtrace/libtrace.h"

#include "../../../generic/include/elf_loader.h"
#include "../../../generic/include/pr_loader.h"
#include "../../../generic/include/base_addr.h"
#include "../../../generic/include/thread_struct.h"

#define PR_ROOT_PATH    "/mnt/pr_files/"
#define TRACE_FILE      "/mnt/testscript/trace.txt"
#define DEVMEM          "/dev/mem"

struct app_t {
    char *app_name;
    unsigned char priority;
    unsigned int data_size;
    unsigned int buffer_size;
};
#define NUM_APP         16
#define NUM_APP_TYPE    4
char app_name[NUM_APP_TYPE][128] = {{"vector_mul"},
                                    {"pageranking"},
                                    {"matrix_mul"},
                                    {"fir"}};

void init_app_seq(struct app_t *app_seq) {
    int i;
    for (i = 0; i < NUM_APP; i++) {
        unsigned int index = i / ( NUM_APP / NUM_APP_TYPE);
        app_seq[i].app_name = app_name[index];
        app_seq[i].priority = i % 2;
        switch (index) {
            case 0:    // vector_mul
                app_seq[i].data_size    = 256 * 1024;
                app_seq[i].buffer_size  = 32;
                break;
            case 1:    // pageranking
                app_seq[i].data_size    = 512;
                app_seq[i].buffer_size  = 32;
                break;
            case 2:    // matrix_mul
                app_seq[i].data_size    = 128;
                app_seq[i].buffer_size  = 16;
                break;
            case 3:    //fir
                app_seq[i].data_size    = 256 * 1024;
                app_seq[i].buffer_size  = 32;
                break;
            default:
                break;
        }
    }
}

void random_app(unsigned int *sequence) {
    int i;
    srand((unsigned) time(NULL));
    for(i = 0; i < NUM_APP; i++) {
        sequence[i] = i;
    }
    for(i = NUM_APP - 1; i >= 1; i--) {
        unsigned int temp;
        unsigned int ii = rand() % i;
        temp = sequence[i];
        sequence[i] = sequence[ii];
        sequence[ii] = temp;
    }
}

void print_seq(unsigned int *sequence, struct app_t *app_seq) {
    int i;
    for (i = 0; i < NUM_APP; i++) {
        printf("%d:\n", i);
        printf("  app[%2d].name:        %s\n", sequence[i], app_seq[sequence[i]].app_name);
        printf("  app[%2d].priority:    %d\n", sequence[i], app_seq[sequence[i]].priority);
        printf("  app[%2d].data_size:   %d\n", sequence[i], app_seq[sequence[i]].data_size);
        printf("  app[%2d].buffer_size: %d\n", sequence[i], app_seq[sequence[i]].buffer_size);
    }
    for (i = 0; i < NUM_APP; i++) {
        printf("%2d,", sequence[i]);
    }
    printf("\n");
}

void dump_trace(int index) {
    // Read trace information into a file
    // FILE *trace_f = fopen(TRACE_FILE, "a");
    int i;
    unsigned int curr_trace_num = trace_gettotalsize() / sizeof(unsigned int);
    unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
    trace_gettotalcon(trace_ram);
    int cur_size = trace_geteachsize(index) / sizeof(unsigned int);
    int cur_off  = trace_geteachoff(index) / sizeof(unsigned int);
    // printf("trace_num: %d, cur_size: %d, cur_off: %d\n", curr_trace_num, cur_size, cur_off);
    while (trace_ram[cur_size + cur_off - 1] == 0) {
        // printf("Re-read\n");
        trace_gettotalcon(trace_ram);
    }
    // fprintf(trace_f, "\n%d %d\n", num_group, MEM_SIZE);
    // fprintf(trace_f, "%08X\n", timer0);
    // fprintf(trace_f, "%08X\n", timer1);
    // printf("%d %d\n", num_group, MEM_SIZE);
    // printf("%08X\n", timer0);
    // printf("%08X\n", timer1);
    for (i = cur_off; i < cur_off + cur_size; i++) {
        // fprintf(trace_f, "%08X\n", trace_ram[i]);
        printf("%08X\n", trace_ram[i]);
    }

    free(trace_ram);
    // fclose(trace_f);    
}


int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Input: %s [Number of Groups] [Input Data Size] [Buffer Size]\n", argv[0]);
        return 0;
    }

    int MEM_SIZE    = atoi(argv[2]) * 1024;
    int BUF_LEN     = atoi(argv[3]);
    int num_group   = atoi(argv[1]);

    int i;
    reg_clr();
    trace_clr();

    int a_addr_add = ddr_malloc(MEM_SIZE * sizeof(int));
    if (a_addr_add < 0) {
        return 0;
    }
    int b_addr_add = ddr_malloc(MEM_SIZE * sizeof(int));
    if (b_addr_add < 0) {
        return 0;
    }
    int c_addr_add = ddr_malloc(MEM_SIZE * sizeof(int));
    if (c_addr_add < 0) {
        return 0;
    }
    int a_addr_mul = ddr_malloc(MEM_SIZE * sizeof(int));
    if (a_addr_mul < 0) {
        return 0;
    }
    int b_addr_mul = ddr_malloc(MEM_SIZE * sizeof(int));
    if (b_addr_mul < 0) {
        return 0;
    }
    int c_addr_mul = ddr_malloc(MEM_SIZE * sizeof(int));
    if (c_addr_mul < 0) {
        return 0;
    }
    printf("a_addr_add:0x%08X\n", a_addr_add);
    printf("b_addr_add:0x%08X\n", b_addr_add);
    printf("c_addr_add:0x%08X\n", c_addr_add);
    printf("a_addr_mul:0x%08X\n", a_addr_mul);
    printf("b_addr_mul:0x%08X\n", b_addr_mul);
    printf("c_addr_mul:0x%08X\n", c_addr_mul);

    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        return -1;
    }
    // printf("begin to map a, b, and c.\n");
    int *a_add = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_add);
    int *b_add = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_add);
    int *c_add = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_add);
    int *a_mul = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_mul);
    int *b_mul = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_mul);
    int *c_mul = mmap(NULL, MEM_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_mul);

    for (i = 0; i < MEM_SIZE; i++) {
        a_add[i] = i + 3;
        b_add[i] = i;
        c_add[i] = 0;
        a_mul[i] = i + 3;
        b_mul[i] = i;
        c_mul[i] = 0;
    }
    // printf("Initialize finished.\n");
    struct hapara_thread_struct sp_add;
    struct hapara_thread_struct sp_mul;

    int ID_NUM  = MEM_SIZE / BUF_LEN / num_group;

    sp_add.priority             = 0;
    sp_add.argv[0]              = a_addr_add;
    sp_add.argv[1]              = b_addr_add;
    sp_add.argv[2]              = c_addr_add;
    sp_add.argv[3]              = BUF_LEN;
    sp_add.group_size.id0       = 1;
    sp_add.group_size.id1       = ID_NUM;
    sp_add.group_num.id0        = 1;
    sp_add.group_num.id1        = num_group;
    sp_add.elf_info.elf_magic   = 'a';

    sp_mul.priority             = 0;
    sp_mul.argv[0]              = a_addr_mul;
    sp_mul.argv[1]              = b_addr_mul;
    sp_mul.argv[2]              = c_addr_mul;
    sp_mul.argv[3]              = BUF_LEN;
    sp_mul.group_size.id0       = 1;
    sp_mul.group_size.id1       = ID_NUM;
    sp_mul.group_num.id0        = 1;
    sp_mul.group_num.id1        = num_group;
    sp_mul.elf_info.elf_magic   = 'm';

    // Allocate trace space and do pr_load into memory
    // Vector_add
    int trace_off_add = trace_alloc(sp_add.group_num);
    sp_add.trace_ram_off = trace_off_add / sizeof(struct hapara_trace_struct);
    char pr_file_path[128];
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, "vector_add");
    printf("Vector_add: pr_file_path: %s\n", pr_file_path);
    int ret_add = pr_loader(pr_file_path, &sp_add.pr_info);
    if (ret_add < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp_add.pr_info);
    print_struct(&sp_add);

    // Allocate trace space and do pr_load into memory
    // Vector_mul    
    int trace_off_mul = trace_alloc(sp_mul.group_num);
    sp_mul.trace_ram_off = trace_off_mul / sizeof(struct hapara_trace_struct);
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, "vector_mul");
    printf("Vector_mul: pr_file_path: %s\n", pr_file_path);
    int ret_mul = pr_loader(pr_file_path, &sp_mul.pr_info);
    if (ret_mul < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp_mul.pr_info);
    print_struct(&sp_mul);
          
    volatile struct hapara_thread_struct *htdt = mmap(NULL, 
                                             1024,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             devmemfd,
                                             ARM_HTDT_BASE);    


    timer_reset();
    timer_start();
    
    unsigned int timer0;
    unsigned int timer1;

    timer_gettime(&timer0);
    ret_mul = reg_add(&sp_mul);
    ret_add = reg_add(&sp_add);
    while (htdt[ret_mul].isValid != 0 || htdt[ret_add].isValid != 0);
    timer_gettime(&timer1);

    printf("%f\n", (timer1 - timer0) / 100000000.0);

    printf("Dump Vector Add traces\n");
    dump_trace(0);
    printf("Dump Vector Mul traces\n");
    dump_trace(1);
    

    ret_add = reg_del(ret_add);
    if (ret_add == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }
    ret_mul = reg_del(ret_mul);
    if (ret_mul == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }

    int error = 0;
    printf("Vector Add Check:\n");
    for (i = 0; i < MEM_SIZE; i++) {
        if (a_add[i] + b_add[i] != c_add[i]) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

    error = 0;
    printf("Vector Mul Check:\n");
    for (i = 0; i < MEM_SIZE; i++) {
        if (a_mul[i] * b_mul[i] != c_mul[i]) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

go_exit:
    munmap(htdt, 1024);
    munmap(a_add, MEM_SIZE * sizeof(int));
    munmap(b_add, MEM_SIZE * sizeof(int));
    munmap(c_add, MEM_SIZE * sizeof(int));
    munmap(a_mul, MEM_SIZE * sizeof(int));
    munmap(b_mul, MEM_SIZE * sizeof(int));
    munmap(c_mul, MEM_SIZE * sizeof(int));
    close(devmemfd);

    
    int ret;
    ret = ddr_free(sp_add.pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: add ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(sp_mul.pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: mul ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(a_addr_add);
    if (ret < 0) {
        printf("apptest: add ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr_add);
    if (ret < 0) {
        printf("apptest: add ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr_add);
    if (ret < 0) {
        printf("apptest: add ddr_free error: c.\n");
        return 0;
    }
    ret = ddr_free(a_addr_mul);
    if (ret < 0) {
        printf("apptest: mul ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr_mul);
    if (ret < 0) {
        printf("apptest: mul ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr_mul);
    if (ret < 0) {
        printf("apptest: mul ddr_free error: c.\n");
        return 0;
    }  
    printf("apptest end!\n");
    return 0;
}


