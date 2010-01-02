/*
* Name: SPI_MMC.H
* Purpose: SPI mode SD/MMC card interface driver
* Version: V1.03
* Copyright (c) 2006 NXP Semiconductor. All rights reserved.
*---------------------------------------------------------------------*/

#ifndef __SPI_MMC_H__
#define __SPI_MMC_H__

/* SPI select pin */
#define SPI_SEL 0x00100000

/* The max MMC flash size is 256MB */
#define MAX_TIMEOUT 0xFF
#define IDLE_STATE_TIMEOUT

#define OP_COND_TIMEOUT 2
#define SET_BLOCKLEN_TIMEOUT 3
#define WRITE_BLOCK_TIMEOUT 4
#define WRITE_BLOCK_FAIL 5
#define READ_BLOCK_TIMEOUT 6
#define READ_BLOCK_DATA_TOKEN_MISSING 7
#define DATA_TOKEN_TIMEOUT 8
#define SELECT_CARD_TIMEOUT 9
#define SET_RELATIVE_ADDR_TIMEOUT 10

void SPI_Init( void );
void SPI_Send( BYTE *Buf, DWORD Length );
uint32_t SPI_TxRx(uint32_t out);
void SPI_Receive( BYTE *Buf, DWORD Length );
BYTE SPI_ReceiveByte( void );

#endif /* __SPI_MMC_H__ */
