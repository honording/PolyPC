#include "libregister.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void libregister_test() 
{
    int fd;
    fd = open(FILEPATH, O_RDWR);
    if (fd == -1) {
        printf("Open error.\n");
    }
    struct hapara_thread_struct *sp;
    sp = malloc(sizeof(struct hapara_thread_struct))
    if (sp == NULL) {
        printf("malloc error\n");
    }
    sp->valid = 1;                       //1: valid; 0: invalid
    sp->priority = 10;
    sp->type_num = 6;
    sp->next = 1;
    sp->tid = 100;
    sp->hapara_id_pair group_id.id0 = 2;
    sp->hapara_id_pair group_id.id1 = 12;
    if (write(fd, sp, sizeof(struct hapara_thread_struct)) == -1) {
        printf("Write error\n");
    }
    free(sp)
    close(fd);
}