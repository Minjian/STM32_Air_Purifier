#include "sys.h"
#include "breath_led.h"
#include "buzzer.h"
/*
PC 13 -  Blue
PC 14 -  Green
pc 15 -  Red
*/

extern u8 bl_flag;
extern u8 key_time;
extern u8 key_flag;
extern u32 switch_flag;
extern u32 bl;
extern u32 sys_timer;
extern u8 bl_dir;
extern u8  before_bl;
extern u8 now_bl;
extern u8 bl_open_flag;
extern u8 bl_run_flag;

void  TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET) {
		GPIO_ResetBits(GPIOC, GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13);
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
	}
	
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
		if(1 == bl_flag) {
		   GPIO_SetBits(GPIOC, GPIO_Pin_15);
			 GPIO_ResetBits(GPIOC, GPIO_Pin_14|GPIO_Pin_13);
		}else if(2 == bl_flag) {
			 GPIO_SetBits(GPIOC, GPIO_Pin_13);
			 GPIO_ResetBits(GPIOC, GPIO_Pin_15|GPIO_Pin_14);
		}else if(3 == bl_flag) {
			 GPIO_SetBits(GPIOC, GPIO_Pin_15|GPIO_Pin_13);
			 GPIO_ResetBits(GPIOC, GPIO_Pin_14);
		}else if(4 == bl_flag) {
			//GPIO_ResetBits(GPIOC, GPIO_Pin_13);
			GPIO_SetBits(GPIOC, GPIO_Pin_15|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		}
		
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
}


void breath_time4_init(u32 arr,u32 psc)
{

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef  NVIC_InitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_TimeBaseStructure.TIM_Prescaler=psc;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period=arr;
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	TIM_ITConfig(TIM4, TIM_IT_Update|TIM_IT_CC1, ENABLE);
	TIM_Cmd(TIM4, ENABLE);	
}

void breath_led_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC,&GPIO_InitStructure);   	
}

//Timer 3  5 sec，for the Internet connect and exit
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		key_time = 0;
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
}


void TIM7_Int_Init(u32 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE); 
	
	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);	
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_Cmd(TIM7,ENABLE);	
}


void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET)
	{
		if(1 == switch_flag) {
			sys_timer++;
		} else if (0 == switch_flag) {
			sys_timer = 0;
		}

		if(1 == bl_run_flag) {
			if(0 == bl_dir) 
			{
				if((bl>= 1500) &&(bl <= 2000)) bl = bl-4;
				if((bl>=1000) && (bl <= 1500)) bl = bl-3;
				if((bl>=500) && (bl <= 1000))  bl = bl -2;
				if((bl>8)&& (bl <= 500)) bl--;
				bl_flag = before_bl;
			} 
			else if(1 == bl_dir) 
			{
				if((bl>= 1500) &&(bl <= 2000)) bl = bl+4;
				if((bl>=1000) && (bl <= 1500)) bl = bl+3;
				if((bl>=500) && (bl <= 1000))  bl = bl +2;
				if((bl>8)&& (bl <= 500)) bl++;
				bl_flag = now_bl;
				if((1 == bl_open_flag) && bl > 1990) {
					
					TIM_Cmd(TIM4, DISABLE);
					switch(now_bl) {
						case 2:
							BLUE;
							break;
						case 3:
							PINK;
							break;
						case 4:
							YELLOW;
							break;
					}
					bl_run_flag = 0;
					before_bl = now_bl;
				}		
			}                
 		if(bl > 1990)bl_dir=0;
		if(bl < 10)bl_dir=1;	
		TIM_SetCompare1(TIM4,bl);
		}
	}
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
}

