#ifndef __LED_H
#define __LED_H
#include "sys.h"
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

#define LED_ON 1
#define LED_OFF 0

#define LED1  PBout(1)
#define LED2  PBout(0)
#define LED3  PCout(4)
#define LED4  PBout(2)
#define LED5  PCout(5)
#define LED6  PAout(7)
#define LED7  PBout(3)
#define LED8  PBout(5)
#define LED9  PBout(8)
#define LED10 PBout(4)
#define LED11 PBout(6)
#define LED12 PBout(7)

void led_init(void);
void led_all_on(void);
void led_all_off(void);
void led_all_off2(void);
void led_time5_init(u32 arr,u32 psc);
#endif
