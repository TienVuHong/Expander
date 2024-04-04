/*
 * Controller.hpp
 *
 *  Created on: 01/07/2021
 *      Author: tienvh
 */

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../service/timer/timer.hpp"
#include "../driver/io/io.hpp"
#include <pthread.h>
#include <unistd.h>

#define ON      LOW_STATE
#define OFF     HIGH_STATE
#define CHANNEL1    26U
#define CHANNEL2    20U
#define CHANNEL3    21U
#define BUTTON_1    17U
#define BUTTON_2    27U
#define BUTTON_3    22U

class Controller
{
public:
    Controller();
    ~Controller();

    void ControlChannel(int channel, int state);
    void ControlOpenDeviceDelay(int channel, int timeSec);
private:
    pthread_t interruptThread;
    static void* interruptProcess(void* arg);
    void setupInterruptThread();
    void cancelInterruptThread();
};

#endif /* CONTROLLER_HPP */