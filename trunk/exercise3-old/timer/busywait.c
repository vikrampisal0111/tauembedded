#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

volatile double g_calib = 0;

void busywaitInit() 
{
	volatile int i = 0;
	T0TCR = BIT1;
	T0TCR = BIT0;      /* start timer */
	while(i++ < 1000) {};
	g_calib = 1000/T0TC;	
	
	T0TCR = 0; 		   /* stop timer */	
	
	g_calib = g_calib / 3;
}


void busywait(uint64_t microseconds) 
{
	uint64_t limit = (double)microseconds * g_calib;
	for(volatile int i = 0; i < limit; i++); 
}


