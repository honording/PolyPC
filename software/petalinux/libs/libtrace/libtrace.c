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
    int ioctl_ret = ioctl(fd, HTRACE_TRACE_GETTOTALCON, buf);
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

void timer_reset()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_RESET);
    close(fd);
}

void timer_start()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_START);
    close(fd);
}

void timer_stop()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_STOP);
    close(fd);
}

void timer_gettime(unsigned int *time)
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, HTRACE_TIMER_GETTIME, time);
    close(fd);
}