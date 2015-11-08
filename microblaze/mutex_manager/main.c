#define MUTEX_MEM_BASE  0x42000000
#define MAX_MUTEX       16

#define RESET           0
#define SET             1

struct mutex_pair_struct {
    unsigned int r_m;
    unsigned int w_m;
};

char hash_table[MAX_MUTEX] = {0};

int main(void)
{
    int i = 0;
    volatile (struct mutex_pair_struct *)mutex_mem = 
        (struct mutex_pair_struct *)MUTEX_MEM_BASE;
    unsigned int r_m = 0;
    unsigned int w_m = 0;
    while (1) {
        for (i = 0; i < MAX_MUTEX; i++) {
            if (mutex_mem[i].r_m == SET) { //some thread want to requst
                if (hash_table[i] == RESET) {
                    //no thread occupy
                    hash_table[i] = SET;
                    mutex_mem[i].w_m = SET;
                } else {
                    //other thread have occupied
                    ;
                }
            } else {
                if (mutex_mem[i].w_m == SET) {
                    //this thread have occupied
                    //but released this time
                    hash_table[i] = RESET;
                    mutex_mem[i] = RESET;
                }
            }
        }    
    }
    return 0;
}