/*
* HttpClient.cpp
* Created by TienVH on 15/06/2021.
* Copyright © 2021 Vu Hong Tien. All rights reserved.
*/

#include "HttpClient.hpp"

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HttpClient::HttpClient()
{
    curl = curl_easy_init();
}

HttpClient::~HttpClient()
{
    curl_easy_cleanup(curl);
}

int HttpClient::request(std::string url, httpMethod method, std::string content, std::string &response, int timeout, json headers)
{
    setUrl(url);
    setContent(content);
    setMethod(method);
    setHeader(headers);
    setTimeOut(timeout);
    return perform(response);
}

void HttpClient::setUrl(std::string url)
{
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

void HttpClient::setMethod(httpMethod method)
{
    switch (method)
    {
        case GET: 
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            break;            
        case POST:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            break;
        case HEAD:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
            break;
        case PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case OPTIONS:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        case TRACE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "TRACE");
            break;
        case CONNECT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "CONNECT");
            break;
        case PATCH:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        default:   
            break;
    }
}

void HttpClient::setHeader(json headers)
{
    struct curl_slist *headersList = NULL;
    if (headers.is_array())
    {
        for (const std::string header : headers)
        {
            headersList = curl_slist_append(headersList, header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
    }
}

void HttpClient::setContent(std::string &curlContent)
{
    curlResponse.clear();
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlResponse);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlContent.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, curlContent.size());
}

void HttpClient::setTimeOut(int timeout)
{
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)timeout);
}

int HttpClient::perform(std::string &response)
{
    int httpCode = 0;
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        response = curl_easy_strerror(res);
    else
        response = curlResponse;
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    return httpCode;
}

// int HttpClient::postImageMultipart(std::string url, std::string data, std::string filepath, std::string &response, int timeout, json headers)
// {
//     curl_httppost *formpost;
//     curl_httppost *lastptr;
//     // std::cout << "vao day 0" << std::endl;
//     // curl_formadd(&formpost,
//     //             &lastptr,
//     //             CURLFORM_COPYNAME, "data",
//     //             CURLFORM_COPYCONTENTS, data.c_str(),
//     //             CURLFORM_END);
//     std::cout << "vao day 1" << std::endl;
//     curl_formadd(&formpost,
//                 &lastptr,
//                 CURLFORM_COPYNAME, "image",
//                 CURLFORM_FILE, filepath.c_str(),
//                 CURLFORM_CONTENTTYPE, "image/jpeg",
//                 CURLFORM_END);
//     std::cout << "vao day 2" << std::endl;
//     setUrl(url);
//     setHeader(headers);
//     curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
//     int ret = perform(response);
//     curl_formfree(formpost);
//     curl_formfree(lastptr);                                
//     return ret;
// }


int HttpClient::postImageMultipart(std::string url, std::string message, std::string filepath, std::string &response, int timeout, json headers)
{
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;
    setUrl(url);
    setMethod(POST);
    setHeader(headers);

    form = curl_mime_init(curl);
    /* Fill in the data field */
    field = curl_mime_addpart(form);
    curl_mime_name(field, "data");
    curl_mime_data(field, message.c_str(), CURL_ZERO_TERMINATED);
    
    /* Fill in the file upload field */
    field = curl_mime_addpart(form);
    curl_mime_name(field, "image");
    curl_mime_filedata(field, filepath.c_str());

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    int ret = perform(response);
    curl_mime_free(form);
    return ret;
}