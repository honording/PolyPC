#ifndef __LIBPR_H__
#define __LIBPR_H__

#include "../../../generic/include/pr_loader.h"

#include "../libddrmalloc/libddrmalloc.h"

#define DEVMEM  	"/dev/mem"
#define MAX_LENGTH	256


int pr_loader(char *file_path, 
              pr_info_t *pr_info);
void disable_mb_pr(pr_info_t *pr_info);

#endif
