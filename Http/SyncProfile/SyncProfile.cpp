#include <iostream>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include "service/base64/base64.hpp"
#include "Database/Database.hpp"
#include "config.h"
#include "Http/HttpClient.hpp"
#include "Http/SyncProfile/SyncProfile.hpp"

static int getToken(HttpClient *client, camera_t &camera);
static void getAllFileName(std::vector<std::string> &filenames, std::string folderPath);
std::string imageToBase64(std::string imagePath);

SyncProfile::SyncProfile(Database *database)
{
    this->database = database;
    mode = database->getMode();
    pthread_create(&ptidPutAll, NULL, putAllThread, this);
    pthread_create(&ptidPutOne, NULL, putOneThread, this);
    pthread_create(&ptidGetAll, NULL, getAllThread, this);
    pthread_detach(ptidPutAll);
    pthread_detach(ptidPutOne);
    pthread_detach(ptidGetAll);
}

SyncProfile::~SyncProfile()
{
    pthread_cancel(ptidPutAll);
    pthread_cancel(ptidPutOne);
    pthread_cancel(ptidGetAll);
}

void SyncProfile::putAllProfile(int channel, camera_t camera)
{
    putAllParam_t param = {channel, camera};
    QueuePutAll.push(param);
}

void SyncProfile::putProfile(int channel, profile_t profile)
{
    putOneParam_t param = {channel, true, profile};
    QueuePutOne.push(param);
}

void SyncProfile::delProfile(int channel, std::string profileid)
{
    profile_t profile = {profileid, "", "", 0};
    putOneParam_t param = {channel, false, profile};
    QueuePutOne.push(param);
}

void* SyncProfile::putAllThread(void *arg)
{
    SyncProfile *sync = (SyncProfile*)arg;
    sync->runPutAllThread();
    pthread_exit(NULL);
}

void* SyncProfile::putOneThread(void *arg)
{
    SyncProfile *sync = (SyncProfile*)arg;
    sync->runPutOneThread();
    pthread_exit(NULL);
}

void* SyncProfile::getAllThread(void *arg)
{
    SyncProfile *sync = (SyncProfile*)arg;
    sync->runGetAllThread();
    pthread_exit(NULL);
}

void SyncProfile::runPutAllThread()
{
    std::cout << "runPutAllThread ==== " << std::endl;
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string url, response, folderPath, profileid;
    std::vector<std::string> vtrImage;
    json jImage = json::array();
    json jProfile, object;
    profile_t profile;
    camera_t camera;
    int channel;
    int ret;
    
    while(true)
    {
        pthread_mutex_lock(&lock);
        mode = database->getMode();
        pthread_mutex_unlock(&lock);
        if (mode != MODE_SERVER){
            sleep(5);
            continue;
        }
        if (QueuePutAll.size())
        {
            std::cout << "New Camera added, put all profile =====" << std::endl;
            channel = QueuePutAll.front().channel;
            camera  = QueuePutAll.front().camera;
            jProfile.clear();

            pthread_mutex_lock(&lock);
            jProfile = database->getAllProfile(channel);
            pthread_mutex_unlock(&lock);

            if(getToken(client, camera))
            {
                for (auto profile : jProfile)
                {
                    profileid = profile["profileid"];
                    folderPath = FOLDER_PROFILES + std::string(profile["profileid"]) + "/";
                    getAllFileName(vtrImage, folderPath);
                    jImage.clear();
                    for(int i = 0; i < vtrImage.size(); i++){
                        std::string encodeStr = imageToBase64(folderPath + vtrImage[i]);
                        jImage.push_back(imageToBase64(folderPath + vtrImage[i]));
                        std::cout << "imagePath: " << folderPath + vtrImage[i] << std::endl;
                        std::cout << "encodeStr: " << encodeStr << std::endl;
                        std::cout << "jImage.size(): " << jImage.size() << std::endl;
                    }
                    object["id"] = profileid;
                    object["use"] = true;
                    object["description"] = "Added by Expander from TienVH with love";
                    object["profile_image"] = jImage;
                    url = "https://" + camera.ip + ":4004/api/setup/face_profile?access_token=" + camera.token;
                    ret = client->request(url, POST, object.dump(), response, 20);
                    std::cout << "Add profile: \"" << profileid << "\"" << std::endl;
                    std::cout << "Camera: " << camera.ip << std::endl;
                    std::cout << "Http_code: " << ret << ", response: " << response << std::endl;
                }
            }
            QueuePutAll.pop();
        }
        else
            sleep(1);
    }
    delete(client);
    pthread_mutex_destroy(&lock);
}

