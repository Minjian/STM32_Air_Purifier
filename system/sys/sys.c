#include "sys.h"  
  
__asm void WFI_SET(void)
{
	WFI;		  
}
//Disable all intterruptions except fault and NMI
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//Enable all interruptions
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//set the stack address
//addr:stack top address
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}

