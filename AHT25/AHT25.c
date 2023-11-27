
#include "gd32f10x.h"
#include "AHT25.h"
#include "systick.h"

uint8_t AHT25_RX_Data[6];

uint32_t AHT25_ADC_Raw;



void AHT25_sendCmd(uint8_t cmd)
{
	
	//i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_10BITS, AHT25_Adress);
    
    uint32_t k = 0;
	
	while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY))
   {
        
        if (k++ > 1000 * 10)
		{
            i2c_stop_on_bus(I2C1);
            return;
        }
    }
    k = 0;
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);
		
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND)); 
	
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, AHT25_Adress << 1 , I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
		
	while (!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND)) 
	{
        if (i2c_flag_get(I2C1, I2C_FLAG_AERR)) 
				{
            i2c_flag_clear(I2C1, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C1);
            return;
        }
        // in case no bus is connected
        if (k++ > 1000 * 10)
	{
            i2c_stop_on_bus(I2C1);
            return;
        }
    };
	i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);
    
	while(!i2c_flag_get(I2C1, I2C_FLAG_TBE));//
		
	i2c_ack_config(I2C1, I2C_ACKPOS_CURRENT);
		
    i2c_data_transmit(I2C1, cmd);
        /* wait until the TBE bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_TBE));
		
	while(!i2c_flag_get(I2C1, I2C_FLAG_TBE));
		
	i2c_stop_on_bus(I2C1);
    /* wait until stop condition generate */ 
	
	while (I2C_CTL0(I2C1) & I2C_CTL0_STOP);  
}



void AHT25_recieve(void)
{
	
	uint32_t k = 0;
	
	uint32_t i = 0;
    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY))
    {
        if (k++ > 1000 * 10)
		{
            i2c_stop_on_bus(I2C1);
            return 0;
        }
    };
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, AHT25_Adress << 1, I2C_RECEIVER);
    /* disable ACK before clearing ADDSEND bit */
    //i2c_ack_config(I2C1, I2C_ACK_DISABLE);
    /* wait until ADDSEND bit is set */
    k = 0;
	while (!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND)) 
	{
       if (i2c_flag_get(I2C1, I2C_FLAG_AERR)) 
        {
            i2c_flag_clear(I2C1, I2C_FLAG_AERR);
            i2c_stop_on_bus(I2C1);
            return 0;
        }
        // in case no bus is connected
        if (k++ > 1000 * 100)
				{
            i2c_stop_on_bus(I2C1);
            return 0;
        }
    };
    k = 0;
	i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);
		
	for(i = 0; i < sizeof(AHT25_RX_Data); i++) 
	{
        if(3 == i) 
        {
            /* wait until the second last data byte is received into the shift register */
            while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
            /* disable acknowledge */
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
        /* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C1, I2C_FLAG_RBNE))
        {
            if (k++ > 1000 * 10)
            {
            i2c_stop_on_bus(I2C1);
            return 0;
            }
        }
        /* read a data from I2C_DATA */
        AHT25_RX_Data[i] = i2c_data_receive(I2C1);
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C1);
    /* wait until stop condition generate */
    while(I2C_CTL0(I2C1) & 0x0200);
    /* enable acknowledge */
    i2c_ack_config(I2C1, I2C_ACK_ENABLE);
		
		
}

void AHT25_getMeasures( double *measuresArr)
{
	
	AHT25_sendCmd(AHT25_MEASURE_REQUEST);
	
	AHT25_recieve();

	double AHT25_Temperature;
	double AHT25_Humidity;
	
		/* Convert to Temperature in °C */
	AHT25_ADC_Raw = (((uint32_t)AHT25_RX_Data[3] & 15) << 16) | ((uint32_t)AHT25_RX_Data[4] << 8) | AHT25_RX_Data[5];
	AHT25_Temperature = (float)(AHT25_ADC_Raw * 200.00 / 1048576.00) - 50.00;
		/* Convert to Relative Humidity in % */
	AHT25_ADC_Raw = ((uint32_t)AHT25_RX_Data[1] << 12) | ((uint32_t)AHT25_RX_Data[2] << 4) | (AHT25_RX_Data[3] >> 4);
	AHT25_Humidity = (float)(AHT25_ADC_Raw*100.00/1048576.00);
		
	measuresArr[0] = AHT25_Temperature;
	measuresArr[1] = AHT25_Humidity;
	
}