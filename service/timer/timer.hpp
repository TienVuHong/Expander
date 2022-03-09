/*
 * timer.h
 *
 *  Created on: 15/05/2019
 *      Author: dongnx
 *  Edited on:  01/07/2021
 *      Editor: tienvh
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <stdint.h>

void process_timer_events(void);
void start_timer(uint32_t millis, void(*callback)(void* data), void* ptr);
void cancel_timer(void(*callback)(void* data));
bool is_timer_running(void(*callback)(void* data));
void delay_ms(uint32_t millis);

#endif /* TIMER_HPP */
