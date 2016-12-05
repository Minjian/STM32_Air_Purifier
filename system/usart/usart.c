#include "sys.h"
#include "usart.h"	
	 
//for using uCOS
#if SYSTEM_SUPPORT_UCOS
#include "includes.h" //for uCOS  
#endif

//the following codes is to support printf (for debug)
#if 1
#pragma import(__use_no_semihosting)                             
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
   
void _sys_exit(int x) 
{ 
	x = x; 
} 
//overload fputc function 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);  
	USART1->DR = (u8)ch;      
	return ch;
}
#endif

#define EN_USART_RX 0

#if EN_USART1_RX
  	
u8 USART_RX_BUF[USART_REC_LEN];

//bit15，	recv complete flag
//bit14，	recv 0x0d
//bit13~0，	effective bit for data
u16 USART_RX_STA=0;       //Recv status

void uart_init(u32 bound){
   	//GPIO端口设置
  	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //enable GPIOA clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//enable USART1 clock
	
	//USART1 Config
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 and GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure); //Init PA9，PA10

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9 AF as USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10 AF as USART1
	
   //USART1 Config(8n1 protocal)
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  	USART_Init(USART1, &USART_InitStructure); //Init USART1
	
  	USART_Cmd(USART1, ENABLE);  //Enable USART1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//Open interruption

	//Usart1 NVIC 配置
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//Interruption Channel for USART1
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_Init(&NVIC_InitStructure);	//Init NVIC
#endif
	
}

void USART1_IRQHandler(void) //Interruption Function for USART1
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //Receive Interrupt(the data must end with 0x0d 0x0a)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);
		
		if((USART_RX_STA&0x8000)==0)//Recv incomplete
		{
			if(USART_RX_STA&0x4000)//Recv 0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//err, restart
				else USART_RX_STA|=0x8000;	//Recv complete
			}
			else //not recv 0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//err, restart 
				}		 
			}
		}   		 
  } 
} 
#endif	
