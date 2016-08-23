#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#define DO_BIN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../../generic/include/base_addr.h"

#define PR_FILE_PATH    "/mnt/pr_files/vector_add/pr.bin"

#ifdef DO_BIN
#include "pr_bs_le.h"
#endif

#define ARM_ICAP        0x42000000
#define ICAP_SPAN       0x800000    //8MB

#define DEVMEM          "/dev/mem"
#define PR_SIZE         493268

int main(int argc, char *argv[])
{
#ifdef DO_BIN
    // Vivado Command to generate the BIN file
    // which is bitswapped and little-endian
    // write_cfgmem -format bin -interface smapx32 -loadbit "up 0 ...bin" -file ...bin

    // Linux Command to generate the .h file from BIN file
    // xxd -i -c 4 ...bin ...h
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
    return 0;
}