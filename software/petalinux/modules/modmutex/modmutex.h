#ifndef __MODMUTEX_H__
#define __MODMUTEX_H__

#include <linux/types.h>
#include <linux/mutex.h>

#include "../../../generic/include/debug.h"
#include "../../../generic/include/mutex_manager.h"
#include "../../../generic/include/base_addr.h"

#define MUTEX_BASE_ADDR ARM_MUTEX_MANAGER_BASE
#define MUTEX_MEM_SIZE  0x1000
#define MUTEX_SIZE      sizeof(struct hapara_mutex_pair)
#define MUTEX_NUM       16

#define SET             1
#define UNSET           0

// struct hapara_mutex_pair
// {
//     volatile uint32_t    reg0;
//     volatile uint32_t    reg1;
//     volatile uint32_t    reg2;
// };

struct hapara_mutex_manager
{
    volatile void *mmio;
    struct mutex mutex_internal;
};

extern void hapara_req_lock(unsigned int num);
extern void hapara_rel_lock(unsigned int num);

#endif
