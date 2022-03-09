/*
 * HttpHandle.hpp
 *
 *  Created on: 06/07/2021
 *      Author: tienvh
 * 
 */

#ifndef HTTP_HANDLE_HPP
#define HTTP_HANDLE_HPP

void httpHandleInit();
void apiNetworkInfo(struct evhttp_request *request);
void apiDeviceInfo(struct evhttp_request *request);
void apiLogin(struct evhttp_request *request);
void apiUser(struct evhttp_request *request);
void apiIp(struct evhttp_request *request);
void apiServer(struct evhttp_request *request);
void apiReset(struct evhttp_request *request);
void apiConfig(struct evhttp_request *request);
void apiDeviceConfig(struct evhttp_request *request);
void apiOpenDevice(struct evhttp_request *request);
void apiDatabaseEvent(struct evhttp_request *request, const char *param);
void apiDatabase(struct evhttp_request *request);
void cameraOpendoorHandle(struct evhttp_request *request, int channel);
void cameraEventHandle(struct evhttp_request *request);
void cameraEventMediaHandle(struct evhttp_request *request, const char *imageid);
void apiNotSupport(struct evhttp_request *request);

#endif /* HTTP_HANDLE_HPP */