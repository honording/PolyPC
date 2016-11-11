#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

typedef int d_type;

#define PRIVATE_MEM

#define BUF_LENGTH 16
#ifdef PRIVATE_MEM
#include "string.h"
d_type a_buffer[BUF_LENGTH][BUF_LENGTH];
d_type b_buffer[BUF_LENGTH][BUF_LENGTH];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int c_addr,
            unsigned int length,
            unsigned int id0,
            unsigned int id1,
            d_type *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;
    unsigned int raw_off = id0 * BUF_LENGTH * length;
    unsigned int col_off = id1 * BUF_LENGTH;

    unsigned int start_off = raw_off + col_off;

    unsigned int num_of_chunk = length / BUF_LENGTH;
    int i, j;
    int m, n, k;
    volatile int vi = 0;
    volatile int vj = 0;
    d_type c_buffer[BUF_LENGTH][BUF_LENGTH] = {0};
    for (i = 0; i < num_of_chunk; vi = ++i) {
        for (j = 0; j < BUF_LENGTH; vj = ++j) {
            d_type *ina = &(data[a + raw_off + j * length + i * BUF_LENGTH]);
            d_type *inb = &(data[b + col_off + (i * BUF_LENGTH + j) * length]);
            memcpy((d_type *)a_buffer[j], (const d_type *)ina, sizeof(d_type) * BUF_LENGTH);
            memcpy((d_type *)b_buffer[j], (const d_type *)inb, sizeof(d_type) * BUF_LENGTH);
        }
        if (vj == BUF_LENGTH) {
            for (m = 0; m < BUF_LENGTH; m++) {          // raw
                for (n = 0; n < BUF_LENGTH; n++) {      // col
                    for (k = 0; k < BUF_LENGTH; k++) {
#pragma HLS PIPELINE
                        c_buffer[m][n] += a_buffer[m][k] * b_buffer[k][n];
                    }
                }
            }
        }
    }
    if (vi == num_of_chunk) {
        for (i = 0; i < BUF_LENGTH; i++) {
            d_type *inc = &(data[c + start_off + i * length]);
            memcpy((const d_type *)inc, (d_type *)c_buffer[i], sizeof(d_type) * BUF_LENGTH);
        }
    }
}

void matrix_mul(volatile unsigned int *id,
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
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, id0, id1, data);
                internal_id = *id;
                if (internal_id == 0xFFFFFFFF) {
                    *barrier = 1;
                }
            }
        }
    }
}

