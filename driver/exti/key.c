#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "key.h"
#include "breath_led.h"
#include "led.h"


/* 
Column keys: PC 6 7 8 
Row keys: PB 15 14 13 12

3*4 keyboard
Column GPIO Output Mode
Row GPIO Input Mode
We use Scan Mode to check which button is pressed
(For example, the button located at the PC6 and PB5, 
then PC6 output is at high level, and we can get high level from PB15 )

Scan Mode:
	First PC6 output high level, PC7/8 is at low level, 
		then check PB12/13/14/15 to see which one is at high level, 
	none is at hight level, then set PC7 at high level,
		then check PBs again,
	none is at hight level, then set PC8 at hight level,
*/


void key_init()
{	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/*
Scan Mode
*/
u8 key_up = 1;//1 means released, 0 means pressed
u8 key_scan()
{
	extern u8 key_flag;
	int state = 0;
	
	PCout(6) = 0;
	PCout(7) = 1;
	PCout(8) = 1;
	delay_ms(1);
	
	if(key_up && ((0 == PBin(15))||(0 == PBin(14))||(0 == PBin(13))||(0 == PBin(12)))) 
	{
		delay_ms(5);
		key_up = 0;	
		if(0 == PBin(15)) {
			if(0 == key_flag)  return 0;
			return 1;
		} else if(0 == PBin(14)) {
			if(0 == key_flag)  return 0;
			return 4;
		} else if(0 == PBin(13)) {
			if(0 == key_flag)  return 0;
			return 7;
		} else if (0 == PBin(12)) {
			if(0 == key_flag)  return 0;
			return 10;
		}
	} else if((1==PBin(15)) && (1==PBin(14)) && (1==PBin(13)) && (1==PBin(12))){
		state++;
	}
	
	delay_ms(1);
	PCout(6) = 1;
	PCout(7) = 0;
	PCout(8) = 1;
	delay_ms(1);
		
	if(key_up && ((0 == PBin(15))||(0 == PBin(14))||(0 == PBin(13))||(0 == PBin(12)))) 
	{
		delay_ms(5);
		key_up = 0;	
		if(0 == PBin(15)) {
			if(0 == key_flag)  return 0;
			return 2;
		} else if(0 == PBin(14)) {
			if(0 == key_flag)  return 0;
			return 5;
		} else if(0 == PBin(13)) {
			if(0 == key_flag)  return 0;
			return 8;
		} else if (0 == PBin(12)) {
			if(0 == key_flag)  return 0;
			return 11;
		}
	} else if((1==PBin(15)) && (1==PBin(14)) && (1==PBin(13)) && (1==PBin(12))) {
		state++;
	}
		
	delay_ms(1);
	PCout(6) = 1;
	PCout(7) = 1;
	PCout(8) = 0;
	delay_ms(1);
		
	if(key_up && ((0 == PBin(15))||(0 == PBin(14))||(0 == PBin(13))||(0 == PBin(12)))) 
	{
		delay_ms(5);
		key_up = 0;	
		if(0 == PBin(15)) {
			if(0 == key_flag)  return 0;
			return 3;
		} else if(0 == PBin(14)) {
			if(0 == key_flag)  return 0;
			return 6;
		} else if(0 == PBin(13)) {
			if(0 == key_flag)  return 0;
			return 9;
		} else if (0 == PBin(12)) {
			return 12;
		}				
	} else if ((1==PBin(15)) && (1==PBin(14)) && (1==PBin(13)) && (1==PBin(12))){
		state++;
	}
	
	if(3 == state) {
		key_up = 1;
	}
		
	return 0;
}
