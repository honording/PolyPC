#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"


void kernel(
            int *A,
            int buf_size) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    unsigned int off = id1 * buf_size;
    int i;
    volatile int value;
    for (i = 0; i < buf_size; i++) {
        value = A[id0 + off + i];
        value++;
    }
}


int main() {
    /* Generated Codes
     * nums : how many args;
     * type : data type for each;
     */
    setArgv(0, arg0, int *);
    setArgv(1, buf_size, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, buf_size);
        }
    }
    clean_up();
    return 0;
}