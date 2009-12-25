/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * File:
 *    main.c
 *
 * Description:
 *    Sample application that demonstrates how to use the SPI interface
 *    with interrupts.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include <ea_init.h>
#include "startup/config.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

#define NULL 0

#define SPI_SLAVE_CS 0x00800000  //pin P0.23

#define SPI_COMMAND_WRITE          0x00
#define SPI_COMMAND_READ           0x01
#define SPI_COMMAND_WRITE_AND_READ 0x02

typedef struct
{
	tU8  command;
	tU16 length;
	tU8 *pBuff;
	tU8 *pReadyFlag;
} tSpiCommand;

/*****************************************************************************
 * Local variables
 ****************************************************************************/
tSpiCommand spiCommandData;

/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/
int main(void);


/*****************************************************************************
 * Local function prototypes
 ****************************************************************************/
static void delayMs(tU16 delayInMs);
static void spiISR(void) __attribute__ ((interrupt));
static void spiCommand(tU8 command, tU16 length, tU8 *pBuffer, tU8 *pReadyFlag);


/*****************************************************************************
 * Implementation of local functions
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Actual spi ISR.
 *
 ****************************************************************************/
static void
spiISR(void)
{
	volatile tU8 status;

  status = SPI_SPSR;

	//check current command
	if (spiCommandData.command == SPI_COMMAND_WRITE)
	{
		//check if end of transmission
		if (spiCommandData.length <= 1)
		{
      IOSET0 = SPI_SLAVE_CS;              //deactivate SPI

     	if (spiCommandData.pReadyFlag != NULL)
        *spiCommandData.pReadyFlag = TRUE;      //signal that transmission is ready
    }
    else
    {
		  spiCommandData.pBuff++;
		  SPI_SPDR = *spiCommandData.pBuff;
		  spiCommandData.length--;
		}
	}

	else if (spiCommandData.command == SPI_COMMAND_READ)
	{
		*spiCommandData.pBuff = SPI_SPDR;   //get received data byte
	  spiCommandData.pBuff++;
	  spiCommandData.length--;

		//check if end of transmission
		if (spiCommandData.length == 0)
		{
      IOSET0 = SPI_SLAVE_CS;              //deactivate SPI

     	if (spiCommandData.pReadyFlag != NULL)
        *spiCommandData.pReadyFlag = TRUE;      //signal that transmission is ready
		}
		else
		{
      SPI_SPDR = 0xff;   //send dummy byte to ready next byte
		}
	}

  SPI_SPINT   = 0x01;        //reset IRQ flag in spi
  VICVectAddr = 0x00;        //dummy write to VIC to signal end of interrupt
}


/*****************************************************************************
 *
 * Description:
 *    Delay execution by a specified number of milliseconds by using
 *    timer #1. A polled implementation.
 *
 * Params:
 *    [in] delayInMs - the number of milliseconds to delay.
 *
 ****************************************************************************/
static void
delayMs(tU16 delayInMs)
{
  /*
   * setup timer #1 for delay
   */
  TIMER1_TCR = 0x02;          //stop and reset timer
  TIMER1_PR  = 0x00;          //set prescaler to zero
  TIMER1_MR0 = delayInMs * ((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (1000 * VPBDIV_FACTOR));
  TIMER1_IR  = 0xff;          //reset all interrrupt flags
  TIMER1_MCR = 0x04;          //stop timer on match
  TIMER1_TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (TIMER1_TCR & 0x01)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Initiate a spi command sequence that is interruot driven
 *
 * Params:
 *    [in] command    - type of spi command to initiate 
 *    [in] length     - length of data buffer to transmit or receive
 *    [in] pBuffer    - pointer to the data buffer
 *    [in] pReadyFlag - pointer to a flag to signal completion of command
 *
 ****************************************************************************/
static void
spiCommand(tU8 command, tU16 length, tU8 *pBuffer, tU8 *pReadyFlag)
{
	spiCommandData.command    = command;
	spiCommandData.length     = length;
	spiCommandData.pBuff      = pBuffer;
	spiCommandData.pReadyFlag = pReadyFlag;
	if (pReadyFlag != NULL)
  	*pReadyFlag = FALSE;
	
	IOCLR0 = SPI_SLAVE_CS;  //activate SPI
  
  //start transmission with first byte
  if (command == SPI_COMMAND_WRITE)
    SPI_SPDR = *pBuffer;
  else if (command == SPI_COMMAND_READ)
    SPI_SPDR = 0xff;
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the SPI interface to use interrupts.
 *
 ****************************************************************************/
static void
initSpi(void)
{
  //connect SPI bus to IO-pins
  PINSEL0 |= 0x00005500;

  //initialize SPI interface
  SPI_SPCCR = 0x08;    
  SPI_SPCR  = 0x60 | 0x80;   //0x80 = enable interrupts from spi

  //make SPI slave chip select an output and set signal high
  IODIR |= SPI_SLAVE_CS;
  IOSET  = SPI_SLAVE_CS;

  //initialize VIC for SPI interrupts
  VICIntSelect  &= ~0x00000400;     //spi interrupt is assigned to IRQ (not FIQ)
  VICVectAddr10  = (tU32)spiISR;    //register ISR address
  VICVectCntl10  = 0x2A;            //enable vector interrupt for spi
  VICIntEnable   = 0x00000400;      //enable spi interrupt
}


/*****************************************************************************
 * Implementation of public functions
 ****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    The main-function. 
 *
 * Returns:
 *    Always 0, since return value is not used.
 *
 ****************************************************************************/
int
main(void)
{
  //initialize printf()-functionality
  eaInit();

  //print welcome message
  printf("\n*********************************************************");
  printf("\n*");
  printf("\n* (C) 2005 Embedded Artists AB");
  printf("\n*");
  printf("\n* Welcome to this program that demonstrates how to use the");
  printf("\n* SPI interface with interrupts.");
  printf("\n*");
  printf("\n*********************************************************");

  //initialize SPI interface
  initSpi();

  while(1)
  {
  	tU8 dataBuff1[2] = {0xaa, 0x55};
  	tU8 dataBuff2[4];
  	volatile tU8 readyFlag;

    //start spi command
  	spiCommand(SPI_COMMAND_WRITE, sizeof(dataBuff1), dataBuff1, (tU8 *)&readyFlag);
  	
  	//wait until ready...
  	while (readyFlag != TRUE)
  	  ;

    //start spi command
  	spiCommand(SPI_COMMAND_READ, sizeof(dataBuff2), dataBuff2, (tU8 *)&readyFlag);

  	//wait until ready...
  	while (readyFlag != TRUE)
  	  ;

    printf("\nSpi command done! (read byte #1=%d, #2=%d, #3=%d, #4=%d", dataBuff2[0], dataBuff2[1], dataBuff2[2], dataBuff2[3]);

    //wait 1000 ms
    delayMs(1000);
  }

  return 0;
}
