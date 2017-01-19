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

#define ELF_LOAD_ADDR   ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

#define PR_ROOT_PATH    "/mnt/pr_files/"
#define ELF_FILE_NAME   "/mnt/elf_apps/"

// #define TRACE_FILE      "/mnt/testscript/trace.txt"
#define DEVMEM          "/dev/mem"

struct app_t {
    char *hpp_name;
    char *spp_name;
    unsigned int app_index;
    unsigned char priority;
    unsigned int data_size;
    unsigned int buffer_size;
};
#define NUM_APP         16
#define NUM_APP_TYPE    4

char spp_name[NUM_APP_TYPE][128] = {{"vector_mul"},
                                    {"pageranking"},
                                    {"matrix_mul"},
                                    {"fir"}};
char hpp_name[NUM_APP_TYPE][128] = {{"vector_mul1buf"},
                                    {"pageranking1buf"},
                                    {"matrix_mul1buf"},
                                    {"fir1buf"}};

void init_app_seq(struct app_t *app_seq) {
    int i;
    for (i = 0; i < NUM_APP; i++) {
        unsigned int index = i / ( NUM_APP / NUM_APP_TYPE);
        app_seq[i].hpp_name = hpp_name[index];
        app_seq[i].spp_name = spp_name[index];
        app_seq[i].app_index = index;
        app_seq[i].priority = i % 2;
        switch (index) {
            case 0:    // vector_mul
                app_seq[i].data_size    = 32 * 1024;
                app_seq[i].buffer_size  = 1;
                break;
            case 1:    // pageranking
                app_seq[i].data_size    = 1024;
                app_seq[i].buffer_size  = 1;
                break;
            case 2:    // matrix_mul
                app_seq[i].data_size    = 256;
                app_seq[i].buffer_size  = 1;
                break;
            case 3:    //fir
                app_seq[i].data_size    = 32 * 1024;
                app_seq[i].buffer_size  = 1;
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
        printf("  app[%2d].hname:       %s\n", sequence[i], app_seq[sequence[i]].hpp_name);
        printf("  app[%2d].sname:       %s\n", sequence[i], app_seq[sequence[i]].spp_name);
        printf("  app[%2d].priority:    %d\n", sequence[i], app_seq[sequence[i]].priority);
        printf("  app[%2d].data_size:   %d\n", sequence[i], app_seq[sequence[i]].data_size);
        printf("  app[%2d].buffer_size: %d\n", sequence[i], app_seq[sequence[i]].buffer_size);
    }
    for (i = 0; i < NUM_APP; i++) {
        printf("%2d,", sequence[i]);
    }
    printf("\n");
}

void dump_trace(int index, char *TRACE_FILE) {
    FILE *trace_f = fopen(TRACE_FILE, "a");
    fprintf(trace_f, "%d\n", index);
    int i;
    unsigned int curr_trace_num = trace_gettotalsize() / sizeof(unsigned int);
    if (curr_trace_num * 4 > 64 * 1024) {
        printf("ERROR: curr_trace_num exceeds 64KByte: %d\n", curr_trace_num);
        return;
    }
    unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
    if (trace_ram == NULL) {
        printf("trace_ram allocation out of memory.\n");
        return;
    }
    trace_gettotalcon(trace_ram);
    int cur_size = trace_geteachsize(index) / sizeof(unsigned int);
    int cur_off  = trace_geteachoff(index) / sizeof(unsigned int);
    while (trace_ram[cur_size + cur_off - 1] == 0) {
        trace_gettotalcon(trace_ram);
    }
    for (i = cur_off; i < cur_off + cur_size; i++) {
        fprintf(trace_f, "%08X\n", trace_ram[i]);
    }

    free(trace_ram);
    fclose(trace_f);    
}

// #define SIMPLE

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Input: %s [Number of Groups] [HW Config]\n", argv[0]);
        printf("HW Config: 1g4s2h\n");
        printf("HW Config: 2g4s4h\n");
        printf("HW Config: 4g4s8h\n");
        return 0;
    }

    int num_group   = atoi(argv[1]);

    int ret;
    int i;
    reg_clr();
    trace_clr();

    struct hapara_thread_struct sp[NUM_APP];
    int ret_num[NUM_APP];
    struct app_t app_seq[NUM_APP];
    unsigned int sequence[NUM_APP] = {7, 9, 14, 11, 8, 1, 5, 6, 3, 10, 15, 2, 4, 0, 13, 12};
    init_app_seq(app_seq);
    // random_app(sequence);


    char trace_file_name[128];
    char sw_group[] = "_0.txt";
    strcpy(trace_file_name, "/mnt/testscript/trace_");
    strcat(trace_file_name, argv[2]);
    strcat(trace_file_name, "_");
    strcat(trace_file_name, argv[1]);
    strcat(trace_file_name, ".txt");
    printf("%s\n", trace_file_name);

    pr_info_t   pr_info[NUM_APP_TYPE];
    elf_info_t  elf_info[NUM_APP_TYPE];
    char pr_file_path[128];
    char elf_file_path[128];
    for (i = 0; i < NUM_APP_TYPE; i++) {
        int ret;
        strcpy(pr_file_path, PR_ROOT_PATH);
        strcat(pr_file_path, argv[2]);
        strcat(pr_file_path, "/");
        strcat(pr_file_path, hpp_name[i]);
        printf("pr_file_path: %s\n", pr_file_path);
        ret = pr_loader(pr_file_path, &(pr_info[i]));
        if (ret < 0) {
            printf("apptes: pr_loader error: %s\n", hpp_name[i]);
            return 0;
        }
        printf("  PR Addr: 0x%08X\n", pr_info[i].ddr_addr);

        strcpy(elf_file_path, ELF_FILE_NAME);
        strcat(elf_file_path, spp_name[i]);
        strcat(elf_file_path, ".elf");
        printf("elf_file_path: %s\n", elf_file_path);
        ret = elf_loader(elf_file_path, ELF_START_ADDR, &(elf_info[i]));
        if (ret < 0) {
            printf("apptest: elf_loader error\n");
            return 0;
        }
        printf("  ELF size: 0x%08X\n", elf_info[i].thread_size);
        printf("  ELF addr: 0x%08X\n", elf_info[i].ddr_addr);
        printf("  ELF dmas: 0x%08X\n", elf_info[i].DMA_size);
    }

    for (i = 0; i < NUM_APP; i++) {
        unsigned int index = sequence[i];
        char magic                  = app_seq[index].hpp_name[0];
        unsigned int app_index      = app_seq[index].app_index;
        unsigned char priority      = app_seq[index].priority;
        unsigned int data_size      = app_seq[index].data_size;
        unsigned int buffer_size    = app_seq[index].buffer_size;
        switch (magic) {
            case 'v':   // vector_mul
                sp[i].priority              = priority;
                sp[i].argv[0]               = ddr_malloc(data_size * sizeof(int));
                sp[i].argv[1]               = ddr_malloc(data_size * sizeof(int));
                sp[i].argv[2]               = ddr_malloc(data_size * sizeof(int));
                sp[i].argv[3]               = buffer_size;
                sp[i].cur_group_id.id0      = 0;
                sp[i].cur_group_id.id1      = 0;
                sp[i].group_size.id0        = 1;
                sp[i].group_size.id1        = data_size / buffer_size / num_group;
                sp[i].group_num.id0         = 1;
                sp[i].group_num.id1         = num_group;
                sp[i].elf_info.elf_magic    = magic;
                sp[i].elf_info.main_addr    = elf_info[0].main_addr;
                sp[i].elf_info.stack_addr   = elf_info[0].stack_addr;
                sp[i].elf_info.thread_size  = elf_info[0].thread_size;
                sp[i].elf_info.ddr_addr     = elf_info[0].ddr_addr;
                sp[i].elf_info.DMA_size     = elf_info[0].DMA_size;
                sp[i].pr_info.ddr_addr      = pr_info[0].ddr_addr;
                sp[i].pr_info.num_pr_file   = pr_info[0].num_pr_file;
                sp[i].pr_info.each_size     = pr_info[0].each_size;
                sp[i].trace_ram_off         = trace_alloc(sp[i].group_num) / sizeof(struct hapara_trace_struct);
                break;
            case 'p':   // pageranking
                sp[i].priority              = priority;
                sp[i].argv[0]               = ddr_malloc(data_size * data_size * sizeof(float));
                sp[i].argv[1]               = ddr_malloc(data_size * sizeof(float));
                sp[i].argv[2]               = ddr_malloc(data_size * sizeof(float));
                sp[i].argv[3]               = data_size;
                sp[i].argv[4]               = buffer_size;
                sp[i].cur_group_id.id0      = 0;
                sp[i].cur_group_id.id1      = 0;
                sp[i].group_size.id0        = 1;
                sp[i].group_size.id1        = data_size / num_group;
                sp[i].group_num.id0         = 1;
                sp[i].group_num.id1         = num_group;
                sp[i].elf_info.elf_magic    = magic;
                sp[i].elf_info.main_addr    = elf_info[1].main_addr;
                sp[i].elf_info.stack_addr   = elf_info[1].stack_addr;
                sp[i].elf_info.thread_size  = elf_info[1].thread_size;
                sp[i].elf_info.ddr_addr     = elf_info[1].ddr_addr;
                sp[i].elf_info.DMA_size     = elf_info[1].DMA_size;
                sp[i].pr_info.ddr_addr      = pr_info[1].ddr_addr;
                sp[i].pr_info.num_pr_file   = pr_info[1].num_pr_file;
                sp[i].pr_info.each_size     = pr_info[1].each_size;
                sp[i].trace_ram_off         = trace_alloc(sp[i].group_num) / sizeof(struct hapara_trace_struct);
                break;
            case 'm':   // matrix_mul
                sp[i].priority              = priority;
                sp[i].argv[0]               = ddr_malloc(data_size * data_size * sizeof(int));
                sp[i].argv[1]               = ddr_malloc(data_size * data_size * sizeof(int));
                sp[i].argv[2]               = ddr_malloc(data_size * data_size * sizeof(int));
                sp[i].argv[3]               = data_size;
                sp[i].argv[4]               = buffer_size;
                sp[i].cur_group_id.id0      = 0;
                sp[i].cur_group_id.id1      = 0;
                sp[i].group_size.id0        = data_size / buffer_size / num_group;
                sp[i].group_size.id1        = data_size / buffer_size / num_group;
                sp[i].group_num.id0         = num_group;
                sp[i].group_num.id1         = num_group;
                sp[i].elf_info.elf_magic    = magic;
                sp[i].elf_info.main_addr    = elf_info[2].main_addr;
                sp[i].elf_info.stack_addr   = elf_info[2].stack_addr;
                sp[i].elf_info.thread_size  = elf_info[2].thread_size;
                sp[i].elf_info.ddr_addr     = elf_info[2].ddr_addr;
                sp[i].elf_info.DMA_size     = elf_info[2].DMA_size;
                sp[i].pr_info.ddr_addr      = pr_info[2].ddr_addr;
                sp[i].pr_info.num_pr_file   = pr_info[2].num_pr_file;
                sp[i].pr_info.each_size     = pr_info[2].each_size;
                sp[i].trace_ram_off         = trace_alloc(sp[i].group_num) / sizeof(struct hapara_trace_struct);
                break;
            case 'f':   // fir
                sp[i].priority              = priority;
                sp[i].argv[0]               = ddr_malloc(data_size * sizeof(int));
                sp[i].argv[1]               = ddr_malloc(data_size * sizeof(int));
                sp[i].argv[2]               = buffer_size;
                sp[i].cur_group_id.id0      = 0;
                sp[i].cur_group_id.id1      = 0;
                sp[i].group_size.id0        = 1;
                sp[i].group_size.id1        = data_size / buffer_size / num_group;
                sp[i].group_num.id0         = 1;
                sp[i].group_num.id1         = num_group;
                sp[i].elf_info.elf_magic    = magic;
                sp[i].elf_info.main_addr    = elf_info[3].main_addr;
                sp[i].elf_info.stack_addr   = elf_info[3].stack_addr;
                sp[i].elf_info.thread_size  = elf_info[3].thread_size;
                sp[i].elf_info.ddr_addr     = elf_info[3].ddr_addr;
                sp[i].elf_info.DMA_size     = elf_info[3].DMA_size;
                sp[i].pr_info.ddr_addr      = pr_info[3].ddr_addr;
                sp[i].pr_info.num_pr_file   = pr_info[3].num_pr_file;
                sp[i].pr_info.each_size     = pr_info[3].each_size;
                sp[i].trace_ram_off         = trace_alloc(sp[i].group_num) / sizeof(struct hapara_trace_struct);
                break;
            default:
                printf("Magic number is wrong.\n");
                break;
        }
        printf("%s\n", app_seq[index].hpp_name); 
        printf("%s\n", app_seq[index].spp_name); 
        print_struct(&sp[i]);
    }

    // -----------------------------------------------------------
    // --------------------Do Priority------------------------
    // Every entry has the same priority. Magic is good.   
    // printf("Do Priority.............\n");
    // hapara_timer_reset();
    // hapara_timer_start();
    // ret = reg_add_all(sp, ret_num, NUM_APP);
    // for (i = 0; i < NUM_APP; i++) {
    //     printf("Waiting..%d\n", i);
    //     dump_trace(i, trace_file_name);
    // }
    // // Delete htdt register
    // for (i = 0; i < NUM_APP; i++) {
    //     ret = reg_del(ret_num[i]);
    //     if (ret == -1) {
    //         printf("Reg del error %d.\n", i);
    //         return 0;
    //     }
    // }

    // -----------------------------------------------------------
    // --------------------Do BaselinePlus------------------------
    // Every entry has the same priority. Magic is good.
    // printf("Do BaselinePlus.............\n");
    for (i = 0; i < NUM_APP; i++) {
        sp[i].priority = 0;
        print_struct(&sp[i]);
    }
    hapara_timer_reset();
    hapara_timer_start();
    ret = reg_add_all(sp, ret_num, NUM_APP);
    for (i = 0; i < NUM_APP; i++) {
        printf("Waiting..%d\n", i);
        dump_trace(i, trace_file_name);
    }    
    // Delete htdt register
    for (i = 0; i < NUM_APP; i++) {
        ret = reg_del(ret_num[i]);
        if (ret == -1) {
            printf("Reg del error %d.\n", i);
            return 0;
        }
    }
    // -----------------------------------------------------------
    // --------------------Do Baseline------------------------
    // Every entry has the same priority. Magic does not work.
    // printf("Do Baseline.............\n");
    // for (i = 0; i < NUM_APP; i++) {
    //     sp[i].priority = i + 1;
    //     sp[i].elf_info.elf_magic = i + 1; 
    //     print_struct(&sp[i]);
    // }
    // hapara_timer_reset();
    // hapara_timer_start();
    // ret = reg_add_all(sp, ret_num, NUM_APP);
    // for (i = 0; i < NUM_APP; i++) {
    //     printf("Waiting..%d\n", i);
    //     dump_trace(i, trace_file_name);
    // }    
    // // Delete htdt register
    // for (i = 0; i < NUM_APP; i++) {
    //     ret = reg_del(ret_num[i]);
    //     if (ret == -1) {
    //         printf("Reg del error %d.\n", i);
    //         return 0;
    //     }
    // }

    // unregister ddr_malloc
    for (i = 0; i < NUM_APP; i++) {
        unsigned int index = sequence[i];
        unsigned int app_index      = app_seq[index].app_index;
        switch (app_index) {
            case 3:     // fir
                ret = ddr_free(sp[i].argv[0]);
                if (ret < 0) {
                    printf("argv[0] ddr_free error: %d\n", i);
                    return 0;
                }
                ret = ddr_free(sp[i].argv[1]);
                if (ret < 0) {
                    printf("argv[1] ddr_free error: %d\n", i);
                    return 0;
                }  
                break;
            default:    // others
                ret = ddr_free(sp[i].argv[0]);
                if (ret < 0) {
                    printf("argv[0] ddr_free error: %d\n", i);
                    return 0;
                }
                ret = ddr_free(sp[i].argv[1]);
                if (ret < 0) {
                    printf("argv[1] ddr_free error: %d\n", i);
                    return 0;
                }  
                ret = ddr_free(sp[i].argv[2]);
                if (ret < 0) {
                    printf("argv[2] ddr_free error: %d\n", i);
                    return 0;
                }  
                break;
        }
    }
    // free pr
    for (i = 0; i < NUM_APP_TYPE; i++) {
        ret = ddr_free(pr_info[i].ddr_addr);
        if (ret < 0) {
            printf("pr ddr_free error: %d\n", i);
            return 0;
        }        
    }
    // free elf
    for (i = 0; i < NUM_APP_TYPE; i++) {
        ret = ddr_free(elf_info[i].ddr_addr);
        if (ret < 0) {
            printf("elf ddr_free error: %d\n", i);
            return 0;
        }        
    }
    return 0;
}


