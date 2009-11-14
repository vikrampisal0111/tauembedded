#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

/* I2C Interface 0 
#define I2C0_BASE_ADDR		0xE001C000
#define I20CONSET      (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x00))
#define I20STAT        (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x04))
#define I20DAT         (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x08))
#define I20ADR         (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x0C))
#define I20SCLH        (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x10))
#define I20SCLL        (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x14))
#define I20CONCLR      (*(volatile unsigned long *)(I2C0_BASE_ADDR + 0x18))
*/


#define TTEMP 	((uint8_t)0x0)
#define TCONF 	((uint8_t)0x1)
#define THYST 	((uint8_t)0x2)
#define TOS		((uint8_t)0x3)

#define CLOCKS_PCLK		3000000
#define I2C_BIT_RATE	400000

#include "i2c.c"


float tempdata_to_celsius(uint16_t temp) 
{
#define BIT16		0x8000
#define MASK_8BIT 	0xFF

	int msb = (temp & BIT16) >> 15;
	int ctemp = (temp >> 7) & MASK_8BIT;

	if(msb == 1) {
		printf("xxx\n");
		// Negatibe num: flip all bits and add 1
		ctemp = -((~ctemp & MASK_8BIT) + 1);
	}
	
	// LSB == 0.5C
	return ctemp/2.0;
}


#include <stdio.h>	
	
int main()
{
	/*	
	int32_t return_code; 

	return_code = i2cMasterTransact(0x90, 0, 0, response, 2);
	*/

	printf("%d", temp_to_int(0xE700));
}





