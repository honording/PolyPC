#include <linux/types.h>
#include <linux/cdev.h>

#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

#define SCHE_BASE_ADDR  0x0
#define SCHE_SIZE       0x1

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

struct hapara_register {
    struct cdev cdev;
    void *mmio;
};