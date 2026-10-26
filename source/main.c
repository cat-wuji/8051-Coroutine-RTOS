#ifndef _MAIN_C_
#define _MAIN_C_
/*********************************************************************************************************************/
#include "include/ca51m151_config.h"		
#include "include/ca51m151sfr.h"
#include "include/ca51m151xsfr.h"
#include "include/gpiodef_m151.h"
#include "include/system_clock.h"

#include "include/uart.h"
#include "include/delay.h"
/*********************************************************************************************************************/
#include "RTOS/rtos.h"
 

/*********************************************************************************************************************/

#define INT_TIME			100		//定时时间，单位为us

#define	T_VAL				(unsigned char)(0x100 - (INT_TIME*(FOSC/1000))/12000)

 

void Timer0_Init(void)
{
	TMOD = (TMOD&0xFC)|0x02; 		//模式选择: 定时器0，模式2。
	TH0 = T_VAL;    				//装载重载值
	TL0 = T_VAL;    				//装载计数初值
	
	TR0 = 1;       					//定时器0使能  
	ET0 = 1;       					//定时器0中断使能
	
//	PT0 = 1;       					//设置定时器0中断优先级为高优先级	
}
/*********************************************************************************************************************/

void LED_Task(void) {
    while (1) {
        P02 = ~P02;
        uart_printf("RTOS任务1测试\n");
        RTOS_TaskDelay(500);   /* 真正保存 SP 切走，500 tick 后从这行之后继续 */
    }
}

void Key_Task(void) {
    while (1) {
         uart_printf("RTOS任务2测试\n");
      RTOS_TaskDelay(500); 
//        if (P03 == 0) {
//            RTOS_TaskDelay(20);        /* 消抖，阻塞式写法 */
//            if (P03 == 0) {
//                /* 确认按下 */
//                while (P03 == 0)
//                    RTOS_TaskDelay(10); /* 等释放，代码线性直观 */
//            }
//        }
//        RTOS_Yield();
      
    }
}
/*********************************************************************************************************************/
void System_Init(void)
{
	LVDCON = 0xE2;					//开启LVD，设置为低电压复位模式，检测电压为2.7V
#ifdef SYSCLK_8MHZ					
	CKDIV = 0;						//系统时钟上电默认为IRCH的二分频(4MHz),运行8MHz,则CKDIV设置为0
#endif
#ifdef UART1_EN
	Uart1_Initial(UART1_BAUTRATE);
#endif	
#ifdef UART2_EN
	Uart2_Initial(UART2_BAUTRATE);
#endif
  
  GPIO_Init(P02F,OUTPUT);
  GPIO_Init(P03F,INPUT);  
}
void main(void) {
    /* 硬件初始化 */
  System_Init();
  	EA = 1;							//开全局中断
#ifdef PRINT_EN
	uart_printf("Timer0 Mode2 Demo Code\n");
#endif

	  Timer0_Init();  
    RTOS_Init();
    RTOS_TaskCreate(LED_Task, 2);
    RTOS_TaskCreate(Key_Task, 1);
    RTOS_Start();       /* 不会返回 */
}

/* Timer0 1ms 中断 */
void Timer0_ISR(void) interrupt 1 {
      static u8 tick_cnt = 0;
  /* 累加10次，凑够 1ms，提供给 RTOS 作为心跳 */
    if (++tick_cnt >= 10) {
        tick_cnt = 0;
        RTOS_Scheduler();   /* 只递减 DelayTick，不切换任务 */
    }
}


 
#endif

 


 