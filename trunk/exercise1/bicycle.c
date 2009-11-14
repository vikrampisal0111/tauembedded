#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

#define STATE_OFF 	0
#define STATE_ON 	1
#define STATE_BLINK 2

volatile int state = STATE_OFF; 

void __attribute__ ((interrupt("FIQ"))) fiq_isr(void) {
		  
  if (state == STATE_OFF)
  {
	  if (EXTINT & BIT1)
	  {
		  EXTINT = BIT1;       /* clear the interrupt flag */
		  /* toggle the LED ON             */
		  IOPIN0 &=  ~BIT10;
		  
		  state = STATE_ON;
	  }
  }
  else if (state == STATE_ON)
  {
	  if (EXTINT & BIT1)
	  {
		  EXTINT = BIT1;       /* clear the interrupt flag */
		  T0MR0 = 299999;
		  T0MCR = BIT0 | BIT1; /* reset & Interrupt on match */
		  T0TCR = BIT0;        /* start timer */

		  state = STATE_BLINK;
	  }
  }
  else {
	  if (T0IR & BIT0) {
		  if (IOPIN0 & BIT10) IOPIN0 &= ~BIT10; 
		  else                IOPIN0 |=  BIT10;
		  T0IR = BIT0;
	  }
	  if (EXTINT & BIT1)
	  {
		  EXTINT = BIT1;       /* clear the interrupt flag */
		  T0TCR = 0;
		  
		  /* toggle the LED Off             */
		  IOPIN0 |= BIT10;
		    
		  state = STATE_OFF;		  		  
	  }
  }
}

int main(void) {
  IODIR0 = BIT10;	
  
  PINSEL0 = BIT29;      /* select EINT1 for P0.14      */ 
  EXTMODE = BIT1;       /* INT1 is edge sensitive      */
                        /* falling edge by default     */
  EXTINT = BIT1;       /* clear the interrupt flag    */
  
  VICIntSelect = BIT4 | BIT15; /* EINT1 is the fast interrupt */
  VICIntEnable = BIT4 | BIT15; /* enable EINT1                */

  /* toggle the LED Off             */
  IOPIN0 |= BIT10;
  
  enable_interrupts();
  
  while (1) {
  }
}