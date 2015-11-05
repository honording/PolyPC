#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../libs/libregister/libregister.h"
#include <stdio.h>

int main() 
{
    printf("Enter main\n");
    int ret = 0;
    struct hapara_thread_struct *sp;
    sp = malloc(sizeof(struct hapara_thread_struct));
    if (sp == NULL) {
        printf("Malloc error.@main\n");
        return 0;
    }

    set_struct(sp, 1, 10, 6, 0, 100, 2, 12);
    ret = reg_add(sp);
    if (ret == -1) {
        printf("Reg add error 0.@main\n");
        return 0;
    }
    printf("Add location: %d\n", ret);
    set_struct(sp, 1, 2, 8, 0, 99, 3, 21);
    ret = reg_add(sp);
    if (ret == -1) {
        printf("Reg add error 1.@main\n");
        return 0;
    }
    printf("Add location: %d\n", ret);

    ret = read_struct(sp, 0);
    if (ret < 0) {
        printf("read error 0.@main\n");
        return 0;
    }
    print_struct(sp);

    ret = read_struct(sp, 1);
    if (ret < 0) {
        printf("read error 1.@main\n");
        return 0;
    }
    print_struct(sp);

    printf("Exit main\n");
    free(sp);
    return 0;  
}