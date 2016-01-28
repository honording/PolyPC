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

static int hapara_test_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static const struct file_operations hapara_test_fops = {
    .owner = THIS_MODULE,
    .open = hapara_test_open,
    .release = hapara_test_release,
    .llseek = hapara_test_llseek,
    .read = hapara_test_read,
    .write = hapara_test_write,
    .unlocked_ioctl = hapara_test_ioctl,
};

static int __init hapara_test_init(void)
{
    return 0;   
}

static int __exit hapara_test_exit(void)
{
    return 0;
}

module_init(hapara_test_init);
module_exit(hapara_test_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Driver for host threads to hapara_test.");
