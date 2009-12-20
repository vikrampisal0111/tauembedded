/* Sivan: not yet ported & tested ! */

/*
 * USB Mouse Example.
 * 
 * Sivan Toledo and Wouter van Ooijen 
 */

#include <stdint.h>
#include <lpc2000/io.h>
#include <lpc2000/interrupt.h>

#define CLOCKS_PCLK 60000000

#include "vic.c"
//#include "busywait.c"

#define UART0_BAUD_RATE 19200
#include "uart0-polling.c"

#define  print_char(x) uart0SendByte(x)
#include "print.c"

#define EOF (-1)
#define ASSERT(x)
#define DBG(x) print(x)
#define DBGNUM(x) printNum(x)
#define DBGHEX(x,n) printHex(x,n)

typedef int bool;

#include "usb.h"

//#define GET_REPORT      0x01
//#define GET_IDLE        0x02
//#define GET_PROTOCOL    0x03
//#define SET_REPORT      0x09
//#define SET_IDLE        0x0A
//#define SET_PROTOCOL    0x0B

/* Class Descriptor Types */
#define USB_CLASS_HID               0x03

#define DESC_HID         0x21
#define DESC_REPORT      0x22
//#define DESC_PHY         0x23

#define HID_PROTOCOL_NONE    0x00
#define HID_PROTOCOL_KEYBOAD 0x01
#define HID_PROTOCOL_MOUSE   0x02

#define HID_SUBCLASS_NONE    0x00
#define HID_SUBCLASS_BOOT    0x01

#define HID_EP 1

/*** USB-related functions and data structures ***/

static const uint8_t hid_report_descriptor[] = {
  0x05, 0x01, /* Usage Page (Generic Desktop)             */
  0x09, 0x02, /* Usage (Mouse)                            */
  0xA1, 0x01, /* Collection (Application)                 */
  0x09, 0x01, /*  Usage (Pointer)                         */
  0xA1, 0x00, /*  Collection (Physical)                   */
  0x05, 0x09, /*      Usage Page (Buttons)                */
  0x19, 0x01, /*      Usage Minimum (01)                  */
  0x29, 0x03, /*      Usage Maximum (03)                  */
  0x15, 0x00, /*      Logical Minimum (0)                 */
  0x25, 0x01, /*      Logical Maximum (1)                 */
  0x95, 0x03, /*      Report Count (3)                    */
  0x75, 0x01, /*      Report Size (1)                     */
  0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
  0x95, 0x01, /*      Report Count (1)                    */
  0x75, 0x05, /*      Report Size (5)                     */
  0x81, 0x01, /*      Input (Constant)    ;5 bit padding  */
  0x05, 0x01, /*      Usage Page (Generic Desktop)        */
  0x09, 0x30, /*      Usage (X)                           */
  0x09, 0x31, /*      Usage (Y)                           */
  0x15, 0x81, /*      Logical Minimum (-127)              */
  0x25, 0x7F, /*      Logical Maximum (127)               */
  0x75, 0x08, /*      Report Size (8)                     */
  0x95, 0x02, /*      Report Count (2)                    */
  0x81, 0x06, /*      Input (Data, Variable, Relative)    */
  0xC0, 0xC0
};

static const uint8_t usb_descriptors[] = {
  0x12,                /* length                      */
  DESC_DEVICE,
  LE_WORD(0x0110),     /* USB version (BCD)           */
  0x00,                /* Class (in interface desc)   */
  0x00,                /* Device Sub Class            */
  0x00,                /* Device Protocol             */
  MAX_PACKET_SIZE0,    /* max packet size for EP 0    */
  LE_WORD(0x16C0),     /* Vendor: Wouter van Ooijen   */
  LE_WORD(  1000),     /* Product: lab-use only #0    */
  LE_WORD(0x0100),     /* Device release number (BCD) */
  1,                   /* Manufacturer String Index   */
  2,                   /* Product String Index        */
  0,                   /* SerialNumber String Index   */
  1,                   /* Number of Configurations    */

  0x09,                /* length                      */
  DESC_CONFIGURATION,
  LE_WORD(34),         /* Total Length                */
  1,                   /* Number of Interfaces        */
  1,                   /* This Configuration          */
  0,                   /* Configuration String Index  */
  USB_ATTR_BUS_POWERED,/* Attributes                  */
  50,                  /* Max Power (unit is 2mA)     */

  0x09,                /* length                      */
  DESC_INTERFACE,
  0,                   /* Interface Number            */
  0,                   /* Alternate Setting Number    */
  1,                   /* Number of endpoints         */
  USB_CLASS_HID,       /* Class code                  */
  HID_SUBCLASS_NONE,   /* Subclass code               */
  HID_PROTOCOL_MOUSE,  /* Protocol code               */
  0,                   /* Interface string index      */

  0x09,                /* length                      */
  DESC_HID,            /* Descriptor type             */
  LE_WORD(0x0111),     /* HID Spec, BCD format (1.11) */
  0x00,                /* Country Code (none)         */
  1,                   /* Number of class descriptors */
  DESC_REPORT,         /* Report descriptor type      */
  LE_WORD(sizeof(hid_report_descriptor)),
    
  0x07,                /* length                      */
  DESC_ENDPOINT,
  USB_EP_IN | HID_EP,  /* Endpoint Address            */
  USB_EP_ATTR_INTERRUPT, /* Attributes                */
  LE_WORD(3),          /* Max Packet Size             */
  10,                  /* Polling Interval (in ms)    */
  
  0x04,
  DESC_STRING,
  LE_WORD(0x0409),     /* Language: US English        */   

  12,
  DESC_STRING,
  'T', 0, 'l', 0, 'e', 0, 'd', 0, 'o', 0,

  18,
  DESC_STRING,
  'U', 0, 'S', 0, 'B', 0, 'M', 0, 'o', 0, 'u', 0, 's', 0, 'e', 0, 

  0                    /* terminator                  */
};

