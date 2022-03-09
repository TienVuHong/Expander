/*
 * HttpServer.hpp
 *
 *  Created on: 17/02/2020
 *      Author: hoapq
 *  Copyright © 2020 Pham Quang Hoa. All rights reserved.
 */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#define LIBEVENT_LOOP_INTERVAL 30
#define SERVER_PORT 55555

class HttpServer {
public:
    HttpServer();
    virtual ~HttpServer();
    virtual void loop();

private:
    struct event_base* base;
    struct evhttp* server;
    static void callback(struct evhttp_request* request, void* param);
    static void processPath(HttpServer *server, struct evhttp_request* request, const char *path, const char *query);
};

#endif /* HTTPSERVER_HPP */
