#include"server.hpp"
#include"client.hpp"
#include"proxy.hpp"
#include"handler.hpp"
#include <arpa/inet.h>
#include"cache.hpp"
#include<fstream>

std::ofstream logmsg=std::ofstream("/var/log/erss/proxy.log", std::fstream::app);
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

void cleanup() {
    logmsg.close();
}

int main(){
    atexit(cleanup);
    Server server(NULL, "12345", 128); //linux's max number is 128
    LRUCache lruCache; //if fails, exit the program
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(sockaddr_storage);
    int id=0;
    //test, to clear all the file content
    std::ofstream file0("GET304CacheResp.txt", std::ios::out | std::ios::trunc);
    file0.close();
    std::ofstream file1("GETAllReq.txt", std::ios::out | std::ios::trunc);
    file1.close();
    std::ofstream file2("GETCache.txt", std::ios::out | std::ios::trunc);
    file2.close();
    std::ofstream file3("GETDirectResp.txt", std::ios::out | std::ios::trunc);
    file3.close();
    std::ofstream file4("GETHasCacheReq.txt", std::ios::out | std::ios::trunc);
    file4.close();
    std::ofstream file5("GETNoCacheReq.txt", std::ios::out | std::ios::trunc);
    file5.close();
    std::ofstream file6("GETRevalResp.txt", std::ios::out | std::ios::trunc);
    file6.close();
    while(true){
        std::cout<<"Master thread is in the new loop"<<std::endl;
        int client_fd=server.serverAccept((sockaddr *)&socket_addr, &socket_addr_len);
        if(client_fd==-1){
            pthread_mutex_lock(&lock);
            logmsg << "(no-id): ERROR Proxy server cannot accept connection on socket" <<std::endl;
            pthread_mutex_unlock(&lock);
            continue;
        }
        pthread_t thread;
        HttpClient* pHttpClient=new HttpClient(lruCache);  //smart pointers?
        pHttpClient->fd= client_fd;
        pHttpClient->id=id;
        pHttpClient->ip=inet_ntoa(((sockaddr_in *)(&socket_addr))->sin_addr);
        ++id;//lock??
        pthread_create(&thread, NULL, handler, pHttpClient);
    }
    return 0;
}