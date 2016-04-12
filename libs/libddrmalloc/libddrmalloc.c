#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "libddrmalloc.h"

int ddr_free(int addr)
{
    int fd;
    int ret = -1;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return ret;
    ret = ioctl(fd, DDRMALLOC_FREE, &addr);
    close(fd);
    return ret;
}

int ddr_malloc(int size)
{
    int fd;
    int ret = -1;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return ret;
    ret = ioctl(fd, DDRMALLOC_ALLOC, &size);
    close(fd);
    return ret;
}

void ddr_list_print()
{
    printf("Enter ddr print\n");
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, DDRMALLOC_PRINT);
    close(fd);
    printf("Leave ddr print\n");
}
