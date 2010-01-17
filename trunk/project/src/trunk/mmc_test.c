
#include <stdio.h>
#include <stdint.h>
#include <io.h>
#include "uart0.h"
#include "spi1.h"
#include "mmc.h"


extern BYTE* MMCWRData; 
extern BYTE* MMCRDData; 

int main(void) { 
  int i;
  int testok = 1;
  uart0Init();  
  SPI_Init();

  if (mmc_init() == 0)
	printf("MMC Init success!\n");
  else
	printf("MMC Init failure!\n");
  
  //setup write data.
  for (i = 0; i < 512; i++)
	MMCWRData[i] = 512-i;

  if (mmc_write_block(51) == 0)
	printf("MMC Write block 51 success!\n");
  else
	printf("MMC Read block 51 failure!\n");
  
    if (mmc_read_block(51) == 0)
	printf("MMC Read block 51 success!\n");
  else
	printf("MMC Read block 51 failure!\n");
	
  printf("Block 51 Read Data\n");
  printf("------------------\n");
  for (i=0;i<512;i++)
  {
	printf("%d",MMCRDData[i]);
	if (MMCRDData[i] != MMCWRData[i])
		testok = 0;
	if ((i+1)%32 ==0)
		printf("\n");
  }
  
  if (testok)
	printf("Test passed!\n");
  else
    printf("Test failed!\n");

}
