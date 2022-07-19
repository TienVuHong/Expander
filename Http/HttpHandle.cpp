/*
 * HttpHandle.cpp
 *
 *  Created on: 06/07/2021
 *      Author: tienvh
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/event_struct.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/bufferevent_struct.h>
#include <event2/bufferevent_compat.h>
#include <event2/tag.h>
#include <event2/tag_compat.h>

// #include <opencv2/opencv.hpp>
// #include <opencv4/opencv2/opencv.hpp>

#include "../service/datetime/datetime.hpp"
#include "../service/common/common.h"
#include "../service/base64/base64.h"
#include "../Authenticate/Authenticate.hpp"
#include "../Controller/Controller.hpp"
#include "../Database/Database.hpp"
#include "../EventManager/EventManager.hpp"
#include "../Log/Log.hpp"
#include "../json.h"
#include "../config.h"

#include "SyncProfile/SyncProfile.hpp"
#include "HttpClient.hpp"
#include "HttpForward.hpp"
#include "HttpHandle.hpp"

#define SEND_ERROR_RETURN(request, errCode, mess)   \
        {                                           \
            sendError(request, errCode, mess);      \
            return;                                 \
        }

static bool equal(const char *src, const char *dst);
static void sendError(struct evhttp_request *request, int status, const std::string &message);
static void sendJson(struct evhttp_request *request, int status, const json &j);
static void sendText(struct evhttp_request *request, int status, const std::string &text);
static void sendVideo(struct evhttp_request *request, int status, const std::string &videoPath);
static const json        readHeader(struct evhttp_request *request);
static const std::string readIpClient(struct evhttp_request *request);
static const std::string readContent(struct evhttp_request *request);
static const std::string readAuthHeader(struct evhttp_request *request);
static const std::string readMethod(struct evhttp_request *request);
static const std::string copyContent(struct evhttp_request *request);
static const std::string readMultipartMessage(struct evhttp_request *request);
static int readMultipartImage(struct evhttp_request *request);

static void changeDeviceIP(std::string ip, std::string gw, std::string dns);
static void reboot(void *arg);
void processProfile(struct evhttp_request *request, json jProfileArray, int channel);

Log             *logi;
Controller      *control;
Database        *database;
HttpClient      *client;
HttpForward     *forward;
Authenticate    *authenticate;
EventManager    *eventManager;
SyncProfile     *syncprofile;

void httpHandleInit()
{
    std::string cmd =   std::string("mkdir -v ") +
                        FOLDER_AIVEX + " " +
                        FOLDER_LOG + " " +
                        FOLDER_DATABASE + " " +
                        FOLDER_PROFILES + " " +
                        FOLDER_IMAGES;
    system(cmd.c_str());
    logi            = new Log();
    control         = new Controller();
    database        = new Database();
    client          = new HttpClient();
    authenticate    = new Authenticate();
    eventManager    = new EventManager();
    forward         = new HttpForward(database);
    syncprofile     = new SyncProfile(database);
}

static bool equal(const char *src, const char *dst) { return strcmp(src, dst) == 0; }

static const std::string readIpClient(struct evhttp_request *request)
{
    char *client_ip;
    u_short client_port;

    evhttp_connection_get_peer(evhttp_request_get_connection(request), &client_ip, &client_port);
    std::string ip(client_ip);
    return ip;
}

static const std::string copyContent(struct evhttp_request *request) {
    std::string content;
    struct evbuffer *buffer;
    int len = 0;
    buffer = evhttp_request_get_input_buffer(request);
    while (len < evbuffer_get_length(buffer)) {
        int n;
        char cbuf[1024];
        n = evbuffer_copyout(buffer, cbuf, sizeof(cbuf));
        if (n > 0) {
            content += std::string(cbuf, n);
            len += n;
        }
    }
    return content;
}

static const std::string readContent(struct evhttp_request *request)
{
    std::string content;
    struct evbuffer *buffer;
    buffer = evhttp_request_get_input_buffer(request);
    while (evbuffer_get_length(buffer) > 0)
    {
        int n;
        char cbuf[1024];
        n = evbuffer_remove(buffer, cbuf, sizeof(cbuf));
        if (n > 0)
        {
            content += std::string(cbuf, n);
        }
    }
    return content;
}

static const std::string readMultipartMessage(struct evhttp_request *request) {
    struct evbuffer *buffer;
    buffer = evhttp_request_get_input_buffer(request);
    std::string search = "{";
    struct evbuffer_ptr start = evbuffer_search(buffer, search.c_str(), search.size(), NULL);
    struct evbuffer_ptr end = evbuffer_search_eol(buffer, &start, NULL , EVBUFFER_EOL_ANY);
    size_t size =  end.pos - start.pos + 1;
    if (size > 0) {
        char data[size];
        ev_ssize_t readData =  evbuffer_copyout_from(buffer, &start, data, size);
        if (readData > 0) {
            return std::string((const char*)data);
        }
    }
    return std::string();
}

static int readMultipartImage(struct evhttp_request *request, char **data, size_t &size) {
    struct evbuffer *buffer;
    buffer = evhttp_request_get_input_buffer(request);
    std::string search = "Content-Type";
    struct evbuffer_ptr searchPtr =  evbuffer_search(buffer, search.c_str(), search.size(), NULL);
    struct evbuffer_ptr spacePtr = evbuffer_search_eol(buffer, &searchPtr, NULL , EVBUFFER_EOL_ANY);
    search = "----";
    struct evbuffer_ptr endPtr =  evbuffer_search(buffer, search.c_str(), search.size(), &searchPtr);
    evbuffer_ptr_set(buffer, &spacePtr, 4, EVBUFFER_PTR_ADD);
    size =  endPtr.pos - spacePtr.pos - 2;
    if (size > 0) {
        *data = (char*)malloc(size * sizeof(char));
        ev_ssize_t readData = evbuffer_copyout_from(buffer, &spacePtr, *data, size);
        if (readData > 0)
            return true;
    }
    return false;
}

std::string readMultipartContent(struct evhttp_request *request, const char *imageid) 
{
    struct evbuffer *buffer;
    std::string header;
    std::string filepath = std::string();
    buffer = evhttp_request_get_input_buffer(request);
    std::string search = "Content-Type";
    struct evbuffer_ptr searchPtr =  evbuffer_search(buffer, search.c_str(), search.size(), NULL);
    struct evbuffer_ptr spacePtr = evbuffer_search_eol(buffer, &searchPtr, NULL , EVBUFFER_EOL_ANY);
    search = "----";
    struct evbuffer_ptr endPtr =  evbuffer_search(buffer, search.c_str(), search.size(), &searchPtr);
    evbuffer_ptr_set(buffer, &spacePtr, 4, EVBUFFER_PTR_ADD);
    char cbuf[1024];
    ev_ssize_t testData =  evbuffer_copyout_from(buffer, &searchPtr, cbuf, (spacePtr.pos - searchPtr.pos));
    
    if (testData > 0) {
        header += std::string(cbuf, testData);
    }
    size_t fileSize =  endPtr.pos - spacePtr.pos - 2;
    std::cout << "Header : " << header << std::endl;
    
    if (fileSize > 0) {
        uint8_t* data = (uint8_t*)malloc(fileSize);
        ev_ssize_t readData =  evbuffer_copyout_from(buffer, &spacePtr, data, fileSize);
        if (readData > 0) {
            if (header.find("image") != std::string::npos)
                filepath = FOLDER_IMAGES + IMAGE_NAME(imageid);
            else
                filepath = FOLDER_IMAGES + VIDEO_NAME(imageid);
            
            FILE * pFile;
            pFile = fopen(filepath.c_str(), "w+b");
            fwrite (data, 1, readData, pFile);
            fclose (pFile);
        }
        free(data);
    }
    return filepath;
}   

static const json readHeader(struct evhttp_request *request)
{
    struct evkeyvalq *header = evhttp_request_get_input_headers(request);
    struct evkeyval *kv = header->tqh_first;
    json jHeader = json::array();
    std::string str;
    while(kv)
    {
        // printf("{key:%s, value:%s}\n", kv->key, kv->value);
        str = std::string(kv->key) + ": " + std::string(kv->value);
        jHeader.push_back(str);
        kv = kv->next.tqe_next;
    }
    return jHeader;
}

static const std::string readAuthHeader(struct evhttp_request *request)
{
    struct evkeyvalq *header = evhttp_request_get_input_headers(request);
    if (evhttp_find_header(header, "Authorization") != NULL)
    {
        std::string authorization =
            std::string(evhttp_find_header(header, "Authorization"));
        std::string delimiter = " ";
        std::vector<std::string> authorizationInfor = splitString(authorization, delimiter);
        return authorizationInfor[1];
    }
    else
    {
        return std::string();
    }
}

static const std::string readMethod(struct evhttp_request *request)
{
    std::string cmdType;
    switch (evhttp_request_get_command(request))
    {
        case EVHTTP_REQ_GET:
            cmdType = "GET";
            break;
        case EVHTTP_REQ_POST:
            cmdType = "POST";
            break;
        case EVHTTP_REQ_HEAD:
            cmdType = "HEAD";
            break;
        case EVHTTP_REQ_PUT:
            cmdType = "PUT";
            break;
        case EVHTTP_REQ_DELETE:
            cmdType = "DELETE";
            break;
        case EVHTTP_REQ_OPTIONS:
            cmdType = "OPTIONS";
            break;
        case EVHTTP_REQ_TRACE:
            cmdType = "TRACE";
            break;
        case EVHTTP_REQ_CONNECT:
            cmdType = "CONNECT";
            break;
        case EVHTTP_REQ_PATCH:
            cmdType = "PATCH";
            break;
        default:
            cmdType = "unknown";
            break;
    }
    return cmdType;
}

void sendError(struct evhttp_request *request, int status, const std::string &message)
{
    json j;
    j["error"] = true;
    j["detail"] = message;
    sendJson(request, status, j);
}

void sendJson(struct evhttp_request *request, int status, const json &j)
{
    struct evbuffer *buffer;
    std::string content;
    std::string header;
    buffer = evbuffer_new();
    content = j.dump();
    evbuffer_add(buffer, content.data(), content.size());

    evhttp_add_header(request->output_headers, "Content-Type",
                      "application/json");
    evhttp_add_header(request->output_headers, "Content-Length",
                      std::to_string(content.size()).c_str());
    evhttp_add_header(request->output_headers, "Access-Control-Allow-Origin",
                      "*");
    evhttp_send_reply(request, status, NULL, buffer);
    evbuffer_free(buffer);
}

void sendText(struct evhttp_request *request, int status, const std::string &text)
{
    struct evbuffer *buffer;
    std::string header;
    buffer = evbuffer_new();
    evbuffer_add(buffer, text.data(), text.size());

    evhttp_add_header(request->output_headers, "Content-Type",
                      "text/plain; charset=us-ascii");
    evhttp_add_header(request->output_headers, "Content-Length",
                      std::to_string(text.size()).c_str());
    evhttp_add_header(request->output_headers, "Access-Control-Allow-Origin",
                    "*");
    evhttp_send_reply(request, status, NULL, buffer);
    evbuffer_free(buffer);
}

void sendVideo(struct evhttp_request *request, int status, const std::string &videoPath)
{
    struct evbuffer *buffer;
    std::string header;
    buffer = evbuffer_new();

    char *buff;  // buffer to store file contents
    long size;   // file size
    std::ifstream file(
        videoPath,
        std::ios::in | std::ios::binary |
            std::ios::ate);  // open file in binary mode, get pointer at the end
                             // of the file (ios::ate)
    size = file.tellg();     // retrieve get pointer position
    file.seekg(
        0, std::ios::beg);  // position get pointer at the begining of the file
    buff = new char[size];  // initialize the buffer
    file.read(buff, size);  // read file to buffer
    file.close();

    evbuffer_add(buffer, buff, size);
    evhttp_add_header(request->output_headers, "Content-Type", "video/mp4");
    evhttp_add_header(request->output_headers, "Content-Length",
                      std::to_string(size).c_str());

    evhttp_send_reply(request, status, NULL, buffer);
    evbuffer_free(buffer);
}

static void reboot(void *arg)
{
#ifdef RASPBERRY_PI
    system("echo \"1\" | sudo -S -k reboot");
#else
    system("echo \"123\" | sudo -S -k reboot");
#endif
}

static void changeDeviceIP(std::string ip, std::string gw, std::string dns)
{
#ifdef RASPBERRY_PI
    std::ofstream fout("/home/pi/temp.conf");
    std::ifstream fin("/etc/dhcpcd.conf");
    std::string line;
    std::cout << "changeIP" << std::endl;
    if (fin.is_open())
    {
        while ( getline(fin, line) )
        {
            std::cout << line << std::endl;
            fout << line << std::endl;
            if (line == "# Expander IP")
            {
                std::cout << "Changing ... " << std::endl;
                std::cout << fin.tellg() << std::endl;
                fout << "interface eth0" << std::endl;
                fout << "static ip_address=" + ip << std::endl;
                fout << "static routers=" + gw << std::endl;
                std::string dnsString = (dns == "") ? "static domain_name_servers=8.8.8.8 8.8.4.4" : "static domain_name_servers=" + dns;
                fout << dnsString << std::endl;
                break;
            }
        }
        fin.close();
        fout.close();
        system("cp /home/pi/temp.conf /etc/dhcpcd.conf");
        system("rm /home/pi/temp.conf");
    }
    else
        std::cout << "unable to open file" << std::endl;
#else
    std::ofstream fout("dhcpcd1.conf");
    std::ifstream fin("/media/tienvh/Workspace/1.AIView/AIVEX/source/trunk/dhcpcd.conf");
    std::string line;
    std::cout << "changeIP" << std::endl;
    if (fin.is_open())
    {
        while ( getline(fin, line) )
        {
            std::cout << line << std::endl;
            fout << line << std::endl;
            if (line == "# Expander IP")
            {
                std::cout << "Changing ... " << std::endl;
                std::cout << fin.tellg() << std::endl;
                fout << "interface eth0" << std::endl;
                fout << "static ip_address=" + ip << std::endl;
                fout << "static routers=" + gw << std::endl;
                std::string dnsString = (dns == "") ? "static domain_name_servers=8.8.8.8 8.8.4.4" : "static domain_name_servers=" + dns;
                fout << dnsString << std::endl;
                break;
            }
        }

        fin.close();
        fout.close();
        system("cp dhcpcd1.conf /media/tienvh/Workspace/1.AIView/AIVEX/source/trunk/dhcpcd.conf");
        system("rm dhcpcd1.conf");
    }
    else 
        std::cout << "unable to open file" << std::endl;
#endif
}

void apiNetworkInfo(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "GET")
        {
            json jInfo;
            jInfo["IpAdress"] = getCmdResult("hostname -I | cut -d ' ' -f1");
            jInfo["NetMask"] = getCmdResult("ifconfig | grep 'broadcast'| grep 'netmask' | cut -d' ' -f13");
            jInfo["Gateway"] = getCmdResult("ip route | grep 'default via' | cut -d ' ' -f3");
            jInfo["Broadcast"] = getCmdResult("ifconfig | grep 'broadcast'| grep 'netmask' | cut -d' ' -f16");
            jInfo["MAC"] = getCmdResult("ifconfig | grep 'ether' | cut -d' ' -f10");

            // jInfo["NetMask"] = getCmdResult(R"(ifconfig | grep 'broadcast' | cut -d$'\n' -f1 | cut -d' ' -f13)");
            // jInfo["Gateway"] = getCmdResult(R"(ip route | grep 'eth0' | cut -d$'\n' -f1 | cut -d' ' -f3)");
            // jInfo["Broadcast"] = getCmdResult(R"(ifconfig | grep 'broadcast'| grep 'netmask' | cut -d$'\n' -f1 | cut -d' ' -f16)");
            // jInfo["MAC"] = getCmdResult(R"(ifconfig | grep 'ether' | cut -d$'\n' -f1 | cut -d' ' -f10)");

            std::string dnsString = getCmdResult("cat /etc/resolv.conf | grep 'nameserver' | awk '{print $2}'");
            std::cout << "dnsString = " << dnsString << std::endl;
            std::vector<std::string> DNS = splitString(dnsString, "\n");
            if (DNS.size() < 1)
            {
                jInfo["DNS1"] = "";
                jInfo["DNS2"] = "";
            }
            else if (DNS.size() == 1)
            {
                jInfo["DNS1"] = DNS[0];
                jInfo["DNS2"] = "";
            }
            else
            {
                jInfo["DNS1"] = DNS[0];
                jInfo["DNS2"] = DNS[1];
            }

            sendJson(request, 200, jInfo);
            logi->writeLog(readIpClient(request), readMethod(request), "/api/network/info", "GET network info");
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiNetworkInfo()");
        std::cerr << e.what() << '\n';
    }
}

void apiDeviceInfo(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "GET")
        {
            json jInfo;
            jInfo["DeviceName"] = "AIView Expander";
            jInfo["OsVersion"] = getCmdResult("hostnamectl | grep 'Operating System' | cut -d':' -f2");
            jInfo["SwVersion"] = SOFTWARE_VERSION;
            jInfo["HwVersion"] = HARDWARE_VERSION;

            sendJson(request, 200, jInfo);
            logi->writeLog(readIpClient(request), readMethod(request), "/api/device/info", "GET device info");
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiDeviceInfo()");
        std::cerr << e.what() << '\n';
    }
}

void apiLogin(struct evhttp_request *request)
{
    try 
    {
        if (readMethod(request) == "POST")
        {
            std::string loginInfo = readContent(request);
            auto jInfo = json::parse(loginInfo);
            if (jInfo.contains("user") && jInfo.contains("password"))
            {
                std::string userID = jInfo["user"];
                std::string pass = jInfo["password"];
                if (database->checkLoginInfo(userID, pass))
                {
                    std::string tokenGen = authenticate->tokenGenerate(userID, pass);
                    json jRespondToken;
                    jRespondToken["user"] = userID;
                    jRespondToken["token"] = tokenGen;
                    sendJson(request, 200, jRespondToken);
                    logi->writeLog(readIpClient(request), readMethod(request), "/api/login", "LOGIN user: " + userID);
                }
                else
                {
                    sendError(request, 400, "User or password is incorrect");
                }
            }
            else
            {
                sendError(request, 400, "Insert user and password to login");
            }
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch (const std::exception &e)
    {
        sendError(request, 500, "Error in apiLogin()");
        std::cerr << e.what() << '\n';
    }
}

void apiUser(struct evhttp_request *request)
{
    try 
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto jUser = json::parse(content);
            int state;
            if (jUser.contains("user") && jUser.contains("password") && jUser.contains("status"))
            {
                int status = jUser["status"];
                std::string userId = jUser["user"];
                std::string pass = jUser["password"];
                if (status == 1)
                {
                    state = database->addUser(userId, pass);
                    if (state)
                    {
                        std::string token = authenticate->tokenGenerate(userId, pass);
                        json jRespondToken;
                        jRespondToken["user"] = userId;
                        jRespondToken["token"] = token;
                        sendJson(request, 200, jRespondToken);
                        logi->writeLog(readIpClient(request), readMethod(request), "/api/user", "ADD: " + userId);
                    }
                    else
                    {
                        sendError(request, 400, "Username already exists.");
                    }
                }
                else if (status == -1)
                {
                    state = database->removeUser(userId);
                    if ( state )
                    {
                        sendJson(request, 200, "User removed successfully.");
                        logi->writeLog(readIpClient(request), readMethod(request), "/api/user", "REMOVE: " + userId);
                    }
                    else
                    {
                        sendError(request, 400, "Username could not be found");
                    }
                }
                else if (status == 0)
                {
                    if (jUser.contains("passwordNew"))
                    {
                        std::string passOld = jUser["password"];
                        std::string passNew = jUser["passwordNew"];
                        state = database->changePassword(userId, passOld, passNew);
                        if ( state )
                        {
                            std::string token = authenticate->tokenGenerate(userId, passNew);
                            json jRespondToken;
                            jRespondToken["user"] = userId;
                            jRespondToken["token"] = token;
                            sendJson(request, 200, jRespondToken);
                            logi->writeLog(readIpClient(request), readMethod(request), "/api/user", "CHANGE password: " + userId);
                        }
                        else
                        {
                            sendError(request, 400, "Username or password is incorrect.");
                        }
                    }
                    else
                    {
                        sendError(request, 400, "Insert passwordNew to change passowrd");
                    }
                }
                else
                {
                    sendError(request, 400, "status is not valid.");
                }
            }
            else
            {
                sendError(request, 400, "Insert user, password and status to do with user.");
            }
        }
        else if (readMethod(request) == "GET")
        {

        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch (const std::exception &e)
    {
        sendError(request, 500, "Error in apiUser()");
        std::cerr << e.what() << '\n';
    }
}

void apiIp(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto jEvent = json::parse(content);
            if (jEvent.contains("ip") && jEvent.contains("gw") && jEvent.contains("dns1") && jEvent.contains("dns2"))
            {
                std::string ip = jEvent["ip"];
                std::string gw = jEvent["gw"];
                std::string dns1 = jEvent["dns1"];
                std::string dns2 = jEvent["dns2"];
                std::cout << ip << std::endl;
                std::cout << gw << std::endl;
                std::cout << dns1 << std::endl;
                std::cout << dns2 << std::endl;
                std::string dns = "";
                if (dns1 != "") dns += dns1;
                if (dns2 != "") dns += " " + dns2;
                std::cout << dns << std::endl;
                if (ip != "" && gw != "")
                {
                    changeDeviceIP(ip, gw, dns);
                    sendJson(request, 200, "change ip successfully.");
                    start_timer(4000, reboot, NULL);
                }
                else
                    sendError(request, 400, "insert correct format of ip and gw");
                logi->writeLog(readIpClient(request), readMethod(request), "/api/ip", "CHANGE ip: " + ip + " gw: " + gw + " dns: " + dns);
            }
            else
            {
                sendError(request, 400, "Insert ip, gw, dns1 and dns2");
            }
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch (const std::exception &e)
    {
        sendError(request, 500, "Error in apiIp()");
        std::cerr << e.what() << '\n';
    }
}

void apiServer(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto j = json::parse(content);
            if (j.contains("use") && j.contains("url") && j.contains("token"))
            {
                server_t server = {j["use"], j["url"], j["token"]};
                int state = database->changeServer(server);
                if (state)
                {
                    sendJson(request, 200, "Change Server IP successfully");
                    logi->writeLog(readIpClient(request), readMethod(request), "/api/server", "CHANGE successfully: use: " + 
                    std::to_string(server.use) + ", url: '" + 
                    server.url + "'");
                }
                else
                {
                    sendJson(request, 400, "Error while change Server IP");
                    logi->writeLog(readIpClient(request), readMethod(request), "/api/server", "CHANGE Server IP Error: use: " + 
                    std::to_string(server.use) + ", url: '" + 
                    server.url + "'");
                }
            }
            else
            {
                sendError(request, 400, "Insert ip and port");
            }
        }
        else if (readMethod(request) == "GET")
        {
            json jServer;
            server_t server = database->getServer();
            jServer["use"] = server.use;
            jServer["url"] = server.url;
            jServer["token"] = server.token;
            sendJson(request, 200, jServer);
            logi->writeLog(readIpClient(request), readMethod(request), "/api/server", "GET server url");
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        sendError(request, 400, "api_server_ip.fail.clients");
    }
}

void apiReset(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            start_timer(4000, reboot, NULL);
            logi->writeLog(readIpClient(request), readMethod(request), "/api/reset", "REBOOT");
            sendJson(request, 200, "Reboot successfully");
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiReset()");
        std::cerr << e.what() << '\n';
    }
}

void apiConfig(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto jConfig = json::parse(content);
            if (jConfig.contains("channel") && jConfig.contains("action"))
            {
                int channel = jConfig["channel"];
                int action  = jConfig["action"];
                if (channel < 1 || channel > 3)
                {
                    sendError(request, 400, "channel must be 1,2 or 3");
                    return;
                }

                /*
                *   check action
                */
                switch (action)
                {
                    case 0: // change time open
                    {
                        if (jConfig.contains("timeopen") && 
                            jConfig["timeopen"].is_number_integer())
                        {
                            int timeOpen = jConfig["timeopen"];
                            database->changeTimeOpen(channel, timeOpen);
                            sendJson(request, 200, "Change timeopen successfully");
                            logi->writeLog(
                                readIpClient(request),
                                readMethod(request),
                                "/api/config",
                                "CHANGE timeopen, channel = " + std::to_string(channel) + ", timeopen = " + std::to_string(timeOpen)
                            );
                        }
                        else
                        {
                            sendError(request, 400, "Insert timeopen");
                        }
                        break;
                    }
                    case 1: // get time open
                    {
                        json jTimeOpen;
                        int timeOpen = database->getTimeOpen(channel);
                        jTimeOpen["timeopen"] = timeOpen;
                        sendJson(request, 200, jTimeOpen);
                        logi->writeLog(
                            readIpClient(request),
                            readMethod(request),
                            "/api/config",
                            "GET timeopen, channel = " + std::to_string(channel) + ", timeopen = " + std::to_string(timeOpen)
                        );
                        break;
                    }
                    case 2: // change type event
                    {
                        if (jConfig.contains("typeevent") && 
                            jConfig["typeevent"].is_number_integer())
                        {
                            int typeEvent = jConfig["typeevent"];
                            database->changeTypeEvent(channel, typeEvent);
                            sendJson(request, 200, "Change typeevent successfully");
                            logi->writeLog(
                                readIpClient(request),
                                readMethod(request),
                                "/api/config",
                                "CHANGE typeevent, channel = " + std::to_string(channel) + ", typeevent = " + std::to_string(typeEvent)
                            );
                        }
                        else
                        {
                            sendError(request, 400, "Insert typeevent");
                        }
                        break;
                    }     
                    case 3: // get type event
                    {
                        json jTypeEvent;
                        int typeEvent = database->getTypeEvent(channel);
                        jTypeEvent["typeevent"] = typeEvent;
                        sendJson(request, 200, jTypeEvent);
                        logi->writeLog(
                            readIpClient(request),
                            readMethod(request),
                            "/api/config",
                            "GET typeevent, channel = " + std::to_string(channel) + ", typeevent = " + std::to_string(typeEvent)
                        );
                        break;
                    }
                    case 4: // change camera ip
                    {
                        if (jConfig.contains("cameraip") && 
                            jConfig["cameraip"].is_array())
                        {
                            std::string oldCameraIp;
                            database->getCameraIp(channel, oldCameraIp);
                            json jOldCameraIp = json::parse(oldCameraIp);
                            camera_t newCameraIp;
                            json jNewCameraIp;
                            if (jOldCameraIp.size() < jConfig["cameraip"].size()){
                                for (auto newCam : jConfig["cameraip"]){
                                    jNewCameraIp = newCam;
                                }
                                newCameraIp.ip = jNewCameraIp["ip"];
                                newCameraIp.username = jNewCameraIp["username"];
                                newCameraIp.password = jNewCameraIp["password"];
                                syncprofile->putAllProfile(channel, newCameraIp);
                            }
                            std::string cameraIp = jConfig["cameraip"].dump();
                            int ret = database->changeCameraIp(channel, cameraIp);
                            if (!ret)
                                sendJson(request, 400, "Change camera ip error");
                            else
                                sendJson(request, 200, "Change camera ip successfully");
                            logi->writeLog(
                                readIpClient(request),
                                readMethod(request),
                                "/api/config",
                                "CHANGE camera ip, channel = " + std::to_string(channel) + ", cameraip = " + cameraIp
                            );
                        }
                        else
                        {
                            sendError(request, 400, "Insert cameraip");
                        }
                        break;
                    }
                    case 5: // get camera ip
                    {
                        json jCameraIp;
                        std::string cameraIp;
                        database->getCameraIp(channel, cameraIp);
                        jCameraIp["cameraip"] = json::parse(cameraIp);
                        sendJson(request, 200, jCameraIp);
                        logi->writeLog(
                            readIpClient(request),
                            readMethod(request),
                            "/api/config",
                            "GET cameraip, channel = " + std::to_string(channel) + ", cameraip = " + cameraIp
                        );
                        break;
                    }
                    default:
                    {
                        sendError(request, 400, "action not support");
                    }
                }
            }
            else
            {
                sendError(request, 400, "Insert channel and action");
            }
        }
        else
        {
            sendError(request, 400, "Wrong method");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiConfig()");
        std::cerr << e.what() << '\n';
    }
}

