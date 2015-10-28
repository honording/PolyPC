#include <stdint.h>

/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
*/

#define FILEPATH    "/dev/hapara_reg"
#define u8 uint8_t
#define u32 uint32_t


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
