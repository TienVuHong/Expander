/******************************************************************************** 
Copyright (C) 2008-2009, SmartHome Co., Ltd. All rights reserved.
Product: AICAM
Module: 
Version: 1.0
Author: HieuPV
Created: 
Modified: 
Released: 
Description: 
Note: <Note>
********************************************************************************/
#pragma once
#include <iostream>
#include <ctime>
#include <cmath>
#include <chrono>
#include "../common/common.h"


/**
struct of datetime info
@param:
@param second<int>
@param minute<int>
@param hour<int>
@param date<int>
@param month<int>
@param year<int>
@param day<int>
*/
typedef struct datetime_t{
    int second;
    int minute;
    int hour;
    int date;
    int month;
    int year;
    int day;
}datetime_t;

std::string weekdayString[7] = {"Saturday","Sunday","Monday","Tuesday", "Wednesday","Thursday","Friday"};
int wday[7] = {7, 0, 2, 3, 4, 5, 6};

static char* GetDateTime()
{
    static char d_time[50];
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    sprintf(d_time, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return d_time;
}

static unsigned long long GetTimeStamp()
{
    const auto p1 = std::chrono::system_clock::now();
    long time_stamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
    return time_stamp;
}

int zellersAlgorithm(int day, int month, int year)
{
    int mon;
    if(month > 2)
        mon = month; //for march to december month code is same as month
    else
    {
        mon = (12+month); //for Jan and Feb, month code will be 13 and 14
        year--; //decrease year for month Jan and Feb
    }
    int y = year % 100; //last two digit
    int c = year / 100; //first two digit
    int w = (day + floor((13*(mon+1))/5) + y + floor(y/4) + floor(c/4) + (5*c));
    w = w % 7;
    return wday[w];
}

datetime_t getDatetime()
{
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    int weekday = zellersAlgorithm(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    datetime_t datetime;
    datetime.second = tm.tm_sec;
    datetime.minute = tm.tm_min;
    datetime.hour   = tm.tm_hour;
    datetime.date   = tm.tm_mday;
    datetime.month  = tm.tm_mon + 1;
    datetime.year   = tm.tm_year + 1900;
    datetime.day    = weekday;
    return datetime;
}

std::string time2string(int hour, int minute, int second)
{
    std::string timeString =    (hour < 10 ? ("0" + std::to_string(hour)) : std::to_string(hour)) + ":" +
                                (minute < 10 ? ("0" + std::to_string(minute)) : std::to_string(minute)) + ":" +
                                (second < 10 ? ("0" + std::to_string(second)) : std::to_string(second));
    return timeString;
}

std::string date2string(int year, int month, int date)
{
    std::string dateString =    (year < 10 ? ("0" + std::to_string(year)) : std::to_string(year)) + "-" +
                                (month < 10 ? ("0" + std::to_string(month)) : std::to_string(month)) + "-" +
                                (date < 10 ? ("0" + std::to_string(date)) : std::to_string(date));
    return dateString;
}

int daytimeToMinute(int hour, int min)
{
    return (hour * 60 + min);
}

std::string getWeekday()
{
    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    std::string wday = weekdayString[zellersAlgorithm(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900)];
    return wday;
}

void iso8601GetDatetime(std::string iso8601, std::string &date, std::string &time)
{
    std::vector<std::string> vtr = splitString(iso8601, "T");
    date = vtr[0];
    std::vector<std::string> vtr1 = splitString(vtr[1], "+");
    time = vtr1[0];
}
