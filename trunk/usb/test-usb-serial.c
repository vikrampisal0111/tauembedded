/*
	LPCUSB, an USB device driver for LPC microcontrollers	
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
	Minimal implementation of a USB serial port, using the CDC class.
	This example application simply echoes everything it receives right back
	to the host.

	Windows:
	Extract the usbser.sys file from .cab file in C:\WINDOWS\Driver Cache\i386
	and store it somewhere (C:\temp is a good place) along with the usbser.inf
	file. Then plug in the LPC214x and direct windows to the usbser driver.
	Windows then creates an extra COMx port that you can open in a terminal
	program, like hyperterminal.

	Linux:
	The device should be recognised automatically by the cdc_acm driver,
	which creates a /dev/ttyACMx device file that acts just like a regular
	serial port.
  
  Modified by Sivan Toledo and Wouter van Ooijen

*/
#include <stdint.h>
#include <string.h> /* for memcpy */

#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

//#include "ctc.h"
#include "vic.c"
//#include "busywait.h"

#define CLOCKS_PCLK 60000000

#define UART0_BAUD_RATE 19200
#include "uart0-polling.c"

#define  print_char(x) uart0SendByte(x)
#include "print.c"

#define ASSERT(x)
#define DBG(x) print(x)
#define DBGNUM(x) printNum(x)
#define DBGHEX(x,n) printHex(x,n)

typedef int bool;

#include "usb.h"

//#define INT_IN_EP		  0x81
//#define BULK_OUT_EP   0x02
//#define BULK_IN_EP    0x82

#define INT_EP  1
#define BULK_EP 2

#define MAX_PACKET_SIZE	64

#define CS_INTERFACE			      0x24
//#define CS_ENDPOINT				      0x25

static const uint8_t usb_descriptors[] = {
  0x12,                /* length                      */
  DESC_DEVICE,
  LE_WORD(0x0110),     /* USB version (BCD)           */
  0x02,                /* Class = CDC (Communication) */
  0x00,                /* Device Sub Class            */
  0x00,                /* Device Protocol             */
  MAX_PACKET_SIZE0,    /* max packet size for EP 0    */
  LE_WORD(0x16C0),     /* Vendor: Wouter van Ooijen   */
  LE_WORD(  1001),     /* Product: lab-use only #1    */
  LE_WORD(0x0100),     /* Device release number (BCD) */
  1,                   /* Manufacturer String Index   */
  2,                   /* Product String Index        */
  3,                   /* SerialNumber String Index   */
  1,                   /* Number of Configurations    */

  0x09,
  DESC_CONFIGURATION,
  LE_WORD(67),         /* total lendth                */
  0x02,                /* number of interfaces        */
  0x01,                /* This Configuration          */
  0,                   /* Configuration String Index  */ 
  USB_ATTR_BUS_POWERED,/* Attributes                  */
  50,                  /* Max Power (unit is 2mA)     */

  0x09,
  DESC_INTERFACE,
  0,                   /* Interface Number            */
  0,                   /* Alternate Setting Number    */
  1,                   /* Number of endpoints         */
  0x02,                /* Class = Communication       */
  0x02,                /* Subclass = Abs. Ctl. Manag. */
  0x01,                /* Protocol code = V.25ter     */
  0,                   /* Interface string index      */

  0x05,
  CS_INTERFACE,
  0x00,                /* Subtype = Header Descriptor */
  LE_WORD(0x0110),     /* CDC Release Number          */
  
  0x05,
  CS_INTERFACE,
  0x01,                /* Subtype = Call Management   */
  0x01,                /* Capabilities = device 
                              handles call management */
  1,                   /* bDataInterface              */

  0x04,
  CS_INTERFACE,
  0x02,                /* Subtype = Abs. Ctl. Manage. */
  0x02,                /* Capabilities: 
                          SET/GET LINE CODING,
                          SET CONTROL LINE,
                          SERIAL STATE notifications  */

  0x05,
  CS_INTERFACE,
  0x06,                /* Subtype = Union             */
  0x00,                /* MasterInterface             */
  0x01,                /* Slave Interface             */

  0x07,
  DESC_ENDPOINT,
  USB_EP_IN | INT_EP, /* Endpoint Address             */
  0x03,               /* Attributes = Interrupt       */
  LE_WORD(8),         /* Max Packet Size              */
  10,                 /* Interval                     */
  
  0x09,
  DESC_INTERFACE,     /* This is the Data Interface   */
  1,                  /* Interface Number             */
  0,                  /* Alternate Setting            */
  2,                  /* NumEndPoints                 */
  0x0A,               /* Interface Class = Data       */
  0x00,               /* Interface Subclass           */
  0x00,               /* Interface Protocol           */
  0x00,               /* Interface String Index       */

  0x07,
  DESC_ENDPOINT,
  USB_EP_OUT | BULK_EP, /* Endpoint Address           */
  0x02,               /* Attributes = bulk            */
  LE_WORD(MAX_PACKET_SIZE),
  0x00,               /* Interval (not used for bulk) */

  0x07,
  DESC_ENDPOINT,
  USB_EP_IN | BULK_EP,/* Endpoint Address             */
  0x02,               /* Attributes = bulk            */
  LE_WORD(MAX_PACKET_SIZE),
  0x00,               /* Interval (not used for bulk) */

  0x04,
  DESC_STRING,
  LE_WORD(0x0409),    /* Language = US English        */

  0x0E,
  DESC_STRING,
  'L', 0, 'P', 0, 'C', 0, 'U', 0, 'S', 0, 'B', 0,

  0x14,
  DESC_STRING,
  'U', 0, 'S', 0, 'B', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0,

  0x12,
  DESC_STRING,
  'D', 0, 'E', 0, 'A', 0, 'D', 0, 'C', 0, '0', 0, 'D', 0, 'E', 0,

  0 /* terminator */
};

