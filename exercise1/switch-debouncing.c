#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

#define STATE_ON 	1
#define STATE_OFF 	0

volatile int state = STATE_OFF; 

void __attribute__ ((interrupt("FIQ"))) fiq_isr(void) {

  EXTINT = BIT1;       /* clear the interrupt flag */
		  
  if (state == STATE_OFF)
  {
	  VICIntEnClr = BIT15;
	  VICIntEnable = BIT4;

	  T0MR0 = 299999;
	  T0MCR = BIT0 | BIT1; /* reset & Interrupt on match */
	 
	  /* toggle the LED              */
	  if (IOPIN0 & BIT10) IOPIN0 &= ~BIT10; 
	  else                IOPIN0 |=  BIT10;
	  
	  state = STATE_ON;
	  T0TCR = BIT0;        /* start timer */
  }
  else 
  {
	  T0IR = BIT0;
	  T0TCR = 0;
	  
	  VICIntEnClr = BIT4;
	  VICIntEnable = BIT15; /* enable EINT1 */

	  state = STATE_OFF;
  }
}

int main(void) {
  IODIR0 = BIT10;	
  
  PINSEL0 = BIT29;      /* select EINT1 for P0.14      */ 
  EXTMODE = BIT1;       /* INT1 is edge sensitive      */
                        /* falling edge by default     */
  EXTINT  = BIT1;       /* clear the interrupt flag    */
  
  VICIntSelect = BIT4 | BIT15; /* EINT1 is the fast interrupt */
  VICIntEnable = BIT15; /* enable EINT1                */
  
  enable_interrupts();
  
  while (1) {
  }
}