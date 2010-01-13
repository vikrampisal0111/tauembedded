#include <stdio.h>

#include "lpc214x.h"                        /* LPC21xx definitions */
#include "type.h"
//#include "irq.h"
//#include "ssp.h"
#include "uart0.h"

#include "network.h"
#include "enc28j60.h"

//BYTE SPIWRData[BUFSIZE];
//BYTE SPIRDData[BUFSIZE];
//DWORD CurrentTxIndex = 0;
//DWORD CurrentRxIndex = 0;

int main(void)
{
    unsigned int i;
    uint8_t macaddr[6] = { 0x00, 0xf8, 0xc1, 0xd8, 0xc7, 0xa6 };

    VPBDIV = 0x02;

//    init_VIC();
    uart0Init();
    printf("Uart Init\n");
    fflush(stdout);

    //return 0;

    network_init();
    printf("network init\n");

    printf("Started\n");
    fflush(stdout);

    enc28j60_set_mac_address(macaddr);
    printf("Set mac addr\n");
    for(i = 0; i < 6; i++) {
	macaddr[i] = 0;
    }
    enc28j60_get_mac_address((uint8_t *)&macaddr);
    printf("mac==%x:%x:%x:%x:%x:%x", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4],macaddr[5]);
    fflush(stdout);
    return 0;
}
