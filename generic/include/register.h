#ifndef _REGISTER_H_
#define _REGISTER_H_

#define REG_MAGIC       'r'
#define REG_MAX         5
#define REG_CLR         _IO(REG_MAGIC, 0)
#define REG_ADD         _IOW(REG_MAGIC, 1, struct hapara_thread_struct)
#define REG_DEL         _IOW(REG_MAGIC, 2, int)
#define REG_SEARCH_DEL  _IOW(REG_MAGIC, 3, int)
#define REG_PRINT_LIST  _IO(REG_MAGIC, 4)

/*
#define OFF_VALID       0
#define OFF_TYPE        1
#define OFF_PRIORITY    2
#define OFF_NEXT        3
*/
#define OFF_TID         4

#define RESERVED_TID   0

#endif
