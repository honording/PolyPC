#include <linux/types.h>
#include <linux/cdev.h>

#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

struct hapara_id_pair {
    u32 id0;
    u32 id1;
};

struct hapara_thread_struct {
    char[MAX_NAME_LENGTH] app_name; // bitstream name;
    u32 tid;
    u8 priority;
    struct hapara_id_pair group_id;
    u8 next;
};

struct hapara_register {
    struct cdev cdev;
};