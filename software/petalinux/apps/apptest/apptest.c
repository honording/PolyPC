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

#define	ELF_LOAD_ADDR	ARM_DDR_BASE
#define ELF_START_ADDR  SLAVE_INST_MEM_BASE

// #define MEM_SIZE        4096
#define DEVMEM          "/dev/mem"

// #define BUF_LEN     32
// #define ID_NUM      (MEM_SIZE / BUF_LEN)
#define TAP         5


#define XPAR_PRC_0_BASEADDR         0x42000000
#define PRC_ICAP_SPAN               0xFFFF    //64KB
#define     rm_math_STATUS        0X00000
#define     rm_math_CONTROL       0X00000
#define     rm_math_SW_TRIGGER    0X00004
#define     rm_math_TRIGGER0      0X00020
#define     rm_math_RM_ADDRESS0   0X00040
#define     rm_math_RM_CONTROL0   0X00044
#define     rm_math_BS_ID0        0X00060
#define     rm_math_BS_ADDRESS0   0X00064
#define     rm_math_BS_SIZE0      0X00068

void Xil_Out32(unsigned int *icap, unsigned int off, unsigned int val) { 
    icap[off >> 2] = val;
}

unsigned int Xil_In32(unsigned int *icap, unsigned int off) {
    return icap[off >> 2];
}

int do_pr(int pr_size, unsigned int pr_begin_addr, unsigned int vsm) {
    unsigned int vsm_off = vsm << 7;
    printf("PR DDR address:0x%X; pr_size: %d; vsm: %d\n", pr_begin_addr, pr_size, vsm);
    int devmemfd = open(DEVMEM, O_RDWR | O_SYNC);
    if (devmemfd < 1) {
        printf("apploadpr: devmem open failed.\n");
        return 0;
    }
    unsigned int *icap = mmap(NULL, PRC_ICAP_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, XPAR_PRC_0_BASEADDR);

    unsigned int Status;
    unsigned int loading_done=0;

    Xil_Out32(icap,vsm_off | rm_math_CONTROL,0);
    while(!(Xil_In32(icap,vsm_off | rm_math_STATUS)&0x80));
    Xil_Out32(icap,vsm_off | rm_math_BS_ADDRESS0,  pr_begin_addr + vsm * pr_size);
    Xil_Out32(icap,vsm_off | rm_math_BS_SIZE0,   pr_size);
    Xil_Out32(icap,vsm_off | rm_math_TRIGGER0,0);
    Xil_Out32(icap,vsm_off | rm_math_RM_ADDRESS0,0);
    Xil_Out32(icap,vsm_off | rm_math_RM_CONTROL0,0x1FF0);

    printf("Adder RM address  = %x\n",Xil_In32(icap,vsm_off | rm_math_BS_ADDRESS0));
    printf("Adder RM size     = %d Bytes\n",Xil_In32(icap,vsm_off | rm_math_BS_SIZE0));
    Xil_Out32(icap,vsm_off | rm_math_CONTROL,2);
    Status=Xil_In32(icap,vsm_off | rm_math_SW_TRIGGER);
    if(!(Status&0x8000)) {
      Xil_Out32(icap,vsm_off | rm_math_SW_TRIGGER,0);
    }
    loading_done = 0;
    while(!loading_done) {
        Status=Xil_In32(icap,vsm_off | rm_math_STATUS)&0x07;
        switch(Status) {
            case 7 : 
                printf("RM loaded\n"); 
                loading_done=1; 
                break;
            case 6 : 
                printf("RM is being reset\n"); 
                break;
            case 5 : 
                printf("Software start-up step\n"); 
                break;
            case 4 : 
                // printf("."); 
                break;
            case 2 : 
                printf("Software shutdown\n"); 
                break;
            case 1 : 
                printf("Hardware shutdown\n"); 
                break;
            default:
                break;
        }
    }
    printf("VADD Reconfiguration Completed!\n\n");

pr_exit:
    munmap(icap, PRC_ICAP_SPAN);
    close(devmemfd);
    return 1;
}



int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("Input: %s [Number of Groups] [Input Data Size] [Buffer Size] [Benchmark]\n", argv[0]);
        return 0;
    }
    int i;
    reg_clr();
    trace_clr();

    int MEM_SIZE = atoi(argv[2]) * 1024;
    int BUF_LEN = atoi(argv[3]);
    int num_group = atoi(argv[1]);

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

    sp.argv[0] = a_addr;
    // sp.argv[1] = BUF_LEN;
    sp.argv[1] = b_addr;
    // sp.argv[2] = BUF_LEN;
    sp.argv[2] = c_addr;
    sp.argv[3] = BUF_LEN;
    sp.group_size.id0 = 1;
    sp.group_size.id1 = ID_NUM;
    sp.group_num.id0 = 1;
    sp.group_num.id1 = num_group;
    sp.elf_info.elf_magic = 'v';
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
    strcpy(pr_file_path, PR_ROOT_PATH);
    strcat(pr_file_path, argv[4]);
    ret = pr_loader(pr_file_path, &sp.pr_info);
    printf("PR FILE PATH: %s\n", pr_file_path);
    if (ret < 0) {
        printf("apptes: pr_loader error\n");
        return 0;
    }

    printf("Do PR.\n");
    for (i = 0; i < 4; i++) {
        // do_pr(sp.pr_info.each_size, sp.pr_info.ddr_addr, i);
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



    struct timeval t1, t2;
    double timeuse;

    timer_reset();
    timer_start();
    // printf("Add htdt struct...\n");
    
    unsigned int timer0;
    unsigned int timer1;

    timer_gettime(&timer0);

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
    timer_gettime(&timer1);

    // timeuse = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;


    printf("%f\n", (timer1 - timer0) / 100000000.0);
    sleep(1);

    // Read trace information into a file
    FILE *trace_f = fopen(TRACE_FILE, "a");
    unsigned int curr_trace_num = trace_geteachsize(0) / sizeof(unsigned int);
    unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
    trace_gettotalcon(trace_ram);
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
        // printf("apptest: Reg del error 0.\n");
        return 0;
    } 
    int error = 0;
    int filter[TAP] = {3,2,1,2,3};
    int j;
    // for (i = TAP - 1; i < MEM_SIZE; i++) {
    //     int sum = 0;
    //     for (j = 0; j < TAP; j++) {
    //         sum += filter[j] * a[i + (j - TAP + 1)];
    //     }
    //     if (b[i] != sum) {
    //         // // printf("%d:%d\n", i, b[i]);
    //         error++;
    //         // if (error > 4) {
    //         //     break;
    //         // }
    //     }
    // }
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


