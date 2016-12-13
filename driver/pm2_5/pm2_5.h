#ifndef __PM_2_5_H
#define __PM_2_5_H
#include "sys.h"

struct pm2_5{
	u8 data[7]; //data 7 bits  for data store
	u8 flag;   //0-uncompleted  1-completed
	u8 count;
};

void pm2_5_init(void);


#endif
