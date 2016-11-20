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
#define NUM_APP         4
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

// #define SIMPLE

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Input: %s [Number of Groups] [HW Config]\n", argv[0]);
        printf("HW Config: 1g4s4h\n");
        printf("HW Config: 2g4s8h\n");
        printf("HW Config: 4g2s8h\n");
        return 0;
    }
#ifdef SIMPLE
    int MEM_SIZE_VM     = 256 * 1024;
    int MEM_SIZE_PR     = 512;
    int BUF_LEN         = 32;
#endif

    int num_group   = atoi(argv[1]);

    int i;
    reg_clr();
    trace_clr();

#ifndef SIMPLE
    struct hapara_thread_struct sp[NUM_APP];

    struct app_t app_seq[NUM_APP];
    unsigned int sequence[NUM_APP];
    init_app_seq(app_seq);
    random_app(sequence);


    pr_info_t  pr_info[NUM_APP_TYPE];
    char pr_file_path[128];

    for (i = 0; i < NUM_APP_TYPE; i++) {
        int ret;
        strcpy(pr_file_path, PR_ROOT_PATH);
        strcat(pr_file_path, argv[2]);
        strcat(pr_file_path, "/");
        strcat(pr_file_path, app_name[i]);
        printf("pr_file_path: %s\n", pr_file_path);
        ret = pr_loader(pr_file_path, &(pr_info[i]));
        if (ret < 0) {
            printf("apptes: pr_loader error: %s\n", app_name[i]);
            return 0;
        }
        printf("  Addr: 0x%08X\n", pr_info[i].ddr_addr);
    }

    for (i = 0; i < NUM_APP; i++) {
        unsigned int index = sequence[i];
        char magic                  = app_seq[index].app_name[0];
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
                sp[i].group_size.id1        = data_size / num_group / 2;
                sp[i].group_num.id0         = 1;
                sp[i].group_num.id1         = num_group;
                sp[i].elf_info.elf_magic    = magic;
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
                sp[i].pr_info.ddr_addr      = pr_info[3].ddr_addr;
                sp[i].pr_info.num_pr_file   = pr_info[3].num_pr_file;
                sp[i].pr_info.each_size     = pr_info[3].each_size;
                sp[i].trace_ram_off         = trace_alloc(sp[i].group_num) / sizeof(struct hapara_trace_struct);
                break;
            default:
                printf("Magic number is wrong.\n");
                break;
        }
    }
    int ret;
    int ret_num[NUM_APP];
    hapara_timer_reset();
    hapara_timer_start();
    ret = reg_add_all(sp, ret_num, NUM_APP);

    for (i = 0; i < NUM_APP; i++) {
        switch (sp[i].elf_info.elf_magic) {
            case 'v':
                printf("%s\n", app_name[0]);
                break;
            case 'p':
                printf("%s\n", app_name[1]);
                break;
            case 'm':
                printf("%s\n", app_name[2]);
                break;
            case 'f':
                printf("%s\n", app_name[3]);
                break;
            default:
                printf("magic error.\n");
                break;
        }
        print_struct(&sp[i]);
    }
    for (i = 0; i < NUM_APP; i++) {
        printf("Waiting..%d\n", i);
        dump_trace(i);
    }
    return 0;
#endif;

