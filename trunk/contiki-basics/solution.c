#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_leds,  "Blink leds process");
PROCESS(blink_leds_fast,  "Blink leds faster process");

AUTOSTART_PROCESSES(&hello_world_process, &blink_leds);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
	static struct etimer timer;
	PROCESS_BEGIN();

	etimer_set(&timer, CLOCK_SECOND * 4);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

		process_start(&blink_leds_fast, NULL);

		printf("hello!\n");
		etimer_reset(&timer);	
	}

	PROCESS_END();
}

PROCESS_THREAD(blink_leds, ev, data)
{
	static struct etimer timer;
	PROCESS_BEGIN();

	etimer_set(&timer, CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_on(LEDS_GREEN);
		printf("led on!\n");

		etimer_reset(&timer);
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_off(LEDS_GREEN);
		printf("led off!\n");
		etimer_reset(&timer);	
	}

	PROCESS_END();
}



PROCESS_THREAD(blink_leds_fast, ev, data)
{
	static struct etimer timer;
	PROCESS_BEGIN();

	etimer_set(&timer, CLOCK_SECOND/10);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		leds_on(LEDS_RED);
		printf("fast led on!\n");

		etimer_reset(&timer);
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_off(LEDS_RED);
		printf("fast led off!\n");
		etimer_reset(&timer);	
	}

	PROCESS_END();
}


