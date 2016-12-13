#ifndef _DHT11_H
#define _DHT11_H

u8 DHT11_Init(void);
u8 DHT11_Read_Data(u8 *temp_h, u8 *temp_l, u8 *humi_h, u8 *humi_l) ;

#endif
