#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"

#define TAP 5
int filter[TAP] = {3,2,1,2,3};

void kernel(
            int *A,
            int *B,
            int buf_size) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    unsigned int off = id1 * buf_size;
    int i, j;
    for (i = 0; i < buf_size; i++) {
        int sum = 0;
        for (j = 0; j < TAP; j++) {
            sum += A[id0 + off + i + j] * filter[j];
        }
        B[id0 + off + i] = sum;
    }
}


int main() {
    /* Generated Codes
     * nums : how many args;
     * type : data type for each;
     */
    setArgv(0, arg0, int *);
    setArgv(1, arg1, int *);
    setArgv(2, buf_size, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, arg1, buf_size);
        }
    }
    clean_up();
    return 0;
}