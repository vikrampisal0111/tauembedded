/*
 * Copyright (c) 2009, Manish Shakya,Real Time Solutions Pvt. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */
/**

Based upon

Advairtizer V1.0
www.braintechnology.de
*/

//
//#include <global.h>
//#include <uip/uip.h>
//#include <uip/uip_arp.h>
//#include <uip/timer.h>

//#include "peripherals.h"
#include <stdio.h>
#include "type.h"
#include "trace.h"
#include "lpc214x.h"

#include "../spi/spi.h"
#include "enc28j60.h"

uint8_t Enc28j60Bank;
uint16_t NextPacketPtr;

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define CS_ETHERNET    20
#define RESET_ETHERNET    11
#define INTR_ETHERNET	15


#define ETH_CTRLOR_CS_INIT()		\
    {					\
	SCS = 0x01;			\
	FIO0DIR |= (1<<CS_ETHERNET);	\
    }

#define ETH_CTRLOR_CS_HIGH()		    \
    {					    \
     FIO0MASK =~ (1 << CS_ETHERNET);	    \
     FIO0PIN = (1 << CS_ETHERNET);	    \
	{				    \
	    unsigned i;			    \
	    for(i = 0; i < 100; i++);	    \
	}				    \
    }

#define ETH_CTRLOR_CS_LOW()		\
    {					\
	FIO0MASK =~ (1 << CS_ETHERNET);	\
	FIO0PIN =~ (1 << CS_ETHERNET);	\
	{				\
	    unsigned i;			\
	    for(i=0; i<100; i++);	\
	}				\
    }

#define ETH_CTRLOR_RESET_INIT()		\
    {					\
	SCS = 0x01;			\
	FIO0DIR |= ( 1 << RESET_ETHERNET);	\
    }

#define ETH_CTRLOR_RESET_HIGH()			\
    {						\
	FIO0MASK = ~(1 << RESET_ETHERNET);	\
	FIO0PIN = (1 << RESET_ETHERNET);        \
	{					\
	    unsigned i;				\
	    for(i = 0; i < 100; i++);		\
	}					\
    }

#define ETH_CTRLOR_RESET_LOW()		    \
    {					    \
	FIO0MASK = ~(1 << RESET_ETHERNET);  \
	FIO0PIN = ~(1 << RESET_ETHERNET);   \
	{				    \
	    unsigned i;			    \
	    for(i = 0; i < 100; i++);	    \
	}				    \
    }

#define ETH_CTRLOR_SPI_INIT()	 SPI_Init()
#define ETH_CTRLOR_SPI(X)  	 SPI_TxRx((X))

#define ETH_CTRLOR_INT_INIT()		    \
    {					    \
	SCS = 0x01;			    \
	FIO0DIR &= ~(1 << INTR_ETHERNET);   \
    }

#define ETH_CTRLOR_INT_IN(X)		    \
    {					    \
	FIO0MASK = ~(1 << INTR_ETHERNET);    \
	X = FIO0PIN;			    \
    }


uint8_t  enc28j60_read_op(uint8_t op, uint8_t address)
{

    uint8_t data;
    /* assert CS*/
    ETH_CTRLOR_CS_LOW();
    ETH_CTRLOR_SPI( op | (address & ADDR_MASK));
    data = ETH_CTRLOR_SPI(0);
    if(address & 0x80)
    {
	data=ETH_CTRLOR_SPI(0);
    }
    ETH_CTRLOR_CS_HIGH();	
    return data;
}

void enc28j60_write_op(uint8_t op, uint8_t address, uint8_t data)
{
    /* assert CS*/
    ETH_CTRLOR_CS_LOW();	 

    // issue write command
    ETH_CTRLOR_SPI( op | (address & ADDR_MASK));

    // write data
    ETH_CTRLOR_SPI(data);

    // release CS
    ETH_CTRLOR_CS_HIGH();//CS auf High
}