#include "fifo.c"

static uint8_t _vcom_buffer[64];

#define VCOM_FIFO_SIZE  128

static uint8_t _vcom_txdata[VCOM_FIFO_SIZE];
static uint8_t _vcom_rxdata[VCOM_FIFO_SIZE];

static fifo_t txfifo;
static fifo_t rxfifo;

void vcomInit(void) {
  fifoInit(&txfifo, _vcom_txdata, VCOM_FIFO_SIZE);
  fifoInit(&rxfifo, _vcom_rxdata, VCOM_FIFO_SIZE);
}

static void vcomBulkOut(uint8_t EP, uint8_t EPStatus) {
	int i, n;

	if (fifoFree(&rxfifo) < MAX_PACKET_SIZE)
		return; /* may not fit, we drop the data */

  n = usbRead(EP, _vcom_buffer, sizeof(_vcom_buffer));
	for (i = 0; i < n; i++)
		fifoPut(&rxfifo, _vcom_buffer[i]);
}

static void vcomBulkIn(uint8_t EP, uint8_t EPStatus) {
	int i;

	if (fifoAvailable(&txfifo) == 0) {
		usbEnableNAKInterrupts(0);
		return;
	}

	for (i = 0; i < MAX_PACKET_SIZE; i++)
		if (!fifoGet(&txfifo, &_vcom_buffer[i])) break;

	if (i > 0) usbWrite(EP, _vcom_buffer, i);
}

void vcomPutchar(int c) {
  vicDisable(INT_CHANNEL_USB);
  fifoPut(&txfifo, c);
  usbEnableNAKInterrupts(INACK_BI);
  vicEnable(INT_CHANNEL_USB);
}

int vcomGetchar(void)
{
  uint8_t c;
  bool    eof;

  vicDisable(INT_CHANNEL_USB);
  eof = fifoGet(&rxfifo, &c);
  vicEnable(INT_CHANNEL_USB);  
  
  return eof ? c : -1;
}

