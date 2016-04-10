#ifndef _BASE_ADDR_H_
#define _BASE_ADDR_H_

#define ARM_MUTEX_MANAGER_BASE              0x40010000
#define ARM_HTDT_BASE                       0x40000000
#define ARM_DDR_BASE                        0x60000000

#define DMA_LOCAL_MEM_BASE                  0xC0000000
#define DMA_INST_MEM_BASE                   0xC2000000
#define DMA_DDR_BASE                        0x60000000

#define SCHEDULER_DMA_BASE                  0x44A10000
#define SCHEDULER_MUTEX_MANAGER_BASE        0x40010000
#define SCHEDULER_HTDT_BASE                 0x40000000
#define SCHEDULER_LOCAL_MEM_BASE            0xC0000000
#define SCHEDULER_ID_GENERATOR_BASE         0x44A00000

#define SLAVE_LOCAL_MEM_BASE                0xC0000000
#define SLAVE_INST_MEM_BASE                 0x00008000
#define SLAVE_DDR_BASE                      0x60000000

#endif