/*-----------------------------------------------------------------------
 * Name: SPI_MMC.C
 * Purpose: SPI and SD/MMC command interface Module
 * Version: V1.03
 * Copyright (c) 2006 NXP Semiconductor. All rights reserved.
 *---------------------------------------------------------------------*/
#include "lpc214x.h" /* LPC214x definitions */
#include "type.h"
#include "spi.h"

/* SPI Status register */
#define SSPSR_TFE	1 << 0
#define SSPSR_TNF	1 << 1
#define SSPSR_RNE	1 << 2
#define SSPSR_RFF	1 << 3
#define SSPSR_BSY	1 << 4

void SPI_Init( void )
{
    DWORD portConfig;
    BYTE i, Dummy;

    SSPCR1 = 0x00; /* SSP master (off) in normal mode */

    //portConfig = PINSEL1;
    //PINSEL1 = portConfig | 0x00A8;
    //IODIR0 = SPI_SEL; /* SSEL is output */
    //IOSET0 = SPI_SEL; /* set SSEL to high */

    // Select pins
#define CS_SEL	    BIT26|BIT27
#define SCK_SEL	    BIT3
#define MOSI_SEL    BIT7
#define MISO_SEL    BIT5
    PINSEL0 &= ~CS_SEL;
    PINSEL1 |= SCK_SEL | MISO_SEL | MOSI_SEL;

    // Select pin directions
    IODIR0 |= BIT17 | BIT19;
    IODIR0 &= ~BIT18;

    // ENC28J60 ETH_CS is P0.13. Set CS to high
    IOSET0 = BIT13;

    PCONP |= BIT10;

    /* Set PCLK 1/2 of CCLK */
    VPBDIV = 0x02;
    /* Set data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0,
       and SCR is 15 */
    SSPCR0 = 0x0707;
    /* SSPCPSR clock prescale register, master mode, minimum divisor
       is 0x02*/
    SSPCPSR = 0x2;
    /* Device select as master, SSP Enabled, normal operational mode */
    SSPCR1 = 0x02;
    for ( i = 0; i < 8; i++ )
    {
        Dummy = SSPDR; /* clear the RxFIFO */
    }

    return;
}


/*
 * SPI Send a block of data based on the length
 */
void SPI_Send( BYTE *buf, DWORD Length )
{
    BYTE Dummy;
    if ( Length == 0 )
        return;
    while ( Length != 0 )
    {

        /* as long as TNF bit is set, TxFIFO is not full, I can write */
        while ( !(SSPSR & SSPSR_TNF) );
        SSPDR = *buf;
        /* Wait until the Busy bit is cleared */
        while ( !(SSPSR & SSPSR_BSY) );
        Dummy = SSPDR; /* Flush the RxFIFO */
        Length--;
        buf++;
    }
    return;
}

uint8_t SPI_TxRx(uint8_t out)
{
    uint8_t in;
    
    SSPDR = out;
   
    while (!(SSPSR & SSPSR_BSY));
   
     in = SSPDR;
    if (in != -1) {
	   printf("%x <--> %x\n", out, in);
     }
    return in;
}

/*
 * SPI receives a block of data based on the length
 */
void SPI_Receive( BYTE *buf, DWORD Length )
{
    DWORD i;
    for (i = 0; i < Length; i++)
    {
        *buf = SPI_ReceiveByte();
        buf++;
    }
    return;
}


/*
 * SPI Receive Byte, receive one byte only, return Data byte
 * used a lot to check the status.
 */
BYTE SPI_ReceiveByte( void )
{
    BYTE data;
    /* wrtie dummy byte out to generate clock, then read data from
       MISO */
    SSPDR = 0xFF;
    /* Wait until the Busy bit is cleared */
    while ( SSPSR & 0x10 );
    data = SSPDR;
    return ( data );
}


