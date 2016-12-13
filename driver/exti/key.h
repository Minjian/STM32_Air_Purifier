#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

#define KEY_ON  1
#define KEY_OFF 0

#define KEY_COL_1  PCin(6)
#define KEY_COL_2  PCin(7)
#define KEY_COL_3  PCin(8)

#define KEY_ROW_1  PBin(15)
#define KEY_ROW_2  PBin(14)
#define KEY_ROW_3  PBin(13)
#define KEY_ROW_4  PBin(12)

#define KEY_COL_ON do{PCout(6)=1;PCout(7)=1;PCout(8)=1;}while(0)
#define KEY_ROW_ON do{PBout(15)=1;PBout(14)=1;PBout(13)=1;PBout(12)=1;}while(0)

//0x40020800 PC
//0x40020400 PB

#define KEY_COL_OUT  do{GPIOC->MODER &= (~(0x3f << 12)); GPIOC->MODER |= (0x15 << 12);}while(0)
#define KEY_COL_IN   do{GPIOC->MODER &= (~(0x3f << 12));}while(0)

#define KEY_ROW_OUT  do{GPIOB->MODER &= (~(0xff << 24)); GPIOB->MODER |= (0x55 << 24);}while(0)
#define KEY_ROW_IN   do{GPIOB->MODER &= (~(0xff << 24)); }while(0)

void key_init(void);
u8 key_scan(void);

#endif

