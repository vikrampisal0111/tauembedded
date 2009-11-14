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

#define OFF_FREQ_HZ		0	
#define ON_FREQ_HZ		500
#define FREQ_STEP		ON_FREQ_HZ/8

#define JOYSTICK_LEFT   ((IOPIN0 & BIT17) && (IOPIN0 & BIT19))
#define JOYSTICK_RIGHT 	((IOPIN0 & BIT18) && (IOPIN0 & BIT20))

#define MICROSEC	1000000


volatile int freq = OFF_FREQ;
volatile int wait_time = 100;

void change_freq()
{
	if (JOYSTICK_LEFT)
	{
		freq += FREQ_STEP;
	}
	else if (JOYSTICK_RIGHT)
	{
		freq -= FREQ_STEP;
	}

}

void rgbInit() {
	IODIR0 = ~(BIT16 | BIT17 | BIT18 | BIT19 | BIT20);
	IODIR0 = BIT7 | BIT8 | BIT9;
}

int main()
{
	IODIR0 |= BIT10;

	rgbInit();
	busywaitInit();

	while(1) {
		busywait(v);
		change_freq();
		toggle_rgb();
	}

}





