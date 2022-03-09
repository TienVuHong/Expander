/*
* HttpClient.hpp
* Created by TienVH on 15/06/2021.
* Copyright © 2021 Vu Hong Tien. All rights reserved.
*/

#ifndef _HttpClient_hpp
#define _HttpClient_hpp

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <event2/http.h>
// #include <opencv4/opencv2/opencv.hpp>
#include "../json.h"

#define DEFAULT_TIMEOUT  10

typedef enum httpMethod{
    GET     = 1 << 0,
	POST    = 1 << 1,
	HEAD    = 1 << 2,
	PUT     = 1 << 3,
	DELETE  = 1 << 4,
	OPTIONS = 1 << 5,
	TRACE   = 1 << 6,
	CONNECT = 1 << 7,
	PATCH   = 1 << 8
}httpMethod;

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    /**
    function to send request to a HTTP server with content is a string
    @param:
    @param url<string>: ip and port of destination HTTP server
    @param method<httpMethod>: method of request, commonly is GET, POST
    @param content<string>: content or payload of HTTP request  
    @param response<string>: reference to store response of request.
    @param timeout<int>: timeout of request, default = 10
    @param headers<json>: json array for fill in header of the request, default is empty: []
    @return 1 if request successfully, 0 if error
    */ 
    int request(
        std::string url, 
        httpMethod method, 
        std::string content, 
        std::string &response, 
        int timeout = DEFAULT_TIMEOUT, 
        json headers = json::array()
        );

    int postImageMultipart(
        std::string url, 
        std::string message, 
        std::string filepath, 
        std::string &response, 
        int timeout = DEFAULT_TIMEOUT, 
        json headers = json::array()
    );
    // int sendData(cv::Mat& img, std::string url, std::vector<std::string> headers);
private:
    CURL *curl;
    std::string curlResponse;

    void setUrl(std::string url);
    void setMethod(httpMethod method);
    void setHeader(json headers);
    void setContent(std::string &str);
    void setTimeOut(int timeout);
    int perform(std::string &response);
};

#endif