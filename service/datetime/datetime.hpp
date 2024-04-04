#ifndef DATETIME_HPP
#define DATETIME_HPP

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

datetime_t getDatetime();
std::string time2string(int hour, int minute, int second);
std::string date2string(int year, int month, int date);
int daytimeToMinute(int hour, int min);
std::string getWeekday();
void iso8601GetDatetime(std::string iso8601, std::string &date, std::string &time);

#endif //DATETIME_HPP