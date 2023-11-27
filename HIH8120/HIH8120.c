#include "gd32f10x_i2c.h"
#include "HIH8120.h"
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include "systick.h"

uint8_t HIH8120_RX_Data[3];


void HIH8120_requestParam(void)
{
	
	while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)){}

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
		
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){} 
	
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, HIH8120_ADDRESS << 1 , I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
		
	uint32_t k = 0;
	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) 
	{
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) 
	{
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return;
        }
        // in case no bus is connected
        if (k++ > 1000 * 100)
				{
            i2c_stop_on_bus(I2C0);
            return;
        }
    };
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);////must be
	while(!i2c_flag_get(I2C0, I2C_FLAG_TBE)){}//
		
	i2c_ack_config(I2C0, I2C_ACKPOS_CURRENT);
		
        /* wait until the TBE bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE)){}
		
	i2c_stop_on_bus(I2C0);
    /* wait until stop condition generate */ 
	
	while (I2C_CTL0(I2C0) & I2C_CTL0_STOP){}  
		
}

void HIH8120_recieve(void)
{
	
	uint8_t i = 0;
	//i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, HIH8120_ADDRESS);
	
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)){}

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
		
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){} 
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, HIH8120_ADDRESS << 1, I2C_RECEIVER);
    /* disable ACK before clearing ADDSEND bit */
    //i2c_ack_config(I2C1, I2C_ACK_DISABLE);
    /* wait until ADDSEND bit is set */
    uint32_t k = 0;
	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) 
	{
        if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) 
		{
            i2c_flag_clear(I2C0, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C0);
            return 0;
        }
        // in case no bus is connected
        if (k++ > 1000 * 100)
				{
            i2c_stop_on_bus(I2C0);
            return 0;
        }
    };
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
		
	for(i = 0; i < sizeof(HIH8120_RX_Data); i++) 
	{
        if(1 == i) 
        {
            /* wait until the second last data byte is received into the shift register */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
            /* disable acknowledge */
            i2c_ack_config(I2C0, I2C_ACK_DISABLE);
        }
        /* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
        /* read a data from I2C_DATA */
        HIH8120_RX_Data[i] = i2c_data_receive(I2C0);
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    /* wait until stop condition generate */
    while(I2C_CTL0(I2C0) & 0x0200);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
		
}


void HIH8120_getMeasures( double *measuresArr)
{
	uint32_t HIH8120_ADC_Raw;
	double HIH8120_Temperature;
	double HIH8120_Humidity;

	HIH8120_requestParam();

	HIH8120_recieve();
	
	if(~HIH8120_RX_Data[0] & 0x80)
	{
		/* Convert to Temperature in °C */
        HIH8120_ADC_Raw = (((uint32_t)HIH8120_RX_Data[2] << 6) | ((uint32_t)HIH8120_RX_Data[3] >> 2));
		HIH8120_Temperature = (float)(HIH8120_ADC_Raw * 165.00 / 16382.00) - 40.00;
		/* Convert to Relative Humidity in % */
		HIH8120_ADC_Raw = (((uint32_t)HIH8120_RX_Data[0] & 0x3F) << 8) | ((uint32_t)HIH8120_RX_Data[1]);
		HIH8120_Humidity = (float)(HIH8120_ADC_Raw*100.00/16382.00);
	}
		
	measuresArr[0] = HIH8120_Temperature;
	measuresArr[1] = HIH8120_Humidity;
	
}