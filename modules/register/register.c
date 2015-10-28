#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include <asm/io.h>

#include "register.h"

static dev_t dev_num;
static struct class *cl;
static struct hapara_register *hapara_registerp;

static int register_open(struct inode *inode, struct file *filp)
{
    filp->private_data = hapara_registerp;
    return 0;
}

static int register_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t register_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct hapara_register *dev = filp->private_data;
    if (copy_to_user(buf, dev->mmio + p, count))
        return -EFAULT;
    else {
        *ppos += count;
        ret = count;
    }
    return ret;
}

static ssize_t register_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct hapara_register *dev = filp->private_data;
    if (copy_from_user(dev->mmio + p, buf, count)) 
        return -EFAULT;
    else {
        *ppos += count;
        ret = count;
    }
    return ret;
}

static loff_t register_llseek(struct file *filp, loff_t offset, int orig)
{   //0: set, 1: cur, 2: end
    loff_t ret = 0;
    switch (orig) {
    case 0:
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset;
        ret = filp->f_pos;
        break;
    case 1:
        if ((filp->f_pos + offset) < 0) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos += offset;
        ret = filp->f_pos;
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static long register_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static const struct file_operations register_fops = {
    .owner = THIS_MODULE,
    .open = register_open,
    .release = register_release,
    .llseek = register_llseek,
    .read = register_read,
    .write = register_write,
    .unlocked_ioctl = register_ioctl,
};

static int __init register_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME);
    if (ret < 0) {
        goto failure_dev_reg;
    }

    cl = class_create(THIS_MODULE, MODULE_NAME);
    if (cl == NULL) {
        goto failure_cl_cr;
    }
    if (device_create(cl, NULL, dev_num, NULL, MODULE_NAME) == NULL) {
        goto failure_dev_cr;
    }
    hapara_registerp = kzalloc(sizeof(struct hapara_register), GFP_KERNEL);
    if (!hapara_registerp) {
        goto failure_alloc;       
    }
    cdev_init(&hapara_registerp->cdev, &register_fops);
    hapara_registerp->cdev.owner = THIS_MODULE;
    if (cdev_add(&hapara_registerp->cdev, dev_num, 1) == -1) {
        goto failure_alloc;
    }

    hapara_registerp->mmio = ioremap(SCHE_BASE_ADDR, SCHE_SIZE);

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

static int __exit register_exit(void)
{
    cdev_del(&hapara_registerp->cdev);
    device_destroy(cl, dev_num);
    class_destroy(cl);
    kfree(hapara_registerp);
    iounmap(hapara_registerp->mmio);
    unregister_chrdev_region(dev_num, 1);
}

module_init(register_init);
module_exit(register_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Driver for host threads to register.");