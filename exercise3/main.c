#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>


#define CLOCKS_PCLK		3000000
#define I2C_BIT_RATE	400000

#include "i2c.c"

int main()
{
	int32_t return_code; 

	return_code = i2cMasterTransact(0x90, 0, 0, response, 2);
}





