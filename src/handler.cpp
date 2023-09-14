#include"handler.hpp"
#include"proxy.hpp"
#include"request.hpp"
#include"response.hpp"
#include"client.hpp"
#include"cache.hpp"
#include"handlerHelper.hpp"
#include <sys/socket.h>
#include <memory>
#include<vector>
#include<cstring>
#include<string>
#include <fstream>

void* handler(void* pClient){
    std::unique_ptr<HttpClient> uniqptrHttpClient((HttpClient*)pClient);//RAII
    HttpClient* pHttpClient= uniqptrHttpClient.get();
    std::vector<char> msg(MAX_SIZE, 0);
    int size=recv(pHttpClient->fd, &(msg.data()[0]), MAX_SIZE, 0);// the size not fixed? can't receive all the message once?
    if(size==-1){ 
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Proxy server cannot receive messages on socket"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    if(size==0){
        std::cout<<"the request received is 0 byte"<<std::endl;
        return NULL;
    }
    //test
    std::cout<<"I received a request!"<<std::endl;
    std::cout<<"size is "<<size<<std::endl;
    for(int i=0;i<size;++i)
        std::cout<<msg[i];
    std::cout<<std::endl;
    std::cout<<"msg[size] "<<((msg[size]=='\0')?"is char 0":"not char 0")<<std::endl;//debug test

    Request req;
    req.parseRequest(msg, size);
    //test
    std::cout<<"I parsed a request!"<<std::endl;
    if(!req.getIsValid()){ // has bug, the browser didn't display anything
        //test
        std::cout<<"I am in 400!"<<std::endl;
        const char* negativeResponse="HTTP/1.1 400 Bad Request\r\n\r\n";// http 2?
        respondToClientInOneLine(negativeResponse, pHttpClient);
        //test
        std::cout<<"I responded to browser with 400 Bad Request!"<<std::endl;
        return NULL;
    }
    // to be verified
    pthread_mutex_lock(&lock);
    logmsg <<pHttpClient->id<<": \""<<req.getReqLine()<<"\" from "<<pHttpClient->ip<<" @ "<<getCurTime()<<std::endl; 
    pthread_mutex_unlock(&lock);
    if(req.getMethod()=="CONNECT"){
        //test
        std::cout<<"I am in CONNECT!"<<std::endl;
        handleCONNECT(pHttpClient, req);
    }
    else if(req.getMethod()=="POST"){
        //test
        std::cout<<"In POST, msg[size] "<<((msg[size]=='\0')?"is char 0":"not char 0")<<std::endl;//debug test
        std::cout<<"I am in POST!"<<std::endl;
        handlePOST(pHttpClient, req, msg, size);
    }
    else if(req.getMethod()=="GET"){
        //test
        std::cout<<"I am in GET!"<<std::endl;
        handleGET(pHttpClient, req, msg, size);
    }
    else{
        //test
        std::cout<<"I am in 501!"<<std::endl;
        const char* negativeResponse="HTTP/1.1 501 Not Implemented\r\n\r\n";// http 2?
        respondToClientInOneLine(negativeResponse, pHttpClient);
    }
    close(pHttpClient->fd);
    return NULL;
}

