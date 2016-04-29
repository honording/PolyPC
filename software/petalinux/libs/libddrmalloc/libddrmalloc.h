#ifndef _LIBDDRMALLOC_H_
#define _LIBDDRMALLOC_H_

#include "../../../generic/include/ddrmalloc.h"

#define FILEPATH    "/dev/hapara_ddr_malloc"

int ddr_malloc(int size);
int ddr_malloc(int addr);

void ddr_list_print();


#endif