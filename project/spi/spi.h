#ifndef _SPI_H_
#define _SPI_H_

/* SPI select pin */ 
#define SPI_SEL      (1 << 11) 

void SPI_Init( void ); 
void SPI_Send( BYTE *Buf, DWORD Length ); 
void SSP_SendRecvByte( BYTE *Buf, DWORD Length ); 
BYTE SSP_SendRecvByteByte( void ); 

#endif //_SPI_H_
