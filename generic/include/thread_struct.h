#ifndef _THREAD_STRUCT_H_
#define _THREAD_STRUCT_H_

#ifdef __USER_PROGRAMS__
#include <stdint.h>
#else
#include <linux/types.h>
#endif

#include "elf_loader.h"
#include "pr_loader.h"

#define ARGC    16

struct hapara_id_pair {
    uint32_t id0;
    uint32_t id1;
};

// struct hapara_reg_pair {
//     uint8_t off;
//     uint32_t target;
// };

// struct hapara_thread_struct {
//     uint8_t valid;                       //1: valid; 0: invalid
//     uint8_t type;
//     uint8_t priority;
//     uint8_t next;
//     uint8_t tid;
//     struct hapara_id_pair group_id;
//     elf_info_t elf_info;
// }__attribute__((aligned(4)));

struct hapara_thread_struct {
    uint8_t isValid;
    uint8_t priority;
    uint8_t prev;
    uint8_t next;
    int tid;
    struct hapara_id_pair group_size;       //size per group
    struct hapara_id_pair group_num;        //number of groups
    struct hapara_id_pair cur_group_id;
    elf_info_t elf_info;
    pr_info_t  pr_info;
    uint32_t argv[ARGC];
}__attribute__((aligned(4)));

#endif
