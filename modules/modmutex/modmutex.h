#ifndef __MODMUTEX_H__
#define __MODMUTEX_H__

#include <linux/types.h>
#include <linux/mutex.h>

#include "../../../generic/include/debug.h"
#include "../../../generic/include/mutex_manager.h"

#define MUTEX_BASE_ADDR 0x40000000
#define MUTEX_MEM_SIZE  0x1000
//#define MUTEX_SIZE      8
#define MUTEX_NUM       16

#define SET             1
#define UNSET           0

struct hapara_mutex_pair
{
    uint32_t    reg0;
    uint32_t    reg1;
    uint32_t    reg2;
};

struct hapara_mutex_manager
{
    void *mmio;
    struct mutex mutex_internal;
};

extern void hapara_req_lock(unsigned int num);
extern void hapara_rel_lock(unsigned int num);

#endif
