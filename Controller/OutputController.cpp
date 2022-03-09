/*
* OutputController.cpp
* Created by DuNTK on 11/01/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#include "OutputController.hpp"

OutputController::OutputController() {
    // wiringPiSetup();
    // pinMode(25, OUTPUT);
    // digitalWrite(25, 1);
    // pinMode(28, OUTPUT);
    // digitalWrite(28, 1);
    // pinMode(29, OUTPUT);
    // digitalWrite(29, 1);

    activate_1 = 0;
    activate_2 = 0;
    activate_3 = 0;
    activate_4 = 0;
    state_1 = 0;
    state_2 = 0;
    state_3 = 0;
    state_4 = 0;
}

OutputController::~OutputController() {
    pthread_cancel(this->dev1);
    pthread_cancel(this->dev2);
    pthread_cancel(this->dev3);
    pthread_cancel(this->dev4);
}

void OutputController::runThread() {
    pthread_create(&this->dev1, NULL, dev1Func, this);
    pthread_create(&this->dev2, NULL, dev2Func, this);
    pthread_create(&this->dev3, NULL, dev3Func, this);
    pthread_create(&this->dev4, NULL, dev4Func, this);
    pthread_detach(this->dev1);
    pthread_detach(this->dev2);
    pthread_detach(this->dev3);
    pthread_detach(this->dev4);
}

int OutputController::process(int id, int state) {
    switch (id) {
        case 1:
            this->activate_1 = 1;
            this->state_1 = state;
            return 1;
            break;
        case 2:
            this->activate_2 = 1;
            this->state_2 = state;
            return 1;
            break;
        case 3:
            this->activate_3 = 1;
            this->state_3 = state;
            return 1;
            break;
        case 4:
            this->activate_4 = 1;
            this->state_4 = state;
            return 1;
        default:
            this->activate_1 = 0;
            this->activate_2 = 0;
            this->activate_3 = 0;
            return 0;
            break;
    }
}

void* OutputController::dev1Func(void* arg) {
    OutputController* thread = (OutputController*)arg;
    thread->runThread1();
    pthread_exit(NULL);
}

void* OutputController::dev2Func(void* arg) {
    OutputController* thread = (OutputController*)arg;
    thread->runThread2();
    pthread_exit(NULL);
}

void* OutputController::dev3Func(void* arg) {
    OutputController* thread = (OutputController*)arg;
    thread->runThread3();
    pthread_exit(NULL);
}

void* OutputController::dev4Func(void* arg) {
    OutputController* thread = (OutputController*)arg;
    thread->runThread4();
    pthread_exit(NULL);
}


void OutputController::runThread1() {
    while (1) {
        usleep(10000);
        if (this->activate_1 && this->state_1) {
            std::cout << "ON - " << this->state_1 << std::endl;
            // digitalWrite(25, !this->state_1);          
            sleep(20);
            std::cout << "OFF" << std::endl;
            // digitalWrite(25, this->state_1);
            this->activate_1 = 0;
        }
        // else digitalWrite(25, 1);
    }
}

void OutputController::runThread2() {
    while (1) {
        usleep(10000);
        if (this->activate_2 && this->state_2) {
            std::cout << "ON - " << this->state_2 << std::endl;
            // digitalWrite(28, !this->state_2);
            sleep(20);
            std::cout << "OFF" << std::endl;
            // digitalWrite(28, this->state_2);
            this->activate_2 = 0;
        }
        // else digitalWrite(28, 1);
    }
}

void OutputController::runThread3() {
    while (1) {
        usleep(10000);
        if (this->activate_3 && this->state_3) {
            std::cout << "ON - " << this->state_1 << std::endl;
            // digitalWrite(29, !this->state_3);          
            sleep(20);
            std::cout << "OFF" << std::endl;
            // digitalWrite(29, this->state_3);
            this->activate_3 = 0;
        }
        // else digitalWrite(29, 1);
    }
}

void OutputController::runThread4() {
    while (1) {
        usleep(10000);
        if (this->activate_4 && this->state_4) {
            std::cout << "ON - " << this->state_4 << std::endl;
            // digitalWrite(23, !this->state_1);          
            sleep(5);
            std::cout << "OFF" << std::endl;
            // digitalWrite(23, this->state_1);
            this->activate_4 = 0;
        }
        // else digitalWrite(23,this->state_1);
    }
}



json OutputController::getStatus(int deviceId) {
    json j;
    int state = 0;
    switch (deviceId) {
        case 1:          
            // state = digitalRead(25);
            j["DeviceId"] = 1;
            j["DeviceName"] = "Cửa số 1";
            j["Open"] = state;
            j["Active"] = 1;
            std::cout << "Get device 1 info" << std::endl;
            break;
        case 2:
            // state = digitalRead(28);
            j["DeviceId"] = 2;
            j["DeviceName"] = "Cửa số 2";
            j["Open"] = state;
            j["Active"] = 1;
            std::cout << "Get device 2 info" << std::endl;
            break;
        case 3:
            // state = digitalRead(29);
            j["DeviceId"] = 3;
            j["DeviceName"] = "Cửa số 3";
            j["Open"] = state;
            j["Active"] = 1;
            std::cout << "Get device 3 info" << std::endl;
            break;
        case 4:
            // state = digitalRead(23);
            j["DeviceId"] = 4;
            j["DeviceName"] = "Cửa số 4";
            j["Open"] = state;
            j["Active"] = 1;
            std::cout << "Get device 4 info" << std::endl;
            break;
        default:
            // state = digitalRead();
            j["DeviceId"] = -1;
            j["DeviceName"] = "Cửa số X";
            j["Open"] = state;
            j["Active"] = -1;          
            std::cout << "Get device ? info" << std::endl;            
            break;
    }
    return j;
}



