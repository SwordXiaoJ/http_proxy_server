#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include<string>
#include<netdb.h>
#include <unistd.h>
#include<iostream>

class Server{
    private:
        addrinfo *host_info_list;
        sockaddr_in local_addr; // used to get IP and port
    public:
        int socket_fd;
        std::string IP; // better not use it
        std::string hostname; //at most 256 chars
        uint16_t port;
        // create a listening server with socket_fd, the third parameter is used to specify the backlog in listen function
        // if hostname is NULL, it allows all the network interfaces
        Server(const char* hostname, const char* port, int backlog);
        // return a corresponding socket with the check of error case
        int serverAccept(sockaddr* addr, socklen_t* addrlen);
        // make sure all the data is sent, unless meeting error case which would cause exit(EXIT_FAILURE)
        // traditional flag of socket send function is set to 0
        // void serverSend(int sockfd, const void* msg, int len); // here sockfd could be different from socket_fd
        // void serverRecv(int sockfd, void* buf, int len);
        std::string getIP();
        std::string getHostName();
        uint16_t getPortNum();
        ~Server(){
            freeaddrinfo(host_info_list);
            close(socket_fd);
            //test
            std::cout<<"server closed"<<std::endl;
        }
        
};
#endif