void apiDeviceConfig(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto jConfig = json::parse(content);
            if (jConfig.contains("mode"))
            {
                if (jConfig["mode"] >= MODE_SERVER && jConfig["mode"] <= MODE_PEER)
                {
                    database->changeMode(jConfig["mode"]);
                    syncprofile->mode = jConfig["mode"];
                    sendJson(request, 200, "Change mode successfully");
                    logi->writeLog(
                        readIpClient(request),
                        readMethod(request),
                        "/api/deviceconfig",
                        "CHANGE mode = " + std::to_string((int)jConfig["mode"])
                    );
                }
                else
                {
                    sendError(request, 400, "not support mode");
                }
            }
            else
            {
                sendError(request, 400, "insert mode");
            }
        }
        else if (readMethod(request) == "GET")
        {
            json object;
            object["mode"] = database->getMode();
            sendJson(request, 200, object);
            logi->writeLog(
                readIpClient(request),
                readMethod(request),
                "/api/deviceconfig",
                "GET mode = " + std::to_string((int)object["mode"])
            );
        }
        else
        {
            sendError(request, 400, "Wrong request method");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void apiOpenDevice(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string tokenCheck = readAuthHeader(request);
            int loginStatus = authenticate->loginAuthen(tokenCheck);
            if (!tokenCheck.empty())
            {
                if (loginStatus == 0)
                {
                    std::string content = readContent(request);
                    auto j = json::parse(content);
                    if (j.contains("deviceid") && j.contains("open"))
                    {
                        int deviceid;
                        int open;
                        REQUIRED(j, deviceid);
                        REQUIRED(j, open);
                        if (deviceid >= 1 && deviceid <= 3)
                        {
                            control->ControlOpenDeviceDelay(deviceid, database->getTimeOpen(deviceid));
                            // control->ControlChannel(deviceid, open);
                            sendJson(request, 200, "Allow access");
                            logi->writeLog(
                            readIpClient(request), readMethod(request),
                            "/api/opendevice",
                            "OPEN deviceid:" + std::to_string(deviceid) + " - value:" + std::to_string(open));
                        }
                        else 
                        {
                            sendJson(request, 200, "Block access - wrong deviceid");
                            logi->writeLog(
                            readIpClient(request), readMethod(request),
                            "/api/opendevice",
                            "BLOCK deviceid:" + std::to_string(deviceid) + " - value:" + std::to_string(open));
                        }
                    }
                    else
                    {
                        sendError(request, 400, "Insert deviceid and open.");
                    }
                }
                else
                {
                    sendError(request, 400, "Login error. Please try again.");
                }
            }
            else
            {
                sendError(request, 400, "The request must have the authorization");
            }
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiOpenDevice()");
        std::cerr << e.what() << '\n';
    }
}

