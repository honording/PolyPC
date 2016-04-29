
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h> 
#include <linux/sched.h>

#include <asm/uaccess.h> 
#include <asm/io.h>

#include "modmutex.h"

static struct hapara_mutex_manager *mutex_managerp;

static void print_mutex(struct hapara_mutex_pair * mutex_mem);

void hapara_req_lock(unsigned int num)
{
/*
    note:   W_t: reg0;
            R_t: reg1;
            WR_t:reg2;

    1.Set W_t to its ID.
    2.Check R_t see if it equals to its ID. if not goto 1.
    3.Once approved, Reset W_t.
*/
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Enter hapara_req_lock\n");
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
#endif
    mutex_lock(&mutex_managerp->mutex_internal);
    int pid = current->pid;
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Current offset: %d, name:%s, PID:%d\n", num, current->comm, pid);
#endif
    while (((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg1 != pid)
        ((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg0 = pid;
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
    ((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg0 = UNSET;
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Out hapara_reg_lock\n");
#endif
}

void hapara_rel_lock(unsigned int num)
{
/*
    note:   W_t: reg0;
            R_t: reg1;
            WR_t:reg2;

    1.Set WR_t to its ID.
    2.Keep polling WR_t to check if it has been reset.
    3.Once approved, continue to run.
*/
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Enter hapara_rel_lock\n");
#endif
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
    ((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg2 = current->pid;
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
    while (((struct hapara_mutex_pair *)mutex_managerp->mmio + num)->reg2 != UNSET)
        ;    
    //print_mutex(((struct hapara_mutex_pair *)(mutex_managerp->mmio) + num));
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Out mutex manager lock\n");
#endif
    mutex_unlock(&mutex_managerp->mutex_internal);
#ifdef __MUTEX_DEBUG__
    printk(KERN_DEBUG "Out hapara_rel_lock\n");
#endif
}

static int __init hapara_mutex_init(void)
{
    mutex_managerp = kzalloc(sizeof(struct hapara_mutex_manager), GFP_KERNEL);
    if (!mutex_managerp)
        return -1;

#ifdef  __MUTEX_DDR_MEM__
#ifdef  __MUTEX_DEBUG__
    printk(KERN_DEBUG "Mutex mapped to DDR.\n");uendif
#endif
    mutex_managerp->mmio = kzalloc(sizeof(MUTEX_SIZE * MUTEX_NUM), GFP_KERNEL);
    if (!mutex_managerp->mmio) {
        kfree(mutex_managerp);
        return -1;
    }
#else
#ifdef __MUTEX_DEBUG__ 
    printk(KERN_DEBUG "Mutex mapped to BRAM.\n");
#endif
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

static void print_mutex(struct hapara_mutex_pair * mutex_mem) 
{
    printk(KERN_NOTICE "reg0:%d;", mutex_mem->reg0);
    printk(KERN_NOTICE "reg1:%d;", mutex_mem->reg1);
    printk(KERN_NOTICE "reg2:%d.\n", mutex_mem->reg2);
}

EXPORT_SYMBOL(hapara_req_lock);
EXPORT_SYMBOL(hapara_rel_lock);
module_init(hapara_mutex_init);
module_exit(hapara_mutex_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Mutex manager providing shared functions.");
