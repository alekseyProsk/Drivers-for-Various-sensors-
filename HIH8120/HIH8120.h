#ifndef HIH8120
#define HIH8120
#include "gd32f10x.h"

#define HIH8120_ADDRESS 0x27
#define HIH8120_CMD_READ 0x18


void HIH8120_requestParam(void);

void HIH8120_recieve(void);

void HIH8120_getMeasures( double *measuresArr);

#endif