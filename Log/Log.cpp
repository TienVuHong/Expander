/*
* Log.hpp
* Created by DuNTK on 02/06/2021.
* Copyright © 2021 BKAV. All rights reserved.
*/

#include "Log.hpp"

Log::Log() {
    char date[12];
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(date, sizeof(date), "%F", localtime(&now));
    fileName = "AIVEX/Log/" + std::string(date) + ".log";
    std::cout << "Open file: " << fileName << std::endl;
    fs.open(fileName, std::ios::out | std::ios::app);
    fs.close();
}

Log::~Log() {
    fs.close();
}

void Log::writeLog(std::string info) {
    std::string log;
    char buf[32];
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(buf, sizeof(buf), "%F %T", localtime(&now));
    log = std::string(buf) + info + '\n';
    fs.open(fileName, std::ios::app);
    fs << log;
}

void Log::writeLog(std::string ip, std::string reqMethod, std::string urlPath, std::string info) {
    std::string log;
    char buf[32];
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(buf, sizeof(buf), "%F %T", localtime(&now));
    log = std::string(buf) + " [" + ip + "] " + reqMethod + " " + urlPath + ": \"" + info + "\"\n";
    std::cout << "Log: " << log << std::endl;
    fs.open(fileName, std::ios::app);
    fs << log;
    fs.close();
}