#ifdef SIMPLE
    int a_addr_sub = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (a_addr_sub < 0) {
        return 0;
    }
    int b_addr_sub = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (b_addr_sub < 0) {
        return 0;
    }
    int c_addr_sub = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (c_addr_sub < 0) {
        return 0;
    }
    int a_addr_mul = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (a_addr_mul < 0) {
        return 0;
    }
    int b_addr_mul = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (b_addr_mul < 0) {
        return 0;
    }
    int c_addr_mul = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (c_addr_mul < 0) {
        return 0;
    }
    int a_addr_add = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (a_addr_add < 0) {
        return 0;
    }
    int b_addr_add = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (b_addr_add < 0) {
        return 0;
    }
    int c_addr_add = ddr_malloc(MEM_SIZE_VM * sizeof(int));
    if (c_addr_add < 0) {
        return 0;
    }
    int a_addr_pr = ddr_malloc(MEM_SIZE_PR * MEM_SIZE_PR * sizeof(float));
    if (a_addr_pr < 0) {
        return 0;
    }
    int b_addr_pr = ddr_malloc(MEM_SIZE_PR * sizeof(float));
    if (b_addr_pr < 0) {
        return 0;
    }
    int c_addr_pr = ddr_malloc(MEM_SIZE_PR * sizeof(float));
    if (c_addr_pr < 0) {
        return 0;
    }

    printf("a_addr_sub:0x%08X\n", a_addr_sub);
    printf("b_addr_sub:0x%08X\n", b_addr_sub);
    printf("c_addr_sub:0x%08X\n", c_addr_sub);
    printf("a_addr_mul:0x%08X\n", a_addr_mul);
    printf("b_addr_mul:0x%08X\n", b_addr_mul);
    printf("c_addr_mul:0x%08X\n", c_addr_mul);
    printf("a_addr_add:0x%08X\n", a_addr_add);
    printf("b_addr_add:0x%08X\n", b_addr_add);
    printf("c_addr_add:0x%08X\n", c_addr_add);
    printf("a_addr_pr :0x%08X\n", a_addr_pr);
    printf("b_addr_pr :0x%08X\n", b_addr_pr);
    printf("c_addr_pr :0x%08X\n", c_addr_pr);
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        return -1;
    }
    // printf("begin to map a, b, and c.\n");
    int *a_sub = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_sub);
    int *b_sub = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_sub);
    int *c_sub = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_sub);
    int *a_mul = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_mul);
    int *b_mul = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_mul);
    int *c_mul = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_mul);
    int *a_add = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_add);
    int *b_add = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_add);
    int *c_add = mmap(NULL, MEM_SIZE_VM * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_add);
    int *a_pr  = mmap(NULL, MEM_SIZE_PR * MEM_SIZE_PR * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, a_addr_pr);
    int *b_pr  = mmap(NULL, MEM_SIZE_PR * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, b_addr_pr);
    int *c_pr  = mmap(NULL, MEM_SIZE_PR * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, c_addr_pr);
    for (i = 0; i < MEM_SIZE_VM; i++) {
        a_mul[i] = i + 3;
        b_mul[i] = i;
        c_mul[i] = 0;
        a_sub[i] = i + 4;
        b_sub[i] = i;
        c_sub[i] = 0;
        a_add[i] = i + 5;
        b_add[i] = i;
        c_add[i] = 0;        
    }
    for (i = 0; i < MEM_SIZE_PR * MEM_SIZE_PR; i++) {
        a_pr[i] = i + 1.3f;
    }
    for (i = 0; i < MEM_SIZE_PR; i++) {
        b_pr[i] = 1.0f / MEM_SIZE_PR;
        c_pr[i] = 0.0;
    }   
    // printf("Initialize finished.\n");
    struct hapara_thread_struct sp[4];


    int ID_NUM      = MEM_SIZE_VM / BUF_LEN / num_group;
    int ID_NUM_PR   = MEM_SIZE_PR / num_group / 2;

    sp[0].priority             = 0;
    sp[0].argv[0]              = a_addr_sub;
    sp[0].argv[1]              = b_addr_sub;
    sp[0].argv[2]              = c_addr_sub;
    sp[0].argv[3]              = BUF_LEN;
    sp[0].cur_group_id.id0     = 0;
    sp[0].cur_group_id.id1     = 0;
    sp[0].group_size.id0       = 1;
    sp[0].group_size.id1       = ID_NUM;
    sp[0].group_num.id0        = 1;
    sp[0].group_num.id1        = num_group;
    sp[0].elf_info.elf_magic   = 's'; 

    sp[1].priority             = 0;
    sp[1].argv[0]              = a_addr_mul;
    sp[1].argv[1]              = b_addr_mul;
    sp[1].argv[2]              = c_addr_mul;
    sp[1].argv[3]              = BUF_LEN;
    sp[1].cur_group_id.id0     = 0;
    sp[1].cur_group_id.id1     = 0;
    sp[1].group_size.id0       = 1;
    sp[1].group_size.id1       = ID_NUM;
    sp[1].group_num.id0        = 1;
    sp[1].group_num.id1        = num_group;
    sp[1].elf_info.elf_magic   = 'm';

    sp[2].priority             = 0;
    sp[2].argv[0]              = a_addr_add;
    sp[2].argv[1]              = b_addr_add;
    sp[2].argv[2]              = c_addr_add;
    sp[2].argv[3]              = BUF_LEN;
    sp[2].cur_group_id.id0     = 0;
    sp[2].cur_group_id.id1     = 0;
    sp[2].group_size.id0       = 1;
    sp[2].group_size.id1       = ID_NUM;
    sp[2].group_num.id0        = 1;
    sp[2].group_num.id1        = num_group;
    sp[2].elf_info.elf_magic   = 'a';

    sp[3].priority              = 0;
    sp[3].argv[0]               = a_addr_pr;
    sp[3].argv[1]               = b_addr_pr;
    sp[3].argv[2]               = c_addr_pr;
    sp[3].argv[3]               = MEM_SIZE_PR;
    sp[3].argv[4]               = BUF_LEN;
    sp[3].cur_group_id.id0      = 0;
    sp[3].cur_group_id.id1      = 0;    
    sp[3].group_size.id0        = 1;
    sp[3].group_size.id1        = ID_NUM_PR;
    sp[3].group_num.id0         = 1;
    sp[3].group_num.id1         = num_group;
    sp[3].elf_info.elf_magic   = 'f'; 

    // Allocate trace space and do pr_load into memory
    // Vector_sub
    int trace_off_sub = trace_alloc(sp[0].group_num);
    sp[0].trace_ram_off = trace_off_sub / sizeof(struct hapara_trace_struct);
    char pr_file_path[128];
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, argv[2]);
    strcat(pr_file_path, "/vector_sub");
    printf("Vector_sub: pr_file_path: %s\n", pr_file_path);
    int ret_sub = pr_loader(pr_file_path, &sp[0].pr_info);
    if (ret_sub < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp[0].pr_info);
    print_struct(&sp[0]);

    // Allocate trace space and do pr_load into memory
    // Vector_mul    
    int trace_off_mul = trace_alloc(sp[1].group_num);
    sp[1].trace_ram_off = trace_off_mul / sizeof(struct hapara_trace_struct);
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, argv[2]);
    strcat(pr_file_path, "/vector_mul");
    printf("Vector_mul: pr_file_path: %s\n", pr_file_path);
    int ret_mul = pr_loader(pr_file_path, &sp[1].pr_info);
    if (ret_mul < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp[1].pr_info);
    print_struct(&sp[1]);

    // Vector add
    int trace_off_add = trace_alloc(sp[2].group_num);
    sp[2].trace_ram_off = trace_off_add / sizeof(struct hapara_trace_struct);
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, argv[2]);
    strcat(pr_file_path, "/vector_add");
    printf("Vector_add: pr_file_path: %s\n", pr_file_path);
    int ret_add = pr_loader(pr_file_path, &sp[2].pr_info);
    if (ret_add < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp[2].pr_info);
    print_struct(&sp[2]);

    // Pageranking
    int trace_off_pr = trace_alloc(sp[3].group_num);
    sp[3].trace_ram_off = trace_off_pr / sizeof(struct hapara_trace_struct);
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, argv[2]);
    strcat(pr_file_path, "/pageranking");
    printf("Pageranking: pr_file_path: %s\n", pr_file_path);
    int ret_pr = pr_loader(pr_file_path, &sp[3].pr_info);
    if (ret_pr < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }
    // disable_mb_pr(&sp[3].pr_info);
    print_struct(&sp[3]);

    volatile struct hapara_thread_struct *htdt = mmap(NULL, 
                                             4 * sizeof(struct hapara_thread_struct),
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             devmemfd,
                                             ARM_HTDT_BASE);    

    int ret;
    hapara_timer_reset();
    hapara_timer_start();
    
    unsigned int timer0;
    unsigned int timer1;

    hapara_timer_gettime(&timer0);
    // ret_sub = reg_add(&sp[0]);
    // ret_mul = reg_add(&sp[1]);
    // ret_add = reg_add(&sp[2]);
    // ret_pr  = reg_add(&sp[3]);
    int ret_num[4];
    ret = reg_add_all(sp, ret_num, 4);

    // while (htdt[ret_mul].isValid != 0 
            // || 
           // htdt[ret_sub].isValid != 0 
            // ||
           // htdt[ret_add].isValid != 0
           // );
    // sleep(1);
    hapara_timer_gettime(&timer1);

    printf("%f\n", (timer1 - timer0) / 100000000.0);

    printf("Dump Vector Sub traces\n");
    dump_trace(0);
    printf("Dump Vector Mul traces\n");
    dump_trace(1);
    printf("Dump Vector Add traces\n");
    dump_trace(2);   
    printf("Dump Pageranking traces\n");
    dump_trace(3);   

    ret_sub = reg_del(ret_sub);
    if (ret_sub == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }
    ret_mul = reg_del(ret_mul);
    if (ret_mul == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }

    ret_add = reg_del(ret_add);
    if (ret_add == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }

    ret_pr  = reg_del(ret_pr);
    if (ret_pr == -1) {
        printf("apptest: Reg del error 0.\n");
        return 0;
    }

    int error = 0;
    printf("Vector Sub Check:\n");
    for (i = 0; i < MEM_SIZE_VM; i++) {
        if (a_sub[i] - b_sub[i] != c_sub[i]) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

    error = 0;
    printf("Vector Mul Check:\n");
    for (i = 0; i < MEM_SIZE_VM; i++) {
        if (a_mul[i] * b_mul[i] != c_mul[i]) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

    error = 0;
    printf("Vector Add Check:\n");
    for (i = 0; i < MEM_SIZE_VM; i++) {
        if (a_add[i] + b_add[i] != c_add[i]) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

    int j, k;
    error = 0;
    printf("Pageranking Check:\n");
    for (i = 0; i < MEM_SIZE_PR; i++) {
        float sum = 0;
        for (j = 0; j < MEM_SIZE_PR; j++) {
            sum += a_pr[i * MEM_SIZE_PR + j] * b_pr[j];
        }
        if ( *((unsigned *)&sum) != *((unsigned *)&(c_pr[i])) ) {
            error++;
        }
    }
    printf("Number of Error: %d\n", error);

go_exit:
    munmap(htdt, 4 * sizeof(struct hapara_thread_struct));
    munmap(a_sub, MEM_SIZE_VM * sizeof(int));
    munmap(b_sub, MEM_SIZE_VM * sizeof(int));
    munmap(c_sub, MEM_SIZE_VM * sizeof(int));
    munmap(a_mul, MEM_SIZE_VM * sizeof(int));
    munmap(b_mul, MEM_SIZE_VM * sizeof(int));
    munmap(c_mul, MEM_SIZE_VM * sizeof(int));
    munmap(a_add, MEM_SIZE_VM * sizeof(int));
    munmap(b_add, MEM_SIZE_VM * sizeof(int));
    munmap(c_add, MEM_SIZE_VM * sizeof(int));
    munmap(a_pr, MEM_SIZE_PR * MEM_SIZE_PR * sizeof(float));
    munmap(b_pr, MEM_SIZE_PR * sizeof(float));
    munmap(c_pr, MEM_SIZE_PR * sizeof(float));
    close(devmemfd);

    

    ret = ddr_free(sp[2].pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: add ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(sp[1].pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: mul ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(sp[0].pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: sub ddr_free error: pr.\n");
        return 0;
    }
    ret = ddr_free(sp[3].pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: pr ddr_free error: pr.\n");
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
    ret = ddr_free(a_addr_sub);
    if (ret < 0) {
        printf("apptest: sub ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr_sub);
    if (ret < 0) {
        printf("apptest: sub ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr_sub);
    if (ret < 0) {
        printf("apptest: sub ddr_free error: c.\n");
        return 0;
    }  
    ret = ddr_free(a_addr_pr);
    if (ret < 0) {
        printf("apptest: pr ddr_free error: a.\n");
        return 0;
    }   
    ret = ddr_free(b_addr_pr);
    if (ret < 0) {
        printf("apptest: pr ddr_free error: b.\n");
        return 0;
    }  
    ret = ddr_free(c_addr_pr);
    if (ret < 0) {
        printf("apptest: pr ddr_free error: c.\n");
        return 0;
    }  
    printf("apptest end!\n");
    return 0;
#endif
}


