#include "sys.h"
#include "zigbee.h"

//USART3_TX  PB10
//USART3_RX  PB11
//The data are sent and received by following the protocal of Nanjing Internet of things 

extern u8  zigbee_cmd;
extern u8  zigbee_state;
extern u8  zigbee_cmd_len;
extern u8  parse_cmd[32];

void USART3_IRQHandler()
{
	u8 data;
	if (USART_GetITStatus(USART3,USART_IT_RXNE) == SET) {
		
		data = USART_ReceiveData(USART3);

		if((0xff) == data && (0 == zigbee_state)){
			zigbee_cmd_len++;
			zigbee_state = 0x1;
		}else if(0x1 == zigbee_state) {
			if(0x55 != data){
				//first 0XFF, then 0x55
				zigbee_cmd_len = 0;
				zigbee_state = 0;
			}
			zigbee_cmd_len++;
			zigbee_state = 0x09;
		//If the data is not 0xff, then put the data into the parse_cmd
		}else if( (0xff != data) && (0x09 == zigbee_state)){
			zigbee_cmd_len++;
			parse_cmd[zigbee_cmd_len - 3] = data;
		}else if((0xff == data) && (0x09 == zigbee_state)){
			zigbee_cmd_len++;
			zigbee_state = 0x0b;
		}else if(0x0b == zigbee_state){
			zigbee_cmd_len = 0;
			if(0xaa != data) {
				zigbee_state = 0;
			}
			zigbee_state = 0x1b;
		}
	}
}

//USART3 Init
void zigbee_init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//PB10--USART3_TX
	//PB11--USART3_RX
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength =  USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStruct);


	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(USART3, ENABLE);
}