json param2json(const char *param)
{
    if (param == NULL)
        return json::object();
    std::vector<std::string> vtrParam = splitString(std::string(param), "&");
    std::vector<std::string> vtrTemp;
    json jParam = {};
    for (std::string element : vtrParam)
    {
        vtrTemp.clear();
        vtrTemp = splitString(element, "=");
        jParam[vtrTemp[0]] = vtrTemp[1];
    }
    return jParam;
}

void apiDatabaseEvent(struct evhttp_request *request, const char *param)
{
    try
    {
        if (readMethod(request) == "GET")
        {
            json jCondition, jEvent;
            jCondition = param2json(param);
            std::cout << "jParam: " << jCondition.dump() << std::endl;
            eventManager->get(jEvent, jCondition);
            sendJson(request, 200, jEvent);
        }
        else
        {
            sendError(request, 400, "Wrong request method.");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiDatabaseEvent()");
        std::cerr << e.what() << '\n';
    }
    
}

void apiDatabase(struct evhttp_request *request)
{
    try
    {
        if (readMethod(request) == "POST")
        {
            std::string content = readContent(request);
            auto j = json::parse(content);
            int state = 0;
            if (j.contains("profileid") && j.contains("channel") && j.contains("action"))
            {
                std::string profileid = j["profileid"];
                int channel = j["channel"];
                int action = j["action"];
                if (channel <= 0 || channel > 3)
                    SEND_ERROR_RETURN(request, 400, "Channel must be 1,2 or 3");

                switch (action)
                {
                    case 1: // Add profile
                        if (
                            j.contains("schedule")  && j["schedule"].is_array() && 
                            j.contains("images")    && j["images"].is_array()   &&
                            j.contains("status")    && j["status"].is_number_integer()
                        )
                        {
                            profile_t profile;
                            profile.profileid   = profileid;
                            profile.status      = j["status"];
                            profile.schedule    = j["schedule"].dump();
                            profile.images      = j["images"].dump();
                            state = database->addProfile(profile, channel);
                            if (state)
                            {
                                sendJson(request, 200,"Add profile successfully.");
                                syncprofile->putProfile(channel, profile);
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                                    "Add profileid: " + profileid + 
                                    " ,status: " + std::to_string(profile.status) +
                                    " ,schedule: " + profile.schedule +
                                    " ,images: " + profile.images +
                                    " ,channel " + std::to_string(channel));
                            }
                            else
                            {
                                sendError(request, 400,"Profile has already existed.");
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database", "ADD Profile has already existed: " + profileid);
                            }
                        }
                        else
                        {
                            sendError(request,  400, "Insert correct schedule, images, status");
                        }
                        break;
                    case -1: // Remove profile
                        state = database->removeProfile(profileid, channel);
                        if (state)
                        {
                            sendJson(request, 200,"Remove profile successfully.");
                            syncprofile->delProfile(channel, profileid);
                            logi->writeLog(readIpClient(request), readMethod(request),"/api/database", "REMOVE profile: " + profileid + ", channel " + std::to_string(channel));
                        }
                        else 
                        {
                            sendError(request, 400,"There is no such profile");
                            logi->writeLog(readIpClient(request), readMethod(request),"/api/database", "REMOVE profile: " + profileid + "doesn't exist");
                        }
                        break;
                    case 0: // Get all profile
                    {
                        json jFaceArray = database->getAllProfile(channel);
                        sendJson(request, 200, jFaceArray);
                        logi->writeLog(readIpClient(request), readMethod(request),"/api/database", "GET all profile: " + jFaceArray.dump());
                        break;
                    }
                    case -2: //delete all profile
                    {
                        int ret = database->deleteAllProfile(channel);
                        sendJson(request, (400 - ret * 200), "DELETE all profile channel " + std::to_string(channel) + (ret ? " successfully" : " Error"));
                        logi->writeLog(readIpClient(request), 
                                        readMethod(request),
                                        "/api/database",
                                        "DELETE all profile channel " + std::to_string(channel) + (ret ? " successfully" : " Error"));
                    }
                    case 2: // Change schedule of profile
                        if (j.contains("schedule") && j["schedule"].is_array())
                        {
                            profile_t profile;
                            profile.profileid = profileid;
                            profile.schedule = j["schedule"].dump();
                            state = database->changeProfileSchedule(profile, channel);
                            if (state)
                            {
                                sendJson(request, 200,"Change profile schedule successfully.");
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                                    "Change " + profileid + 
                                    ", schedule: " + profile.schedule +
                                    ", channel " + std::to_string(channel));
                            }
                            else 
                            {
                                sendError(request, 400,"Change profile schedule fail");
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                                    "Change profile schedule: " + profileid + 
                                    " fail (profileid unfoundable or error in reading Database)");
                            }
                        }
                        else
                        {
                            sendError(request,  400, "Insert correct schedule");
                        }
                        break;
                    case 3: // Get schedule of profile
                    {
                        json jSchedule = database->getProfileSchedule(profileid, channel);
                        if (!jSchedule.is_null())
                        {
                            sendJson(request, 200, jSchedule);
                        }
                        else
                        {
                            sendError(request, 400, "Not found profileid.");
                        }
                        logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                            "Get schedule from " + profileid + " : " + jSchedule.dump());
                        break;
                    }
                    case 4: // Change profle status
                    {
                        if (j.contains("status") && j["status"].is_number_integer())
                        {
                            profile_t profile;
                            profile.profileid = profileid;
                            profile.status = j["status"];
                            if (profile.status < 0 || profile.status > 2)
                                SEND_ERROR_RETURN(request, 400, "status must be 0, 1 or 2");

                            state = database->changeProfileStatus(profile, channel);
                            if (state)
                            {
                                sendJson(request, 200,"Change profile status successfully.");
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                                    "Change " + profileid + 
                                    ", status: " + std::to_string(profile.status) +
                                    ", channel " + std::to_string(channel));
                            }
                            else 
                            {
                                sendError(request, 400,"Change profile status fail");
                                logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                                    "Change profile status: " + profileid + " fail");
                            }
                        }
                        else
                        {
                            sendError(request,  400, "Insert correct status");
                        }
                        break;
                    }
                    case 5: // Get profile status
                    {
                        json jStatus;
                        int status = database->getProfileStatus(profileid, channel);
                        if (status > 0)
                        {
                            jStatus["status"] = status;
                            sendJson(request, 200, jStatus);
                        }
                        else
                        {
                            sendError(request, 400, "Read profile status fail");
                        }
                        logi->writeLog(readIpClient(request), readMethod(request),"/api/database",
                            "Get status from " + profileid + " : " + std::to_string(status));
                        break;
                    }
                    default:
                        sendError(request, 400, "Wrong action");
                }
            }
            else
            {
                sendError(request,  400, "Insert profileid, channel and action");
            }
        }
        else
        {
            sendError(request, 400, "Wrong request method");
        }
    }
    catch(const std::exception& e)
    {
        sendError(request, 500, "Error in apiDatabase()");
        std::cerr << e.what() << '\n';
    }
}

