#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_green_led_process, "Blink green led process");
PROCESS(blink_red_led_process, "Blink red led process");
PROCESS(test_button_process, "Test button");
PROCESS(event_receiver_process, "Event receiver process");
AUTOSTART_PROCESSES(&hello_world_process,&blink_green_led_process,&test_button_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
	static struct etimer et1;
	static process_event_t event;
	PROCESS_BEGIN();
	event = process_alloc_event();

	printf("hello!\n");

	process_start(&event_receiver_process, &event);
	etimer_set(&et1, CLOCK_SECOND * 4);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));

		if (ev == PROCESS_EVENT_TIMER) {
			printf("hello !\n");

			etimer_reset(&et1);
			process_start(&blink_red_led_process, NULL);
			process_post(&event_receiver_process, event, NULL);
		}
		
	}

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_green_led_process, ev, data)
{
	static struct etimer et1;
	PROCESS_BEGIN();

	etimer_set(&et1, CLOCK_SECOND );
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));

		if (ev == PROCESS_EVENT_TIMER) {
			leds_toggle(LEDS_GREEN);
			etimer_reset(&et1);
		}
	}

	PROCESS_END();
}

PROCESS_THREAD(blink_red_led_process, ev, data)
{
	static struct etimer et1;
	PROCESS_BEGIN();

	etimer_set(&et1, CLOCK_SECOND/10);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));

		if (ev == PROCESS_EVENT_TIMER) {
			leds_toggle(LEDS_RED);
			etimer_reset(&et1);
		}
	}

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_button_process, ev, data)
{
	static int count = 0;
	PROCESS_BEGIN();

	button_sensor.activate();

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
				data == &button_sensor);

		count++;
		printf("Button press %d\n", count);
	}

	PROCESS_END();
}

static void toggle_blue_led(struct rtimer *t, void *ptr)
{
	leds_toggle(LEDS_BLUE);

	rtimer_set(t, RTIMER_TIME(t) + RTIMER_ARCH_SECOND, 0,
			toggle_blue_led, ptr);
}

PROCESS_THREAD(event_receiver_process, ev, data)
{
	static int count = 0;
	static process_event_t event; 
	static struct rtimer rt1;
	PROCESS_BEGIN();
	event = (*(process_event_t *)data);

	rtimer_init();		

	rtimer_set(&rt1, RTIMER_NOW() + RTIMER_ARCH_SECOND, 0,
		toggle_blue_led, NULL); 	

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == event);
		printf("Received event\n");
	}

	PROCESS_END();
}


