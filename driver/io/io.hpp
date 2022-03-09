/*
 * io.h
 *
 *  Created on: Jul 28, 2018
 *      Author: dongnx
 */

#ifndef __SRS_IO_H__
#define __SRS_IO_H__

#define GPIO_STATUS_ERROR	-1

#define INPUT_MODE		0
#define OUTPUT_MODE		1

#define LOW_STATE		0
#define HIGH_STATE		1

#define EDGE_FA

typedef struct gpio_t gpio_t;

typedef struct gpio_p {
	int(*dispose)(gpio_t*);
	int(*set_direction)(gpio_t*, int);
	int(*get_direction)(gpio_t*);
	int(*set_state)(gpio_t*, int);
	int(*get_state)(gpio_t*);
	int(*set_edge)(gpio_t*, const char*);
} gpio_p;

struct gpio_t {
	const gpio_p* proc;
	int pin;
	int fd;
};

enum edge_t
{
	NONE = 1,
	RISING,
	FALLING,
	BOTH
};

gpio_t* gpio_create(int pin);
int gpio_dispose(gpio_t* object);
int gpio_set_direction(gpio_t* object, int direction);
int gpio_get_direction(gpio_t* object);
int gpio_set_state(gpio_t* object, int state);
int gpio_get_state(gpio_t* object);
int gpio_set_edge(gpio_t* object, const char* edge); //none, rising, falling or both
int gpio_get_edge(gpio_t* object);
int gpio_set_pullup(gpio_t* object);
int gpio_set_pulldown(gpio_t* object);

#endif /* __SRS_IO_H__ */
