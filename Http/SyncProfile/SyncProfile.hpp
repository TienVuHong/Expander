#ifndef SYNC_PROFILE_HPP
#define SYNC_PROFILE_HPP

#include <queue>

/**
struct of camera info
@param:
@param ip<string>
@param username<string>
@param password<string>
@param token<string>
*/ 
typedef struct camera_t
{
    std::string ip;
    std::string username;
    std::string password;
    std::string token;
}camera_t;

/**
struct for put all queue and thread
@param:
@param channel<int>: channel in which camera was added
@param camera<camera_t>: camera: ip, username, password, token
*/
typedef struct putAllParam_t{
    int channel;
    camera_t camera;
}putAllParam_t;

/**
struct for put one queue and thread
@param:
@param channel<int>: channel in which camera was added
@param action<bool>: true - add, false - delete
@param profile<profile_t>: profile: profileid, username, password, token
*/
typedef struct putOneParam_t{
    int channel;
    bool action;
    profile_t profile;
}putOneParam_t;

class SyncProfile
{
public:
    SyncProfile(Database *database);
    ~SyncProfile();
    int mode;
    void putAllProfile(int channel, camera_t camera);
    void putProfile(int channel, profile_t profile);
    void delProfile(int channel, std::string profileid);
private:
    Database *database;
    std::queue<putAllParam_t> QueuePutAll;
    std::queue<putOneParam_t> QueuePutOne;
    pthread_t ptidPutAll;
    pthread_t ptidPutOne;
    pthread_t ptidGetAll;
    void runPutAllThread();
    void runPutOneThread();
    void runGetAllThread();
    static void* putAllThread(void *arg);
    static void* putOneThread(void *arg);
    static void* getAllThread(void *arg);
};

#endif /* SYNC_PROFILE_HPP */