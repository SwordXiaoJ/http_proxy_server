#include"client.hpp"
#include<netdb.h>
#include<iostream>
#include<cstring>
//adapted from duke ece 650 tcp_example
Client::Client(const char* hostname, const char* port){
    int status=0;
    addrinfo host_info;
    memset(&host_info, 0, sizeof(host_info));// from <cstring>
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    // host_info.ai_flags = AI_PASSIVE;
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        // std::cerr << "Error at Client: cannot get address info for host" << std::endl;
        // std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        // exit(EXIT_FAILURE);
        //test
        std::cout<<"Hostname inside client constructor:" <<std::string(hostname)<<std::endl;
        std::cout<<"Port inside client constructor:" <<std::string(port)<<std::endl;
        std::cout<<"Error at Client: cannot get address info for host" <<std::endl;
        throw -1;
    }
    socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
    if (socket_fd == -1) {
        // std::cerr << "Error at Client: cannot create socket" << std::endl;
        // std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        // exit(EXIT_FAILURE);
        //test
        std::cout<<"Error at Client: cannot create socket"<<std::endl;
        throw -2;
    }
    // // Set socket options for timeout
    // struct timeval tv;
    // tv.tv_sec = 5;  // Timeout of 5 seconds
    // tv.tv_usec = 0;
    // if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
    //     perror("Error setting socket options");
    //     throw -3;
    // }
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        // std::cerr << "Error at Client: cannot connect to socket" << std::endl;
        // std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        // exit(EXIT_FAILURE);
        //test
        std::cout<<"Error at Client: cannot connect to socket" <<std::endl;
        throw -4;
  }
}

