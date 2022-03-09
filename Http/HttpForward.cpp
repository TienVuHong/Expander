#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "../Database/Database.hpp"
#include "../json.h"
#include "../config.h"
#include "HttpClient.hpp"
#include "HttpForward.hpp"

#define TIME_SLEEP_NOTCONNECTED 60
#define TIME_SLEEP_CONNECTED    2

HttpForward::HttpForward(Database *database)
{
    this->database = database;
    databaseForward = new DatabaseForward();
    pthread_create(&ptidForwardMain, NULL, &forwardMainProcess, this);
    pthread_create(&ptidForwardMedia, NULL, &forwardMediaProcess, this);
    pthread_create(&ptidForwardBackup, NULL, &forwardBackupProcess, this);
    pthread_detach(ptidForwardMain);
    pthread_detach(ptidForwardMedia);
    pthread_detach(ptidForwardBackup);
}

HttpForward::~HttpForward()
{
    pthread_cancel(ptidForwardMain);
    pthread_cancel(ptidForwardMedia);
    pthread_cancel(ptidForwardBackup);
    delete(databaseForward);
}

void HttpForward::sendJsonForward(json jEvent)
{
    QueueMain.push(jEvent);
}

void HttpForward::sendForwardMedia(json jEvent)
{
    QueueMedia.push(jEvent);
}

void HttpForward::sendForwardBackup(json jEvent)
{
    databaseForward->push(jEvent);
}

void* HttpForward::forwardMainProcess(void *arg)
{
    HttpForward *threadMain = (HttpForward*)arg;
    threadMain->runThreadForwardMain();
    pthread_exit(NULL);
}

void* HttpForward::forwardMediaProcess(void *arg)
{
    HttpForward *threadMedia = (HttpForward*)arg;
    threadMedia->runThreadForwardMedia();
    pthread_exit(NULL);
}

void* HttpForward::forwardBackupProcess(void *arg)
{
    HttpForward *threadBackup = (HttpForward*)arg;
    threadBackup->runThreadForwardBackup();
    pthread_exit(NULL);
}

void HttpForward::runThreadForwardMain()
{
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string response;
    server_t server;
    json jEvent;
    int reVal;

    while (true)
    {
        if (QueueMain.size())
        {
            pthread_mutex_lock(&lock);
            {
                server = database->getServer();
                response = "No Server";
                if (server.use == true)
                {
                    server.url += (server.token == "") ? "" : ("?access_token=" + server.token);
                    jEvent = QueueMain.front();
                    reVal = client->request(server.url, POST, std::string(jEvent["content"]), response, DEFAULT_TIMEOUT, jEvent["headers"]);
                    if (reVal != 200)
                    {
                        sendForwardBackup(jEvent);
                    }
                }
                std::cout << "Forward main http_code: " << reVal << ", response: " << response << std::endl;
                QueueMain.pop();
            }
            pthread_mutex_unlock(&lock);
        }
        else
            usleep(100000);
    }
    delete(client);
    pthread_mutex_destroy(&lock);
}

void HttpForward::runThreadForwardMedia()
{
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string response, content;
    server_t server;
    json jEvent;
    int reVal;

    while (true)
    {
        if (QueueMedia.size())
        {
            std::cout << "=== New image exits forward" << std::endl;
            pthread_mutex_lock(&lock);
            {
                server = database->getServer();
                response = "No Server";
                if (server.use == true)
                {
                    jEvent = QueueMedia.front();
                    server.url += "/" + std::string(jEvent["imageid"]) + ((server.token == "") ? "" : ("?access_token=" + server.token));
                    content = std::string(jEvent["content"]);
                    std::cout << "=== send image, content size: " << content.length() << std::endl;
                    reVal = client->request(server.url, POST, content, response, DEFAULT_TIMEOUT, jEvent["headers"]);
                    if (reVal != 200)
                    {
                        jEvent["content"] = jEvent["filepath"];
                        sendForwardBackup(jEvent);
                    }
                }
                std::cout << "Forward media http_code: " << reVal << ", response: " << response << std::endl;
                QueueMedia.pop();
            }
            pthread_mutex_unlock(&lock);
        }
        else
            usleep(100000);
    }
    delete(client);
    pthread_mutex_destroy(&lock);
}

void HttpForward::runThreadForwardBackup()
{
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string url, info, response;
    int retCode, timeSleep;
    forwardMedia_t param;
    server_t server;
    json jEvent;

    while (true)
    {
        // Ping server
        // something here
        std::cout << "Forward backup ping" << std::endl;
        pthread_mutex_lock(&lock);
        server = database->getServer();
        pthread_mutex_unlock(&lock);
        timeSleep = TIME_SLEEP_NOTCONNECTED;
        if (server.use == true)
        {
            pthread_mutex_lock(&lock);
            retCode = databaseForward->exist();
            pthread_mutex_unlock(&lock);
            if (databaseForward->exist())
            {
                pthread_mutex_lock(&lock);
                databaseForward->peak(jEvent);
                pthread_mutex_unlock(&lock);
                
                url = server.url + 
                ((jEvent.contains("imageid")) ? ("/" + std::string(jEvent["imageid"])) : std::string()) + 
                ((server.token  != "") ? ("?access_token=" + server.token) : std::string());
                std::cout << "url = " << url << std::endl;

                if (jEvent.contains("imageid"))
                    retCode = client->postImageMultipart(url, std::string(jEvent["message"]), std::string(jEvent["filepath"]), response, DEFAULT_TIMEOUT, jEvent["headers"]);
                else
                    retCode = client->request(url, POST, std::string(jEvent["content"]), response, DEFAULT_TIMEOUT, jEvent["headers"]);
                std::cout << "Forward backup http_code: " << retCode << ", response: " << response << std::endl;
                if (retCode == 200){ // if send to server successfully, then pop that event from database
                    pthread_mutex_lock(&lock);
                    databaseForward->pop();
                    pthread_mutex_unlock(&lock);
                    timeSleep = TIME_SLEEP_CONNECTED;
                }
            }
        }
        sleep(timeSleep);
    }
    delete(client);
    pthread_mutex_destroy(&lock);
}
