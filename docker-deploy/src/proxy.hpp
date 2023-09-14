#ifndef __PROXY_HPP__
#define __PROXY_HPP__
#include<fstream>
#include<pthread.h>
#include<string>
#include"cache.hpp"

extern std::ofstream logmsg;
extern pthread_mutex_t lock;

struct HttpClient{
        int fd;
        std::string ip;
        int id;
        LRUCache & lruCache;
        HttpClient(LRUCache & cache):lruCache(cache){}
};

void cleanup();
#endif