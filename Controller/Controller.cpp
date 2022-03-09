/*
 * Controller.cpp
 *
 *  Created on: 01/07/2021
 *      Author: tienvh
 */


#include "Controller.hpp"

void controllerInit();
void turnOnChannel1(void *arg);
void turnOnChannel2(void *arg);
void turnOnChannel3(void *arg);
void turnOffChannel1(void *arg);
void turnOffChannel2(void *arg);
void turnOffChannel3(void *arg);
void turnOnChannel(uint8_t channel);
void turnOffChannel(uint8_t channel);
void controlChannel(uint8_t channel, uint8_t state);
void openDeviceDelay(uint8_t channel, uint32_t timeSec);

gpio_t* obj_channel_1 = gpio_create(CHANNEL1);
gpio_t* obj_channel_2 = gpio_create(CHANNEL2);
gpio_t* obj_channel_3 = gpio_create(CHANNEL3);
gpio_t* obj_button_1 = gpio_create(BUTTON_1);
gpio_t* obj_button_2 = gpio_create(BUTTON_2);
gpio_t* obj_button_3 = gpio_create(BUTTON_3);

void controllerInit()
{
    gpio_set_direction(obj_channel_1, OUTPUT_MODE);
    gpio_set_state(obj_channel_1, OFF);
    gpio_set_direction(obj_channel_2, OUTPUT_MODE);
    gpio_set_state(obj_channel_2, OFF);
    gpio_set_direction(obj_channel_3, OUTPUT_MODE);
    gpio_set_state(obj_channel_3, OFF);
    //Config for button 1
    gpio_set_direction(obj_button_1, INPUT_MODE);
    gpio_set_pullup(obj_button_1);
    gpio_set_edge(obj_button_1, "falling");
    //Config for button 2
    gpio_set_direction(obj_button_2, INPUT_MODE);
    gpio_set_pullup(obj_button_2);
    gpio_set_edge(obj_button_2, "falling");
    //Config for button 3
    gpio_set_direction(obj_button_3, INPUT_MODE);
    gpio_set_pullup(obj_button_3);
    gpio_set_edge(obj_button_3, "falling");
    printf("controllerInit >>>>>>>>>>>>> OK\n");
}

void turnOnChannel1(void *arg)
{
    gpio_set_state(obj_channel_1, ON);
    printf("Channel 1 - ON\n");
}

void turnOnChannel2(void *arg)
{
    gpio_set_state(obj_channel_2, ON);
    printf("Channel 2 - ON\n");
}

void turnOnChannel3(void *arg)
{
    gpio_set_state(obj_channel_3, ON);
   printf("Channel 3 - ON\n");
}

void turnOffChannel1(void *arg)
{
    gpio_set_state(obj_channel_1, OFF);
    printf("Channel 1 - OFF\n");
}

void turnOffChannel2(void *arg)
{
    gpio_set_state(obj_channel_2, OFF);
    printf("Channel 2 - OFF\n");
}

void turnOffChannel3(void *arg)
{
    gpio_set_state(obj_channel_3, OFF);
    printf("Channel 3 - OFF\n");
}

void turnOnChannel(uint8_t channel)
{
    switch (channel)
    {
        case 1:
            turnOnChannel1(NULL);
            break;
        case 2:
            turnOnChannel2(NULL);
            break;
        case 3:
            turnOnChannel3(NULL);
            break;
        default:
            break;
    }
}

void turnOffChannel(uint8_t channel)
{
    switch (channel)
    {
        case 1:
            turnOffChannel1(NULL);
            break;
        case 2:
            turnOffChannel2(NULL);
            break;
        case 3:
            turnOffChannel3(NULL);
            break;
        default:
            break;
    }
}

void controlChannel(uint8_t channel, uint8_t state)
{
    if (state == ON)
        turnOnChannel(channel);
    else
        turnOffChannel(channel);
}

void openDeviceDelay(uint8_t channel, uint32_t timeSec)
{
    if (channel == 1)
    {
        turnOnChannel1(NULL);
        start_timer(timeSec * 1000, turnOffChannel1, NULL);
    }
    else if(channel == 2)
    {
        turnOnChannel2(NULL);
        start_timer(timeSec * 1000, turnOffChannel2, NULL);
    }
    else if(channel == 3)
    {
        turnOnChannel3(NULL);
        start_timer(timeSec * 1000, turnOffChannel3, NULL);
    }
    else
        return;
}

