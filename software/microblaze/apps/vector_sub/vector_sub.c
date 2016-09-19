/*
 * opencl_kernel.c
 *
 *  Created on: Apr 15, 2016
 *      Author: hding
 */


#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"


void kernel(
            int *A,
            int *B,
            int *C) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    C[id0 + id1] = A[id0 + id1] - B[id0 + id1];
}


int main() {
    /* Generated Codes
     * nums : how many args;
     * type : data type for each;
     */
    setArgv(0, arg0, int);
    setArgv(1, arg1, int);
    setArgv(2, arg2, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, arg1, arg2);
        }
    }
    clean_up();
    return 0;
}
