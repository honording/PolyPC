#ifndef _MODREGISTER_H_
#define _MODREGISTER_H_

#include "../../generic/include/thread_struct.h"
#include "../../generic/include/register.h"

#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

#define VALID           1
#define INVALID         0

#define SCHE_BASE_ADDR  0x40000000
#define SCHE_SIZE       0x1000
#define SLOT_SIZE       16
#define MAX_SLOT        256         //0x1000 / 16 = 256


struct hapara_register {
    struct cdev cdev;
    void *mmio;
};

#endif