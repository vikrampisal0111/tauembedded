#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>



void __attribute__ ((interrupt("FIQ"))) fiq_isr(void) {
	T0IR = BIT0; /* Clear interrupt */
	systick_periodic_task();
}


void systickInit() {
  
  T0MR0 = 2999;		    /* Count 1 millisecond (1/1000 sec) */
  T0MCR = BIT0 | BIT1;  /* reset & Interrupt on match */
  T0TCR = BIT0;         /* start timer */

  VICIntSelect = BIT4; 
  VICIntEnable = BIT4; 
  
  enable_interrupts();
}
