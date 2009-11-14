#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>
 
uint32_t msecs = 0;

void toggle_led() {
	if(IOPIN0 & BIT10) IOPIN0 &= ~BIT10;
	else			   IOPIN0 |= BIT10;
}

void toggle_rgb() {
	if(IOPIN0 & BIT9) IOPIN0 &= ~BIT9;
	else			   IOPIN0 |= BIT9;
	if(IOPIN0 & BIT8) IOPIN0 &= ~BIT8;
	else			   IOPIN0 |= BIT8;
	if(IOPIN0 & BIT7) IOPIN0 &= ~BIT7;
	else			   IOPIN0 |= BIT7;

}

void systick_periodic_task() {
	msecs++;

	if ((msecs & 0x3ff) == 0) {
		toggle_led();
	}
}

#include "timer/systick.c"
#include "timer/busywait.c"

volatile int v = 100;

void dimmChange()
{
	if ((IOPIN0 & BIT17) && (IOPIN0 & BIT19))
	{
		// Left.
//		v += 1000000;
		v += 1000;
		PWMPR += 1000;
//		toggle_led();
	}
	else if ((IOPIN0 & BIT18) && (IOPIN0 & BIT20))
	{
		// Right.
//		v -= 1000000;
		v -= 1000;
		PWMPR -= 1000;
//		toggle_led();

	}
	
}

int main()
{
	IODIR0 = ~(BIT16 | BIT17 | BIT18 | BIT19 | BIT20);
	IODIR0 = BIT7 | BIT8 | BIT9;
	IODIR0 |= BIT10;


	PWMTCR = BIT1;
	PWMMR2 = 10000; // initial match value for PWM2.	
	PWMMCR = BIT7; // Reset on PWMMR2 match.
	PWMPCR = BIT2;
	PWMTCR = BIT0 | BIT3; // Enable counters and enable PWM.

	busywaitInit();
	while(1) {
		busywait(v);
		dimmChange();
		toggle_rgb();
		//toggle_led();
	}

}