/* ordered OUT then IN for every EP in use */
static usb_ep_handler_t* const usb_ep_handlers[] = {
   usbEP0OutHandler, usbEP0InHandler, /* EP  0 Out, In */
   0,                0,               /* EP  1 Out, In */
   vcomBulkOut,      vcomBulkIn,      /* EP  2 Out, In */
};


#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

// data structure for GET__vcom_line_coding / SET__vcom_line_coding class requests
typedef struct {
  uint32_t  bit_rate;
  uint8_t   stop_bits;
  uint8_t   parity;
  uint8_t   data_bits;
} vcom_line_coding_t;

static vcom_line_coding_t _vcom_line_coding = {115200, 0, 0, 8};

static bool usb_control_class_handler(void) {
  switch (usbRequest.request) {

  case SET_LINE_CODING:
    print("SET_LINE_CODING\n");
    memcpy((uint8_t *)&_vcom_line_coding, usbControlTransferPtr, 7);
    usbControlTransferLen = 7;

    print("bit_rate = ");
    printNum(_vcom_line_coding.bit_rate);
    print("\n");
    break;

  case GET_LINE_CODING:
    DBG("GET_LINE_CODING\n");

    usbControlTransferPtr = (uint8_t*) &_vcom_line_coding;
    usbControlTransferLen = 7;
    break;
    
  case SET_CONTROL_LINE_STATE:
    // bit0 = DTR, bit = RTS
    //DBG("SET_CONTROL_LINE_STATE %X\n", pSetup->wValue);
    print("SET_CONTROL_LINE_STATE ");
    printHex(usbRequest.data,8);
    print("\n");
    break;

  default:
    return FALSE;
  }
  return TRUE;
}

static void usb_device_status_handler(uint8_t dev_status) {
  if (dev_status & DEV_STATUS_RESET) {
    vcomInit();
  }
}

#define usb_SOF_handler()

#include "usb.c"


#include "lcd.c"


int main(void) {
  int c;

  char buff[100];
  int offset = 0;
  int started = 0;
	
  //ctc_clocksInit(12000000,60000000,1);
  //ctc_mamInit(MAM_FULLY_ENABLED);
  //ramvectorsInit();
  //busywaitInit();
  
  VPBDIV = 0x00000001;  /* PCLK = CCLK */
  PLLCFG  = 0x00000024; /* Fosc=12MHz, CCLK=60MHz */
  PLLCON  = 0x00000001; /* enable the PLL  */
  PLLFEED = 0x000000AA; /* feed sequence   */
  PLLFEED = 0x00000055;
  while (!(PLLSTAT & 0x00000400));
  PLLCON = 3;     // enable and connect
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  
  vicInit();
  uart0Init();
  
  DBG("Initializing USB stack\n");

  usbInit();

  vcomInit();
  
  DBG("Starting USB communication\n");

  interruptsEnable();
  
  usbConnect();

  lcdInit();

  // echo any character received (do USB stuff in interrupt)
  while (1) {
	  c = vcomGetchar(); 
	  if (c != -1) {
		  // show on console
		  if ((c == 9) || (c == 10) || (c == 13) || ((c >= 32) && (c <= 126))) {
			  uint8_t buf[2]; 
			  buf[0] = c; buf[1] = 0;
			  DBG((char*) buf);
		  } else {
			  DBG(".");
		  }
		  vcomPutchar(c);

		  if (c == 13) {
			  DBG("Cmd entered.\n");
			  buff[offset] = '\0';
			  offset = 0;
			  if (!strcmp(buff, "start"))
			  {
				started = 1;
   			  }
			  else if (started) {
				  if (!strcmp(buff, "bl-on"))
				  {
					  lcd_backlight_on();
				  }
				  else if (!strcmp(buff, "bl-off"))
				  {
					  lcd_backlight_off();
				  }
				  else if (!strcmp(buff, "clear"))
				  {
					  lcdClear();						  
				  }
				  else {
					  lcdPrintString(buff);
				  }
			  }
		  }
		  else {
			  buff[offset++] = c;
		  }
	  }
  }
}
