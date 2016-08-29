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

#define PR_FILE_PATH    "/mnt/pr_files/vector_add/pr.bin"

#include "pr_add.h"
#include "pr_sub.h"

#define XPAR_PRC_0_BASEADDR         0x42000000
#define ICAP_SPAN                   0xFFFF    //64KB

#define DEVMEM          "/dev/mem"
#define PR_MAZ_SIZE     0x100000
#define PR_SIZE         493268

#define rp_math_STATUS        0X00000
#define rp_math_CONTROL       0X00000
#define rp_math_SW_TRIGGER    0X00004
#define rp_math_TRIGGER0      0X00020

#define rp_math_RM_ADDRESS0   0X00040
#define rp_math_RM_CONTROL0   0X00044

#define rp_math_BS_ID0        0X00060
#define rp_math_BS_ADDRESS0   0X00064
#define rp_math_BS_SIZE0      0X00068

void Xil_Out32(unsigned int *icap, unsigned int off, unsigned int val) { 
    icap[off >> 2] = val;
}

unsigned int Xil_In32(unsigned int *icap, unsigned int off) {
    return icap[off >> 2];
}

int main(int argc, char *argv[])
{

    // Vivado Command to generate the BIN file
    // which is bitswapped and little-endian
    // write_cfgmem -format bin -interface smapx32 -loadbit "up 0 ...bin" -file ...bin

    // Linux Command to generate the .h file from BIN file
    // xxd -i -c 4 ...bin ...h
    /*
     * This is old version with bram ICAP 
#ifdef DO_BIN
    printf("Begin to open devmem.\n");
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        printf("apptest: devmem open failed.\n");
        return -1;
    }
    printf("Begin to map icap space.\n");
    unsigned int *icap = mmap(NULL, ICAP_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, ARM_ICAP);
    unsigned int *icap_buffer = (unsigned int *)__vector_add_pblock_group0_s0_partial_bin;
    unsigned int buffer_len = __vector_add_pblock_group0_s0_partial_bin_len / 4; 
    // unsigned int *icap_buffer = (unsigned int *)__vector_add_pblock_group0_s0_partial_8_bin;
    // unsigned int buffer_len = __vector_add_pblock_group0_s0_partial_8_bin_len / 4; 
    printf("Begin to memcpy PR buffer\n");
    memcpy(icap, icap_buffer, buffer_len);
    munmap(icap, ICAP_SPAN);
    close(devmemfd);
#else 
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        printf("apptest: devmem open failed.\n");
        return -1;
    }
    printf("Begin to map icap space.\n");
    int *icap = mmap(NULL, ICAP_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, ARM_ICAP);
    FILE *fp = NULL;
    printf("Begin to read pr.bin file.\n");
    fp = fopen(PR_FILE_PATH, "r");
    if (NULL == fp) {
        perror("pr_loader: open /pr.bin error.\n");
        munmap(icap, ICAP_SPAN);
        close(devmemfd);
        return -1;
    }
    printf("Begin to do PR.\n");
    int read_num = fread(icap, sizeof(char), PR_SIZE, fp);
    printf("Read size: %d\n", read_num);
    munmap(icap, ICAP_SPAN);
    close(devmemfd);
    printf("PR done.\n");
#endif
    */

    int pr_ddr_addr = ddr_malloc(MEM_SIZE * sizeof(int));
    if (pr_ddr_addr < 0) {
        printf("apploadpr: ddr_malloc error a\n");
        return 0;
    }

    printf("Begin to open devmem.\n");
    int devmemfd = open(DEVMEM, O_RDWR | O_SYNC);
    if (devmemfd < 1) {
        printf("apploadpr: devmem open failed.\n");
        return -1;
    }
    printf("Begin to map icap space.\n");
    unsigned int *icap      = mmap(NULL, ICAP_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, XPAR_PRC_0_BASEADDR);
    printf("Begin to map pr_ddr_pt space.\n");
    unsigned char *pr_ddr_pt = mmap(NULL, PR_MAZ_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, pr_ddr_addr);
    if (strcmp(argv[0], "add") == 0) {
        memcpy(pr_ddr_pt, __add_bin, __add_bin_len);
    } else {
        memcpy(pr_ddr_pt, __sub_bin, __sub_bin_len);
    }

    unsigned int Status;
    unsigned int loading_done=0;

    printf("Putting the PRC core's Math RP in Shutdown mode\n");
    Xil_Out32(icap,rp_math_CONTROL,0);
    printf("Waiting for the shutdown to occur\n");
    while(!(Xil_In32(icap,rp_math_STATUS)&0x80));
    printf("Math RP is shutdown\n");

    printf("Initializing RM bitstream address and size registers for Math and Shift RMs\n");
    Xil_Out32(icap,rp_math_BS_ADDRESS0,  pr_ddr_addr);
    Xil_Out32(icap,rp_math_BS_SIZE0,   PR_SIZE);

    print("Initializing RM trigger ID registers for Math and Shift RMs\n");
    Xil_Out32(icap,rp_math_TRIGGER0,0);

    print("Initializing RM address and control registers for Math and Shift RMs\n");
    Xil_Out32(icap,rp_math_RM_ADDRESS0,0);
    Xil_Out32(icap,rp_math_RM_CONTROL0,0xFF0);

    print("Reading RM bitstreams address and size registers for Math and Shift RMs\n");
    printf("Adder RM address  = %x\n",Xil_In32(icap,rp_math_BS_ADDRESS0));
    printf("Adder RM size     = %d Bytes\n",Xil_In32(icap,rp_math_BS_SIZE0));

    print("Reading RM Trigger and address registers for Math and Shift RMs\n");
    printf("Adder RM Trigger0 = %x\n",Xil_In32(icap,rp_math_TRIGGER0));
    printf("Adder RM Address0 = %x\n",Xil_In32(icap,rp_math_RM_ADDRESS0));

    print("Putting the PRC core's Math RP in Restart with Status mode\n");
    Xil_Out32(icap,rp_math_CONTROL,2);
    printf("Reading the Math RP status=%x\n",Xil_In32(icap,rp_math_STATUS));

    printf("\n");

    printf("Generating software trigger for VADD reconfiguration\n");
    Status=Xil_In32(icap,rp_math_SW_TRIGGER);
    if(!(Status&0x8000)) {
      printf("Starting VADD Reconfiguration\n");
      Xil_Out32(icap,rp_math_SW_TRIGGER,0);
    }
    loading_done = 0;
    while(!loading_done) {
        printf("0x%x\n", Xil_In32(icap,rp_math_STATUS)&0x78);
        Status=Xil_In32(icap,rp_math_STATUS)&0x07;
      switch(Status) {
        case 7 : print("RM loaded\n"); loading_done=1; break;
        case 6 : print("RM is being reset\n"); break;
        case 5 : print("Software start-up step\n"); break;
        case 4 : print("Loading new RM\n"); break;
        case 2 : print("Software shutdown\n"); break;
        case 1 : print("Hardware shutdown\n"); break;
      }
    }
    int ret = ddr_free(pr_ddr_addr);
    if (ret < 0) {
        printf("apploadpr: ddr_free error: pr_ddr_addr.\n");
        return 0;
    }
    printf("VADD Reconfiguration Completed!\n");
    munmap(icap, ICAP_SPAN)
    munmap(pr_ddr_pt, PR_MAZ_SIZE);
    close(devmemfd);

    return 0;
}