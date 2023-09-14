#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__
#include"proxy.hpp"
#include"request.hpp"
#include"response.hpp"
#include"client.hpp"
#include"cache.hpp"

void* handler( void* pClient);
std::string getCurTime();
const int MAX_SIZE=65536;
void handleCONNECT(HttpClient* pClient, Request& req);
void handlePOST(HttpClient* pClient, Request& req, std::vector<char>& msg, int size);
void handleGET(HttpClient* pClient, Request& req, std::vector<char>& msg, int size);


#endif