void handleCONNECT(HttpClient* pHttpClient, Request& req){
    //test
    std::cout<<"I am in CONNECT handler!"<<std::endl;
    Client* pProxyClient;
    try{ // create a client connected to the original server
        pProxyClient = new Client(req.getTargetHost().c_str(), req.getTargetPort().c_str()); 
    }
    catch(const int& e){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed to create a client connected to the original server in CONNECT request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        //test
        std::cout<<"I can't create a client conneceted to original server in CONNECT handler!"<<std::endl;
        return;
    }
    //test
    std::cout<<"I successfully created a client conneceted to original server in CONNECT handler!"<<std::endl;
    std::unique_ptr<Client> uniqptrClient(pProxyClient);//RAII
    const char* positiveResponse="HTTP/1.1 200 OK\r\n\r\n";//two \r\n!!!
    if(respondToClientInOneLine(positiveResponse, pHttpClient)==-1)// can't send 200 ok to client(browser)
        return;
    //test
    std::cout<<"I successfully sent a 200 ok to browser in CONNECT handler!"<<std::endl;
    std::vector<int> fd={pHttpClient->fd, pProxyClient->socket_fd}; //C++11
    while(true){
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd[0], &readfds);//socket for client(browser)
        FD_SET(fd[1], &readfds); //socket for original server 
        if(select(std::max(fd[0], fd[1])+1, &readfds, NULL, NULL, NULL)<=0){
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": Tunnel closed"<<std::endl; 
            pthread_mutex_unlock(&lock);
            //test
            std::cout<<"I got out of CONNECT tunnel!"<<std::endl;
            return;
        }
        for(int i=0;i<2;++i){
            if(FD_ISSET(fd[i], &readfds)){
                std::vector<char> buf(MAX_SIZE, 0);
                int size = recv(fd[i], &(buf.data()[0]), MAX_SIZE, 0);
                //test
                std::cout<<"Size received from one end in CONNECT tunnel:"<<size<<std::endl;
                if(size==-1){ // ==0 is closed, ==-1 is exception
                    pthread_mutex_lock(&lock);
                    logmsg <<pHttpClient->id<<": ERROR happened in receiving messages in CONNECT tunnel"<<std::endl; 
                    pthread_mutex_unlock(&lock);
                    //test
                    std::cout<<"I am out of Connect channel because size received is -1!"<<std::endl;
                    return;
                }
                if(size==0){
                    pthread_mutex_lock(&lock);
                    logmsg <<pHttpClient->id<<": Tunnel closed"<<std::endl; 
                    pthread_mutex_unlock(&lock);
                    //test
                    std::cout<<"I got out of CONNECT tunnel(recv size==0)!"<<std::endl;
                    return;
                }
                if(send(fd[1-i], &(buf.data()[0]), size, 0)==-1){
                    pthread_mutex_lock(&lock);
                    logmsg <<pHttpClient->id<<": ERROR happened in sending messages in CONNECT tunnel"<<std::endl; 
                    pthread_mutex_unlock(&lock);
                    std::cout<<"I am out of Connect channel because size sent is negative!"<<std::endl;
                    return;
                }
                //test
                std::cout<<"I passed one message in CONNECT tunnel!"<<std::endl;
                break;
            }
        }
    }
}

void handlePOST(HttpClient* pHttpClient, Request& req, std::vector<char>& msg, int size){
    //test
    std::cout<<"I am in POST handler!"<<std::endl;
    std::cout<<"In POST handler, msg[size] "<<((msg[size]=='\0')?"is char 0":"not char 0")<<std::endl;//debug test
    //test
    std::ofstream outputFile("GETCache.txt");//will overwrite
    std::cout<<"I opened GETCache.txt!"<<std::endl;
    outputFile << pHttpClient->lruCache.showCache();
    outputFile.close();
    std::cout<<"I'm done with writing cache into file"<<std::endl;
    Client* pProxyClient;
    try{ // create a client connected to the original server
        pProxyClient = new Client(req.getTargetHost().c_str(), req.getTargetPort().c_str()); // is it safe?
    }
    catch(const int& e){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed to create a client connected to the original server in POST request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return;
    }
    std::unique_ptr<Client> uniqptrClient(pProxyClient);//RAII
    //malformed?
    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Requesting \""<<req.getReqLine()<<"\" from "<<req.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);

    if(req.getIsChunked()){//not tested and not sure
         relayChunkedMsgToServer(msg, size, pHttpClient, pProxyClient);    
    }
    else{//not chunked
        std::cout<<"I'm going to recvAllRequest"<<std::endl;
        if(recvAllRequest(pHttpClient->fd, msg, size, req)==-1){//update the msg and size and req if there're remaining messages
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": ERROR can't recv all the messages from client in POST request"<<std::endl; 
            pthread_mutex_unlock(&lock);
            return;
        }
        std::cout<<"string sent to server is "<<size+1<<" bytes"<<std::endl;
        msg[size]=0;
        ++size;//add a '\0' to the message
        for(int i=0;i<size;++i)
            std::cout<<msg[i];
        std::cout<<std::endl;
        if(send(pProxyClient->socket_fd, &(msg.data()[0]), size, MSG_NOSIGNAL)==-1){//change from 0 to MSG_NOSIGNAL
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": ERROR happened in sending messages to original server in POST request"<<std::endl; 
            pthread_mutex_unlock(&lock);
            return;
    }    
    }
    //test
    std::cout<<"I sent a message to original server in POST handler!"<<std::endl;
    std::vector<char> buf(MAX_SIZE, 0);
    int recvSize=0;
    recvSize = recv(pProxyClient->socket_fd, &(buf.data()[0]), MAX_SIZE, 0);
    // while((recvSize = recv(pProxyClient->socket_fd, &(buf.data()[totalRecvSize]), MAX_SIZE-totalRecvSize, 0))>0){
    //     totalRecvSize+=recvSize;
    // }
    if(recvSize==-1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR happened in receiving messages in POST request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return;
    }
    //test
    std::cout<<"I received a message from original server in POST handler!"<<std::endl;
    for(int i=0;i<recvSize;++i)
        std::cout<<buf[i];
    std::cout<<std::endl;
    Response response;
    response.parseResponse(buf, recvSize);
    if(!response.getIsValid()){
        const char* negativeResponse = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
        respondToClientInOneLine(negativeResponse, pHttpClient);
        return;
    }
    //test
    std::cout<<"I parsed a response in POST handler!"<<std::endl;
    // do we need to print this message if the response is corrupted?
    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Received \""<<response.getResponseLine()<<"\" from "<<req.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);
    if(response.getIsChunked()){//to be abstracted
         relayChunkedMsgToClient(buf, recvSize, pHttpClient, pProxyClient);
    }
    else{
        if(recvAllResponse(pProxyClient->socket_fd, buf, recvSize, response)==-1){//multiple http response?? maybe wrong parsing, like both 200 and 400?
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": ERROR can't recv all the messages from server in POST request"<<std::endl; 
            pthread_mutex_unlock(&lock);
            return;
        }
        // slightly different from respondToClient function
        respondToClientInManyLines(buf, recvSize, pHttpClient, response);
        //test
        std::cout<<"I'm after respondToClientInManyLines"<<std::endl;
    }
    //test
    std::cout<<"I am done with POST request!"<<std::endl;
}

