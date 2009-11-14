#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>
 
uint32_t msecs = 0;

void toggle_led() {
	if(IOPIN0 & BIT10) IOPIN0 &= ~BIT10;
	else			   IOPIN0 |= BIT10;
}

void systick_periodic_task() {
	msecs++;

	if ((msecs & 0x3ff) == 0) {
		toggle_led();
	}
}

#include "timer/systick.c"
#include "timer/busywait.c"

volatile int v = 0;

int main()
{
	IODIR0 = BIT10;
	PINSEL0 = BIT29;
#if 0 
	systickInit();

	while(1) {
		v = 10;
	}
#endif

	busywaitInit();
	while(1) {
		busywait(1000000);
		toggle_led();
	}

}
