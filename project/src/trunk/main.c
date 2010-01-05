#include <stdio.h>
#include "uart0.h"
#include "network.h"
#include "enc28j60.h"

void uip_log(char *m)
{
	printf("uIP log message: %s\n", m);
}


int main(void)
{
    unsigned int i;
    uint8_t macaddr[6] = { { 0x00, 0x04, 0x0e, 0xf8, 0xb7, 0xf6 } };
    
    VPBDIV = 0x02;
    uart0Init();
    printf("Uart Init\n");
    printf("network init\n");
    network_init();

    printf("Started\n");
    fflush(stdout);
    enc28j60_set_mac_address(macaddr);
    printf("Set mac addr\n");
    for(i = 0; i < 6; i++) {
	macaddr[i] = 0;
    }
    enc28j60_get_mac_address((uint8_t *)&macaddr);
    printf("mac==%x:%x:%x:%x:%x:%x", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4],macaddr[5]);

    return 0;
}
