#ifndef _INTERRUPT_C_
#define _INTERRUPT_C_

/*********************************************************************************************************************/
#include "include/ca51m151_config.h"		
#include "include/ca51m151sfr.h"
#include "include/ca51m151xsfr.h"
#include "include/gpiodef_m151.h"

#include "include/uart.h"
/*********************************************************************************************************************/
void INT2_ISR (void) interrupt 4 
{
#ifdef UART1_EN
	UART1_ISR();
#endif
}

void INT4_ISR (void) interrupt 6 
{
#ifdef UART2_EN
	UART2_ISR();
#endif
}

/*********************************************************************************************************************/
#endif
