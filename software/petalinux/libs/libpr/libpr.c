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
    	perror("pr_loader: open /info error.\n");
    	return -1;
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

    /* Move pr.bin into DDR memory
     */
    strcpy(file_name, file_path);
    strcat(file_name, "/pr.bin");
    fp = fopen(file_name, "r");
    if (NULL == fp) {
    	perror("pr_loader: open /pr.bin error.\n");
		munmap(exec, total_size);
    	close(devmemfd);
        if (ddr_free(pr_ddr_addr) == -1) {
            perror("pr_loader: pr ddr free got error.");
        }
    	return -1;
    }
	int read_num = fread(exec, sizeof(char), total_size, fp);
	if (read_num != total_size) {
		perror("pr_loader: read pr.bin to DDR error.");
		fclose(fp);
		munmap(exec, total_size);
    	close(devmemfd);
        if (ddr_free(pr_ddr_addr) == -1) {
            perror("pr_loader: pr ddr free got error.");
        }
        return 0;
	}
    fclose(fp);
	munmap(exec, total_size);
    close(devmemfd);
    pr_info->ddr_addr 		= pr_ddr_addr;
    pr_info->num_pr_file 	= num_pr;
    pr_info->each_size		= size_pr;
    return 1;
}

void disable_mb_pr(pr_info_t *pr_info) {
    pr_info->each_size = -1;
} 