//static uint8_t report[3] = {0, 0, 0};

static struct {
  uint8_t buttons;
  int8_t  dx;
  int8_t  dy;
} mouse_report;

#define SWITCHES_MASK (BIT16 | BIT17 | BIT18 | BIT19 | BIT20)

//static int  _iFrame = 0;

static void usb_SOF_handler() {
  uint32_t state = FIO0PIN;
  
  if ((state & BIT18) && mouse_report.dx > -127) mouse_report.dx--;
  if ((state & BIT19) && mouse_report.dx <  127) mouse_report.dx++;
  if ((state & BIT20) && mouse_report.dy > -127) mouse_report.dy--;
  if ((state & BIT17) && mouse_report.dy <  127) mouse_report.dy++;
  
  if (state & BIT16) mouse_report.buttons |= BIT0;
  
  if (state & BIT14) mouse_report.buttons |= BIT1;
  
  //  usbEnableNAKInterrupts(INACK_II);
}

static void mouseInterruptIn(uint8_t EP, uint8_t EPStatus) {
  //int i, iLen;

  usbWrite(EP, (uint8_t*) &mouse_report, 3);
  mouse_report.buttons = 0;
  mouse_report.dx = 0;
  mouse_report.dy = 0;
}

/* ordered OUT then IN for every EP in use */
static usb_ep_handler_t* const usb_ep_handlers[] = {
   usbEP0OutHandler, usbEP0InHandler,  /* EP  0 Out, In */
   0,                mouseInterruptIn, /* EP  1 Out, In */
};

#define usb_control_standard_custom_handler() hidCustomRequestHandler()

static bool hidCustomRequestHandler(void) {
  uint8_t type, index;
  
  DBG("XXX custom handler\n");
  if ((usbRequest.type == 0x81)     // standard IN request for interface
      && (usbRequest.request == REQ_GET_DESCRIPTOR)) {
    
    type = GET_DESC_TYPE(usbRequest.data);
    index = GET_DESC_INDEX(usbRequest.index);
    switch (type) {

    case DESC_HID_REPORT:
      DBG("hid report!\n");
      // report
      usbControlTransferPtr = &hid_report_descriptor;
      usbControlTransferLen = sizeof(hid_report_descriptor);
      break;

    case DESC_HID_HID:
    case DESC_HID_PHYSICAL:
    default:
        DBG("sometime else...\n");
        // search descriptor space
        return usbGetDescriptor(usbRequest.data, 0);
    }

    
    return TRUE;
  }

  return FALSE;
}

#define usb_control_class_handler() FALSE
/* handler for CDC class requests */
//static int  _iIdleRate = 0;
//static uint8_t idle_reply;
static bool xxxusb_control_class_handler(void) {

  switch (usbRequest.request) {

#if 0
  case HID_GET_IDLE:
    DBG("GET IDLE, val,idx=");
    DBGNUM(usbRequest.data);
    DBG(",");
    DBGNUM(usbRequest.index);
    DBG("\n");
    idle_reply = (_iIdleRate / 4) & 0xFF;
    usbControlTransferPtr = &idle_reply;
    usbControlTransferLen = 1;
    break;

  case HID_SET_IDLE:
    DBG("SET IDLE, val,idx=");
    DBGNUM(usbRequest.data);
    DBG(",");
    DBGNUM(usbRequest.index);
    DBG("\n");
    _iIdleRate = ((usbRequest.data >> 8) & 0xFF) * 4;
    break;
#endif

  default:
    DBG("Unhandled class request ");
    DBGNUM(usbRequest.request);
    DBG("\n");
    return FALSE;
  }
  return TRUE;
}


static void usb_device_status_handler(uint8_t dev_status) {
  if (dev_status & DEV_STATUS_RESET) {
    DBG("device reset\n");
  }
}

#include "usb.c"

int main(void) {
  //int c;
  
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

  SCS      = BIT0 | BIT1;   /* use fast I/O registers    */
    
  //busywaitInit();
  uart0Init();
  
  DBG("Initialising USB stack\n");

  usbInit();
  usbEnableNAKInterrupts(INACK_II);
  
  DBG("Starting USB communication\n");

  FIO0DIR &= ~SWITCHES_MASK; /* all inputs */


  interruptsEnable();
  
  usbConnect();
  
  // echo any character received (do USB stuff in interrupt)
  while (1) {
  }
}
