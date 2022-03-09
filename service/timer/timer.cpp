/*
 * timer.c
 *
 *  Created on: 15/05/2019
 *      Author: dongnx
 *  Edited on:  01/07/2021
 *      Editor: tienvh
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "timer.hpp"

#define MAX_TIMER_COUNT		    64

#define TIMER_EXPIRED(timer) ((uint32_t)(get_tick_count() - timer.start) >= timer.time)
#define CLOCKS_PER_MSEC ((__clock_t) 1000)

uint32_t get_tick_count();

typedef struct timer_tick_t {
	uint32_t start;
	uint32_t time;
	void(*callback)(void* param);
	void* param;
} timer_tick_t;

static timer_tick_t timers[MAX_TIMER_COUNT] = { 0 };

uint32_t get_tick_count()
{   
    return (uint32_t)(clock() / CLOCKS_PER_MSEC);
}

void process_timer_events(void) {
	int index;
	static void* param = NULL;
	static void(*callback)(void* param) = NULL;

	for (index = 0; index < MAX_TIMER_COUNT; index++)
	{
		if ((timers[index].callback != NULL) && TIMER_EXPIRED(timers[index]))
		{
			callback = timers[index].callback;
			param = timers[index].param;
			timers[index].callback = NULL;
			callback(param);
		}
	}
}

void start_timer(uint32_t millis, void(*callback)(void* data), void* ptr) {
	int index;
	for (index = 0; index < MAX_TIMER_COUNT; index++)
	{
		if ((timers[index].callback == NULL) || (timers[index].callback == callback))
		{
			timers[index].start = get_tick_count();
			timers[index].callback = callback;
			timers[index].time = millis;
			timers[index].param = ptr;

			for (index = index + 1; index < MAX_TIMER_COUNT; index++) {
				if (timers[index].callback == callback)
					timers[index].callback = NULL;
			}
			return;
		}
	}
	// reboot();
}

void cancel_timer(void(*callback)(void* data)) {
	int index;
	for (index = 0; index < MAX_TIMER_COUNT; index++) {
		if (timers[index].callback == callback) {
			timers[index].callback = NULL;
			break;
		}
	}
}

bool is_timer_running(void(*callback)(void* data)) {
	int index;
	for (index = 0; index < MAX_TIMER_COUNT; index++) {
		if (timers[index].callback == callback)
			return true;
	}
	return false;
}

void delay_ms(uint32_t millis)
{
    uint32_t start = get_tick_count();
    while (get_tick_count() - start < millis){};
}