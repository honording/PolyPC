#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FILEPATH    "/dev/hapara_reg"
#define u8 uint8_t
#define u32 uint32_t


struct hapara_id_pair {
    u32 id0;
    u32 id1;
};


struct hapara_thread_struct {
    u8 valid;                       //1: valid; 0: invalid
    u8 priority;
    u8 type_num;
    u8 next;
    u32 tid;
    struct hapara_id_pair group_id;
}; // 1+1+1+1+4+4+4=16 bytes


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
    sp->type_num = 6;
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
    printf("type_num = %d\n", sp->type_num);
    printf("next = %d\n", sp->next);
    printf("tid = %d\n", sp->tid);
    printf("group_id 0 = %d\n", sp->group_id.id0);
    printf("group_id 1 = %d\n", sp->group_id.id1);
}