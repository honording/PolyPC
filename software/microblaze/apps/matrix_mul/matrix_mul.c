#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "../../generic/CL/cl.h"

#define BUF_LENGTH 16

void kernel(
            int *A,
            int *B,
            int *C,
            int num_nodes) {
    unsigned int id0 = getGlobalID(0);
    unsigned int id1 = getGlobalID(1);
    unsigned int raw_off = id0 * BUF_LENGTH;
    unsigned int col_off = id1 * BUF_LENGTH;

    int i, j;
    int m;
    for (i = 0; i < BUF_LENGTH; i++) {
        for (j = 0; j < BUF_LENGTH; j++) {
            int sum = 0;
            for (m = 0; m < num_nodes; m++) {
                sum += A[(raw_off + i) * num_nodes + m] *
                       B[m * num_nodes + col_off + j];
            }
            C[(raw_off + i) * num_nodes + col_off + j] = sum;
        }
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
    setArgv(3, num_nodes, int);
    if (!setjmp(buf)) {
        while (1) {
            kernel(arg0, arg1, arg2, num_nodes);
        }
    }
    clean_up();
    return 0;
}
