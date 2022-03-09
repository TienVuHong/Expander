
#ifndef HttpForward_hpp
#define HttpForward_hpp

#include <queue>

/**
struct for forward json queue and thread
@param:
@param content<string>: body of request
@param headers<std::vector<std::string>>
*/
typedef struct forwardParam_t {
    std::string content;
    std::vector<std::string> headers;
}forwardParam_t;

/**
struct for forward media(image) queue and thread
@param:
@param content<string>: body of request
@param imageid<string>: imageid of request
@param headers<std::vector<std::string>>
*/
typedef struct forwardMedia_t {
    std::string content;
    std::string imageid;
    std::vector<std::string> headers;
}forwardMedia_t;

class HttpForward
{
public:
    HttpForward(Database *database);
    ~HttpForward();
    void sendJsonForward(json jEvent);
    void sendForwardMedia(json jEvent);
    void sendForwardBackup(json jEvent);

private:
    Database *database;
    DatabaseForward *databaseForward;
    // DatabaseEvent *databaseEvent;
    std::string serverUrl;
    // std::queue<forwardParam_t> QueueMain;
    // std::queue<forwardMedia_t> QueueMedia;
    // std::queue<forwardMedia_t> QueueBackup;
    std::queue<json> QueueMain;
    std::queue<json> QueueMedia;
    std::queue<json> QueueBackup;
    pthread_t ptidForwardMain;
    pthread_t ptidForwardMedia;
    pthread_t ptidForwardBackup;
    static void* forwardMainProcess(void *arg);
    static void* forwardMediaProcess(void *arg);
    static void* forwardBackupProcess(void *arg);
    void runThreadForwardMain();
    void runThreadForwardMedia();
    void runThreadForwardBackup();
};

#endif /* HttpForward_hpp */