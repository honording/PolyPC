#include "../../generic/include/hw_config.h"
#include "../../generic/include/base_addr.h"

#define MUTEX_MEM_BASE  MUTEX_MANAGER_BASE
#define MAX_MUTEX       NUM_MUTEX

#define RESET           0

struct mutex_pair_struct {
    unsigned int r_m;
    unsigned int w_m;
    unsigned int wr_m;
};

int main(void)
{
    int i = 0;
    struct mutex_pair_struct * mutex_mem;
    mutex_mem = (struct mutex_pair_struct *)MUTEX_MEM_BASE;
    while (1) {
        for (i = 0; i < MAX_MUTEX; i++) {
            if (mutex_mem[i].w_m == 0)
                mutex_mem[i].w_m = mutex_mem[i].r_m;
            if (mutex_mem[i].wr_m != 0) {
                mutex_mem[i].w_m = 0;
                mutex_mem[i].wr_m = 0;
            }
        }
    }
    return 0;
}

