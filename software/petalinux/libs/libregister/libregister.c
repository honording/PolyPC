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

void reg_clr()
{
    // printf("reg_clr@libregister, in.\n");
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return;
    ioctl(fd, REG_CLR);
    close(fd);
    // printf("reg_clr@libregister, out.\n");
}

int reg_add(struct hapara_thread_struct *thread_info)
{
    // printf("reg_add@libregister. in.\n");
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
    // printf("reg_add@libregister. out\n");
    return ret;
}

int reg_del(int location)
{
    // printf("Enter reg_del.\n");
    int fd;
    int ioctl_ret = 0;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) {
        // printf("red_del: open device error.\n");
        return -1;
    }
    ioctl_ret = ioctl(fd, REG_DEL, &location);
    close(fd);
    // printf("Leave reg_del\n");
    return ioctl_ret;
}

int reg_search_del(int tid)
{
    int fd;
    int ioctl_ret = 0;
    int ret = 0;
    int off = OFF_TID;
    // if (off > 255 || target > 255)
    //     return -1;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    ioctl_ret = ioctl(fd, REG_SEARCH_DEL, &tid);
    if (ret < 0)
        ret = -1;
    else
        ret = ioctl_ret;
    close(fd);
    return ret;
}

int read_struct(struct hapara_thread_struct *thread_info, unsigned int offset)
{
    int fd;
    int ret = 0;
    if (offset > 255) {
        printf("Invalid offset.@read_struct\n");
        return -1;
    }
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) {
        printf("Open Error.@read_struct\n");
        return -1;
    }
    ret = lseek(fd, offset * sizeof(struct hapara_thread_struct), SEEK_SET);
    if (ret < 0) {
        printf("lseek error.@read_struct\n");
        close(fd);
        return -1;
    }
    ret = read(fd, thread_info, sizeof(struct hapara_thread_struct));
    if (ret < 0) {
        printf("Read error.@read_struct\n");
        ret = -1;
    }
    close(fd);
    return ret;
}

/*

void set_struct(struct hapara_thread_struct *thread_info,
                unsigned int valid,
                unsigned int priority,
                unsigned int type,
                unsigned int next,
                unsigned int tid,
                unsigned int id0,
                unsigned int id1,
                unsigned int main_addr,
                unsigned int stack_addr,
                unsigned int thread_size)
//set_struct(sp, 1, 10, 1, 0, 100, 0, 0,     elf_info.main_addr, 
//                                           elf_info.stack_addr, 
//                                           elf_info.thread_size);

{
    if (valid > 255 ||
        priority > 255 ||
        type > 255 ||
        next > 255 ||
        tid > 255) {
        printf("invalid input.@set_struct\n");
        return;
    }
    thread_info->valid = valid;
    thread_info->priority = priority;
    thread_info->type = type;
    thread_info->next = next;
    thread_info->tid = tid;
    thread_info->group_id.id0 = id0;
    thread_info->group_id.id1 = id1;
    thread_info->elf_info.main_addr = main_addr;
    thread_info->elf_info.stack_addr = stack_addr;
    thread_info->elf_info.thread_size = thread_size;
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
*/

// struct hapara_thread_struct {
//     uint8_t isValid;
//     uint8_t priority;
//     uint8_t next;
//     uint32_t tid;
//     struct hapara_id_pair group_size;
//     struct hapara_id_pair group_num;
//     struct hapara_id_pair cur_group_id;
//     elf_info_t elf_info;
//     uint32_t argv[ARGC];
// }__attribute__((aligned(4)));

// struct elf_info_struct {
//     unsigned int main_addr;
//     unsigned int stack_addr;
//     unsigned int thread_size;
//     unsigned int ddr_addr;
//     unsigned int DMA_size;
// };
void print_list()
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1)
        return -1;
    ioctl(fd, REG_PRINT_LIST);
    close(fd);   
}
void print_struct(struct hapara_thread_struct *thread_info)
{
    printf("----------------------------------------\n");
    printf("isValid  = %d\n", thread_info->isValid);
    printf("priority = %d\n", thread_info->priority);
    printf("next     = %d\n", thread_info->next);
    printf("tid      = %d\n", thread_info->tid);
    printf("cur_group_id 0 = %d\n", thread_info->cur_group_id.id0);
    printf("cur_group_id 1 = %d\n", thread_info->cur_group_id.id1);
    printf("group_size 0   = %d\n", thread_info->group_size.id0);
    printf("group_size 1   = %d\n", thread_info->group_size.id1);
    printf("group_num 0    = %d\n", thread_info->group_num.id0);
    printf("group_num 1    = %d\n", thread_info->group_num.id1);
    printf("elf_info elf_magic   = 0x%08X\n", thread_info->elf_info.elf_magic);
    printf("elf_info main_addr   = 0x%08X\n", thread_info->elf_info.main_addr);
    printf("elf_info stack_addr  = 0x%08X\n", thread_info->elf_info.stack_addr);
    printf("elf_info thread_size = 0x%08X\n", thread_info->elf_info.thread_size);
    printf("elf_info ddr_addr    = 0x%08X\n", thread_info->elf_info.ddr_addr);
    printf("elf_info DMA_size    = 0x%08X\n", thread_info->elf_info.DMA_size);
    printf("pr_info ddr_addr     = 0x%08X\n", thread_info->pr_info.ddr_addr);
    printf("pr_info num_pr_file  = 0x%d\n", thread_info->pr_info.num_pr_file);
    printf("pr_info each_size    = 0x%d\n", thread_info->pr_info.each_size);
    printf("---------------------------------------\n");
}
