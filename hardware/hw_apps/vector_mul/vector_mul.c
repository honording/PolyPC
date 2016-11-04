#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM

#ifdef PRIVATE_MEM
#include "string.h"
#define N 128
int a_buffer[N];
int b_buffer[N];
int c_buffer[N];
#endif

void kernel(unsigned int a_addr,
			unsigned int b_addr,
			unsigned int c_addr,
            unsigned int buf_size,
            unsigned int id0,
			unsigned int id1,
            volatile int *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;
#ifdef PRIVATE_MEM
    unsigned int off = id1 * buf_size;
    int *ina = &(data[a + off]);
    int *inb = &(data[b + off]);
    int *inc = &(data[c + off]);
    memcpy((int *)a_buffer, (const int *)ina, sizeof(int) * buf_size);
    memcpy((int *)b_buffer, (const int *)inb, sizeof(int) * buf_size);
    int i;
	for (i = 0; i < buf_size; i++) {
#pragma HLS PIPELINE II=1
		c_buffer[i] = a_buffer[i] * b_buffer[i];
	}
	memcpy((const int *)inc, (int *)c_buffer, sizeof(int) * buf_size);
#else
	data[c + id1 + id0] = data[a + id1 + id0] * data[b + id1 + id0];
#endif
}

void vector_mul(volatile unsigned int *id,
                volatile unsigned int *barrier,
                volatile int *data,
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
            unsigned int arg3 = data[SCHE_SLAVE_ARGV_BASE + 3];
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, arg3, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}
