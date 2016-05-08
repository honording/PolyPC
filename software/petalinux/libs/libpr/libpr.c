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

#include <string.h>

#include "libpr.h"

int pr_loader(char *file_path, 
              pr_info_t *pr_info) {
	char file_name[MAX_LENGTH];
    FILE *fp = NULL;
    strcpy(file_name, file_path);
    strcat(file_name, "/info");
    fp = fopen(file_name, "r");
    if (NULL == fp) {
    	printf("pr_loader: open /info error.\n");
    	return 0;
    }
    int num_pr;
    int size_pr;
    fscanf(fp, "%d", &num_pr);
    fscanf(fp, "%d", &size_pr);
    fclose(fp);
    int total_size = num_pr * size_pr;
    int pr_ddr_addr = ddr_malloc(total_size);
	if (pr_ddr_addr == -1) {
        perror("pr_loader: cannot alloct pr ddr space.");
        return -1;
    }
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        perror("pr_loader: devmem open failed.");
        if (ddr_free(pr_ddr_addr) == -1) {
            perror("pr_loader: pr ddr free got error.");
        }
        return -1;
    }
    // printf("Begin to mmap\n");
    char *exec = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, pr_ddr_addr);
    if (!exec) {
        perror("pr_loader: mmap devmem failed.");
        close(devmemfd);
        if (ddr_free(pr_ddr_addr) == -1) {
            perror("pr_loader: pr ddr free got error.");
        }
        return -1;        
    }
    char pr_name[] = "pr00.bin";



	munmap(exec, file_size);
    close(devmemfd);
}