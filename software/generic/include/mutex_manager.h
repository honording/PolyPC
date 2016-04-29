#ifndef	__MUTEX_MANAGER__
#define	__MUTEX_MANAGER__
#define REG_MUTEX	0
#define ICAP_MUTEX  1

struct hapara_mutex_pair
{
	volatile uint32_t reg0;
	volatile uint32_t reg1;
	volatile uint32_t reg2;
};


#endif