void processProfile(struct evhttp_request *request, json jProfileArray, int channel)
{

}

void cameraOpendoorHandle(struct evhttp_request *request, int channel)
{
    try
    {
        int allow;
        std::string IDAccept;
        std::string IDToLog;

        datetime_t currentTime  = getDatetime();
        int timeInMinute        = daytimeToMinute(currentTime.hour, currentTime.minute);
        
        std::string date        = date2string(currentTime.year, currentTime.month, currentTime.date);
        std::string time        = time2string(currentTime.hour, currentTime.minute, currentTime.second);
        std::string type        = (database->getTypeEvent(channel) == FACE_RECOGNITION) ? "facerecognition" : "license";
        std::string content     = readContent(request);
        std::string camera      = readIpClient(request);
        nlohmann::json headers  = readHeader(request);

        json jIDArray = json::array();
        json jContent = json::parse(content);
        std::cout << "content opendoor: " << content << std::endl;
        if (!jContent.contains("type")) SEND_ERROR_RETURN(request, 400, "type not found");

        if (jContent["type"] == "facerecognition")
        {
            std::cout << "facerecognition" << std::endl;
            if (!jContent.contains("profileId")) SEND_ERROR_RETURN(request, 400, "Type: facerecognition, profileId not found");
            jIDArray = jContent["profileId"];
        }
        else if (jContent["type"] == "license")
        {
            std::cout << "license" << std::endl;
            if (!jContent.contains("license")) SEND_ERROR_RETURN(request, 400, "Type: license, license not found");
            jIDArray = jContent["license"];
        }
        else if (jContent["type"] == "vehicledetection")
        {
            std::cout << "vehicle" << std::endl;
            if (!jContent.contains("object_type")) SEND_ERROR_RETURN(request, 400, "object_type not found");
            if (jContent["object_type"] == "car")
            {
                control->ControlOpenDeviceDelay(channel, database->getTimeOpen(channel));
                logi->writeLog(readIpClient(request), readMethod(request),"/opendoor/" + std::to_string(channel), std::string(jContent["object_type"]) + " >>>");
                sendJson(request, 200, "Car");
                return;
            }
            else
            {
                logi->writeLog(readIpClient(request), readMethod(request),"/opendoor/" + std::to_string(channel), std::string(jContent["object_type"]) + " Not Car >>>");
                sendJson(request, 200, "Not car");
                return;
            }
            if (!jContent.contains("license")) SEND_ERROR_RETURN(request, 400, "license not found");
            jIDArray = jContent["license"];
        }

        for (std::string id : jIDArray)
        {
            allow = 0;
            std::cout << "--------------------------> " << id << ", channel: " << channel;
            IDToLog += id + " ";
            if (database->hasProfileExisted(id, channel) || (id == "0"))
            {
                std::cout << " - Existed, ";

                // Check status
                int status = database->getProfileStatus(id, channel);
                if (status == PROFILE_STATUS_DISABLE)
                {
                    std::cout << "Disable --> X" << std::endl;
                    return;
                }
                else if (status == PROFILE_STATUS_ENABLE)
                {
                    std::cout << "Enable ";
                    allow = 1;
                }
                else if (status == PROFILE_STATUS_SCHEDULE)
                {
                    // Check schedule
                    std::cout << "Schedule: ";
                    json jSchedule = database->getProfileSchedule(id, channel);
                    if (!jSchedule.is_array() || !jSchedule.size())
                    {
                        std::cout << "emtpy --> X" << std::endl;
                        return;
                    }

                    for (auto jDayObj : jSchedule)
                    {
                        if (jDayObj["day"] != currentTime.day)
                            continue;
                        std::cout << "day:" << jDayObj["day"] << ", ";
                        std::cout << "now:" << timeInMinute << "-";
                        for (auto timeSlot : jDayObj["timeslots"])
                        {
                            std::cout << "[" << timeSlot["allowedfrom"] << ":" << timeSlot["alloweduntil"] << "] ";
                            if (timeSlot["allowedfrom"] <= timeInMinute && timeInMinute < timeSlot["alloweduntil"])
                            {
                                allow = 1;
                                break;
                            }
                        }
                    }
                }
                // Check if allowed
                if (allow)
                {
                    std::cout << "--> Accept" << std::endl;
                    control->ControlOpenDeviceDelay(channel, database->getTimeOpen(channel));
                    IDAccept += id + " ";
                }
                else
                {
                    std::cout << "--> X" << std::endl;
                }
            }
            else std::cout << " --> X" << std::endl;
        }
        sendJson(request, 200, IDToLog + " - ID are accepted: " + ((IDAccept == "") ? "None" : IDAccept));
        logi->writeLog(readIpClient(request), readMethod(request),"/opendoor/" + std::to_string(channel),
            IDToLog + " - ID are accepted: " + ((IDAccept == "") ? "None" : IDAccept));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        sendError(request, 400, "opendoor_profile.fail.clients");
    }
}

