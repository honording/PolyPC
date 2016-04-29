#ifndef _PR_LOADER_H
#define _PR_LOADER_H


struct pr_info_struct {
    unsigned int begin_addr;
    unsigned int num_pr_file;
    unsigned int total_size;
};

typedef struct pr_info_struct pr_info_t;

#endif