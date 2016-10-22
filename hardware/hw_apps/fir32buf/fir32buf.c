#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

#define PRIVATE_MEM

#ifdef PRIVATE_MEM
#include "string.h"
#define N 	32
#define TAP	5
int a_buffer[N + (TAP - 1)];
int b_buffer[N];
int filter[TAP] = {3,2,1,2,3};
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
    int *ina = &(data[a + off - (TAP - 1)]);
    int *inb = &(data[b + off]);
    memcpy((int *)a_buffer, (const int *)ina, sizeof(int) * (N + (TAP - 1)));
    int i,j;
    for (i = 0; i < N; i++) {
#pragma HLS PIPELINE
    	int sum = 0;
    	for (j = 0; j < TAP; j++) {
    		sum += filter[j] * a_buffer[i + j];
    		if (j == TAP - 1) {
    	    	b_buffer[i] = sum;
    		}
    	}
    }
	memcpy((const int *)inb, (int *)b_buffer, sizeof(int) * N);
#else
//	data[c + id1 + id0] = data[a + id1 + id0] * data[b + id1 + id0];
#endif
}

void fir32buf(volatile unsigned int *id,
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
