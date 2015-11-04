#include "../../include/thread_struct.h"

#define FILEPATH    "/dev/hapara_reg"

struct hapara_id_pair {
    u32 id0;
    u32 id1;
};


struct hapara_thread_struct {
    u8 valid;                       //1: valid; 0: invalid
    u8 priority;
    u8 type_num;
    u8 next;
    u32 tid;
    struct hapara_id_pair group_id;
}; // 1+1+1+1+4+4+4=16 bytes

void libregister_test(void);
void print_struct(struct hapara_thread_struct *sp);
