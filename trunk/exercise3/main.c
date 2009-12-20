#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>
 
#define CLOCKS_PCLK		3000000
#define UART0_BAUD_RATE 19200
#include "uart/uart0-polling.c"
#define  print_char(x) uart0SendByte(x)
#include "uart/print.c"


// Count milliseconds
uint32_t msecs = 0;

void toggle_led() {
	if(IOPIN0 & BIT10) IOPIN0 &= ~BIT10;
	else			   IOPIN0 |= BIT10;
}
/*
void _systick_periodic_task() {
	msecs++;

	if ((msecs & 0x3ff) == 0) { // 1024 milliseconds
		toggle_led();
	}
}
*/

void toggle_rgb() {
	if(IOPIN0 & BIT9) IOPIN0 &= ~BIT9;
	else			   IOPIN0 |= BIT9;

	if(IOPIN0 & BIT8) IOPIN0 &= ~BIT8;
	else			   IOPIN0 |= BIT8;
	
	if(IOPIN0 & BIT7) IOPIN0 &= ~BIT7;
	else			   IOPIN0 |= BIT7;
}

#define MICROSEC		1000000

#define OFF_FREQ_HZ		0	
#define ON_FREQ_HZ		500
#define FREQ_STEP		(ON_FREQ_HZ/8)

#define OFF_WAIT_TIME	(MICROSEC/ON_FREQ_HZ)

#define JOYSTICK_LEFT   (((IOPIN0 & BIT17) == 0) && ((IOPIN0 & BIT19) == 0))
#define JOYSTICK_RIGHT 	(((IOPIN0 & BIT18) == 0) && ((IOPIN0 & BIT20) == 0))

#define IODIR_IN(BITS)  (~(BITS))
#define IODIR_OUT(BITS)	 (BITS)

#define JOYSTICK_BITS	(BIT16 | BIT17 | BIT18 | BIT19 | BIT20)
#define RGB_BITS		(BIT7 | BIT8 | BIT9)

#define LOG_SCALE   1
#define LIN_SCALE   2

volatile int freq = OFF_FREQ_HZ;
volatile int wait_time = OFF_WAIT_TIME;

// Frequency scaling function

// Logarithmic
void inc_freq_log() {
	freq <<= 1;
}

void dec_freq_log() {
	freq >>= 1;
}

// Linear
void inc_freq_lin() {
	freq += FREQ_STEP;
}

void dec_freq_lin() {
	freq -= FREQ_STEP;
}

// Default is linear
void (*inc_freq)() = inc_freq_lin;
void (*dec_freq)()= dec_freq_lin;


// Set frequenct scaling functions
void set_freq_scale(int scale)
{
	switch(scale) 
	{
		case LOG_SCALE:
			freq = 1;
			inc_freq = inc_freq_log;
			dec_freq = dec_freq_log;
			break;
		case LIN_SCALE:
			freq = 0;
			inc_freq = inc_freq_lin;
			dec_freq = dec_freq_lin;
			break;
	}
}

void change_freq()
{
	if (JOYSTICK_LEFT && (freq < ON_FREQ_HZ))
	{
		print("Left\n");
		inc_freq();
	}
	else 
        if (JOYSTICK_RIGHT && (freq > OFF_FREQ_HZ))
	{
		print("Right\n");
		dec_freq();
	}

	if(freq != OFF_FREQ_HZ) 
	{
		wait_time = MICROSEC/freq;
	}
}

void systick_periodic_task() 
{
	msecs++;
	
	if ((msecs & 0xf) == 0) { // 64 milliseconds
		change_freq();
	}

}

#include "timer/systick.c"
#include "timer/busywait.c"


void joystickInit() 
{
	// Mark joystick bits as input bits
	IODIR0 &= IODIR_IN(JOYSTICK_BITS);
}

void rgbInit() 
{
	IODIR0 |= IODIR_OUT(RGB_BITS);
}

int main()
{
	IODIR0 |= BIT10;

	uart0Init();
	joystickInit();
	rgbInit();
	
	busywaitInit();

	while(1) {
		printNum(wait_time);
		busywait(wait_time);
		change_freq();
		if (freq != OFF_FREQ_HZ) {
			toggle_rgb();
		}
	}

}





