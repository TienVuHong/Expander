/*
* Controller.hpp
* Created by DuNTK on 11/01/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#ifndef Controller_hpp
#define Controller_hpp

// #define _POSIX_THREADS

#define HIGH 1
#define LOW 0

class Controller {

public:
    Controller();
    virtual ~Controller();

    int activate_1;
    int activate_2;
    int activate_3;
    int activate_4;
    int state_1;
    int state_2;
    int state_3;
    int state_4;
    
    int process(int id, int state);
    void runThread();

private:
    pthread_t dev1;
    pthread_t dev2;
    pthread_t dev3;
    pthread_t dev4;

    static void* dev1Func(void* arg);
    static void* dev2Func(void* arg);
    static void* dev3Func(void* arg);
    static void* dev4Func(void* arg);

    void runThread1();
    void runThread2();
    void runThread3();
    void runThread4();
};

#endif