void SyncProfile::runPutOneThread()
{
    std::cout << "runPutOneThread ==== " << std::endl;
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string camIp, url, response;
    std::vector<camera_t> vtrCamera;
    json jCamIp, object;
    profile_t profile;
    camera_t camera;
    bool action;
    int channel;
    int ret;

    while(true)
    {
        pthread_mutex_lock(&lock);
        mode = database->getMode();
        pthread_mutex_unlock(&lock);
        if (mode != MODE_SERVER){
            sleep(5);
            continue;
        }
        if (QueuePutOne.size())
        {
            std::cout << "exist new profile to put ==== " << std::endl;
            channel = QueuePutOne.front().channel;
            profile = QueuePutOne.front().profile;
            action  = QueuePutOne.front().action;

            pthread_mutex_lock(&lock);
            database->getCameraIp(channel, camIp);
            pthread_mutex_unlock(&lock);

            jCamIp = json::parse(camIp);
            vtrCamera.clear();
            object.clear();

            for(auto cam : jCamIp)
            {
                camera.ip = cam["ip"];
                camera.username = cam["username"];
                camera.password = cam["password"];
                if(getToken(client, camera)){
                    vtrCamera.push_back(camera);
                }
            }
            for(int i = 0; i < vtrCamera.size(); i++)
            {
                if (action)
                {
                    object["id"] = profile.profileid;
                    object["use"] = true;
                    object["description"] = "Added by Expander from TienVH with love";
                    object["profile_image"] = json::parse(profile.images);
                    url = "https://" + vtrCamera[i].ip + ":4004/api/setup/face_profile?access_token=" + vtrCamera[i].token;
                    ret = client->request(url, POST, object.dump(), response, 20);
                    std::cout << "Add profile: \"" << profile.profileid << "\"" << std::endl;
                    std::cout << "Camera: " << vtrCamera[i].ip << std::endl;
                    std::cout << "Http_code: " << ret << ", response: " << response << std::endl;
                }
                else
                {
                    url = "https://" + vtrCamera[i].ip + ":4004/api/setup/face_profile/" + profile.profileid + "?access_token=" + vtrCamera[i].token;
                    ret = client->request(url, DELETE, object.dump(), response, 20);
                    std::cout << "Delete profile: \"" << profile.profileid << "\"" << std::endl;
                    std::cout << "Camera: " << vtrCamera[i].ip << std::endl;
                    std::cout << "Http_code: " << ret << ", response: " << response << std::endl;
                }
            }
            QueuePutOne.pop();
        }
        else
            sleep(1);
    }
    delete(client);
    pthread_mutex_destroy(&lock);
}

void SyncProfile::runGetAllThread()
{
    std::cout << "runGetAllThread ==== " << std::endl;
    HttpClient *client = new HttpClient();
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return;
    }
    std::string camIp, url, response;
    std::vector<camera_t> vtrCamera;
    json jCamIp, jProfile;
    profile_t profile;
    camera_t camera;
    int channel;
    int ret;

    while(true)
    {
        pthread_mutex_lock(&lock);
        mode = database->getMode();
        pthread_mutex_unlock(&lock);
        if (mode != MODE_PEER){
            sleep(5);
            continue;
        }
        for (channel = 1; channel <= MAX_CHANNEL; channel++)
        {
            pthread_mutex_lock(&lock);
            database->getCameraIp(channel, camIp);
            pthread_mutex_unlock(&lock);

            jCamIp = json::parse(camIp);
            camera = {};
            for(auto cam : jCamIp)
            {
                camera.ip = cam["ip"];
                camera.username = cam["username"];
                camera.password = cam["password"];
                if (getToken(client, camera)) break;
            }
            if (camera.token != std::string()){
                url = "https://" + camera.ip + ":4004/api/setup/face_profile?access_token=" + camera.token;
                ret = client->request(url, GET, "", response, 20);
                if (ret != 200){
                    std::cout << "Get all from profile fail, camera: " << camera.ip << ", http_code: " << ret << std::endl;
                    continue;
                }
                jProfile = json::parse(response);
                for (auto obj : jProfile)
                {
                    profile.profileid   = obj["id"];
                    profile.images      = "[]";
                    profile.schedule    = DEFAULT_SCHEDULE;
                    profile.status      = PROFILE_STATUS_ENABLE;
                    pthread_mutex_lock(&lock);
                    database->addProfile(profile, channel);
                    pthread_mutex_unlock(&lock);
                }
            }
        }
        sleep(5*60);
    }
}

static int getToken(HttpClient *client, camera_t &camera)
{
    json jContent, jResponse;
    jContent["username"] = camera.username;
    jContent["userpwd"] = camera.password;
    std::string response;
    std::string url = "https://" + camera.ip + ":4004/api/login";
    int ret = client->request(url, POST, jContent.dump(), response, 10);
    if (ret == 200)
    {
        try 
        {
            json jResponse = json::parse(response);
            camera.token = jResponse["data"]["access_token"];
            std::cout << "Get token successfully, Response " << camera.ip << ": \n" << response << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        return 1;
    }
    camera.token = std::string();
    std::cout << "get token fail, http_code: " << ret << ", response " << camera.ip << ": " << response << std::endl;
    return 0;
}

void getAllFileName(std::vector<std::string> &filenames, std::string folderPath)
{
    DIR *dir;
    struct dirent *ent;
    filenames.clear();
    if ((dir = opendir(folderPath.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            }
            filenames.push_back(std::string(ent->d_name));
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
    }
}

std::string imageToBase64(std::string imagePath)
{
    std::ifstream infile(imagePath, std::ifstream::binary);
    infile.seekg (0,infile.end);
    long size = infile.tellg();
    infile.seekg (0);
    char* buffer = new char[size];
    infile.read (buffer,size);
    return base64_encode((unsigned char*)buffer, size, false);
}