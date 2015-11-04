#include "../../include/thread_struct.h"
#include <linux/cdev.h>

#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

#define VALID           1
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


struct hapara_register {
    struct cdev cdev;
    void *mmio;
};


#define REG_MAGIC       'r'
#define REG_MAX         4
#define REG_ADD         _IOW(REG_MAGIC, 0, struct hapara_thread_struct)
#define REG_DEL         _IOW(REG_MAGIC, 1, struct hapara_reg_pair)