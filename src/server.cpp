#include"server.hpp"
#include<netdb.h>
#include<iostream>
#include<cstring>
#include <arpa/inet.h>
#include<unistd.h>
#include<string>
#include"proxy.hpp"

//adapted from duke ece 650 tcp_example
Server::Server(const char* hostname, const char* port, int backlog){
    int status=0;
    addrinfo host_info;
    memset(&host_info, 0, sizeof(host_info));// from <cstring>
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error at Server: cannot get address info for host" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error at Server: cannot create socket" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    int yes = 1;
    //it allows the server to reuse the address immediately after it's closed, without waiting for the system to clean up the address.
    //This can be helpful for development and testing, where you want to quickly restart the server without having to wait for the system to release the address.
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error at Server: cannot bind socket" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    status = listen(socket_fd, backlog);
    if (status == -1) {
        std::cerr << "Error at Server: cannot listen on socket" << std::endl; 
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(socket_fd, (sockaddr *)&local_addr, &local_addr_len) == -1) {
        std::cerr << "Error at Server: getsockname failed" << std::endl; 
        exit(EXIT_FAILURE);
    }
    this->IP = std::string(inet_ntoa(local_addr.sin_addr)); // only works for IPv4
    this->port = ntohs(local_addr.sin_port); // used this pointer to avoid confusion with passed in parameter
    char hostName[256];
    if (gethostname(hostName, 256) != 0) {
        std::cerr<<"Error getting hostname"<<std::endl;
        exit(EXIT_FAILURE);
    }
    this->hostname = std::string(hostName);
    //test
    std::cout<<"hostName when creating server:"<<this->hostname<<std::endl;
}
int Server::serverAccept(sockaddr* addr, socklen_t* addrlen){
        int client_fd = accept(socket_fd, addr, addrlen);
        // if (client_fd == -1) {
        //     std::cerr << "Error at Server: cannot accept connection on socket " << std::endl;
        // }
        return client_fd;// need to close it by hand!!!!!!!!!!!
}
std::string Server::getIP(){
    return IP;
}
uint16_t Server::getPortNum(){
    return port;
}
std::string Server::getHostName(){
    return hostname;
}