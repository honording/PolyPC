#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM
#define N               128
#define SIZE_PER_PE     2

#ifdef PRIVATE_MEM
#include "string.h"
float a_buffer0[N];
float a_buffer1[N];
//float a_buffer2[N];
//float a_buffer3[N];

float b_buffer[N];
float sum[SIZE_PER_PE];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int c_addr,
            unsigned int num_nodes,
            unsigned int buf_size,
            unsigned int id0,
            unsigned int id1,
            float *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;

#ifdef PRIVATE_MEM
    unsigned int line_off = id1 * SIZE_PER_PE * num_nodes;
    unsigned int vector_off = id1 * SIZE_PER_PE;
    int i, j;
    int num_buf_chunk = num_nodes / buf_size;
    for (i = 0; i < num_buf_chunk; i++) {
        unsigned int chunk_off = i * buf_size;
        // Each line within a matrix
        float *ina0 = &(data[a + line_off + chunk_off]);
        float *ina1 = &(data[a + line_off + num_nodes + chunk_off]);
//        float *ina2 = &(data[a + line_off + num_nodes * 2 + chunk_off]);
//        float *ina3 = &(data[a + line_off + num_nodes * 3 + chunk_off]);
        // Vector
        float *inb = &(data[b + chunk_off]);

        memcpy((float *)a_buffer0, (const float *)ina0, sizeof(float) * buf_size);
        memcpy((float *)a_buffer1, (const float *)ina1, sizeof(float) * buf_size);
//        memcpy((float *)a_buffer2, (const float *)ina2, sizeof(float) * buf_size);
//        memcpy((float *)a_buffer3, (const float *)ina3, sizeof(float) * buf_size);

        memcpy((float *)b_buffer, (const float *)inb, sizeof(float) * buf_size);
        for (j = 0; j < buf_size; j++) {
#pragma HLS PIPELINE
            sum[0] += a_buffer0[j] * b_buffer[j];
            sum[1] += a_buffer1[j] * b_buffer[j];
//            sum[2] += a_buffer2[j] * b_buffer[j];
//            sum[3] += a_buffer3[j] * b_buffer[j];
            if ((i == num_buf_chunk - 1) && (j == buf_size - 1)) {
                data[c + vector_off] = sum[0];
                data[c + vector_off + 1] = sum[1];
//                data[c + vector_off + 2] = sum[2];
//                data[c + vector_off + 3] = sum[3];
            }
        }
    }
#else
    unsigned int line_off = id1 * num_nodes;
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
            unsigned int arg4 = data[SCHE_SLAVE_ARGV_BASE + 4];
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, arg4, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}