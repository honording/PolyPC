#define SCHE_SLAVE_ARGV_BASE      0x30000020    //0xC0000080
#define SCHE_SLAVE_TRIGGER_BASE   0x30000040    //0xC0000100

void kernel(unsigned int a_addr,
			unsigned int b_addr,
			unsigned int c_addr,
			unsigned int length,
            unsigned int id0,
			unsigned int id1,
            volatile int *data) {
    unsigned int a = a_addr >> 2;
    unsigned int b = b_addr >> 2;
    unsigned int c = c_addr >> 2;
    int i;
    int sum = 0;
    unsigned int col_off = length * id0;
    unsigned int a_off = a + col_off;
    unsigned int b_off = b + id1;
    unsigned int c_off = c + col_off + id1;
    for (i = 0; i < length; i++) {
#pragma HLS PIPELINE
    	sum += data[a_off + i] * data[b_off + length * i];
    	if (i == length - 1) {
    		data[c_off] = sum;
    	}
    }
}

void matrix_mul(volatile unsigned int *id,
                volatile unsigned int *barrier,
                volatile int *data,
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
