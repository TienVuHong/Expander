#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>

#include "Database/Database.hpp"
#include "service/json/json.hpp"
#include "EventManager.hpp"

EventManager::EventManager()
{
    databaseEvent = new DatabaseEvent();
    pthread_create(&ptid, NULL, threadEventManager, this);
    pthread_detach(ptid);
}

EventManager::~EventManager()
{
    pthread_cancel(ptid);
    delete(databaseEvent);
}

void EventManager::push(json event)
{
    QueueEvent.push(event);
}

void* EventManager::threadEventManager(void *arg)
{
    EventManager *evMgr = (EventManager*)arg;
    while(true)
    {
        if (evMgr->QueueEvent.size())
        {
            evMgr->databaseEvent->push(evMgr->QueueEvent.front());
            evMgr->QueueEvent.pop();
        }
        else
            sleep(1);
    }
    pthread_exit(NULL);
} 

void EventManager::get(json &event, json condition)
{
    databaseEvent->get(event, condition);
}

