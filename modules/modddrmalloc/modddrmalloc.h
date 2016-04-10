#ifndef _MODDDRMALLOC_H_
#define _MODDDRMALLOC_H_

#include "../../../generic/include/base_addr.h"

#include <linux/mutex.h>

#define MODULE_NAME     "hapara_ddr_malloc"
#define BEGEIN_ADDR     SLAVE_DDR_BASE
#define MAX_SIZE        0x1F000000          //496MB
/*
 * This boundary is for DMA transferring
 * which means the size to allocate has to be integer multiples of boundary size
 */
#define BOUNDARY        0x1000              //4KB

 struct hapara_ddr_malloc
 {
    int curr_max_size;
    struct cdev cdev;
    struct mutex list_mutex;
 };

#endif