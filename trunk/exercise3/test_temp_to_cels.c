
#include <stdio.h>	
#include <stdint.h>

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

	
int main()
{
	printf("%f\n", tempdata_to_int((uint16_t)0xE700));
	return 1;
}





