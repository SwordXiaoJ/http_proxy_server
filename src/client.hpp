#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__
#include<string>
#include<netdb.h>
#include <unistd.h>
#include<iostream>

class Client{
    private:
        addrinfo *host_info_list;
    public:
        int socket_fd;
        // default constructor
        Client(){}
        // create a connected client with socket_fd
        Client(const char* hostname, const char* port);
        ~Client(){
            freeaddrinfo(host_info_list);
            close(socket_fd);
            //test
            std::cout<<"client closed"<<std::endl;
        }
};
#endif