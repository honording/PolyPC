#include <linux/types.h>
#include <linux/cdev.h>

#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

#define VLIAD           1
#define INVALID         0

#define OFF_VALID       0
#define OFF_PRIORITY    1
#define OFF_TYPE        2
#define OFF_NEXT        3
#define OFF_TID         4

#define SCHE_BASE_ADDR  0x40000000
#define SCHE_SIZE       0x1000
#define SLOT_SIZE       16
#define MAX_SLOT        256         //0x1000 / 16 = 256

struct hapara_id_pair {
    u32 id0;
    u32 id1;
};

struct hapara_thread_struct {
    u8 valid;                       //1: valid; 0: invalid
    u8 priority;
    u8 type;
    u8 next;
    u8 tid;
    struct hapara_id_pair group_id;
}__attribute__((aligned(4)));

struct hapara_register {
    struct cdev cdev;
    void *mmio;
};