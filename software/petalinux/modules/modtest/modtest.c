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

static struct cdev cdev;
static dev_t dev_num;
static struct class *cl;

static void *mmio;

#define MODULE_NAME     "hapara_test"


static int hapara_test_open(struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG "Current name:%s, PID:%d", current->comm, current->pid);
    return 0;
}

static int hapara_test_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t hapara_test_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t hapara_test_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static loff_t hapara_test_llseek(struct file *filp, loff_t offset, int orig)
{   //0: set, 1: cur, 2: end
    return 0;
}

static const struct file_operations hapara_test_fops = {
    .owner = THIS_MODULE,
    .open = hapara_test_open,
    .release = hapara_test_release,
    .llseek = hapara_test_llseek,
    .read = hapara_test_read,
    .write = hapara_test_write,
};

static int __init hapara_test_init(void)
{
    mmio = ioremap(0x40000000, 0x1000);
    printk(KERN_NOTICE "mmio addr:%x\n", mmio);
    *((int *)mmio) = 0xbabeface;
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME);
    if (ret < 0)
        goto failure_dev_reg;
    cl = class_create(THIS_MODULE, MODULE_NAME);
    if (cl == NULL) 
        goto failure_cl_cr;
    if (device_create(cl, NULL, dev_num, NULL, MODULE_NAME) == NULL)
        goto failure_dev_cr;     
    cdev_init(&cdev, &hapara_test_fops);
    cdev.owner = THIS_MODULE;
    if (cdev_add(&cdev, dev_num, 1) == -1)
        goto failure_alloc;


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

static int __exit hapara_test_exit(void)
{
    cdev_del(&cdev);
    device_destroy(cl, dev_num);
    class_destroy(cl);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}

module_init(hapara_test_init);
module_exit(hapara_test_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Driver for host threads to hapara_test.");
