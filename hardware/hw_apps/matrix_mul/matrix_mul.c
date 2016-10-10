#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM

// This also the size of one item
// RAW_BUF_LENGTH * RAW_BUF_LENGTH
#define RAW_BUF_LENGTH 16
//
#define COL_BUF_LENGTH 16

#ifdef PRIVATE_MEM
#include "string.h"
int a_buffer[COL_BUF_LENGTH];
int b_buffer[COL_BUF_LENGTH][RAW_BUF_LENGTH];
int c_buffer[RAW_BUF_LENGTH][RAW_BUF_LENGTH];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int c_addr,
            unsigned int raw_length,
            unsigned int col_length,
            unsigned int col_buf_num,
            unsigned int id0,
            unsigned int id1,
            int *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;
#ifdef PRIVATE_MEM
    int i, j, jj, jjj, m, n;
    // This is global index;
    unsigned int raw_index = id0 * RAW_BUF_LENGTH;
    unsigned int col_index = id1 * RAW_BUF_LENGTH;
    for (i = 0; i < col_buf_num; i++) {
        // Transfer Matrix B to b_buffer
        unsigned int chunk_off = i * COL_BUF_LENGTH;
        for (j = 0; j < COL_BUF_LENGTH; j++) {
            unsigned int col_off = raw_index;
            unsigned int raw_off = (chunk_off + j) * raw_length;
            int *inb = &(data[b + raw_off + col_off]);
            memcpy((int *)b_buffer[j], (const int *)inb, sizeof(int) * RAW_BUF_LENGTH);
            if (j == COL_BUF_LENGTH - 1) {
                // Travel RAW of Matrix A
                for (jj = 0; jj < RAW_BUF_LENGTH; jj++) {
                    // Transfer Matrix A to a_buffer
                    unsigned int raw_off = (raw_index + jj) * col_length;
                    unsigned int col_off = chunk_off;
                    int *ina = &(data[a + raw_off + col_off]);
                    memcpy((int *)a_buffer, (const int *)ina, sizeof(int) * COL_BUF_LENGTH);
                    // Do the calculation
                    // Travel COL of Matrix B
                    for (m = 0; m < RAW_BUF_LENGTH; m++) {
                        for (n = 0; n < COL_BUF_LENGTH; n++) {
                            c_buffer[jj][m] += a_buffer[n] * b_buffer[n][m];
                        }
                    }
                }
                for (jjj = 0; jjj < RAW_BUF_LENGTH; jjj++) {
                    int *inc = &(data[c + (raw_index + jjj) * RAW_BUF_LENGTH + col_index]);
                    memcpy((int *)inc, (const int *)c_buffer[i], sizeof(int) * RAW_BUF_LENGTH);
                }              
            }
        }
    }

#else
    int i;
    int sum = 0;
    unsigned int raw_off = col_length * id0;
    unsigned int a_off = a + raw_off;
    unsigned int b_off = b + id1;
    unsigned int c_off = c + raw_off + id1;
    for (i = 0; i < col_length; i++) {
#pragma HLS PIPELINE
        sum += data[a_off + i] * data[b_off + col_length * i];
        if (i == col_length - 1) {
            data[c_off] = sum;
        }
    }
#endif
}

void matrix_mul(volatile unsigned int *id,
                volatile unsigned int *barrier,
                int *data,
                char htID) {

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE m_axi port=data
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
            // raw length
            unsigned int arg3 = data[SCHE_SLAVE_ARGV_BASE + 3];
            // col length
            unsigned int arg4 = data[SCHE_SLAVE_ARGV_BASE + 4];
            unsigned int col_buf_num = arg3 / RAW_BUF_LENGTH;
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, arg4, col_buf_num, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}
