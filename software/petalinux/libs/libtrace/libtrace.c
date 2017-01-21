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

#include "libtrace.h"

void trace_clr()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TRACE_CLEAR);
    close(fd);
}

int trace_alloc(struct hapara_id_pair id)
{
    int fd;
    unsigned int num_group;
    num_group = id.id0 * id.id1;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_ALLOC, &num_group);
    close(fd);
    return ioctl_ret;
}

int trace_alloc_single(unsigned int num)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_ALLOC, &num);
    close(fd);
    return ioctl_ret;
}

int trace_gettotalnum()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETTOTALNUM);
    close(fd);
    return ioctl_ret;
}

int trace_gettotalsize()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETTOTALSIZE);
    close(fd);
    return ioctl_ret;
}

int trace_gettotalcon(unsigned int *buf)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) 
        return -1;
    // printf("enter trace_gettotalcon.\n");
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETTOTALCON, buf);
    // printf("leave trace_gettotalcon.\n");
    close(fd);
    return ioctl_ret;
}

int trace_geteachsize(unsigned int index)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETEACHSIZE, &index);
    close(fd);
    return ioctl_ret;
}

int trace_geteachoff(unsigned int index)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETEACHOFF, &index);
    close(fd);
    return ioctl_ret;
}

void hapara_timer_reset()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_RESET);
    close(fd);
}

void hapara_timer_start()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_START);
    close(fd);
}

void hapara_timer_stop()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_STOP);
    close(fd);
}

void hapara_timer_gettime(unsigned int *time)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_GETTIME, time);
    close(fd);
}

void hapara_dump_trace(int index, char *TRACE_FILE) {
    FILE *trace_f = fopen(TRACE_FILE, "a");
    fprintf(trace_f, "%d\n", index);
    int i;
    unsigned int curr_trace_num = trace_gettotalsize() / sizeof(unsigned int);
    unsigned int *trace_ram = (unsigned int *)malloc(curr_trace_num * sizeof(unsigned int));
    trace_gettotalcon(trace_ram);
    int cur_size = trace_geteachsize(index) / sizeof(unsigned int);
    int cur_off  = trace_geteachoff(index) / sizeof(unsigned int);
    trace_gettotalcon(trace_ram);
    int struct_num = trace_geteachsize(index) / sizeof(struct hapara_trace_struct); 
    struct hapara_trace_struct *trace_struct = (struct hapara_trace_struct *)(&(trace_ram[cur_off]));
    for (i = 0; i < struct_num; i++) {
        int isZero = 1;
        while (isZero == 1) {
            if (trace_struct[i].after_finish != 0) {
                isZero = 0;
            } else {
                trace_gettotalcon(trace_ram);
            }
        }
    }
    trace_gettotalcon(trace_ram);
    for (i = cur_off; i < cur_off + cur_size; i++) {
        fprintf(trace_f, "%08X\n", trace_ram[i]);
    }

    free(trace_ram);
    fclose(trace_f);    
}