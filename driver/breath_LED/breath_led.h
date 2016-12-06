#ifndef __BREATH_LED_H
#define __BREATH_LED_H
#include "sys.h"
/*
PC 13 -  Blue
PC 14 -  Green
pc 15 -  Red
*/

#define RED  do{PCout(15)=1;PCout(14)=0;PCout(13)=0;}while(0)
#define GREEN  do{PCout(15)=0;PCout(14)=1;PCout(13)=0;}while(0)
#define BLUE do{PCout(15)=0;PCout(14)=0;PCout(13)=1;}while(0)
#define PINK  do{PCout(15)=1;PCout(14)=0;PCout(13)=1;}while(0)
#define YELLOW  do{PCout(15)=1;PCout(14)=1;PCout(13)=0;}while(0)
#define SYAN  do{PCout(15)=0;PCout(14)=1;PCout(13)=1;}while(0)
#define WHITE do{PCout(15)=1;PCout(14)=1;PCout(13)=1;}while(0)

void breath_time4_init(u32 arr,u32 psc);
void breath_led_init(void);

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM7_Int_Init(u32 arr,u16 psc);

#endif
