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
 *    in a polled application, i.e., interrupts are not used.
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

#define SPI_SLAVE_CS 0x00800000  //pin P0.23
#define FAILSAFE_VALUE 5000

//assuming bit6 -> SegA, bit5 -> segB, etc. and zero = led on
static const tU8 sevenSegPattern[10] = {0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1F, 0x01, 0x09};


/*****************************************************************************
 * Public function prototypes
 ****************************************************************************/
int main(void);


/*****************************************************************************
 * Local function prototypes
 ****************************************************************************/
static void delayMs(tU16 delayInMs);


/*****************************************************************************
 * Implementation of local functions
 ****************************************************************************/

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
 *    Sends and received one byte over the SPI serial channel.
 *    A polled implementation.
 *
 * Params:
 *    [in] byte - the byte to send.
 *
 * Return:
 *    The received byte.
 *
 ****************************************************************************/
static tU8
sendSpi(tU8 byte)
{
	tU32 failsafe;
	
  IOCLR0 = SPI_SLAVE_CS;  //activate SPI
  
  SPI_SPDR = byte;
  failsafe = 0;
  while(((SPI_SPSR & 0x80) == 0) && (failsafe < FAILSAFE_VALUE))
    failsafe++;

  IOSET0 = SPI_SLAVE_CS;  //deactivate SPI
  
  //reinitialize SPI if timeout
  if (failsafe >= FAILSAFE_VALUE)
  {
    printf("\nSPI-error: SSEL signal must be high!\n");
    SPI_SPCCR = 0x08;    
    SPI_SPCR  = 0x60;
  }
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
	tU8 counter = 0;

  //initialize printf()-functionality
  eaInit();

  //print welcome message
  printf("\n*********************************************************");
  printf("\n*");
  printf("\n* (C) 2005 Embedded Artists AB");
  printf("\n*");
  printf("\n* Welcome to this program that demonstrates how to use");
  printf("\n* the SPI interface in a polled application.");
  printf("\n* In this case, a 74HC595 shift register is connected");
  printf("\n* to the SPI bus. The 74HC595 has 8 outputs, so this");
  printf("\n* will be a simple port expander...");
  printf("\n* imagine that a 7-segment display is connected to the");
  printf("\n* outputs (out1->segA, out2->segB, etc.");
  printf("\n*");
  printf("\n*********************************************************");

  //connect SPI bus to IO-pins
  PINSEL0 |= 0x00005500;
  
  //initialize SPI interface
  SPI_SPCCR = 0x08;    
  SPI_SPCR  = 0x60;

  //make SPI slave chip select an output and set signal high
  IODIR |= SPI_SLAVE_CS;
  IOSET  = SPI_SLAVE_CS;

  while(1)
  {
    //output bit pattern to external shift register -> 7-segment display
  	sendSpi(sevenSegPattern[counter]);

    //update counter
  	counter++;
  	if (counter >= 10)
  	  counter = 0;

    //wait 500 ms
    delayMs(500);
  }

  return 0;
}
