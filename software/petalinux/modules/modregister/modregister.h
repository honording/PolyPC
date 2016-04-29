#ifndef _MODREGISTER_H_
#define _MODREGISTER_H_

#include "../../../generic/include/thread_struct.h"
#include "../../../generic/include/register.h"
#include "../../../generic/include/debug.h"
#include "../../../generic/include/mutex_manager.h"
#include "../../../generic/include/base_addr.h"


#define MAX_NAME_LENGTH 8
#define MODULE_NAME     "hapara_reg"

#define VALID           1
#define INVALID         0

#define SCHE_BASE_ADDR  ARM_HTDT_BASE
#define SCHE_SIZE       0x8000          //32KByte
// #define SLOT_SIZE       28
#define MAX_SLOT        256         //0x8000 / 28

#define DUMMY           0

struct hapara_register {
    struct cdev cdev;
    void *mmio;
};


#endif
