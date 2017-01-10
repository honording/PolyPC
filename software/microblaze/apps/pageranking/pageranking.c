#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"

// #define SIZE_PER_PE     2

typedef float d_type;

void kernel(
            float *A,
            float *B,
            float *C,
            int num_nodes,
            int buf_size) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    // unsigned int off  = id1 * SIZE_PER_PE;
    unsigned int off = id1 * num_nodes;
    // unsigned int off0 = (off + 0) * num_nodes;
    // unsigned int off1 = (off + 1) * num_nodes;
    int j;
    float sum = 0;
    for (j = 0; j < num_nodes; j++) {
        sum += A[id0 + off + j] * B[id0 + j];
    }
    C[id0 + id1] = sum;
    // sum = 0;
    // for (j = 0; j < num_nodes; j++) {
    //     sum += A[id0 + off1 + j] * B[id0 + j];
    // }
    // C[id0 + off + 1] = sum;
}


int main() {
    /* Generated Codes
     * nums : how many args;
     * type : data type for each;
     */
    setArgv(0, arg0, float *);
    setArgv(1, arg1, float *);
    setArgv(2, arg2, float *);
    setArgv(3, num_nodes, int);
    setArgv(4, buf_size, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, arg1, arg2, num_nodes, buf_size);
        }
    }
    clean_up();
    return 0;
}
