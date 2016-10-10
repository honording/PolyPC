#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM

#ifdef PRIVATE_MEM
#include "string.h"
#define N   32
int a_buffer[N];
int b_buffer[N];
#endif

void kernel(unsigned int a_addr,
            unsigned int b_addr,
            unsigned int id0,
            unsigned int id1,
            volatile int *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
#ifdef PRIVATE_MEM
    unsigned int off = id1 * N;
    int *ina = &(data[a + off]);
    int *inb = &(data[b + off]);
    memcpy((int *)a_buffer, (const int *)ina, sizeof(int) * N);
    memcpy((const int *)inb, (int *)b_buffer, sizeof(int) * N);
#else

#endif
}

void mem_rw(volatile unsigned int *id,
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
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}
