#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

typedef int d_type;

#define PRIVATE_MEM

#ifdef PRIVATE_MEM
#include "string.h"
#define N   128
d_type a_buffer[N];
#endif

void kernel(unsigned int a_addr,
			unsigned int buf_size,
            unsigned int id0,
            unsigned int id1,
            d_type *data) {
    unsigned int a = a_addr >> 2;
#ifdef PRIVATE_MEM
    unsigned int off = id1 * buf_size;
    d_type *ina = &(data[a + off]);
    memcpy((d_type *)a_buffer, (const d_type *)ina, sizeof(d_type) * buf_size);
#else

#endif
}

void mem_r(volatile unsigned int *id,
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
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, id0, id1, data);
                internal_id = *id;
                if (internal_id == 0xFFFFFFFF) {
                    *barrier = 1;
                }
            }
        }
    }
}