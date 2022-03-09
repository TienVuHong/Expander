/*
 * Database.hpp
 *
 *  Created on: 24/05/2021
 *      Author: tienvh
 * 
 */

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <string>
#include <sqlite3.h>
#include "../json.h"

/* Max physical channel of expander */
#define MAX_CHANNEL     3U

/* AI type event */
#define PERSON_DETECTION        0
#define FACE_RECOGNITION        1
#define SOCIAL_DISTANCING       2
#define FACEMASK_DETECTION      3
#define LICENSE_RECOGNITION     4
#define FIRE_DETECTION          5
#define INTRUSION_DETECTION     6
#define WRONG_LANE_VIOLATION    7
#define WRONG_WAY_VIOLATION     8
#define RED_LIGHT_VIOLATION     9

/* Profile status */
#define PROFILE_STATUS_DISABLE      0
#define PROFILE_STATUS_SCHEDULE     1
#define PROFILE_STATUS_ENABLE       2

/* Expander Mode */
#define MODE_SERVER     0u
#define MODE_CLIENT     1u
#define MODE_PEER       2u

/* Default schedule */
#define DEFAULT_SCHEDULE \
"[{\"day\":2,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":3,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":4,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":5,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":6,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":7,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]},\
{\"day\":0,\"timeslots\":[{\"allowedfrom\":360,\"alloweduntil\":1320}]}]"

/**
struct of profile info
@param:
@param profileid<string>: id
@param schedule<string>: schedule
@param images<string>: image array
@param status<int>: enable - disable - schedule
*/
typedef struct profile_t{
    std::string profileid;
    std::string schedule;
    std::string images;
    int status;
}profile_t;

/**
struct of server info
@param:
@param use<bool>: true/false
@param url<string>
@param token<string>
*/
typedef struct server_t{
    bool use;
    std::string url;
    std::string token;
}server_t;

// typedef struct event_t{
//     std::string imageid;
//     json headers;

// }event_t;

class Database
{
public: 
    Database();
    ~Database();

    int addUser(std::string userid, std::string password);
    int removeUser(std::string userid);
    int changePassword(std::string userid, std::string passwordOld, std::string passwordNew);
    bool checkLoginInfo(std::string userid, std::string password);
    std::vector<std::string> loadUser();

    int hasProfileExisted(std::string profileid, int channel);
    int addProfile(profile_t profile, int channel);
    int removeProfile(std::string profileid, int channel);
    int changeProfileSchedule(profile_t profile, int channel);
    int changeProfileStatus(profile_t profile, int channel);
    int getProfileStatus(std::string profileid, int channel);
    json getProfileSchedule(std::string profileid, int channel);
    json getAllProfile(int channel);
    int deleteAllProfile(int channel);

    int changeServer(server_t server);
    server_t getServer();

    int changeTimeOpen(int channel, int second);
    int getTimeOpen(int channel);
    int changeTypeEvent(int channel, int typeEvent);
    int getTypeEvent(int channel);
    int changeCameraIp(int channel, std::string cameraIp);
    int getCameraIp(int channel, std::string &cameraIp);

    int changeMode(int syncMode);
    int getMode();

private:
    sqlite3 *db;
    void createUserTable();
    void createProfileTable(int channel);
    void createServerTable();
    void createConfigTable();
    void createDeviceConfig();
    bool hasUserExisted(std::string userid);
};

class DatabaseForward
{
public:
    DatabaseForward();
    ~DatabaseForward();
    int push(json event);
    int peak(json &event);
    int exist();
    int pop();
private:
    sqlite3 *sqlForward;
    void createForwardTable();
    void resetEventCounter();
};


class DatabaseEvent
{
public:
    DatabaseEvent();
    ~DatabaseEvent();
    int push(json event);
    int get(json &event, json condition);
    int count();
private:
    sqlite3 *sqlEvent;
    void createEventTable();
};
#endif /* DATABASE_HPP */