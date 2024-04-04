#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <queue>

typedef struct {
    int command;
    json event;
} eventManager_t;

class EventManager{
public:
    EventManager();
    ~EventManager();
    void push(json event);
    void get(json &event, json condition);
private:
    DatabaseEvent *databaseEvent;
    std::queue<json> QueueEvent;
    pthread_t ptid;
    static void* threadEventManager(void *arg);
};

#endif /*EVENT_MANAGER_HPP*/