#ifndef _MODDDRMALLOC_H_
#define _MODDDRMALLOC_H_

#include "../../../generic/include/base_addr.h"
#include "../../../generic/include/ddrmalloc.h"

#include <linux/mutex.h>
#include <linux/list.h>

#define MODULE_NAME     "hapara_ddr_malloc"
#define BEGIN_ADDR      SLAVE_DDR_BASE
#define MAX_SIZE        FPGA_DDR_MAX_SIZE          //496MB
/*
 * This boundary is for DMA transferring
 * which means the size to allocate has to be integer multiples of boundary size
 */
#define BOUNDARY        0x1000              //4KB
#define BOUNDARY_MASK   (BOUNDARY - 1)      //0xFFF

struct hapara_ddr_malloc
{
    int max_available_slot;
    int cur_usage;
    struct cdev cdev;
    struct mutex list_mutex;
};

struct hapara_ddr_malloc_list
{
    int is_free;
    int begin_addr;
    int size;
    struct list_head list;
};

LIST_HEAD(dummy);

#endif