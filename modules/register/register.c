#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "register.h"

static dev_t dev_num;
static struct class *cl;
static struct hapara_register *hapara_registerp;

static const struct file_operations register_fops = {
    .owner = THIS_MODULE,
    .open = 
    .release = 
    .read = 
    .write = 
    .unlocked_ioctl = 
};

static int __init register_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME);
    if (ret < 0) {
        goto failure_dev_reg;
    }

    cl = class_create(cl, NULL, dev_num, NULL, MODULE_NAME);
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
    unregister_chrdev_region(dev_num, 1);
}
module_init(register_init);
module_exit(register_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Driver for host threads to register.");