#include "sys.h"
#include "pm2_5.h"
#include "usart.h"

struct pm2_5  pm_data = 
{
	{0},
	0,
	0,
};

void UART5_IRQHandler()
{
	u8 data;
	if (USART_GetITStatus(UART5,USART_IT_RXNE) == SET) {
		
		data = USART_ReceiveData(UART5);

		if((0xaa == data) && (0 == pm_data.count)) {
			pm_data.data[pm_data.count] = 0xaa;
			pm_data.count++;
		}else if((0xff == data) && (6 == pm_data.count)) {
			pm_data.data[pm_data.count] = 0xff;
			pm_data.count = 0;
		} else {
			pm_data.data[pm_data.count] = data;
			pm_data.count++;
		}
	}
	USART_ClearITPendingBit(UART5, USART_IT_RXNE);
}

void pm2_5_init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStruct;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	//GPIO -> UART
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

	//8n1 Protocol
	USART_InitStruct.USART_BaudRate = 2400;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength =  USART_WordLength_8b;
	USART_Init(UART5, &USART_InitStruct);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	NVIC_InitStruct.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(UART5, ENABLE);
}
