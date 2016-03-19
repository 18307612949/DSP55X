#include "soc.h"

interrupt void tim_isr()
{
	CSL_GPIO_REGS->IOOUTDATA2 ^= 0x0002;
	CSL_SYSCTRL_REGS->TIAFR = 0x01;
}

void TIMER_init(void)
{
#define	TIME_CONST	29999104L	// 119.99MHz/4
//#define	TIME_CONST	24993792L	// 99.97MHz/4
	CSL_TIM_0_REGS->TIMPRD1 = TIME_CONST & 0xffff;
	CSL_TIM_0_REGS->TIMPRD2 = TIME_CONST >> 16;
	// prescale 0000=2
	CSL_TIM_0_REGS->TCR = 0x8003;

	/* Enabling Interrupt */
	CSL_CPU_REGS->IER0 |= 0x0010;		// interrupt every 1 second

    CSL_SYSCTRL_REGS->TIAFR = 0x01;
}

