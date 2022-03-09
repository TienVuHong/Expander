/*
 * HttpServer.cpp
 *
 *  Created on: 17/02/2020
 *      Author: hoapq
 *  Copyright © 2020 Pham Quang Hoa. All rights reserved.
 */

#include <iostream>
#include <cstring>
#include <string>
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

#include "HttpServer.hpp"
#include "HttpHandle.hpp"

#define LIBEVENT_LOOP_INTERVAL 30

static std::string printContent(struct evhttp_request *request);

HttpServer::HttpServer() {
    this->base = NULL;
    this->server = NULL;
}

HttpServer::~HttpServer() {
    if (this->server != NULL) {
        evhttp_free(this->server);
    }
    if (this->base != NULL) {
        event_base_free(this->base);
    }
}

static std::string printContent(struct evhttp_request *request) {
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

void HttpServer::loop(/*cv::Mat frame*/) {
    if (this->base == NULL) {
        this->base = event_base_new();
    } else if (this->server == NULL) {
        this->server = evhttp_new(this->base);
        evhttp_set_allowed_methods(
            this->server, EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_PUT |
                              EVHTTP_REQ_DELETE | EVHTTP_REQ_OPTIONS);
        evhttp_set_gencb(this->server, callback, this);

        if (evhttp_bind_socket(this->server, "0.0.0.0", SERVER_PORT) != 0) {
            std::cerr << "Failed to bind to 0.0.0.0:" << SERVER_PORT << std::endl;
            exit(0);
        } else {
            std::cout << "HTTP Server Listening on 0.0.0.0:" << SERVER_PORT << std::endl;
        }
    } else {
        event_base_loop(this->base, EVLOOP_NONBLOCK);
    }
}

bool startWith(const char **src, const char *dst) {
    const char *str = *src;
    size_t len = strlen(dst);
    if (strncmp(str, dst, len) == 0) {
        *src = str + len;
        return true;
    } else {
        return false;
    }
}

bool checkPathEmpty(const char *path)
{
    return (*path == '\0');
}

bool pathCompare(const char **src, const char *dst)
{
    if (startWith(src, dst))
        if (checkPathEmpty(*src))
            return true;
    return false;
}

void HttpServer::callback(struct evhttp_request *request, void *param) {
    HttpServer *server = (HttpServer *)param;
    const char *urisz = evhttp_request_get_uri(request);
    struct evhttp_uri *uri = evhttp_uri_parse(urisz);
 
    if (uri != NULL) {
        // std::cout << "printContent: " << printContent(request) << std::endl;
        const char *path = evhttp_uri_get_path(uri);
        const char *query = evhttp_uri_get_query(uri);
        const char *fragment = evhttp_uri_get_fragment(uri);
        std::cout << "path: " << path << std::endl;
        processPath(server, request, path, query);
        evhttp_uri_free(uri);
    }
}

void HttpServer::processPath(HttpServer *server, struct evhttp_request *request, const char *path, const char *query)
{
    if (startWith(&path, "/api/network/info")) 
    {
        if (checkPathEmpty(path))
            apiNetworkInfo(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/device/info")) 
    {
        if (checkPathEmpty(path))
            apiDeviceInfo(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/login")) 
    {
        if (checkPathEmpty(path))
            apiLogin(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/user")) 
    {
        if (checkPathEmpty(path))
            apiUser(request);
        else goto NOT_SUPPORT;
    }
    else if(startWith(&path, "/api/ip"))
    {
        if (checkPathEmpty(path))
            apiIp(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/server"))
    {
        if (checkPathEmpty(path))
            apiServer(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/reset")) 
    {
        if (checkPathEmpty(path))
            apiReset(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/config"))
    {
        if (checkPathEmpty(path))
            apiConfig(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/deviceconfig"))
    {
        if (checkPathEmpty(path))
            apiDeviceConfig(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/opendevice"))
    {
        if (checkPathEmpty(path))
            apiOpenDevice(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/database/event"))
    {
        if (checkPathEmpty(path))
            apiDatabaseEvent(request, query);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/api/database"))
    {  
        if (checkPathEmpty(path))
            apiDatabase(request);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/opendoor/1"))
    {
        if (checkPathEmpty(path))
            cameraOpendoorHandle(request, 1);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/opendoor/2"))
    {
        if (checkPathEmpty(path))
            cameraOpendoorHandle(request, 2);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/opendoor/3"))
    {
        if (checkPathEmpty(path))
            cameraOpendoorHandle(request, 3);
        else goto NOT_SUPPORT;
    }
    else if (startWith(&path, "/event"))
    {
        if (checkPathEmpty(path))
            cameraEventHandle(request);
        else if (startWith(&path, "/"))
        {
            const char *imageid = path;
            cameraEventMediaHandle(request, imageid);
        }
        else goto NOT_SUPPORT;
    }
    else
    {
        NOT_SUPPORT:
        apiNotSupport(request);
    }
}