void cameraEventHandle(struct evhttp_request *request)
{
    try
    {
        json headers = readHeader(request);
        std::string content = readContent(request);
        std::cout << "Event Json [" << readIpClient(request) << "]: " << content << std::endl;
        auto j = json::parse(content); /////
        json jEvent, jForward;

        jForward["headers"] = headers;
        jForward["content"] = content;
        forward->sendJsonForward(jForward);

        std::string date, time;
        iso8601GetDatetime(std::string(j["time"]), date, time);
        for (json object : j["extras"]["objects"])
        {
            static int index = 0;
            jEvent["date"]      = date;
            jEvent["time"]      = time;
            jEvent["type"]      = j["type"];
            jEvent["camera"]    = readIpClient(request);
            if (jEvent["type"] == "facerecognition")        jEvent["content"] = object["profileId"];
            else if (jEvent["type"] == "license")           jEvent["content"] = object["license"];
            else if (jEvent["type"] == "vehicledetection")  jEvent["content"] = object["license"]["label"];
            jEvent["image"]     = (j["image_id"] != "") ? PATH_SAVE_IMAGES + IMAGE_NAME(j["image_id"]) : "";
            jEvent["video"]     = (j["video_id"] != "") ? PATH_SAVE_IMAGES + VIDEO_NAME(j["video_id"]) : "";
            jEvent["crop"]      = (j["crop_id"].size()) ? PATH_SAVE_IMAGES + IMAGE_NAME(j["crop_id"].at(index)) : "";
            jEvent["location"]  = object["locations"];
            eventManager->push(jEvent);
        }
        logi->writeLog(readIpClient(request), readMethod(request), "/event", "Receive json successfully");
        sendJson(request, 200, "Expander: Event received");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        logi->writeLog(readIpClient(request), readMethod(request), "/event", "Error in cameraEventHandle()");
        sendError(request, 400, "event_profile.fail.clients");
    }
}

