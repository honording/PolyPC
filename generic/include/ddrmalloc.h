#ifndef _DDRMALLOC_H_
#define _DDRMALLOC_H_

// struct ddrmalloc_req_struct
// {
//     unsigned int size;
//     unsigned int addr;
//     int ret_error;
// };

#define DDRMALLOC_MAGIG     'd'
#define DDRMALLOC_MAX       2
#define DDRMALLOC_ALLOC     _IOW(DDRMALLOC_MAGIG, 0, unsigned int)
#define DDRMALLOC_FREE      _IOW(DDRMALLOC_MAGIG, 1, unsigned int)

#endif