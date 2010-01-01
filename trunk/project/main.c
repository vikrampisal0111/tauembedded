
/*
 * USB Mouse Example.
 * 
 * Sivan Toledo and Wouter van Ooijen 
 */
/* Sivan: not yet ported & tested ! */

#include <stdint.h>
#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

#define UART0_BAUD_RATE 19200
#define CLOCKS_PCLK 6000000

#include "uart0-polling.c"
#define print_char(x) uart0SendByte(x)
#include "print.c"

#include "spi/spi.c"
#include "mmc/mmc.c"

int main(void) { 
  int i;
  int testok = 1;
  uart0Init();  
  SPI_Init();
  if (mmc_init() == 0)
	print("MMC Init success!\n");
  else
	print("MMC Init failure!\n");
  
  //setup write data.
  for (i = 0; i < 512; i++)
	MMCWRData[i] = 512-i;

  if (mmc_write_block(51) == 0)
	print("MMC Write block 51 success!\n");
  else
	print("MMC Read block 51 failure!\n");
  
    if (mmc_read_block(51) == 0)
	print("MMC Read block 51 success!\n");
  else
	print("MMC Read block 51 failure!\n");
	
  print("Block 51 Read Data\n");
  print("------------------\n");
  for (i=0;i<512;i++)
  {
	printNum(MMCRDData[i]);
	if (MMCRDData[i] != MMCWRData[i])
		testok = 0;
	if ((i+1)%32 ==0)
		print("\n");
  }
  
  if (testok)
	print("Test passed!\n");
  else
    print("Test failed!\n");
  
  while (1) {
  }

}