void handleGET(HttpClient* pHttpClient, Request& req, std::vector<char>& msg, int size){
    // //add '\0'
    // msg.emplace_back(0);
    // msg[size]=0;
    // ++size;
    //test
    std::cout<<"I am in GET request handler!"<<std::endl;
    LRUCache& lruCache=pHttpClient->lruCache;
    //test
    std::cout<<"I started to get URI!"<<std::endl;
    std::string uri=req.getURI();
    //test
    std::cout<<"I finished getting URI!"<<std::endl;
    //test
        std::ofstream outputFile3("GETAllReq.txt", std::ios::app);
        outputFile3 << req.getReqLine()+"\r\n"+req.getHeader()+"\r\n";
        outputFile3.close();
    if(!lruCache.exists(uri)){//request message doesn't exist in cache
        //test
        std::ofstream outputFile1("GETNoCacheReq.txt", std::ios::app);
        outputFile1 << req.getReqLine()+"\r\n"+req.getHeader()+"\r\n";
        outputFile1.close();
        //test
        std::cout<<"I started with the case where uri doesn't exist in the cache!"<<std::endl;
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": not in cache"<<std::endl; 
        pthread_mutex_unlock(&lock);
        createClientThenSendClientReqHandler(lruCache, uri, pHttpClient, req, msg, size);
        //test
        std::cout<<"I'm done with the case where uri doesn't exist in the cache!"<<std::endl;
    }
    else{//URI exists in the cache
        //test
        std::ofstream outputFile2("GETHasCacheReq.txt", std::ios::app);
        outputFile2 << req.getReqLine()+"\r\n"+req.getHeader()+"\r\n";
        outputFile2.close();
        Response& cacheResp=lruCache.getResponse(uri);
        if(cacheResp.getNoCache()==true){//there's no-cache tag
            //test
            std::cout<<"I started with the case where therer's no-cache tag!"<<std::endl;
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": in cache, requires validation"<<std::endl; 
            pthread_mutex_unlock(&lock);
            createClientThenRevalidateAndSendBack(lruCache, uri, pHttpClient, req, msg, size);
            //test
            std::cout<<"I'm done with the case where therer's no-cache tag!"<<std::endl;
        }
        else{//there's no no-cache tag
            int flag=validateFresh(cacheResp, pHttpClient);
            if(flag==-2){//there's no max-age and expires tag
                //test
                std::cout<<"I started with the case where therer's no max-age and expires tag!"<<std::endl;
                //test
                //createClientThenSendClientReqHandler(lruCache, uri, pHttpClient, req, msg, size);
                createClientThenRevalidateAndSendBack(lruCache, uri, pHttpClient, req, msg, size);
                //test
                std::cout<<"I'm done with the case where therer's no max-age and expires tag!"<<std::endl;
            }
            else if(flag==0){//fresh
                //test
                std::cout<<"I started with the case where it's fresh!"<<std::endl;
                if(loopSend(pHttpClient->fd, lruCache.getMsg(uri), lruCache.getMsg(uri).size())==-1){//may be not efficient
                    pthread_mutex_lock(&lock);
                    logmsg <<pHttpClient->id<<": ERROR Failed to send back messages in GET request"<<std::endl; 
                    pthread_mutex_unlock(&lock);
                }
                pthread_mutex_lock(&lock);
                logmsg <<pHttpClient->id<<": Responding "<<lruCache.getResponse(uri).getResponseLine()<<std::endl; 
                pthread_mutex_unlock(&lock);
                //test
                std::cout<<"The headers of fresh sent-back msg in the cache is:"<<std::endl;
                const Response& temResp=lruCache.getResponse(uri);
                const std::string& tem=temResp.getHeader();
                for(unsigned i=0;i<tem.size();++i)
                    std::cout<<tem[i];
                std::cout<<std::endl;
                //test
                std::cout<<"I'm done' with the case where it's fresh!"<<std::endl;
            }
            else{//flag==-1 stale
                if(cacheResp.getMustVal()){//there's must-revalidate tag in this response
                    //test
                    std::cout<<"I started with the case where there's must-revalidate tag in this response!"<<std::endl;
                    createClientThenRevalidateAndSendBack(lruCache, uri, pHttpClient, req, msg, size);
                    //test
                    std::cout<<"I'm done' with the case where there's must-revalidate tag in this response!"<<std::endl;
                }
                else{//there's no must-revalidate tag in this response
                    if(req.getStale()==""){//there's no max-stale tag in the request
                        //test
                        std::cout<<"I started with the case where there's no max-stale tag in the request!"<<std::endl;
                        createClientThenRevalidateAndSendBack(lruCache, uri, pHttpClient, req, msg, size);
                        //test
                        std::cout<<"I'm done with the case where there's no max-stale tag in the request!"<<std::endl;
                    }
                    else{//there's max-stale tag in the request
                        if(fitsMaxStale(req.getStale(), cacheResp)){
                            //test
                            std::cout<<"I started with the case where it fits max-stale!"<<std::endl;
                            if(loopSend(pHttpClient->fd, lruCache.getMsg(uri), lruCache.getMsg(uri).size())==-1){//may be not efficient
                                pthread_mutex_lock(&lock);
                                logmsg <<pHttpClient->id<<": ERROR Failed to send back messages in GET request"<<std::endl; 
                                pthread_mutex_unlock(&lock);
                            }
                            pthread_mutex_lock(&lock);
                            logmsg <<pHttpClient->id<<": Responding "<<lruCache.getResponse(uri).getResponseLine()<<std::endl; 
                            pthread_mutex_unlock(&lock);
                            //test
                            std::cout<<"The headers of sent-back msg fitting max-stale in the cache is:"<<std::endl;
                            const Response& temResp=lruCache.getResponse(uri);
                            const std::string& tem=temResp.getHeader();
                            for(unsigned i=0;i<tem.size();++i)
                                std::cout<<tem[i];
                            std::cout<<std::endl;
                            //test
                            std::cout<<"I'm done with the case where it fits max-stale!"<<std::endl;
                        }
                        else{
                            //test
                            std::cout<<"I started with the case where it doesn't fits max-stale!"<<std::endl;
                            createClientThenRevalidateAndSendBack(lruCache, uri, pHttpClient, req, msg, size);
                            //test
                            std::cout<<"I'm done with the case where it doesn't fits max-stale!"<<std::endl;
                        }
                    }
                }
            }
        }
    }
    //test
    std::cout<<"I am done with GET request!"<<std::endl;
    std::cout<<"The number of msg in the cache is: "<<lruCache.getNum()<<std::endl;
}

