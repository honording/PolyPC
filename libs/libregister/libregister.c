#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "libregister.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int reg_add(struct hapara_thread_struct *thread_info)
{
    int fd;
    int ioctl_ret = 0;
    int ret = 0;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    ioctl_ret = ioctl(fd, REG_ADD, thread_info);
    if (ret < 0) 
        ret = -1;
    else
        ret = ioctl_ret;
    close(fd);
    return ret;
}

int reg_del(unsigned int off, unsigned int target);
{
    int fd;
    int ioctl_ret = 0;
    int ret = 0;
    if (off > 255 || target > 255) 
        return -1;
    struct hapara_reg_pair pair = {
        .off = (uint8_t)off,
        .target = (uint8_t)target,
    };
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    ioctl_ret = ioctl(fd, REG_DEL, &pair);
    if (ret < 0) 
        ret = -1;
    else
        ret = ioctl_ret;
    close(fd);
    return ret;    
}


void libregister_test(void) 
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) {
        printf("Open error.\n");
    }
    struct hapara_thread_struct *sp;
    sp = malloc(sizeof(struct hapara_thread_struct));
    if (sp == NULL) {
        printf("malloc error\n");
    }
    sp->valid = 1;                       //1: valid; 0: invalid
    sp->priority = 10;
    sp->type = 6;
    sp->next = 1;
    sp->tid = 100;
    sp->group_id.id0 = 2;
    sp->group_id.id1 = 12;
    if (write(fd, sp, sizeof(struct hapara_thread_struct)) == -1) {
        printf("Write error\n");
    }
    free(sp);
    close(fd);

    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) {
        printf("Open Error 2\n");
    }
    sp = malloc(sizeof(struct hapara_thread_struct));
    if (sp == NULL) {
        printf("malloc error 2\n");
    }
    if (read(fd, sp, sizeof(struct hapara_thread_struct)) == -1) {
        printf("Read error\n");
    }
    print_struct(sp);
    free(sp);
    close(fd);
}

void print_struct(struct hapara_thread_struct *sp) 
{
    printf("valid = %d\n", sp->valid);
    printf("priority = %d\n", sp->priority);
    printf("type = %d\n", sp->type);
    printf("next = %d\n", sp->next);
    printf("tid = %d\n", sp->tid);
    printf("group_id 0 = %d\n", sp->group_id.id0);
    printf("group_id 1 = %d\n", sp->group_id.id1);
}
