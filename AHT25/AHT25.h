#ifndef AHT25
#define AHT25

#include "gd32f10x.h"


#define AHT25_Adress 0x38 

#define AHT25_INIT 0x71

#define AHT25_MEASURE_REQUEST 0xAC


void AHT25_sendCmd(uint8_t cmd);

void AHT25_recieve(void);

void AHT25_getMeasures( double *measuresArr);


#endif
