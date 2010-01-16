#include <stdio.h>

#include "lpc214x.h"
#include "type.h"
#include "uart0.h"

#include "network.h"

// TODO Remove this after finished exprimenting & debugging
#include "enc28j60.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
#include "hello-world.h"

void uip_log(char *m)
{
  printf("uIP log message: %s\n", m);
}

int main(void)
{
    unsigned int i;
    uip_eth_addr macaddr = { 
	.addr = { 0x00, 0xf8, 0xc1, 0xd8, 0xc7, 0xa6} 
    };

    uip_ipaddr_t ipaddr;
    struct timer periodic_timer, arp_timer;

    timer_set(&periodic_timer, CLOCK_SECOND / 2);
    timer_set(&arp_timer, CLOCK_SECOND * 10);
 
    VPBDIV = 0x02;

    uart0Init();
    printf("Uart Init\n");

    network_init();
    printf("network init\n");
 
    uip_init();
    printf("uIP init\n");

    uip_setethaddr(macaddr);
    printf("Setting MAC address '%x:%x:%x:%x:%x:%x' with uIP\n", 
	    macaddr.addr[0], macaddr.addr[1], macaddr.addr[2], macaddr.addr[3], macaddr.addr[4],macaddr.addr[5]);

    for(i = 0; i < 6; i++) {
	macaddr.addr[i] = 0;
    }

    enc28j60_get_mac_address((uint8_t *)&(macaddr.addr));
    printf("Testing using driver: MAC == %x:%x:%x:%x:%x:%x", 
	    macaddr.addr[0], macaddr.addr[1], macaddr.addr[2], macaddr.addr[3], macaddr.addr[4],macaddr.addr[5]);

    uip_ipaddr(ipaddr, 12,12,12,13);
    uip_sethostaddr(ipaddr);
    printf("Set host IP addr to: %d.%d.%d.%d", 12,12,12,13);

    uip_ipaddr(ipaddr, 12,12,12,12);
    uip_setdraddr(ipaddr);
    printf("Set my ip addr to: %d.%d.%d.%d", 12,12,12,12);

    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);

    hello_world_init();
    while(1);

    return 0;
}
