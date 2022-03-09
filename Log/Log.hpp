/*
* Log.hpp
* Created by DuNTK on 02/06/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#ifndef Log_hpp
#define Log_hpp

#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <iomanip>

class Log
{
private:
      
public:
    Log();
    ~Log();

    std::ofstream fs;
    std::string fileName;

    void writeLog(std::string info);
    void writeLog(std::string ip, std::string reqMethod, std::string urlPath, std::string info);
};


#endif
