#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h> 
#include <linux/ioctl.h>
 

#include <asm/uaccess.h> 
#include <asm/io.h>

#include "modddrmalloc.h"

static dev_t dev_num;
static struct class *cl;
static struct hapara_ddr_malloc *hapara_ddr_mallocp;



static int ddr_malloc_open(struct inode *inode, struct file *filp)
{
    filp->private_data = hapara_ddr_mallocp;
    mutex_lock(&hapara_ddr_mallocp->list_mutex);
    return 0;
}

static int ddr_malloc_release(struct inode *inode, struct file *filp)
{
    mutex_unlock(&hapara_ddr_mallocp->list_mutex);
    return 0;
}

static int ddr_allocate(struct hapara_ddr_malloc *dev, unsigned int size)
{
    int ret = -1;
    if (dev->max_available_slot < size)
        return ret;
    struct hapara_ddr_malloc_list *curr_node;
    list_for_each_entry(curr_node, &dummy, list)
    {
        if (curr_node->is_free && curr_node->size == size) {
            curr_node->is_free = 0;
            ret = curr_node->begin_addr;
            break;
        } else if (curr_node->is_free && curr_node->size > size) {
            struct hapara_ddr_malloc_list *new_node =
                    kmalloc(sizeof(struct hapara_ddr_malloc_list), GFP_KERNEL);
            int quotient = 0;
            int old_size = size;
            while (size >= BOUNDARY) {
                size = size >> 12;          //boundary 4K;
                quotient++;
            }
            old_size = old_size - size;
            int new_node_begin_addr = curr_node->begin_addr;
            int new_node_size       = (size == 0)?old_size:old_size+BOUNDARY;
            new_node->is_free       = 0;
            new_node->begin_addr    = new_node_begin_addr;
            new_node->size          = new_node_size;
            curr_node->begin_addr   = curr_node->begin_addr + new_node_size;
            curr_node->size         = curr_node->size - new_node_size;
            list_add(&new_node->list, &curr_node->list);
            ret = new_node_begin_addr;
            break;
        }
    }
    return ret;
}

static void print_list(struct hapara_ddr_malloc_list *head)
{
    struct hapara_ddr_malloc_list *curr;
    list_for_each_entry(curr, &dummy, list)
    {
        printk(KERN_DEBUG "---------------------------------\n");
        printk(KERN_DEBUG "is_free:%d\n", curr->is_free);
        printk(KERN_DEBUG "begin_addr:%d\n", curr->begin_addr);
        printk(KERN_DEBUG "size:%d\n", curr->size);
        printk(KERN_DEBUG "---------------------------------\n");
    }
}

static int ddr_free(struct hapara_ddr_malloc *dev, unsigned int addr) 
{
    return 0;
}

static int ddr_malloc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = -1;
    struct hapara_ddr_malloc *dev = filp->private_data;
    unsigned int kernel_arg;
    if (get_user(kernel_arg, (unsigned int *)arg))
        return ret;
    switch (cmd) {
    case DDRMALLOC_ALLOC:
        ret = ddr_allocate(dev, kernel_arg);
        break;
    case DDRMALLOC_FREE:
        break;
    default:
        break;
    }
    return ret;
}

//Un-initialized components are assigned with NUM by GCC
static const struct file_operations ddr_malloc_fops = {
    .owner          = THIS_MODULE,
    .open           = ddr_malloc_open,
    .release        = ddr_malloc_release,
    .unlocked_ioctl = ddr_malloc_ioctl,
};

static int __init ddr_malloc_init(void)
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
    hapara_ddr_mallocp = kzalloc(sizeof(struct hapara_ddr_malloc), GFP_KERNEL);
    if (!hapara_ddr_mallocp)
        goto failure_alloc;       
    cdev_init(&hapara_ddr_mallocp->cdev, &ddr_malloc_fops);
    hapara_ddr_mallocp->cdev.owner = THIS_MODULE;
    if (cdev_add(&hapara_ddr_mallocp->cdev, dev_num, 1) == -1)
        goto failure_alloc;
    //initialize mutex
    mutex_init(&hapara_ddr_mallocp->list_mutex);
    //initilize struct pointer
    hapara_ddr_mallocp->max_available_slot  = MAX_SIZE;
    hapara_ddr_mallocp->cur_usage           = 0;
    //initilize list head
    struct hapara_ddr_malloc_list * ddr_malloc_head;
    ddr_malloc_head = kmalloc(sizeof(struct hapara_ddr_malloc_list), GFP_KERNEL);
    ddr_malloc_head->is_free = 1;
    ddr_malloc_head->begin_addr = BEGIN_ADDR;
    ddr_malloc_head->size = MAX_SIZE;
    INIT_LIST_HEAD(&ddr_malloc_head->list);
    list_add(&ddr_malloc_head->list, &dummy);
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

static int __exit ddr_malloc_exit(void)
{   
    cdev_del(&hapara_ddr_mallocp->cdev);
    device_destroy(cl, dev_num);
    class_destroy(cl);
    kfree(hapara_ddr_mallocp);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}
module_init(ddr_malloc_init);
module_exit(ddr_malloc_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Interface for users to allocate DDR memory.");