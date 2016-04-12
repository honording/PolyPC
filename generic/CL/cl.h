#ifndef _CL_H_
#define _CL_H_

#include <fsl.h>
#include <pvr.h>
#include <setjmp.h>
#include <xil_io.h>

#define KERNEL_ARGV_BASE    //FIXME

#define getArgv(num) \
        (unsigned int *)(KERNEL_ARGV_BASE) + (num)
#define setArgv(num, member, type) \
        (type *)member = (type *)(getArgv(num))

jmp_buf buf;

typedef struct id_struct_s {
    int isIDGet;
    unsigned int id0;
    unsigned int id1;
} id_struct;


id_struct id = {
        .isIDGet    = 0,
        .id0        = 0,
        .id1        = 0,
};

unsigned int getGlobalID(int d) {
    unsigned int val;
    if (id.isIDGet == 0) {
        getfslx(val, 1, FSL_DEFAULT);
        if (val == 0xFFFFFFFF) {
            //error catch
            id.isIDGet = 0;
            longjmp(buf, 1);
        } else {
            id.isIDGet  = 1;
            id.id0      = val >> 16;
            id.id1      = val & 0x0000FFFF;
        }
        return (d == 0)?id.id0:id.id1;
    }
    id.isIDGet = 0;
    return (d == 0)?id.id0:id.id1;
}


void clean_up() {
    id.isIDGet  = 0;
    id.id0      = 0;
    id.id1      = 0;
}

#endif