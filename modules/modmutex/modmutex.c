
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h> 

#include <asm/uaccess.h> 
#include <asm/io.h>

#include "modmutex.h"

static struct hapara_mutex_manager *mutex_managerp;

void hapara_req_lock(unsigned int num)
{
/*
    note:   W_t: reg1;
            R_t: reg2;

    Set W_t.
    Keep polling R_t to check if it has been set.
    Once approved, continue to run.
*/
    mutex_lock(&mutex_managerp->mutex_internal);
    ((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg1 = SET;
    while (((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg2 != SET)
        ;
}

void hapara_rel_lock(unsigned int num)
{
/*
    note:   W_t: reg1;
            R_t: reg2;

    Reset W_t.
    Keep polling R_t to check if it has been reset.
    Once approved, continue to run.
*/
    ((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg1 = UNSET;
    while (((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg2 != UNSET)
        ;    
    mutex_unlock(&mutex_managerp->mutex_internal);
}

static int __init hapara_mutex_init(void)
{
    mutex_managerp = kzalloc(sizeof(struct hapara_mutex_manager), GFP_KERNEL);
    if (!mutex_managerp)
        return -1;
#ifdef  __MUTEX_DDR_MEM__
    mutex_managerp->mmio = kzalloc(sizeof(MUTEX_SIZE * MUTEX_NUM), GFP_KERNEL);
    if (!mutex_managerp->mmio) {
        kfree(mutex_managerp);
        return -1;
    }
#else
    mutex_managerp->mmio = ioremap(MUTEX_BASE_ADDR, MUTEX_MEM_SIZE);
#endif
    mutex_init(&mutex_managerp->mutex_internal);
    return 0;
}

static int __exit hapara_mutex_exit(void)
{
#ifdef __MUTEX_DDR_MEM__
    kfree(mutex_managerp->mmio);
#endif
    kfree(mutex_managerp);
    return 0;
}

EXPORT_SYMBOL(hapara_req_lock);
EXPORT_SYMBOL(hapara_rel_lock);
module_init(hapara_mutex_init);
module_exit(hapara_mutex_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Mutex manager providing shared functions.");