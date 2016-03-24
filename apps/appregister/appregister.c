#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../libs/libregister/libregister.h"
#include <stdio.h>

int main() 
{
    printf("Enter main\n");
    reg_clr();
    int ret = 0;
    int i = 0;
    struct hapara_thread_struct *sp;
    sp = malloc(sizeof(struct hapara_thread_struct));
    if (sp == NULL) {
        printf("Malloc error.@main\n");
        return 0;
    }

    for (i = 0; i < 5; i++) {
        set_struct(sp, 1, 10, i, 0, 100, 2, 12, 0, 0, 0);
        ret = reg_add(sp);
        if (ret == -1) {
            printf("Reg add error 0.@main\n");
            return 0;
        }
        printf("Add location: %d\n", ret);        
    }

    ret = reg_del(OFF_TYPE, 1);
     if (ret == -1) {
        printf("Reg del error 0.@main\n");
        return 0;
    }
    printf("Del location: %d\n", ret);   

    ret = reg_del(OFF_TYPE, 3);
     if (ret == -1) {
        printf("Reg del error 1.@main\n");
        return 0;
    }
    printf("Del location: %d\n", ret); 

    for (i = 0; i < 5; i++) {
        ret = read_struct(sp, i);
        if (ret < 0) {
            printf("read error %d.@main\n", i);
            return 0;
        }
        print_struct(sp);  
    }

    ret = reg_del(OFF_TYPE, 2);
     if (ret == -1) {
        printf("Reg del error 3.@main\n");
        return 0;
    }
    printf("Del location: %d\n", ret); 

    set_struct(sp, 1, 10, 1, 0, 100, 2, 12, 0, 0, 0);
    ret = reg_add(sp);
    if (ret == -1) {
        printf("Reg add error 0.@main\n");
        return 0;
    }
    printf("Add location: %d\n", ret); 

    for (i = 0; i < 5; i++) {
        ret = read_struct(sp, i);
        if (ret < 0) {
            printf("read error %d.@main\n", i);
            return 0;
        }
        print_struct(sp);  
    }

    printf("Exit main\n");
    free(sp);
    return 0;  
}