Controller::Controller()
{
    controllerInit();
    setupInterruptThread();
}

Controller::~Controller()
{
    cancelInterruptThread();
}

void Controller::ControlChannel(int channel, int state)
{
    controlChannel((uint8_t)channel, (uint8_t)state);
}

void Controller::ControlOpenDeviceDelay(int channel, int timeInSec)
{
    openDeviceDelay((uint8_t)channel, (uint32_t)timeInSec);
}

void Controller::setupInterruptThread()
{
    pthread_create(&interruptThread, NULL, &interruptProcess, NULL);
    pthread_detach(interruptThread);
}

void Controller::cancelInterruptThread()
{
    pthread_cancel(interruptThread);
}

void* Controller::interruptProcess(void *arg) {
#ifdef RASPBERRY_PI
    int old_state_button_1 = 0, current_state_button_1 = 0;
    int old_state_button_2 = 0, current_state_button_2 = 0;
    int old_state_button_3 = 0, current_state_button_3 = 0;
    while (1) {
        usleep(10000);

        // //Process event button 1
        // current_state_button_1 = gpio_get_state(obj_button_1);
        // if (old_state_button_1 == 0 && current_state_button_1 == 1)
        // {
        //     if (gpio_get_edge(obj_button_1) == RISING || gpio_get_edge(obj_button_1) == BOTH)
        //     {
        //         ControlOpenDeviceDelay(1, 5);
        //     }
                
        // }
        // if (old_state_button_1 == 1 && current_state_button_1 == 0)
        // {
        //     if (gpio_get_edge(obj_button_1) == FALLING || gpio_get_edge(obj_button_1) == BOTH)
        //     {
        //         ControlOpenDeviceDelay(1, 5);
        //     }
                
        // }
        // old_state_button_1 = current_state_button_1;

        // //Process event button 2
        // current_state_button_2 = gpio_get_state(obj_button_2);
        // if (old_state_button_2 == 0 && current_state_button_2 == 1)
        // {
        //     if (gpio_get_edge(obj_button_2) == RISING || gpio_get_edge(obj_button_2) == BOTH)
        //         ControlOpenDeviceDelay(2, 5);
        // }
        // if (old_state_button_2 == 1 && current_state_button_2 == 0)
        // {   
        //     if (gpio_get_edge(obj_button_2) == FALLING || gpio_get_edge(obj_button_2) == BOTH)
        //         ControlOpenDeviceDelay(2, 5);
        // }
        // old_state_button_2 = current_state_button_2;

        // //Process event button 3
        // current_state_button_3 = gpio_get_state(obj_button_3);
        // if (old_state_button_3 == 0 && current_state_button_3 == 1)
        // {
        //     if (gpio_get_edge(obj_button_3) == RISING || gpio_get_edge(obj_button_3) == BOTH)
        //         ControlOpenDeviceDelay(3, 5);
        // }
        // if (old_state_button_3 == 1 && current_state_button_3 == 0)
        // {   
        //     if (gpio_get_edge(obj_button_3) == FALLING || gpio_get_edge(obj_button_3) == BOTH)
        //         ControlOpenDeviceDelay(3, 5);
        // }
        // old_state_button_3 = current_state_button_3;

        current_state_button_1 = gpio_get_state(obj_button_1);
        if (old_state_button_1 != current_state_button_1)
        {
            controlChannel(1, (current_state_button_1 == ON) ? ON : OFF);
        }
        old_state_button_1 = current_state_button_1;

        current_state_button_2 = gpio_get_state(obj_button_2);
        if (old_state_button_2 != current_state_button_2)
        {
            controlChannel(2, (current_state_button_2 == ON) ? ON : OFF);
        }
        old_state_button_2 = current_state_button_2;

        current_state_button_3 = gpio_get_state(obj_button_3);
        if (old_state_button_3 != current_state_button_3)
        {
            controlChannel(3, (current_state_button_3 == ON) ? ON : OFF);
        }
        old_state_button_3 = current_state_button_3;
    }
    pthread_exit(NULL);
#endif
    return NULL;
}