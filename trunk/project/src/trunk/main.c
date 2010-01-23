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

// Applications
#include "hello-world.h"
//#include "simple.h"

#define ETH_BUF ((struct uip_eth_hdr *)&uip_buf[0])
/*  
struct test_t {
    uint8_t x;
};

struct test_packed_t {
    struct test_t t;
    uint8_t x;
} __attribute__((packed));
*/
void uip_log(char *m)
{
    printf("uIP log message: %s\n", m);
}

void printHex(uint8_t *buf, unsigned int len);

void printHex(uint8_t *buf, unsigned int len) 
{
    unsigned int i;
    for(i = 0; i < len; i++) {
	if((i % 8) == 0 && i != 0) {
	    printf("|");
	}
	if((i % 32) == 0 && i != 0) {
	    printf("\n");
	}

	printf(" %.2x", buf[i]);
    }
}

int main(void)
{
    unsigned int i;
    struct uip_eth_addr macaddr = { 
	.addr = { 0x00, 0xf8, 0xc1, 0xd8, 0xc7, 0xa6} 
    };

    uip_ipaddr_t ipaddr;
    struct timer periodic_timer, arp_timer;

    timer_set(&periodic_timer, CLOCK_SECOND / 2);
    timer_set(&arp_timer, CLOCK_SECOND * 10);

    VPBDIV = 0x02;

    uart0Init();
    printf("Uart Init\n");
/*  
    struct test_t t1;
    struct test_packed_t t2;
    printf("sizeof(struct test_t) = %d\n", sizeof(t1));
    printf("sizeof(struct test_packed_t) = %d\n", sizeof(t2));
*/
    network_init();
    printf("network init\n");

    uip_init();
    printf("uIP init\n");

    uip_setethaddr(macaddr);
    enc28j60_set_mac_address((uint8_t *)&(macaddr.addr));
    printf("Setting MAC address '%x:%x:%x:%x:%x:%x'\n", 
	    macaddr.addr[0], macaddr.addr[1], macaddr.addr[2], macaddr.addr[3], macaddr.addr[4],macaddr.addr[5]);

    uip_ipaddr(ipaddr, 12,12,12,13);
    uip_sethostaddr(ipaddr);
    printf("Set MY IP addr to: %d.%d.%d.%d\n", 12,12,12,13);

    uip_ipaddr(ipaddr, 12,12,12,12);
    uip_setdraddr(ipaddr);
    printf("Set default router addr to: %d.%d.%d.%d\n", 12,12,12,12);

    //uip_ipaddr(ipaddr, 255,255,255,0);
    //uip_setnetmask(ipaddr);
    //fflush(stdout);

    hello_world_init();
    //simple_init();

    //printf("sizeof(struct uip_eth_hdr): %x", sizeof(struct uip_eth_hdr));
    while(1) {
	uip_len = network_read(uip_buf);
	if(uip_len > 0) 
	{
	    printf("Got packet \n");

	    //printf("uip_len: %d\n", uip_len);

	    //printf("type: %.2x\n", ETH_BUF->type);

	    //printf("type: %.2x", uip_buf[12]);
	    //printf("%.2x\n", uip_buf[13]);

	    //printf("dest: %.2x\n", &(ETH_BUF->dest));
	    //printf("src: %.2x\n", &(ETH_BUF->src));
	    //printf("type: %.2x\n", &(ETH_BUF->type));
 
	    //printf("sizeof(uip_eth_addr): %.2x\n", sizeof(struct uip_eth_addr));

	    //printHex(uip_buf, uip_len);
	    //printf("\n");

	    if(ETH_BUF->type == htons(UIP_ETHTYPE_IP)) 
	    {
		printf("Type: IP\n");
		uip_arp_ipin();
		uip_input();
		/* If the above function invocation resulted in data that
		   should be sent out on the network, the global variable
		   uip_len is set to a value > 0. */
		if(uip_len > 0) {
		    printf("Sending response (uip_len: %d)\n", uip_len);
		    printHex(uip_buf, uip_len); printf("\n");

		    uip_arp_out();
		    network_send(uip_buf, uip_len);
		}
	    } 
	    else if(ETH_BUF->type == htons(UIP_ETHTYPE_ARP)) 
	    {
		printf("Type: ARP\n");
		uip_arp_arpin();
		/* If the above function invocation resulted in data that
		   should be sent out on the network, the global variable
		   uip_len is set to a value > 0. */
		if(uip_len > 0) {
		    network_send(uip_buf, uip_len);
		}
	    }
	} 
	else if(timer_expired(&periodic_timer)) 
	{
	    //printf("Periodic timer\n");
	    timer_reset(&periodic_timer);
	    for(i = 0; i < UIP_CONNS; i++) 
	    {
		uip_periodic(i);
		/* If the above function invocation resulted in data that
		   should be sent out on the network, the global variable
		   uip_len is set to a value > 0. */
		if(uip_len > 0) 
		{
		    uip_arp_out();
		    network_send(uip_buf, uip_len);
		}
	    }

#if UIP_UDP
	    for(i = 0; i < UIP_UDP_CONNS; i++) 
	    {
		uip_udp_periodic(i);
		/* If the above function invocation resulted in data that
		   should be sent out on the network, the global variable
		   uip_len is set to a value > 0. */
		if(uip_len > 0) 
		{
		    uip_arp_out();
		    network_send(uip_buf, uip_len);
		}
	    }
#endif /* UIP_UDP */

	    /* Call the ARP timer function every 10 seconds. */
	    if(timer_expired(&arp_timer)) 
	    {
		//printf("ARP timer\n");
		timer_reset(&arp_timer);
		uip_arp_timer();
	    }
	}
    } // while(1)

    return 0;
}
