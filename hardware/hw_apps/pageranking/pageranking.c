#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM
#define N 32

#ifdef PRIVATE_MEM
#include "string.h"
float a_buffer[N];
float b_buffer[N];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int c_addr,
            unsigned int num_nodes,
            unsigned int num_chunk,
            unsigned int id0,
            unsigned int id1,
            float *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;

    unsigned int line_off = id1 * num_nodes;
#ifdef PRIVATE_MEM
    int i, j;
    float sum = 0.0;
    for (i = 0; i < num_chunk; i++) {
        unsigned int chunk_off = i * N;
        // Each line within a matrix
        float *ina = &(data[a + line_off + chunk_off]);
        // Vector
        float *inb = &(data[b + chunk_off]);
        memcpy((float *)a_buffer, (const float *)ina, sizeof(float) * N);
        memcpy((float *)b_buffer, (const float *)inb, sizeof(float) * N);
        for (j = 0; j < N; j++) {
#pragma HLS PIPELINE
            sum += a_buffer[i] * b_buffer[i];
            if ((i == num_chunk - 1) && (j == N - 1)) {
                data[c + id1] = sum;
            }
        }
    }
#else
    float sum = 0.0;
    int i;
    for (i = 0; i < num_nodes; i++) {
#pragma HLS PIPELINE
        sum += data[a + line_off + id1] * data[b + id1];
        if (i == num_nodes - 1) {
            data[c + id1] = sum;
        }
    }
#endif
}

void pageranking(volatile unsigned int *id,
                 volatile unsigned int *barrier,
                 float *data,
                 char htID) {

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE m_axi depth=16 port=data
#pragma HLS INTERFACE axis port=id
#pragma HLS INTERFACE axis port=barrier
    unsigned int internal_id;
    unsigned int id0, id1;
    while (1) {
        if (data[SCHE_SLAVE_TRIGGER_BASE + htID] == 1) {
            data[SCHE_SLAVE_TRIGGER_BASE + htID] = 0;
            unsigned int arg0 = data[SCHE_SLAVE_ARGV_BASE];
            unsigned int arg1 = data[SCHE_SLAVE_ARGV_BASE + 1];
            unsigned int arg2 = data[SCHE_SLAVE_ARGV_BASE + 2];
            // Number of nodes
            unsigned int arg3 = data[SCHE_SLAVE_ARGV_BASE + 3];
            unsigned int num_chunk = arg3 / N;
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, num_chunk, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}
