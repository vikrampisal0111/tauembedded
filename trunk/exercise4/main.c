#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>
#include <lpc2000/lpc214x.h>

#define TTEMP 	((uint8_t)0x0)
#define TCONF 	((uint8_t)0x1)
#define THYST 	((uint8_t)0x2)
#define TOS		((uint8_t)0x3)

#define CLOCKS_PCLK		3000000
#define I2C_BIT_RATE	375000

#include <stdint.h>

#define UART0_BAUD_RATE 19200
#include "uart/uart0-polling.c"
#define  print_char(x) uart0SendByte(x)
#include "uart/print.c"

#include "vic/vic.c"
#include "i2c/i2c.c"
#include "lcd.c"

#define MASK_8BIT 	0xFF

float tempdata_to_celsius(uint16_t temp) 
{
	int msb = (temp & BIT16) >> 15;
	int ctemp = (temp >> 7) & MASK_8BIT;

	if(msb == 1) {
		// Negatibe num: flip all bits and add 1
		ctemp = -((~ctemp & MASK_8BIT) + 1);
	}
	
	// LSB == 0.5C
	return ctemp/2.0;
}

int main()
{
	int32_t return_code; 
	uint8_t response[2] = {0};
	int temp = 0;
 	char tempStr[3] = {0};

#ifdef USE_FIQ
	// Select an enable I2C0 Interrupts
	// Use FIQ
	VICIntSelect &= ~BIT9;
	VICIntEnable = BIT9;
#endif
	vicInit();

	vicEnableIRQ(INT_CHANNEL_I2C0,
		0, viq_9);
	vicEnable(INT_CHANNEL_I2C0);	
	enable_interrupts();

	uart0Init();
	i2cInit();
	lcdInit();

	while(1) {
		return_code = i2cMasterTransact(0x90, 0, 0, response, 2);
		temp = (int)tempdata_to_celsius((response[0]<<8) + response[1]);

		printNum(temp);
		print("\n");

		tempStr[0] = '0' + (temp / 10);
		tempStr[1] = '0' + (temp % 10);
		lcdGoto(0);
		lcdPrintString(tempStr);
		
		busywait(10000);
	}
}





