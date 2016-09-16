#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif


#include <stdio.h>
#include <string.h>
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

#define PR_FILE_PATH    "/mnt/pr_files/vector_xxx/pr0x.bin"
#define PR_COMB_PATH    "/mnt/pr_files/vector_xxx"
#define PR_INFO_PATH    "/mnt/pr_files/vector_xxx/info"
//                       012345678901234567890123456789012

// #include "pr_add.h"
// #include "pr_sub.h"
// #include "acc_add_bit.h"
// #include "acc_sub_bit.h"

#define XPAR_PRC_0_BASEADDR         0x42000000
#define PRC_ICAP_SPAN                   0xFFFF    //64KB

#define DEVMEM          "/dev/mem"
#define PR_MAX_SIZE     0x100000
// #define PR_SIZE         580532
#define PR_SIZE         __vector_add_pr_add_bin_len

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



int do_pr(int pr_size, unsigned char *pr_data, unsigned int vsm) {
    unsigned int vsm_off = vsm << 7;
    int pr_ddr_addr = ddr_malloc(PR_MAX_SIZE);
    if (pr_ddr_addr < 0) {
        printf("apploadpr: ddr_malloc error a\n");
        return 0;
    }
    printf("PR DDR address:0x%X\n", pr_ddr_addr);
    printf("Begin to open devmem.\n");
    int devmemfd = open(DEVMEM, O_RDWR | O_SYNC);
    if (devmemfd < 1) {
        printf("apploadpr: devmem open failed.\n");
        return 0;
    }
    printf("Begin to map icap space.\n");
    unsigned int *icap          = mmap(NULL, PRC_ICAP_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, XPAR_PRC_0_BASEADDR);
    printf("Begin to map pr_ddr_pt space.\n");
    unsigned int *pr_ddr_pt    = mmap(NULL, PR_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, pr_ddr_addr);
    memcpy(pr_ddr_pt, pr_data, pr_size);
    printf("Copy Sub.\n");

    unsigned int Status;
    unsigned int loading_done=0;

    printf("Putting the PRC core's Math RP in Shutdown mode\n");
    Xil_Out32(icap,vsm_off | rm_math_CONTROL,0);
    printf("Waiting for the shutdown to occur\n");
    while(!(Xil_In32(icap,vsm_off | rm_math_STATUS)&0x80));
    printf("Math RP is shutdown\n");

    printf("Initializing RM bitstream address and size registers for Math and Shift RMs\n");
    Xil_Out32(icap,vsm_off | rm_math_BS_ADDRESS0,  pr_ddr_addr);
    Xil_Out32(icap,vsm_off | rm_math_BS_SIZE0,   pr_size);

    printf("Initializing RM trigger ID registers for Math and Shift RMs\n");
    Xil_Out32(icap,vsm_off | rm_math_TRIGGER0,0);

    printf("Initializing RM address and control registers for Math and Shift RMs\n");
    Xil_Out32(icap,vsm_off | rm_math_RM_ADDRESS0,0);
    Xil_Out32(icap,vsm_off | rm_math_RM_CONTROL0,0x1FF0);

    printf("Reading RM bitstreams address and size registers for Math and Shift RMs\n");
    printf("Adder RM address  = %x\n",Xil_In32(icap,vsm_off | rm_math_BS_ADDRESS0));
    printf("Adder RM size     = %d Bytes\n",Xil_In32(icap,vsm_off | rm_math_BS_SIZE0));

    printf("Reading RM Trigger and address registers for Math and Shift RMs\n");
    printf("Adder RM Trigger0 = %x\n",Xil_In32(icap,vsm_off | rm_math_TRIGGER0));
    printf("Adder RM Address0 = %x\n",Xil_In32(icap,vsm_off | rm_math_RM_ADDRESS0));

    printf("Putting the PRC core's Math RP in Restart with Status mode\n");
    Xil_Out32(icap,vsm_off | rm_math_CONTROL,2);
    printf("Reading the Math RP status=%x\n",Xil_In32(icap,vsm_off | rm_math_STATUS));

    printf("\n");

    printf("Generating software trigger for VADD reconfiguration\n");
    Status=Xil_In32(icap,vsm_off | rm_math_SW_TRIGGER);
    if(!(Status&0x8000)) {
      printf("Starting VADD Reconfiguration\n");
      Xil_Out32(icap,vsm_off | rm_math_SW_TRIGGER,0);
    }
    loading_done = 0;
    while(!loading_done) {
        // printf("0x%x\n", Xil_In32(icap,vsm_off | rm_math_STATUS)&0x78);
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
                printf("."); 
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
    int ret = ddr_free(pr_ddr_addr);
    if (ret < 0) {
        printf("apploadpr: ddr_free error: pr_ddr_addr.\n");
        return 0;
    }
    printf("\nVADD Reconfiguration Completed!\n");

pr_exit:
    munmap(icap, PRC_ICAP_SPAN);
    munmap(pr_ddr_pt, PR_MAX_SIZE);
    close(devmemfd);
    return 1;
}


int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4) {
        printf("Input arguments: [Acc name] (Number of Acc) (Use combine one)\n");
        return 0;
    }
    int numPR = 0;
    int sizePR = 0;
    int ret = 0;
    unsigned char* pr_content = NULL;
    int i, j;
    char pr_file_name[] = PR_FILE_PATH;
    char pr_info_name[] = PR_INFO_PATH;
    char pr_comb_name[] = PR_COMB_PATH;
    // printf("file name: %s\n info name: %s\n", pr_file_name, pr_info_name);
    FILE *fp = NULL;
    for (j = 0; j < 3; j++) {
        pr_info_name[21 + j] = argv[1][j];
    }
    printf("PR info name:%s\n", pr_info_name);
    fp = fopen(pr_info_name, "r");
    fscanf(fp, "%d", &numPR);
    printf("Number of PR files: %d\n", numPR);
    fscanf(fp, "%d", &sizePR);
    printf("PR size: %d\n", sizePR);
    fclose(fp);
    pr_content = (unsigned char *)malloc(sizePR);
    if (argc == 3) {
        numPR = argv[2][0] - '0';
        printf("Seperate User number of PR files: %d\n", numPR);
    }
    if (argc == 4) {
        numPR = argv[3][0] - '0';
        printf("Combine User number of PR files: %d\n", numPR);
    }
    struct hapara_thread_struct sp;
    for (j = 0; j < 3; j++) {
        pr_comb_name[21 + j] = argv[1][j];
    }
    printf("apploadpr: begin to load PR bitstream into memory.\n");
    ret = pr_loader(pr_comb_name, &sp.pr_info);
    if (ret < 0) {
        printf("apploadpr: pr_loader error\n");
        return 0;
    }
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        printf("apploadpr: devmem open failed.\n");
        return -1;
    }
    printf("begin to map a.\n");
    unsigned char *a = mmap(NULL, sizePR * 4 * sizeof(unsigned char), PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, sp.pr_info.ddr_addr);
    for (i = 0; i < numPR; i++) {
        if (argc == 4) {
            printf("Do Combined PR files.\n");
            if (do_pr(sizePR, a + i * sizePR, i) != 1) {
                printf("do_pr error.\n");
                fclose(fp);
                return 0;
            }            
        } else {
            printf("Do seperate PR files.\n");
            for (j = 0; j < 3; j++) {
                pr_file_name[21 + j] = argv[1][j];
            }
            pr_file_name[28] = '0' + i;
            printf("PR file name: %s\n", pr_file_name);
            fp = fopen(pr_file_name, "r");

            int read_num = fread(pr_content, sizeof(char), sizePR, fp);
            if (read_num != sizePR) {
                printf("PR read error: %d\n", read_num);
                fclose(fp);
                return 0;
            }
            if (do_pr(sizePR, pr_content, i) != 1) {
                printf("do_pr error.\n");
                fclose(fp);
                return 0;
            }
            fclose(fp);            
        }

    }
    free(pr_content);
    ret = ddr_free(sp.pr_info.ddr_addr);
    if (ret < 0) {
        printf("apptest: ddr_free error: pr.\n");
        return 0;
    }
    printf("Done.\n");
    return 0;
}