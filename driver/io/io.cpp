/*
 * io.c
 *
 *  Created on: Jul 28, 2018
 *      Author: dongnx
 */

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "io.hpp"

static gpio_p gpio_proc = {
	gpio_dispose,
	gpio_set_direction,
	gpio_get_direction,
	gpio_set_state,
	gpio_get_state,
	gpio_set_edge
};

#ifndef RASPBERRY_PI

gpio_t* gpio_create(int pin){return NULL;}
int gpio_dispose(gpio_t* object){return 0;}
int gpio_set_direction(gpio_t* object, int direction){return 0;}
int gpio_get_direction(gpio_t* object){return 0;}
int gpio_set_state(gpio_t* object, int state){return 0;}
int gpio_get_state(gpio_t* object){return 0;}
int gpio_set_edge(gpio_t* object, const char* edge){return 0;}
int gpio_get_edge(gpio_t* object){return 0;}
int gpio_set_pullup(gpio_t* object){return 0;}
int gpio_set_pulldown(gpio_t* object){return 0;}

#else

gpio_t* gpio_create(int pin) {
	char buffer[3] = { 0 };
	ssize_t size;

	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd == -1) {
		return NULL;
	}

	size = snprintf(buffer, 3, "%d", pin);
	write(fd, buffer, size);
	close(fd);

	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/value", pin);

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		return NULL;
	}

	gpio_t* object = (gpio_t*)malloc(sizeof(gpio_t));
	object->proc = &gpio_proc;
	object->pin = pin;
	object->fd = fd;
	return object;
}

int gpio_dispose(gpio_t* object) {
	char buffer[3] = { 0 };
	ssize_t size;

	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd == -1) {
		return (-1);
	}

	close(object->fd);

	size = snprintf(buffer, 3, "%d", object->pin);
	write(fd, buffer, size);
	close(fd);

	free(object);
	return 0;
}

int gpio_set_direction(gpio_t* object, int direction) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/direction", object->pin);

	int fd = open(path, O_WRONLY);
	if (fd == -1) {
		return (-1);
	}

	const char* direction_string = (direction == INPUT_MODE) ? "in" : "out";
	ssize_t size = (direction == INPUT_MODE) ? 2 : 3;
	int reval = write(fd, direction_string, size);

	close(fd);
	return reval;
}



int gpio_get_direction(gpio_t* object) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/direction", object->pin);

	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		return (-1);
	}

	char value_string[4]= { 0 };
	if (read(fd, value_string, 4) == -1) {
		return (-1);
	}

	close(fd);
	if (strcmp(value_string, "in"))
	{
		return INPUT_MODE;
	}
	return OUTPUT_MODE;

}

int gpio_set_state(gpio_t* object, int state) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/value", object->pin);

	int fd = open(path, O_WRONLY);
	if (fd == -1) {
		return (-1);
	}

	char value_string = (state == LOW_STATE) ? '0' : '1';
	if (write(fd, &value_string, 1) == -1) {
		return (-1);
	}

	close(fd);
	return 0;
}

int gpio_get_state(gpio_t* object) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/value", object->pin);

	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		return (-1);
	}

	char value_string[3]= { 0 };
	if (read(fd, value_string, 3) == -1) {
		return (-1);
	}

	close(fd);
	return atoi(value_string);
}

int gpio_set_edge(gpio_t* object, const char* edge) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/edge", object->pin);

	int fd = open(path, O_WRONLY);
	if (fd == -1) {
		return (-1);
	}

	write(fd, edge, strlen(edge) + 1);
	close(fd);
	return 0;
}

int gpio_get_edge(gpio_t* object) {
	char path[35] = { 0 };
	snprintf(path, 35, "/sys/class/gpio/gpio%d/edge", object->pin);

	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		return (-1);
	}

	char value_string[10]= { 0 };
	if (read(fd, value_string, 10) == -1) {
		return (-1);
	}
	close(fd);
	if (strcmp(value_string, "none\n") == 0)		return NONE;
	if (strcmp(value_string, "rising\n") == 0) 	return RISING;
	if (strcmp(value_string, "falling\n") == 0)	return FALLING;
	if (strcmp(value_string, "both\n") == 0)		return BOTH;
	return 0;
}

int gpio_set_pullup(gpio_t* object)
{
	char cmd[35] = { 0 };
	snprintf(cmd, 35, "raspi-gpio set %d pu", object->pin);
	system(cmd);
	return 0;
}


int gpio_set_pulldown(gpio_t* object)
{
	char cmd[35] = { 0 };
	snprintf(cmd, 35, "raspi-gpio set %d pd", object->pin);
	system(cmd);
	return 0;
}

#endif //ifndef RASPBERRY_PI