void cameraEventMediaHandle(struct evhttp_request *request, const char *imageid)
{
    try
    {
        // char *image;
        // size_t size;
        // if (!readMultipartImage(request, &image, size)){
        //     logi->writeLog(readIpClient(request), readMethod(request), "/event/" + std::string(imageid), "Cannot read image");
        //     sendJson(request, 400, "Cannot read image");
        //     return; 
        // }
        // std::cout << "imagesize: " << size << std::endl;
        // std::ofstream outfile(FOLDER_IMAGES + IMAGE_NAME(imageid), std::ofstream::binary | std::ofstream::trunc);
        // outfile.write(image, size);
        // free(image);
        // outfile.close();

        json jHeaders           = readHeader(request);
        std::string message     = readMultipartMessage(request);
        std::string filepath    = readMultipartContent(request, imageid);
        std::string content     = readContent(request);
        std::cout << "Event Image [" << readIpClient(request) << "] message: " << message << std::endl;
        if (filepath == std::string())
        {
            logi->writeLog(readIpClient(request), readMethod(request), "/event/" + std::string(imageid), "Cannot read image");
            sendJson(request, 400, "Cannot read image");
            return;
        }

        json jEvent;
        jEvent["imageid"] = std::string(imageid);
        jEvent["headers"] = jHeaders;
        jEvent["message"] = message;
        jEvent["content"] = content;
        jEvent["filepath"] = filepath;
        std::cout << "content size: " << content.length() << std::endl;

        forward->sendForwardMedia(jEvent);
        logi->writeLog(readIpClient(request), readMethod(request), "/event/" + std::string(imageid), "Receive image successfully");
        sendJson(request, 200, "Expander: Image received!");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        sendJson(request, 400, "Expander error cameraEventMediaHandle()");
    }
}

void apiNotSupport(struct evhttp_request *request)
{
    sendError(request, 500, "API not supported");
}