void enc28j60_set_bank(uint8_t address)
{
    // set the bank (if needed)
    if((address & BANK_MASK) != Enc28j60Bank)
    {
	// set the bank
	enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
	enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
	Enc28j60Bank = (address & BANK_MASK);
    }
}
uint8_t enc28j60_read(uint8_t address)
{
    // set the bank
    enc28j60_set_bank(address);
    // do the read
    return enc28j60_read_op(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60_write(uint8_t address, uint8_t data)
{
    // set the bank
    enc28j60_set_bank(address);
    // do the write
    enc28j60_write_op(ENC28J60_WRITE_CTRL_REG, address, data);
}

void enc28j60_set_mac_address(uint8_t *macaddr)
{
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    enc28j60_write(MAADR5, *macaddr++);
    enc28j60_write(MAADR4, *macaddr++);
    enc28j60_write(MAADR3, *macaddr++);
    enc28j60_write(MAADR2, *macaddr++);
    enc28j60_write(MAADR1, *macaddr++);
    enc28j60_write(MAADR0, *macaddr++);
}


void enc28j60_phy_write(uint8_t address, uint16_t data)
{
    // set the PHY register address
    enc28j60_write(MIREGADR, address);

    // write the PHY data
    enc28j60_write(MIWRL, data);
    enc28j60_write(MIWRH, data>>8);
printf("wait phy write\n");
    // wait until the PHY write completes
    while(enc28j60_read(MISTAT) & MISTAT_BUSY);
printf("wait phy write complete\n");
}

void enc28j60_init(void)
{
    uint32_t timeout;
    unsigned char i;
    /* Ethernet CS*/
    ETH_CTRLOR_CS_INIT();
    printf("CD_INIT\n");
    ETH_CTRLOR_RESET_INIT();
    printf("RESET_INIT\n");
    ETH_CTRLOR_INT_INIT();
    printf("INT_INIT\n");
    ETH_CTRLOR_SPI_INIT();
    printf("SPI_INIT\n");


    ETH_CTRLOR_RESET_HIGH();
    printf("Reset_High\n");
    ETH_CTRLOR_CS_HIGH();
    printf("CS_High\n"); 

    // RESET the entire ENC28J60, clearing all registers
    // Also wait for CLKRDY to become set.
    // Bit 3 in ESTAT is an unimplemented bit.  If it reads out as '1' that
    // means the part is in RESET or there is something wrong with the SPI
    // connection.  This loop makes sure that we can communicate with the
    // ENC28J60 before proceeding.

   // printf("\r\nPOR DELAY");
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
printf("after timeouts\n");
#if 0
    // perform system reset
    enc28j60_write_op(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    // check CLKRDY bit to see if reset is complete
    // Errata workaround #2, CLKRDY check is unreliable, delay 1 mS instead

    TRACE("\r\nWaiting for Timeout");
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    for(timeout=0;timeout<1000000;timeout++);
    TRACE("\r\nTimeout completed");

    timeout=0;
    while(!(enc28j60_read(ESTAT) & ESTAT_CLKRDY)){

	timeout++;
	if (timeout > 100000)
	    TRACE("enc28j60_read timeout");
	break;
    };
    TRACE("Checking ESTAT_CLKRDY Completed %d", timeout);
#endif
    do
    {

	for(timeout=0;timeout<1000000;timeout++);
	for(timeout=0;timeout<1000000;timeout++);
	//	enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);//wake up
	for(timeout=0;timeout<1000000;timeout++);
	for(timeout=0;timeout<1000000;timeout++);
	enc28j60_write_op(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	for(timeout=0;timeout<1000000;timeout++);
	for(timeout=0;timeout<1000000;timeout++);
	i=(enc28j60_read(ESTAT) & ESTAT_CLKRDY);
    }while((i & 0x08) || (~i & ESTAT_CLKRDY));
    printf("enc28j60 READY");
    fflush(stdout);
    //        }while(timeout > 100000);
    // do bank 0 stuff
    // initialize receive buffer
    // 16-bit transfers, must write low byte first
    // set receive buffer start address
    NextPacketPtr = RXSTART_INIT;
    enc28j60_write(ERXSTL, RXSTART_INIT&0xFF);
    printf("enc28j60_write(ERXSTL, RXSTART_INIT&0xFF);\n");

    enc28j60_write(ERXSTH, RXSTART_INIT>>8);
    printf("enc28j60_write(ERXSTH, RXSTART_INIT>>8);\n");

    // set receive pointer address
    enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
    printf("enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);\n");

    enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
    printf("enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);\n");

    //	// set receive buffer end
    enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
    enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
    //	// set transmit buffer start

    enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
    enc28j60_write(ETXSTH, TXSTART_INIT>>8);
    //
    //	// do bank 2 stuff
    //	// enable MAC receive
    enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
    //	// bring MAC out of reset
    enc28j60_write(MACON2, 0x00);
    //	// enable automatic padding and CRC operations
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
    // set inter-frame gap (back-to-back)
    enc28j60_write(MABBIPG, 0x12);
    // Allow infinite deferals if the medium is continuously busy 
    // (do not time out a transmission if the half duplex medium is 
    // completely saturated with other people's data)
    printf("before defer\n");
    // Allow infinite deferals if the medium is continuously busy 
    // (do not time out a transmission if the half duplex medium is 
    // completely saturated with other people's data)
    enc28j60_write(MACON2, MACON4_DEFER);
    printf("After defer\n");
    // Late collisions occur beyond 63+8 bytes (8 bytes for preamble/start of frame delimiter)
    // 55 is all that is needed for IEEE 802.3, but ENC28J60 B5 errata for improper link pulse 
    // collisions will occur less often with a larger number.
    enc28j60_write((BYTE)MACLCON2, 63);
    //
    //	// set inter-frame gap (non-back-to-back)
    enc28j60_write(MAIPGL, 0x12);
    enc28j60_write(MAIPGH, 0x0C);
    // Set the maximum packet size which the controller will accept
    enc28j60_write(MAMXFLL, MAX_FRAMELEN&0xFF);	
    enc28j60_write(MAMXFLH, MAX_FRAMELEN>>8);

    //do bank 3 stuff
    //write MAC address
    //NOTE: MAC address in ENC28J60 is byte-backward
    //set Mac Addr.
    printf("Before mac set\n");
{
    uint8_t mymac[6] = {0x00,0x04,0x0e,0xf8,0xb7,0xf6};
    enc28j60_set_mac_address(mymac);
}
    printf("After mac set\n");
// Disable the CLKOUT output to reduce EMI generation
enc28j60_write(ECOCON, 0x00);	// Output off (0V)

printf("before phy write\n");
fflush(stdout);
// no loopback of transmitted frames
enc28j60_phy_write(PHCON2, PHCON2_HDLDIS);


printf("Before if 1\n");
fflush(stdout);
#if 1
{
    int i;
    for(i=0;i<5;i++)
    {
	//			unsigned int j;
	enc28j60_phy_write(PHLCON, 0x3990);//off
	//			 for(timeout=0;timeout<1000000;timeout++);
	//			for(timeout=0;timeout<1000000;timeout++);
	for(timeout=0;timeout<1000000;timeout++);

	enc28j60_phy_write(PHLCON, 0x3880);//on
	//			 for(timeout=0;timeout<1000000;timeout++);
	//		for(timeout=0;timeout<1000000;timeout++);
	for(timeout=0;timeout<1000000;timeout++);


    }
}
#endif
printf("After if 1\n");
fflush(stdout);
enc28j60_phy_write(PHLCON, 0x472);//Display link status and transmit/receive activity (always stretched)
//enc28j60_phy_write(PHLCON1, 0x0000);//Display link status and transmit/receive activity (always stretched)
//        // enable interrutps
//        // EIE: ETHERNET INTERRUPT ENABLE REGISTER
//        // PKTIE: Receive Packet Pending Interrupt Enable bit
//        // INTIE: Global INT Interrupt Enable bit
enc28j60_write_op(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);		
enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

enc28j60_write(ERXFCON,(CRCEN|ANDOR));//Bank1 
}



//uint16_t dev_poll(void)
//{
//	return enc28j60PacketReceive(UIP_BUFSIZE, uip_buf);
//}
//
//void dev_send(void)
//{
//	enc28j60PacketSend(uip_len, uip_buf);
//}

void enc28j60_get_mac_address(uint8_t *macaddr)
{
    // read MAC address registers
    // NOTE: MAC address in ENC28J60 is byte-backward
    *macaddr++ = enc28j60_read(MAADR5);
    *macaddr++ = enc28j60_read(MAADR4);
    *macaddr++ = enc28j60_read(MAADR3);
    *macaddr++ = enc28j60_read(MAADR2);
    *macaddr++ = enc28j60_read(MAADR1);
    *macaddr++ = enc28j60_read(MAADR0);
}

void enc28j60_write_buffer(uint32_t len, uint8_t * data)
{
    // assert CS
    ETH_CTRLOR_CS_LOW(); //CS auf Low
    //   	pause(ONE_US*100);
    //	// issue write command
    ETH_CTRLOR_SPI(ENC28J60_WRITE_BUF_MEM);
    while(len--)
    {
	// write data
	ETH_CTRLOR_SPI(*data++);
    }	
    // release CS
    ETH_CTRLOR_CS_HIGH();
}

void enc28j60_read_buffer(uint32_t len, uint8_t *data)
{
    // assert CS
    ETH_CTRLOR_CS_LOW(); //CS auf Low
    // pause(ONE_US*100);
    // issue read command
    ETH_CTRLOR_SPI(ENC28J60_READ_BUF_MEM);	
    while(len--)
    {
	// read data
	*data++ =ETH_CTRLOR_SPI(0);
    }	

    // release CS
    ETH_CTRLOR_CS_HIGH();
}


void enc28j60_packet_send(uint32_t len, uint8_t *packet)
{
    // Errata sheet
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
    enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF | EIR_TXIF);
    //BFCReg(EIR, EIR_TXERIF | EIR_TXIF);

    // Set the write pointer to start of transmit buffer area
    enc28j60_write(EWRPTL, (unsigned char  )TXSTART_INIT);
    enc28j60_write(EWRPTH, TXSTART_INIT>>8);

    // Set the TXND pointer to correspond to the packet size given
    enc28j60_write(ETXNDL, (TXSTART_INIT+len));
    enc28j60_write(ETXNDH, (TXSTART_INIT+len)>>8);

    // write per-packet control byte
    enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

    // copy the packet into the transmit buffer
    enc28j60_write_buffer(len, packet);

    // send the contents of the transmit buffer onto the network
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}
unsigned int enc28j60_packet_receive(uint32_t maxlen, uint8_t *packet)
{
    uint16_t rxstat;
    uint16_t len;

    if( !enc28j60_read(EPKTCNT) )
	return 0;

    // Set the read pointer to the start of the received packet
    enc28j60_write(ERDPTL, (NextPacketPtr));
    enc28j60_write(ERDPTH, (NextPacketPtr)>>8);

    // read the next packet pointer
    NextPacketPtr  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
    NextPacketPtr |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;

    // read the packet length
    len  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
    len |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;

    // read the receive status
    rxstat  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;

    // limit retrieve length
    // (we reduce the MAC-reported length by 4 to remove the CRC)
    len = IFMIN(len, maxlen);

    // copy the packet from the receive buffer
    enc28j60_read_buffer(len, packet);

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    enc28j60_write(ERXRDPTL, (NextPacketPtr));
    enc28j60_write(ERXRDPTH, (NextPacketPtr)>>8);

    // Errata workaround #13. Make sure ERXRDPT is odd

    {
	uint16_t rs,re;
	rs = enc28j60_read(ERXSTH);
	rs <<= 8;
	rs |= enc28j60_read(ERXSTL);
	re = enc28j60_read(ERXNDH);
	re <<= 8;
	re |= enc28j60_read(ERXNDL);
	if (NextPacketPtr - 1 < rs || NextPacketPtr - 1 > re)
	{
	    enc28j60_write(ERXRDPTL, (re));
	    enc28j60_write(ERXRDPTH, (re)>>8);
	}
	else
	{
	    enc28j60_write(ERXRDPTL, (NextPacketPtr-1));
	    enc28j60_write(ERXRDPTH, (NextPacketPtr-1)>>8);
	}
    }

    // decrement the packet counter indicate we are done with this packet
    // clear the PKTIF: Receive Packet Pending Interrupt Flag bit
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

    return len;
}

// uint32_t enc28j60_is_packet_received()
// {
// 	 uint32_t  X;
//	 ETH_CTRLOR_INT_IN(X);
//	 if(X&(1<<INTR_ETHERNET)==0){
//		 return 1;
//	}else{
//		return 0;
//	}
// }
#ifdef trace
void enc28j60_reg_dump(void){

    trace("\r\nRevID: 0x%x\r\n", enc28j60_read(EREVID));

    trace ( ("\r\nCntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n"));
    trace ( ("         "));
    trace("%02x",enc28j60_read(ECON1));
    trace( ("    "));
    trace("%02x",enc28j60_read(ECON2));
    trace( ("    "));
    trace("%02x",enc28j60_read(ESTAT));
    trace( ("    "));
    trace("%02x",enc28j60_read(EIR));
    trace( ("   "));
    trace("%02x",enc28j60_read(EIE));
    trace( ("\r\n"));

    trace( ("\r\nMAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\r\n"));
    trace( ("        0x"));
    trace("%02x",enc28j60_read(MACON1));
    trace( ("    0x"));
    trace("%02x",enc28j60_read(MACON2));
    trace( ("    0x"));
    trace("%02x",enc28j60_read(MACON3));
    trace( ("    0x"));
    trace("%02x",enc28j60_read(MACON4));
    trace( ("   "));
    trace("%02x",enc28j60_read(MAADR5));
    trace("%02x",enc28j60_read(MAADR4));
    trace("%02x",enc28j60_read(MAADR3));
    trace("%02x",enc28j60_read(MAADR2));
    trace("%02x",enc28j60_read(MAADR1));
    trace("%02x",enc28j60_read(MAADR0));
    trace( ("\r\n"));

    trace( ("\r\nRx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n"));
    trace( ("       0x"));
    trace("%02x",enc28j60_read(ERXSTH));
    trace("%02x",enc28j60_read(ERXSTL));
    trace( (" 0x"));
    trace("%02x",enc28j60_read(ERXNDH));
    trace("%02x",enc28j60_read(ERXNDL));
    trace( ("  0x"));
    trace("%02x",enc28j60_read(ERXWRPTH));
    trace("%02x",enc28j60_read(ERXWRPTL));
    trace( ("  0x"));
    trace("%02x",enc28j60_read(ERXRDPTH));
    trace("%02x",enc28j60_read(ERXRDPTL));
    trace( ("   0x"));
    trace("%02x",enc28j60_read(ERXFCON));
    trace( ("    0x"));
    trace("%02x",enc28j60_read(EPKTCNT));
    trace( ("  0x"));
    trace("%02x",enc28j60_read(MAMXFLH));
    trace("%02x",enc28j60_read(MAMXFLL));
    trace( ("\r\n"));

    trace( ("\r\nTx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n"));
    trace( ("       0x"));
    trace("%02x",enc28j60_read(ETXSTH));
    trace("%02x",enc28j60_read(ETXSTL));
    trace( (" 0x"));
    trace("%02x",enc28j60_read(ETXNDH));
    trace("%02x",enc28j60_read(ETXNDL));
    trace( ("   0x"));
    trace("%02x",enc28j60_read(MACLCON1));
    trace( ("     0x"));
    trace("%02x",enc28j60_read(MACLCON2));
    trace( ("     0x"));
    trace("%02x",enc28j60_read(MAPHSUP));
    trace( ("\r\n"));
    trace( ("\r\nPHY   : PHCON1  PHCON2  PHSTAT1 PHSTAT2\r\n"));
    trace( ("       0x"));
    trace("%02x",enc28j60_read(PHCON1));//ist 16 bit breit nicht 8 !
    trace( ("     0x"));
    trace("%02x",enc28j60_read(PHCON2));//ist 16 bit breit nicht 8 !
    trace( ("     0x"));
    trace("%02x",enc28j60_read(PHSTAT1));//ist 16 bit breit nicht 8 !
    trace( ("     0x"));
    trace("%02x",enc28j60_read(PHSTAT2));//ist 16 bit breit nicht 8 !
    trace( ("\r\n"));

}
#endif

/*
   void nicGetMacAddress(U08 *macaddr)
   {
// read MAC address registers
// NOTE: MAC address in ENC28J60 is byte-backward
 *macaddr++ = enc28j60_read(MAADR5);
 *macaddr++ = enc28j60_read(MAADR4);
 *macaddr++ = enc28j60_read(MAADR3);
 *macaddr++ = enc28j60_read(MAADR2);
 *macaddr++ = enc28j60_read(MAADR1);
 *macaddr++ = enc28j60_read(MAADR0);
 }

 void nicSetMacAddress(U8 *macaddr)
 {
// write MAC address
// NOTE: MAC address in ENC28J60 is byte-backward
enc28j60_write(MAADR5, *macaddr++);
enc28j60_write(MAADR4, *macaddr++);
enc28j60_write(MAADR3, *macaddr++);
enc28j60_write(MAADR2, *macaddr++);
enc28j60_write(MAADR1, *macaddr++);
enc28j60_write(MAADR0, *macaddr++);
}

U08 enc28j60_read_op(U08 op, U08 address)
{

U08 data;
// release CS
clrb_CS_ETHERNET();
//   	pause(ONE_US*100);

SPI0_SPDR = op | (address & ADDR_MASK);
while(!(SPI0_SPSR & SPI0_SPIF)){};
SPI0_SPDR = 0x00;
while(!(SPI0_SPSR & SPI0_SPIF)){};
// do dummy read if needed
if(address & 0x80)
{
SPI0_SPDR = 0x00;
while(!(SPI0_SPSR & SPI0_SPIF)){};
}
data = SPI0_SPDR; 

setb_CS_ETHERNET(); //CS auf High
return data;
}

void enc28j60_write_op(U08 op, U08 address, U08 data)
{
// release CS
clrb_CS_ETHERNET();
//   	pause(ONE_US*100);
// issue write command
SPI0_SPDR = op | (address & ADDR_MASK);
while(!(SPI0_SPSR & SPI0_SPIF)){};
// write data
SPI0_SPDR = data;
while(!(SPI0_SPSR & SPI0_SPIF)){};
// release CS
setb_CS_ETHERNET(); //CS auf High
}

void enc28j60_readBuffer(U16 len, U8 *data)
{
// assert CS
clrb_CS_ETHERNET();
//   	pause(ONE_US*100);
// issue read command
SPI0_SPDR = ENC28J60_READ_BUF_MEM;
while(!(SPI0_SPSR & SPI0_SPIF)){};
while(len--)
{
    // read data
    SPI0_SPDR = 0x00;
    while(!(SPI0_SPSR & SPI0_SPIF)){};
    *data++ = SPI0_SPDR;
}	
// release CS
setb_CS_ETHERNET();
}

void enc28j60_write_buffer(U16 len, U08* data)
{
    // assert CS
    clrb_CS_ETHERNET(); //CS auf Low
    //   	pause(ONE_US*100);
    // issue write command
    SPI0_SPDR = ENC28J60_WRITE_BUF_MEM;
    while(!(SPI0_SPSR & SPI0_SPIF)){};
    while(len--)
    {
	// write data
	SPI0_SPDR = *data++;
	while(!(SPI0_SPSR & SPI0_SPIF)){};
    }	
    // release CS
    setb_CS_ETHERNET();
}

void enc28j60_set_bank(U08 address)
{
    // set the bank (if needed)
    if((address & BANK_MASK) != Enc28j60Bank)
    {
	// set the bank
	enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
	enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
	Enc28j60Bank = (address & BANK_MASK);
    }
}

U08 enc28j60_read(U08 address)
{
    // set the bank
    enc28j60_set_bank(address);
    // do the read
    return enc28j60_read_op(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60_write(U08 address, U08 data)
{
    // set the bank
    enc28j60_set_bank(address);
    // do the write
    enc28j60_write_op(ENC28J60_WRITE_CTRL_REG, address, data);
}

void enc28j60PhyWrite(U08 address, U16 data)
{
    // set the PHY register address
    enc28j60_write(MIREGADR, address);

    // write the PHY data
    enc28j60_write(MIWRL, data);	
    enc28j60_write(MIWRH, data>>8);

    // wait until the PHY write completes
    while(enc28j60_read(MISTAT) & MISTAT_BUSY);
}
void setb_CS_ETHERNET(void){
    F_IO0SET = CS_ETHERNET;
}
void clrb_CS_ETHERNET(void){
    F_IO0CLR = CS_ETHERNET;
}
void spi_init(void){
    // SPI init
    PINSEL0 |= SPI0_IOSET_MASK;
    SPI0_SPCR = SPI0_MSTR; //Enable SPI, SPI in Master Mode	
    SPI0_SPCCR = 8;        //This register controls the frequency of a master?s SCK.
    // 8=7,5Mhz, 10=6Mhz, 12=5Mhz, must>8 !
}

void enc28j60Init(void)
{
    U32 a;
    U32 timeout;
    // interrupt und wol enable an LPC
    PINSEL0 |= ((U32)(0x02<<28)|(0x02<<30));
    spi_init();

    //    do{
    timeout = 0;
    // perform system reset
    enc28j60_write_op(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    // check CLKRDY bit to see if reset is complete
    // Errata workaround #2, CLKRDY check is unreliable, delay 1 mS instead
    pause(ONE_SEC*5);
    //        for(a=0;a<1000000;a++);
    while(!(enc28j60_read(ESTAT) & ESTAT_CLKRDY)){
	timeout++;
	if (timeout > 100000)
	    //                TRACE("enc28j60_read timeout");
	    break;
    };
    //        }while(timeout > 100000);
    // do bank 0 stuff
    // initialize receive buffer
    // 16-bit transfers, must write low byte first
    // set receive buffer start address
    NextPacketPtr = RXSTART_INIT;
    enc28j60_write(ERXSTL, RXSTART_INIT&0xFF);
    enc28j60_write(ERXSTH, RXSTART_INIT>>8);
    // set receive pointer address
    enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
    enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
    // set receive buffer end
    // ERXND defaults to 0x1FFF (end of ram)
    enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
    enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
    // set transmit buffer start
    // ETXST defaults to 0x0000 (beginnging of ram)
    enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
    enc28j60_write(ETXSTH, TXSTART_INIT>>8);

    // do bank 2 stuff
    // enable MAC receive
    enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
    // bring MAC out of reset
    enc28j60_write(MACON2, 0x00);
    // enable automatic padding and CRC operations
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);

    // set inter-frame gap (non-back-to-back)
    enc28j60_write(MAIPGL, 0x12);
    enc28j60_write(MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    enc28j60_write(MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    enc28j60_write(MAMXFLL, MAX_FRAMELEN&0xFF);	
    enc28j60_write(MAMXFLH, MAX_FRAMELEN>>8);

    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    // set Mac Addr.

    volatile unsigned char mymac[6] = {0x00,0x04,0x0e,0xf8,0xb7,0xf6};
    //        U8 mymac[6] = {0x00,0x04,0x0e,0xf8,0xb7,0xf6};
    nicSetMacAddress(mymac);

    // no loopback of transmitted frames
    enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
    // enable interrutps
    // EIE: ETHERNET INTERRUPT ENABLE REGISTER
    // PKTIE: Receive Packet Pending Interrupt Enable bit
    // INTIE: Global INT Interrupt Enable bit
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
    enc28j60_write(ERXFCON,(CRCEN|ANDOR));//Bank1 
    enc28j60PhyWrite(PHLCON, 0x472);//Display link status and transmit/receive activity (always stretched)
    ctl_set_isr(SIR_EINT2, PRIO_INT_ENC28J60, CTL_ISR_TRIGGER_NEGATIVE_EDGE, eint2ISR, 0);// eint2ISR registrieren
    ctl_unmask_isr(SIR_EINT2);
}
//
// * enc28j60 Powersave Mode
// * mode 1 = powerdown
// * mode 0 = wakeup
// * usage:
// * bool result enc28j60_powersave(1);
//
bool enc28j60_powersave(U8 mode){
    bool result = false;
    switch(mode){
	case 1:
	    enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
	    U32 timeout = 0;
	    while(!(enc28j60_read(ESTAT) & ESTAT_RXBUSY)){//Wait for any in-progress packets to finish being received by polling
		timeout++;
		if (timeout > 100000)
		    break;
	    };
	    timeout = 0;
	    while(!(enc28j60_read(ECON1) & ECON1_TXRTS)){//Wait for any current transmissions to end by confirming
		timeout++;
		if (timeout > 100000)
		    break;
	    };
	    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_VRPS);
	    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PWRSV);//Enter Sleep
	    result = true;
	    break;
	case 0:
	    enc28j60_write_op(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);//Enter Sleep
	    timeout = 0;
	    while(!(enc28j60_read(ESTAT) & ESTAT_CLKRDY)){//Wait for PHY to stabilize (wait 300us)
		timeout++;
		if (timeout > 100000)
		    break;
	    };
	    enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	    //set maybee 12.1.5 Link Change Interrupt Flag (LINKIF)
	    result = true;
	    break;
    }
    return result;
}

static void eint2ISR(void){
    ctl_global_interrupts_re_enable_from_isr();

    uip_len = dev_poll();	// look for a packet
    if(uip_len != 0){
	//besser?                        if(ARPBUF->ethhdr.type == HTONS(UIP_ETHTYPE_IP)) {
	if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	    //                               piep_ok();
	    //                                uip_arp_ipin();
	    uip_input();
	    // If the above function invocation resulted in data that
	    // should be sent out on the network, the global variable
	    // uip_len is set to a value > 0. 
	    if(uip_len > 0) {
		uip_arp_out();
		dev_send();
	    }
	    //besser?                        }else if(ARPBUF->ethhdr.type == HTONS(UIP_ETHTYPE_ARP)) {// process an ARP packet
    }else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {// process an ARP packet
	piep_ok();
	uip_arp_arpin();
	// If the above function invocation resulted in data that
	// should be sent out on the network, the global variable
	// uip_len is set to a value > 0.
	if(uip_len > 0) {
	    dev_send();
	}
    }
    }
    ctl_global_interrupts_un_re_enable_from_isr();
    EXTINT |= (1<<2); //clear interrupt
    }

    void enc28j60PacketSend(U16 len, U8 *packet)
    {
	// Set the write pointer to start of transmit buffer area
	enc28j60_write(EWRPTL, TXSTART_INIT);
	enc28j60_write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60_write(ETXNDL, (TXSTART_INIT+len));
	enc28j60_write(ETXNDH, (TXSTART_INIT+len)>>8);

	// write per-packet control byte
	enc28j60_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	// copy the packet into the transmit buffer
	enc28j60_write_buffer(len, packet);

	// send the contents of the transmit buffer onto the network
	enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    }
    unsigned int enc28j60PacketReceive(U16 maxlen, U8 *packet)
    {
	U16 rxstat;
	U16 len;
	U32 timeout = 0;

	// Set the read pointer to the start of the received packet
	enc28j60_write(ERDPTL, (NextPacketPtr));
	enc28j60_write(ERDPTH, (NextPacketPtr)>>8);
	// read the next packet pointer
	NextPacketPtr  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the packet length
	len  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the receive status
	rxstat  = enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;

	// limit retrieve length
	// (we reduce the MAC-reported length by 4 to remove the CRC)
	len = IFMIN(len, maxlen);

	// copy the packet from the receive buffer
	enc28j60_readBuffer(len, packet);

	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	//	enc28j60_write(ERXRDPTL, (NextPacketPtr));
	//	enc28j60_write(ERXRDPTH, (NextPacketPtr)>>8);

	// Errata workaround #13. Make sure ERXRDPT is odd
	//

	uint16_t rs,re;
	rs = enc28j60_read(ERXSTH);
	rs <<= 8;
	rs |= enc28j60_read(ERXSTL);
	re = enc28j60_read(ERXNDH);
	re <<= 8;
	re |= enc28j60_read(ERXNDL);
	if (NextPacketPtr - 1 < rs || NextPacketPtr - 1 > re)
	{
	    enc28j60_write(ERXRDPTL, (re));
	    enc28j60_write(ERXRDPTH, (re)>>8);
	}
	else
	{
	    enc28j60_write(ERXRDPTL, (NextPacketPtr-1));
	    enc28j60_write(ERXRDPTH, (NextPacketPtr-1)>>8);
	}

	// decrement the packet counter indicate we are done with this packet
	// clear the PKTIF: Receive Packet Pending Interrupt Flag bit
	enc28j60_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

	return len;
    }

    void enc28j60RegDump(void){

	TRACE("RevID: 0x%x\n", enc28j60_read(EREVID));

	TRACE ( ("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\n"));
	TRACE ( ("         "));
	TRACE("%02x",enc28j60_read(ECON1));
	TRACE( ("    "));
	TRACE("%02x",enc28j60_read(ECON2));
	TRACE( ("    "));
	TRACE("%02x",enc28j60_read(ESTAT));
	TRACE( ("    "));
	TRACE("%02x",enc28j60_read(EIR));
	TRACE( ("   "));
	TRACE("%02x",enc28j60_read(EIE));
	TRACE( ("\n"));

	TRACE( ("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\n"));
	TRACE( ("        0x"));
	TRACE("%02x",enc28j60_read(MACON1));
	TRACE( ("    0x"));
	TRACE("%02x",enc28j60_read(MACON2));
	TRACE( ("    0x"));
	TRACE("%02x",enc28j60_read(MACON3));
	TRACE( ("    0x"));
	TRACE("%02x",enc28j60_read(MACON4));
	TRACE( ("   "));
	TRACE("%02x",enc28j60_read(MAADR5));
	TRACE("%02x",enc28j60_read(MAADR4));
	TRACE("%02x",enc28j60_read(MAADR3));
	TRACE("%02x",enc28j60_read(MAADR2));
	TRACE("%02x",enc28j60_read(MAADR1));
	TRACE("%02x",enc28j60_read(MAADR0));
	TRACE( ("\n"));

	TRACE( ("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\n"));
	TRACE( ("       0x"));
	TRACE("%02x",enc28j60_read(ERXSTH));
	TRACE("%02x",enc28j60_read(ERXSTL));
	TRACE( (" 0x"));
	TRACE("%02x",enc28j60_read(ERXNDH));
	TRACE("%02x",enc28j60_read(ERXNDL));
	TRACE( ("  0x"));
	TRACE("%02x",enc28j60_read(ERXWRPTH));
	TRACE("%02x",enc28j60_read(ERXWRPTL));
	TRACE( ("  0x"));
	TRACE("%02x",enc28j60_read(ERXRDPTH));
	TRACE("%02x",enc28j60_read(ERXRDPTL));
	TRACE( ("   0x"));
	TRACE("%02x",enc28j60_read(ERXFCON));
	TRACE( ("    0x"));
	TRACE("%02x",enc28j60_read(EPKTCNT));
	TRACE( ("  0x"));
	TRACE("%02x",enc28j60_read(MAMXFLH));
	TRACE("%02x",enc28j60_read(MAMXFLL));
	TRACE( ("\n"));

	TRACE( ("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\n"));
	TRACE( ("       0x"));
	TRACE("%02x",enc28j60_read(ETXSTH));
	TRACE("%02x",enc28j60_read(ETXSTL));
	TRACE( (" 0x"));
	TRACE("%02x",enc28j60_read(ETXNDH));
	TRACE("%02x",enc28j60_read(ETXNDL));
	TRACE( ("   0x"));
	TRACE("%02x",enc28j60_read(MACLCON1));
	TRACE( ("     0x"));
	TRACE("%02x",enc28j60_read(MACLCON2));
	TRACE( ("     0x"));
	TRACE("%02x",enc28j60_read(MAPHSUP));
	TRACE( ("\n"));
	TRACE( ("PHY   : PHCON1  PHCON2  PHSTAT1 PHSTAT2\n"));
	TRACE( ("       0x"));
	TRACE("%02x",enc28j60_read(PHCON1));//ist 16 bit breit nicht 8 !
	TRACE( ("     0x"));
	TRACE("%02x",enc28j60_read(PHCON2));//ist 16 bit breit nicht 8 !
	TRACE( ("     0x"));
	TRACE("%02x",enc28j60_read(PHSTAT1));//ist 16 bit breit nicht 8 !
	TRACE( ("     0x"));
	TRACE("%02x",enc28j60_read(PHSTAT2));//ist 16 bit breit nicht 8 !
	TRACE( ("\n"));

    }
    */


