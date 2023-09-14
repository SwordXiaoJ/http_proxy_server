#ifndef __HANDLERHELPER_HPP__
#define __HANDLERHELPER_HPP__
#include"handler.hpp"

//send self-defined one-line message to client
int respondToClientInOneLine(const char* response, const HttpClient* pClient); 
//send multiple-line messages from original server to client
int respondToClientInManyLines(const std::vector<char>& msg, const int size, const HttpClient* pHttpClient, const Response& response);
//check and receive all the messages from client sending POST request, update the msg and size and req inside
//msg and size and request are the first received message's info
//for not-chunked message, receive all the messages
// return -1 if there's exception, else 0
int recvAllRequest(const int socket_fd, std::vector<char>& msg, int& size, Request& request);
int recvAllResponse(const int socket_fd, std::vector<char>& msg, int& size, Response& response);
//return -1 if error
int relayChunkedMsgToServer(std::vector<char>& msg, int& size, const HttpClient* pHttpClient, const Client* pProxyClient);
int relayChunkedMsgToClient(std::vector<char>& msg, int& size, const HttpClient* pHttpClient, const Client* pProxyClient);
//revalidate and store(may not if specified) in the cache and update the msg and size sent back
//it will check if Etag and or last-modifies exists inside and if not, send the message from client to server
//clientMsg is the GET message sent by browser, clientSize is its size
int revalidateAndSendBack(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Client* pProxyClient, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq);
int loopSend(const int socket_fd, const std::vector<char>& msg, const int size);
int shouldNotCache(const Response& resp);
// the revalidation process of cache message with either Etag or Expires(must has one, revalidate function handles case when message may have no these two headers)
int revalidateWithTagAndSendBack(LRUCache& lruCache, const bool isEtag, const std::string& uri, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq, const Response& cacheResp, const HttpClient* pHttpClient, const Client* pProxyClient);
// //used to modify the certain headers of http message in the cache 
// void msgSetMaxAge(std::vector<char>& msg, const std::string& maxAge);
// void msgSetDate(std::vector<char>& msg, const std::string& Date);
// void msgSetExpires(std::vector<char>& msg, const std::string& Expires);

// used to handle the not-chunked!!! GET response from server, including cache manipulation and sending back, thread-safe(has lock inside)
int handleGETresponse(const HttpClient* pHttpClient, LRUCache& lruCache, const std::string& uri, const Response& recvResp, const std::vector<char>& recvMsg, const int recvSize);
//except for currently valid situation, it's logged inside this func
int validateFresh(Response& response, const HttpClient* pHttpClient);
// used to send client's request directly to orignal server and then send it back(may store in the cache)
int sendClientReqHandler(LRUCache& lruCache, const std::string& uri, const Client* pProxyClient, const HttpClient* pHttpClient, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq);
int createClientThenRevalidateAndSendBack(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Request& req, const std::vector<char>& msg, int size);
int createClientThenSendClientReqHandler(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Request& req, const std::vector<char>& msg, int size);
// check if it fits max-stale in the case that either max-age or expires exist
bool fitsMaxStale(const std::string& maxStale, const Response& response);
std::string getCurTime();
time_t getCurTimeSecond();
#endif