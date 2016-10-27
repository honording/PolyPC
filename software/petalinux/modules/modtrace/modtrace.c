#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h> 
#include <linux/ioctl.h>
#include <linux/sched.h>
 

#include <asm/uaccess.h> 
#include <asm/io.h>

#include "modtrace.h"

static dev_t dev_num;
static struct class *cl;
static struct hapara_trace *hapara_tracep;

unsigned int off_array[MAX_TRACE_SLOT];
unsigned int total_num = 0;
unsigned int total_off = 0;


static int trace_open(struct inode *inode, struct file *filp)
{
    filp->private_data = hapara_tracep;
    return 0;
}

static int trace_release(struct inode *inode, struct file *filp)
{
    return 0;
}


static int trace_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = -1;

    struct hapara_trace *dev = filp->private_data;
    struct hapara_timer_struct *timer = 
        (struct hapara_timer_struct *)(dev->mmio_timer);
    unsigned int *trace_ram = (unsigned int *)(dev->mmio_trace);

    unsigned int num_group;
    unsigned int off;
    unsigned int conf;
    unsigned int time;
    unsigned int *user_buf = NULL;

    switch (cmd) {
        case HTRACE_TRACE_CLEAR:
            total_num = 0;
            total_off = 0;
            ret = 0;
            break;
        case HTRACE_TRACE_ALLOC:
            if (get_user(num_group, (unsigned int *)arg)) {
                return -EINVAL;
            }             
            total_num++;
            total_off += num_group * sizeof(struct hapara_trace_struct);
            off_array[total_num] = total_off; 
            return off_array[total_num - 1];
            break;
        case HTRACE_TRACE_GETTOTALNUM:
            return total_num;
            break;
        case HTRACE_TRACE_GETTOTALSIZE:
            return total_off;
            break;
        case HTRACE_TRACE_GETTOTALCON:
            user_buf = (unsigned int *)arg;
            if (copy_to_user(user_buf, trace_ram, total_off)) {
                return -EFAULT;
            }
            ret = 0;
            break;
        case HTRACE_TRACE_GETEACHSIZE:
            if (get_user(off, (unsigned int *)arg)) {
                return -EINVAL;
            }
            return off_array[off + 1] - off_array[off];
            break;
        case HTRACE_TRACE_GETEACHOFF:
            if (get_user(off, (unsigned int *)arg)) {
                return -EINVAL;
            }
            return off_array[off];
            break;
        case HTRACE_TIMER_RESET:
            timer->tlr  = 0;
            // conf = timer->tcsr;
            // timer->tcsr = conf | XTC_CSR_LOAD_MASK;
            // timer->tcsr = conf;
            timer->tcsr = 0x120;
            timer->tcsr = 0;
            ret = 0;
            break;
        case HTRACE_TIMER_START:
            // conf = timer->tcsr;
            // timer->tcsr = conf | XTC_CSR_ENABLE_TMR_MASK;
            timer->tcsr = 0x80;
            ret = 0;
            break;
        case HTRACE_TIMER_STOP:
            conf = timer->tcsr;
            conf &= ~(XTC_CSR_ENABLE_TMR_MASK);
            timer->tcsr = conf;
            ret = 0;
            break;
        case HTRACE_TIMER_GETTIME:
            time = timer->tcr;
            if (put_user(time, (unsigned int *)arg)) {
                return -EINVAL;
            }
            // printk(KERN_ALERT "Timer: %X\n", time);
            return 0;
            break;
        default:
            ret = -EINVAL;
            break;
    }
    return ret;
}

static const struct file_operations trace_fops = {
    .owner = THIS_MODULE,
    .open = trace_open,
    .release = trace_release,
    .unlocked_ioctl = trace_ioctl,
};

static int __init trace_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME);
    if (ret < 0)
        goto failure_dev_reg;
    cl = class_create(THIS_MODULE, MODULE_NAME);
    if (cl == NULL) 
        goto failure_cl_cr;
    if (device_create(cl, NULL, dev_num, NULL, MODULE_NAME) == NULL)
        goto failure_dev_cr;
    hapara_tracep = kzalloc(sizeof(struct hapara_trace), GFP_KERNEL);
    if (!hapara_tracep)
        goto failure_alloc;       
    cdev_init(&hapara_tracep->cdev, &trace_fops);
    hapara_tracep->cdev.owner = THIS_MODULE;
    if (cdev_add(&hapara_tracep->cdev, dev_num, 1) == -1)
        goto failure_alloc;
    hapara_tracep->mmio_timer = ioremap(TIMER_BASE, TIMER_SPAN);
    hapara_tracep->mmio_trace = ioremap(TRACE_BASE, TRACE_SPAN);
    total_num = 0;

    return 0;

    failure_alloc:
    device_destroy(cl, dev_num);

    failure_dev_cr:
    class_destroy(cl);

    failure_cl_cr:
    unregister_chrdev_region(dev_num, 1);

    failure_dev_reg:
    return -1;   
}

static int __exit trace_exit(void)
{
    cdev_del(&hapara_tracep->cdev);
    device_destroy(cl, dev_num);
    class_destroy(cl);
    kfree(hapara_tracep);
    iounmap(hapara_tracep->mmio_timer);
    iounmap(hapara_tracep->mmio_trace);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}

module_init(trace_init);
module_exit(trace_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Driver for tracing purposes.");
