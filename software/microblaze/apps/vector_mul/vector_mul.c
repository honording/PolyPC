#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"


#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"


void kernel(
            int *A,
            int *B,
            int *C,
            int buf_size) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    unsigned int off = id1 * buf_size;
    int i;
    for (i = 0; i < buf_size; i++) {
        C[id0 + off + i] = A[id0 + off + i] * B[id0 + off + i];
    }
}


int main() {
    /* Generated Codes
     * nums : how many args;
     * type : data type for each;
     */
    setArgv(0, arg0, int *);
    setArgv(1, arg1, int *);
    setArgv(2, arg2, int *);
    setArgv(3, buf_size, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, arg1, arg2, buf_size);
        }
    }
    clean_up();
    return 0;
}
