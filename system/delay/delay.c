#include "delay.h"
#include "sys.h"

//header for uCOS
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"	//for uCOS
#endif

 
static u8  fac_us=0;//us factor
static u16 fac_ms=0;//ms factor

#ifdef OS_CRITICAL_METHOD 	//if OS_CRITICAL_METHOD defined,means using ucosII.
//systick interruption handler,when using uCOS
void SysTick_Handler(void)
{				   
	OSIntEnter();		//enter interruption
    OSTimeTick();       //use uCOS's TimeTick
    OSIntExit();        //start tasks, change to soft interruption
}
#endif


//SYSTICK is 1/8 HCLK
void delay_init(u8 SYSCLK)
{
#ifdef OS_CRITICAL_METHOD 	//if OS_CRITICAL_METHOD defined,means using ucosII.
	u32 reload;
#endif
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us=SYSCLK/8;
	    
#ifdef OS_CRITICAL_METHOD 	//if OS_CRITICAL_METHOD defined,means using ucosII.
	reload=SYSCLK/8;		//count per second   
	reload*=1000000/OS_TICKS_PER_SEC;//under 168M, valus is around 0.7989s	
	fac_ms=1000/OS_TICKS_PER_SEC;//min delay unit in uCOS
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD=reload; 	//Interrupt every 1/OS_TICKS_PER_SEC second	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;   	//Start SYSTICK
#else
	fac_ms=(u16)fac_us*1000;//not ucos,the count needed for every ms   
#endif
}								    

#ifdef OS_CRITICAL_METHOD 	//if OS_CRITICAL_METHOD defined,means using ucosII.
		    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;	//LOAD value
	ticks=nus*fac_us; 			//ticks needed 
	tcnt=0;
	OSSchedLock();				//Stop uCOS schedule，avoid interrupting us delay
	told=SysTick->VAL;        	//first value
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;//timeout, break
		}  
	};
	OSSchedUnlock();			//open uCOS schedule
}


void delay_ms(u16 nms)
{	
	if(OSRunning==OS_TRUE&&OSLockNesting==0)//OS is running   
	{		  
		if(nms>=fac_ms)//the time is greater than the min delay unit
		{
   			OSTimeDly(nms/fac_ms);	//uCOS delay
		}
		nms%=fac_ms;
	}
	delay_us((u32)(nms*1000));		//normal delay
}
#else  //without using uCOS
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us;	  		 
	SysTick->VAL=0x00;        //clear counter
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //start counting
	do
	{
		temp=SysTick->CTRL;
	}
	while((temp&0x01)&&!(temp&(1<<16)));//wait for the time 
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //close counter
	SysTick->VAL =0X00;       //clear counter 
}

void delay_xms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;
	SysTick->VAL =0x00;           //clear counter
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //start counting 
	do
	{
		temp=SysTick->CTRL;
	}
	while((temp&0x01)&&!(temp&(1<<16)));//wait for the time   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //close counter
	SysTick->VAL =0X00;       //clear counter 	  	    
} 

void delay_ms(u16 nms)
{	 	 
	u8 repeat=nms/540;	//540 is for considering cpu performance increased
						//like when cpu up to 248M,the max delay_xms is around 541ms
	u16 remain=nms%540;
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	if(remain)delay_xms(remain);
	
} 
#endif
