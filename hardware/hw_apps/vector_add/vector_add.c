//#define SCHE_SLAVE_ARGV_BASE     0xF0000080    //0xC0000000
//#define SCHE_SLAVE_TRIGGER_BASE 0xF0000040  //0xC0000100

#define SCHE_SLAVE_ARGV_BASE     0xF0000000    //0xC0000000
#define SCHE_SLAVE_TRIGGER_BASE 0xF0800000  //0xC2000000

void kernel(unsigned int arg0, unsigned int arg1, unsigned int arg2, 
            unsigned int id0, unsigned int id1, 
            int *data) {
    unsigned int index0 = arg0 >> 2;
    unsigned int index1 = arg1 >> 2;
    unsigned int index2 = arg2 >> 2;
    data[index2 + id0 + id1] = data[index0 + id0 + id1] + data[index1 + id0 + id1];
}

void vector_add(volatile unsigned int *id,
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
            internal_id = *id;
            while (internal_id != 0xFFFFFFFF) {
                id0 = internal_id >> 16;
                id1 = internal_id & 0x0000FFFF;
                kernel(arg0, arg1, arg2, id0, id1, data);
                internal_id = *id;
            }
        }
    }
}
