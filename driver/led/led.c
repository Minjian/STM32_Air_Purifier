#include "sys.h"
#include "led.h"
/*
LED1  PB1
LED2  PB0
LED3  PC4
LED4  PB2
LED5  PC5
LED6  PA7
LED7  PB3
LED8  PB5
LED9  PB8
LED10 PB4
LED11 PB6
LED12 PB7
*/

extern u8 led1_stat ;
extern u8 led2_stat ;
extern u8 led3_stat ;
extern u8 led4_stat ;
extern u8 led5_stat ;
extern u8 led6_stat ;
extern u8 led7_stat ;
extern u8 led8_stat ;
extern u8 led9_stat ;
extern u8 led10_stat ;
extern u8 led11_stat ;
extern u8 led12_stat ;


void  TIM5_IRQHandler(void)
{
	
	if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET) {
			LED1 = LED_ON;
			LED2 = LED_ON;
			LED3 = LED_ON;
			LED4 = LED_ON;
			LED5 = LED_ON;
			LED6 = LED_ON;
			LED7 = LED_ON;
			LED8 = LED_ON;
			LED9 = LED_ON;
			LED10 = LED_ON;
			LED11 = LED_ON;
			LED12 = LED_ON;	
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);
	}
	
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET) {	
		if(0 == led1_stat)  LED1 = LED_OFF;
		if(0 == led2_stat)  LED2 = LED_OFF;
		if(0 == led3_stat)  LED3 = LED_OFF;
		if(0 == led4_stat)  LED4 = LED_OFF;
		if(0 == led5_stat)  LED5 = LED_OFF;
		if(0 == led6_stat)  LED6 = LED_OFF;
		if(0 == led7_stat)  LED7 = LED_OFF;
		if(0 == led8_stat)  LED8 = LED_OFF;
		if(0 == led9_stat)  LED9 = LED_OFF;
		if(0 == led10_stat) LED10 = LED_OFF;
		if(0 == led11_stat) LED11 = LED_OFF;
		if(0 == led12_stat) LED12 = LED_OFF;		
		TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
	}
}


void led_time5_init(u32 arr,u32 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef  NVIC_InitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);

	NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

	TIM_TimeBaseStructure.TIM_Prescaler=psc;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period=arr;
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);

	TIM_ITConfig(TIM5, TIM_IT_Update|TIM_IT_CC1, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
}

void led_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1|GPIO_Pin_2|
	GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}


void led_all_on()
{
	LED1 = LED_ON;
	LED2 = LED_ON;
	LED3 = LED_ON;
	LED4 = LED_ON;
	LED5 = LED_ON;
	LED6 = LED_ON;
	LED7 = LED_ON;
	LED8 = LED_ON;
	LED9 = LED_ON;
	LED10 = LED_ON;
	LED11 = LED_ON;
	LED12 = LED_ON;
}

void led_all_off()
{
	LED1 = LED_OFF;
	LED2 = LED_OFF;
	LED3 = LED_OFF;
	LED4 = LED_OFF;
	//LED5 = LED_OFF;
	LED6 = LED_OFF;
	LED7 = LED_OFF;
	//LED8 = LED_OFF;
	LED9 = LED_OFF;
	LED10 = LED_OFF;
	//LED11 = LED_OFF;
	LED12 = LED_OFF;
}

void led_all_off2()
{
	LED1 = LED_OFF;
	LED2 = LED_OFF;
	LED3 = LED_OFF;
	LED4 = LED_OFF;
	LED5 = LED_OFF;
	LED6 = LED_OFF;
	LED7 = LED_OFF;
	LED8 = LED_OFF;
	LED9 = LED_OFF;
	LED10 = LED_OFF;
	LED11 = LED_OFF;
	LED12 = LED_OFF;
}
