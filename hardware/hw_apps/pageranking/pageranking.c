#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

typedef float d_type;

#define PRIVATE_MEM
#define N               128
#define SIZE_PER_PE     2

#ifdef PRIVATE_MEM
#include "string.h"
d_type a_buffer0[N];
d_type a_buffer1[N];
d_type b_buffer[N];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int c_addr,
            unsigned int num_nodes,
            unsigned int buf_size,
            unsigned int id0,
            unsigned int id1,
            d_type *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;

    unsigned int line_off = id1 * SIZE_PER_PE * num_nodes;
    unsigned int vector_off = id1 * SIZE_PER_PE;
    int i, j;
    int num_buf_chunk = num_nodes / buf_size;
    d_type sum[SIZE_PER_PE] = {0.0f, 0.0f};
    for (i = 0; i < num_buf_chunk; i++) {
        unsigned int chunk_off = i * buf_size;
        d_type *ina0 = &(data[a + line_off + chunk_off]);
        d_type *ina1 = &(data[a + line_off + num_nodes + chunk_off]);
        d_type *inb = &(data[b + chunk_off]);

        memcpy((d_type *)a_buffer0, (const d_type *)ina0, sizeof(d_type) * buf_size);
        memcpy((d_type *)a_buffer1, (const d_type *)ina1, sizeof(d_type) * buf_size);
        memcpy((d_type *)b_buffer, (const d_type *)inb, sizeof(d_type) * buf_size);
        for (j = 0; j < buf_size; j++) {
#pragma HLS PIPELINE
            sum[0] += a_buffer0[j] * b_buffer[j];
            sum[1] += a_buffer1[j] * b_buffer[j];
            if ((i == num_buf_chunk - 1) && (j == buf_size - 1)) {
                data[c + vector_off] = sum[0];
                data[c + vector_off + 1] = sum[1];
            }
        }
    }
}

void pageranking(volatile unsigned int *id,
                 volatile unsigned int *barrier,
                 volatile unsigned int *barrier_rel,
                 d_type *data,
                 char htID) {

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE m_axi depth=16 port=data
#pragma HLS INTERFACE axis port=id
#pragma HLS INTERFACE axis port=barrier
#pragma HLS INTERFACE axis port=barrier_rel
    unsigned int internal_id;
    unsigned int id0, id1;
    unsigned int trigger;
    unsigned int zero = 0;
    d_type trigger_d_type;
    while (1) {
        trigger_d_type = data[SCHE_SLAVE_TRIGGER_BASE + htID];
        trigger = *((unsigned int *)&trigger_d_type);
        if (trigger == 2) *barrier = 2;
        if (trigger == 1) {
            data[SCHE_SLAVE_TRIGGER_BASE + htID] = *((d_type *)&zero);
            d_type arg0_d_type = data[SCHE_SLAVE_ARGV_BASE];
            unsigned int arg0 = *((unsigned int *)&arg0_d_type);
            d_type arg1_d_type = data[SCHE_SLAVE_ARGV_BASE + 1];
            unsigned int arg1 = *((unsigned int *)&arg1_d_type);
            d_type arg2_d_type = data[SCHE_SLAVE_ARGV_BASE + 2];
            unsigned int arg2 = *((unsigned int *)&arg2_d_type);
            d_type arg3_d_type = data[SCHE_SLAVE_ARGV_BASE + 3];
            unsigned int arg3 = *((unsigned int *)&arg3_d_type);
            d_type arg4_d_type = data[SCHE_SLAVE_ARGV_BASE + 4];
            unsigned int arg4 = *((unsigned int *)&arg4_d_type);
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, arg4, id0, id1, data);
                internal_id = *id;
                if (internal_id == 0xFFFFFFFF) {
                    *barrier = 1;
                }
            }
        }
    }
}