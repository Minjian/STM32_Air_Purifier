#include "sys.h"
#include "delay.h"

//DHT11 DQ: PC1

#define DHT11_IO_IN()  {GPIOC->MODER&=~(3<<(1*2));GPIOC->MODER|=0<<1*2;}
#define DHT11_IO_OUT() {GPIOC->MODER&=~(3<<(1*2));GPIOC->MODER|=1<<1*2;}

#define	DHT11_DQ_OUT PCout(1)
#define	DHT11_DQ_IN  PCin(1)

void DHT11_Rst(void)	   
{                 
	DHT11_IO_OUT();
	DHT11_DQ_OUT = 0;
	delay_ms(20);
	DHT11_DQ_OUT = 1; 
	delay_us(30);
}

u8 DHT11_Check(void) 	   
{   
	u8 retry=0;
	DHT11_IO_IN();//SET INPUT	 
	while (DHT11_DQ_IN && (retry<100))
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
	while (!DHT11_DQ_IN && (retry<100))
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}

u8 DHT11_Read_Bit(void) 			 
{
	u8 retry=0;
	while(DHT11_DQ_IN && (retry<100))
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!DHT11_DQ_IN && (retry<100))
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);
	if(DHT11_DQ_IN)return 1;
	else return 0;		   
}

u8 DHT11_Read_Byte(void)    
{        
	u8 i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
	    dat|=DHT11_Read_Bit();
	}						    
	return dat;
}

u8 DHT11_Read_Data(u8 *temp_h, u8 *temp_l, u8 *humi_h, u8 *humi_l)
{        
 	u8 buf[5] = {0};
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)
		{
			buf[i]=DHT11_Read_Byte();
		}
		if(((buf[0]+buf[1]+buf[2]+buf[3])&0xff) == buf[4])
		{
			*humi_h = buf[0];
			*humi_l = buf[1];
			*temp_h = buf[2];
			*temp_l = buf[3];
		} else {
			return 1;
		}
	}else return 1;
	return 0;	    
}

u8 DHT11_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	DHT11_Rst();
	return DHT11_Check();
}
