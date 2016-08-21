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

#include "../../../generic/include/base_addr.h"

#define PR_FILE_PATH    "/mnt/pr_files/vector_add/pr.bin"

#define ARM_ICAP        0x42000000
#define ICAP_SPAN       0x800000    //8MB

#define DEVMEM          "/dev/mem"
#define PR_SIZE         493268

int main(int argc, char *argv[])
{
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
}


