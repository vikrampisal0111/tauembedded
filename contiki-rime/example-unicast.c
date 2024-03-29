/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: example-unicast.c,v 1.2 2009/03/12 21:58:21 adamdunkels Exp $
 */

/**
 * \file
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>


static process_event_t event;
/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
PROCESS(red_led_process, "Red led");
PROCESS(green_led_process, "Green led");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
	static void
recv_uc(struct unicast_conn *c, rimeaddr_t *from)
{
	printf("unicast message received from %d.%d\n",
			from->u8[0], from->u8[1]);
	leds_on(LEDS_RED);
	process_post(&red_led_process, event, NULL);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{

	static struct etimer et;
	PROCESS_EXITHANDLER(unicast_close(&uc);)

	PROCESS_BEGIN();

	event = process_alloc_event();

	process_start(&green_led_process, &event);	
	process_start(&red_led_process, &event);
	unicast_open(&uc, 128, &unicast_callbacks);

	etimer_set(&et, CLOCK_SECOND);

	while(1) {
		rimeaddr_t addr;

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		printf("My addr is: %d.%d\n", rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
		
		packetbuf_copyfrom("Hello", 5);
		addr.u8[0] = 115;
		addr.u8[1] = 237;
		if ((addr.u8[0] != rimeaddr_node_addr.u8[0]) || (addr.u8[0] != rimeaddr_node_addr.u8[0]))
		{
			unicast_send(&uc, &addr);
			leds_on(LEDS_GREEN);

			process_post(&green_led_process, event, NULL);
		}
		etimer_reset(&et);

	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(red_led_process, ev, data)
{
	static int count = 0;
	static process_event_t event; 
	static struct etimer et;
	PROCESS_BEGIN();
	event = (*(process_event_t *)data);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == event);
		
	    etimer_set(&et, CLOCK_SECOND/4); 	
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		leds_off(LEDS_RED);
	}

	PROCESS_END();
}


PROCESS_THREAD(green_led_process, ev, data)
{
	static int count = 0;
	static process_event_t event; 
	static struct etimer et;
	PROCESS_BEGIN();
	event = (*(process_event_t *)data);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == event);
		
	    etimer_set(&et, CLOCK_SECOND/4); 	
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		leds_off(LEDS_GREEN);
	}

	PROCESS_END();
}


