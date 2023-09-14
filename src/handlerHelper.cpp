#include"handlerHelper.hpp"
#include<cstring>
#include<string>
#include <ctime>
#include <fstream>
#include"timeInfo.hpp"
//send self-defined one-line message to client
int respondToClientInOneLine(const char* response, const HttpClient* pHttpClient){
    if(send(pHttpClient->fd, response, std::strlen(response), 0)==-1){// needed?
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR happened in sending"<<std::string(response)<<" to client"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
        }
    else{
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": Responding "<<std::string(response)<<std::endl; 
        pthread_mutex_unlock(&lock);
        return 0;
    }
}

//send multiple-line messages from original server to client (logmsg response is the first line of sent msg)
int respondToClientInManyLines(const std::vector<char>& msg, const int size, const HttpClient* pHttpClient, const Response& response){
    if(send(pHttpClient->fd, &(msg.data()[0]), size, 0)==-1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR happened in sending messages back in POST request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    //test
    std::cout<<"Message sent back:"<<std::endl;
    for(int i=0;i<size;++i)
        std::cout<<msg[i];
    std::cout<<std::endl;
    std::cout<<"I'm after Messages sent back"<<std::endl;
    pthread_mutex_lock(&lock);
    logmsg <<pHttpClient->id<<": Responding "<<response.getResponseLine()<<std::endl; 
    pthread_mutex_unlock(&lock);
    return 0;
}

int recvAllRequest(const int socket_fd, std::vector<char>& msg, int& size, Request& request)
{
    std::string body = request.getBody();
    std::string content_length = request.getContentLen();
    if(content_length == "")
    {
        if(body != "")
        {
            return -1;//means error
        }
        else//body is null,it means hava reveived all
        {
            return 0;//means success receive all
        } 
    }
    //content lenghth is not null,it means it have body
    else
    {   
        int content_len_num = std::stoi(content_length);
        //4 means the blank line(\r\n) and the (\r\n) in the end
        //test 
        std::cout<<"The size of msg is"<<size<<std::endl;
        int remaining_body_len = content_len_num- (size - request.getHeader().size()- request.getReqLine().size() -4);
        //test
        std::cout<<"remaing_body"<<remaining_body_len<<std::endl;
        //test
        std::cout<<"I'm in recvAllRequest func else block"<<std::endl;
        if(remaining_body_len == 0)
        {
            return 0;
        }
        else
        {
            int resized_len = 3 * std::stoi(content_length) + request.getHeader().size() + 2;
        if(resized_len > MAX_SIZE)
        {
            msg.resize(resized_len);
        }
        int recvSize = 0;
        //test
        //std::cout<<"content_length is "<<std::stoi(content_length)<<std::endl;
        
        while ((recvSize = recv(socket_fd, &(msg.data()[size]), std::max(MAX_SIZE,resized_len)-size, 0)) > 0)
        {
            size += recvSize;
            //test
            std::cout<<"I'm in recvAllRequest func else block while loop, current size is:"<<size<<std::endl;
            remaining_body_len-=recvSize;
            //test
            std::cout<<"remaining_body_len is:"<<remaining_body_len<<std::endl;
            if(remaining_body_len==0){
                request.parseRequest(msg,size);
                return 0;
            }   
        }
        if(recvSize == -1)
        {
            return -1;
        }
        request.parseRequest(msg,size);
        return 0;
        }
        
    }
}

int recvAllResponse(const int socket_fd, std::vector<char>& msg, int& size, Response& response)
{
    //test
    std::cout<<"I'm in the recvAllResponse func"<<std::endl;
    std::string body = response.getBody();
    std::string content_length = response.getContentLen();
    if(content_length == "")
    {
        if(body != "")
        {
            return -1;//means error
        }
        else//body is null,it means hava reveived all
        {
            return 0;//means success receive all
        } 
    }
    //content lenghth is not null,it means it have body
    else
    {   
        //test
        std::cout<<"I started int content_len_num = std::stoi(content_length) statement"<<std::endl;
        int content_len_num = std::stoi(content_length);
        //4 means the blank line(\r\n) and the (\r\n) in the end
        int remaining_body_len =content_len_num - (size - (int)response.getHeader().size()- (int)response.getResponseLine().size()-4) ;
        //test
        std::cout<<"remaing_body: "<<remaining_body_len<<std::endl;
        //test
        std::cout<<"I'm in recvAllResponse func else block"<<std::endl;
        if(remaining_body_len == 0)
        {
            return 0;
        }
        else
        {
            int resized_len = 3 * std::stoi(content_length) + response.getHeader().size() + 2;
        if(resized_len > MAX_SIZE)
        {
            msg.resize(resized_len);
        }
        int recvSize = 0;
        //test
        //std::cout<<"content_length is "<<std::stoi(content_length)<<std::endl;
        
        while ((recvSize = recv(socket_fd, &(msg.data()[size]), std::max(MAX_SIZE,resized_len)-size, 0)) > 0)
        {
            size += recvSize;
            //test
            std::cout<<"I'm in recvAllResponse func else block while loop, current total size is:"<<size<<std::endl;
            remaining_body_len-=recvSize;
            //test
            std::cout<<"remaining_body_len is:"<<remaining_body_len<<std::endl;
            if(remaining_body_len==0){
                response.parseResponse(msg,size);
                if (!response.getIsValid())
                    return -1;
                return 0;
            }       
        }
        if(recvSize == -1)
        {
            return -1;
        }
        response.parseResponse(msg,size);
        if (!response.getIsValid())
            return -1;
        return 0;
        }
        
    }
}

int relayChunkedMsgToServer(std::vector<char>& msg, int& size, const HttpClient* pHttpClient, const Client* pProxyClient){
    //test
    std::cout<<"I'm in relayChunkedMsgToServer func"<<std::endl;
    while(true){
    // msg[size]=0;
    // ++size;//not sure if we need to add this?
    if(send(pProxyClient->socket_fd, &(msg.data()[0]), size, MSG_NOSIGNAL)==-1){//change from 0 to MSG_NOSIGNAL
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR happened in sending messages to original server"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if(msg[0]=='0'){// last chunk
        //test
        std::cout<<"I'm done with relayChunkedMsgToServer func"<<std::endl;
        return 0;
    }    
    if((size=recv(pHttpClient->fd, &(msg.data()[0]), MAX_SIZE, 0))==-1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR happened in receiving messages from client(browser)"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    }
}

int relayChunkedMsgToClient(std::vector<char>& buf, int& recvSize, const HttpClient* pHttpClient, const Client* pProxyClient){
    //test
    std::cout<<"I'm in relayChunkedMsgToClient func"<<std::endl;
    std::cout<<"The first msg received is:"<<std::endl;
    for(int i=0;i<recvSize;++i)
        std::cout<<buf[i];
    std::cout<<std::endl;
    if(buf[recvSize-5]=='0'){//send all in the first msg
        if(send(pHttpClient->fd, &(buf.data()[0]), recvSize, 0)==-1){
                pthread_mutex_lock(&lock);
                logmsg <<pHttpClient->id<<": ERROR happened in sending messages to client(browser)"<<std::endl; 
                pthread_mutex_unlock(&lock);
                return -1;
        }
        return 0;
    }
        while(true){
            // buf[recvSize-1]=0; // not sure????
            //test
            std::cout<<"before sending"<<std::endl;
            if(send(pHttpClient->fd, &(buf.data()[0]), recvSize, 0)==-1){
                pthread_mutex_lock(&lock);
                logmsg <<pHttpClient->id<<": ERROR happened in sending messages to client(browser)"<<std::endl; 
                pthread_mutex_unlock(&lock);
                return -1;
            }
            //test
            std::cout<<"after sending, before if statement"<<std::endl;
            if(buf[0]=='0'){
                //test
                std::cout<<"I'm done with relayChunkedMsgToClient func"<<std::endl;
                return 0;
            }
            //test
            std::cout<<"after if statement, before recv"<<std::endl;
            if((recvSize=recv(pProxyClient->socket_fd, &(buf.data()[0]), MAX_SIZE, 0))==-1){
                pthread_mutex_lock(&lock);
                logmsg <<pHttpClient->id<<": ERROR happened in receiving messages from server"<<std::endl; 
                pthread_mutex_unlock(&lock);
                return -1;
            }
        }
}

// let outside code block to log error message if there is 
int revalidateAndSendBack(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Client* pProxyClient, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq){
    //test
    std::cout<<"I'm in revalidateAndSendBack func, current msg header in the cache is:"<<std::endl;
    const Response& temResp=lruCache.getResponse(uri);
    const std::string& tem=temResp.getHeader();
    for(unsigned i=0;i<tem.size();++i)
        std::cout<<tem[i];
    std::cout<<std::endl;
    std::cout<<"(This is the msg header of URI:"<<uri<<std::endl;
    const Response& cacheResp=lruCache.getResponse(uri);//???
    if(cacheResp.getETag()==""&&cacheResp.getLM()==""){
        //test
        std::cout<<"I'm in revalidateAndSendBack func if-else statement, I don't have either ETag or LM"<<std::endl;
        if(sendClientReqHandler(lruCache, uri, pProxyClient, pHttpClient, clientMsg, clientSize, clientReq)==-1)
            return -1;
        return 0;
    }
    else if(cacheResp.getETag()!=""){//has ETag
    // it has return value, we use it for error-checking
        //test
        std::cout<<"I'm in revalidateAndSendBack func if-else statement, I have ETag"<<std::endl;
        return revalidateWithTagAndSendBack(lruCache, true, uri, clientMsg, clientSize, clientReq, cacheResp, pHttpClient, pProxyClient);
    }
    else{//has only LastModified
        //test
        std::cout<<"I'm in revalidateAndSendBack func if-else statement, I have only LastModified"<<std::endl;
        return revalidateWithTagAndSendBack(lruCache, false, uri, clientMsg, clientSize, clientReq, cacheResp, pHttpClient, pProxyClient);
    }
}

int revalidateWithTagAndSendBack(LRUCache& lruCache, const bool isETag, const std::string& uri, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq, const Response& cacheResp, const HttpClient* pHttpClient, const Client* pProxyClient){
    //test
    std::cout<<"I'm in revalidateWithTagAndSendBack func"<<std::endl;
    //test
    //return sendClientReqHandler(lruCache, uri, pProxyClient, pHttpClient, clientMsg, clientSize, clientReq);
    std::vector<char> recvMsg(MAX_SIZE, 0);
    int recvSize=0;
    //insert revalidation header, do we need to add '\0'???
    // std::vector<char> revalMsg(clientMsg.begin(), clientMsg.end()); //it's wrong
    std::vector<char> revalMsg;
    for(int i=0;i<clientSize;++i)
        revalMsg.emplace_back(clientMsg[i]);
    std::string revalHeader;
    if(isETag){
        revalHeader="If-None-Match: "+cacheResp.getETag()+"\r\n\r\n";
    }
    else{
        revalHeader="If-Modified-Since: "+cacheResp.getLM()+"\r\n\r\n";
    }
    unsigned oriEnd=revalMsg.size()-2;
    revalMsg.resize(revalMsg.size()-2+revalHeader.size());
    //added newly-created header one-by-one
    for(unsigned i=oriEnd;i<revalMsg.size();++i){
        revalMsg[i]=revalHeader[i-oriEnd];
    }
    //test
    std::cout<<"revalidation msg is :"<<std::endl;
    for(unsigned i=0;i<revalMsg.size();++i)
        std::cout<<revalMsg[i];
    std::cout<<std::endl;
    if(send(pProxyClient->socket_fd, &(revalMsg.data()[0]), revalMsg.size(), MSG_NOSIGNAL)==-1){// because it's short, don't use loopSend
        return -1;
    }
    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Requesting \""<<clientReq.getReqLine()<<"\" from "<<clientReq.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);
    if((recvSize=recv(pProxyClient->socket_fd, &(recvMsg.data()[0]), MAX_SIZE, 0))==-1){
        return -1;
    }
    //test
    std::cout<<"The first msg I received from server in revalidateWithTagAndSendBack func is:"<<std::endl;
    for(int i=0;i<recvSize;++i)
        std::cout<<recvMsg[i];
    std::cout<<std::endl;
    Response recvResp;
    recvResp.parseResponse(recvMsg, recvSize);
    //test
    std::cout<<"I am after parseResponse in revalidateWithTagAndSendBack func"<<std::endl;
    if(!recvResp.getIsValid()){
        //test
        std::cout<<"recvResp is not valid"<<std::endl;
        pthread_mutex_lock(&lock);
        logmsg<<pHttpClient->id<<": WARNING original server sent an invalid format response"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    //test
    std::cout<<"I am after getisValid in revalidateWithTagAndSendBack func"<<std::endl;
    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Received \""<<recvResp.getResponseLine()<<"\" from "<<clientReq.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);
    //test
    std::ofstream outputFile0("GETRevalResp.txt", std::ios::app);
    outputFile0 << recvResp.getResponseLine()+"\r\n"+recvResp.getHeader()+"\r\n";
    outputFile0.close();
    if(recvResp.getStatus()==304){// do we modify the Date inside the cache? It depends!!!!
        //test
        std::cout<<"I am in 304 in revalidateWithTagAndSendBack func"<<std::endl;

        if(recvResp.getMaxAge()!=""){
            lruCache.getResponse(uri).setMaxAge(recvResp.getMaxAge());//not sure if it works
            lruCache.getResponse(uri).setDate(recvResp.getDate());
            //test
            std::cout<<"Current Headers is: "<<recvResp.getHeader()<<std::endl;
        }
        else if(recvResp.getExpires()!=""){
            lruCache.getResponse(uri).setExpires(recvResp.getExpires());
            lruCache.getResponse(uri).setDate(recvResp.getDate());
            //test
            std::cout<<"Current Headers is: "<<recvResp.getHeader()<<std::endl;
        }
        //test
        std::cout<<"uri is"<<uri<<std::endl;
        //test
        std::ofstream outputFile1("GET304CacheResp.txt", std::ios::app);
        outputFile1 << uri+"\r\n"+lruCache.getResponse(uri).getResponseLine()+"\r\n"+lruCache.getResponse(uri).getHeader()+"\r\n";
        outputFile1.close();
        std::cout<<"I closed outputFile1"<<std::endl;
        std::vector<char> cacheMsg;
        try{
            cacheMsg=lruCache.getMsg(uri);//newly added
        }
        catch(...){
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": ERROR cache exceeded capacity, requested msg in cache has been deleted"<<std::endl; 
            pthread_mutex_unlock(&lock);
            return sendClientReqHandler(lruCache, uri, pProxyClient, pHttpClient, clientMsg, clientSize, clientReq);//ask server for msg
        }
        if(loopSend(pHttpClient->fd, cacheMsg, cacheMsg.size())==-1){//to be improved      
            return -1;
        }
        //test
        std::ofstream outputFile4("GET304CacheWholeResp.txt", std::ios::app);
        std::vector<char> temMsg=lruCache.getMsg(uri);
        outputFile4 << "This msg whole size is"<<temMsg.size()<<"\r\n";
        for(unsigned i=0;i<temMsg.size();++i)
            outputFile4 << temMsg[i];
        outputFile4.close();
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": Responding "<<lruCache.getResponse(uri).getResponseLine()<<std::endl; 
        pthread_mutex_unlock(&lock);
        return 0;
    }
    else{//200 ok, it's modified
        if(recvResp.getIsChunked()){//directly send to client
            //test
            std::cout<<"I am in else-isChunked block in revalidateWithTagAndSendBack func"<<std::endl;
            pthread_mutex_lock(&lock);
            logmsg<<pHttpClient->id<<": not cacheable because it's chunked"<<std::endl; 
            pthread_mutex_unlock(&lock);
            if(relayChunkedMsgToClient(recvMsg, recvSize, pHttpClient, pProxyClient)==-1)
                return -1;
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": Responding "<<recvResp.getResponseLine()<<std::endl; 
            pthread_mutex_unlock(&lock);
            return 0;
        }
        else{ //message from server is not chunked
            //test
            std::cout<<"I am in else-else block in revalidateWithTagAndSendBack func"<<std::endl;
            if(recvAllResponse(pProxyClient->socket_fd, recvMsg, recvSize, recvResp)==-1)
                return -1;
            if(handleGETresponse(pHttpClient, lruCache, uri, recvResp, recvMsg, recvSize)==-1)//return 0 or -1
                return -1;
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": Responding "<<recvResp.getResponseLine()<<std::endl; 
            pthread_mutex_unlock(&lock);
            return 0;
        }
    }
}

int loopSend(const int socket_fd, const std::vector<char>& msg, const int size){
    //test
    std::cout<<"I'm in loopSend func"<<std::endl;
    int sendSize = 0;
    int send_cur = 0;
    int try_send_size = size;
    //test
    std::cout<<"The size of msg to loopSend is: "<<size<<std::endl;
    std::cout<<"The msg to loopSend is:"<<std::endl;
    for(int i=0;i<size;++i)
        std::cout<<msg[i];
    std::cout<<std::endl;
    while ((sendSize = send(socket_fd, &(msg.data()[send_cur]), try_send_size, 0)) > 0)
    {
        send_cur = send_cur + sendSize;
        try_send_size = try_send_size - sendSize;
        if(try_send_size == 0)
        {
            return 0;
        }
     }
    if(sendSize == -1)
    {
        return -1;
    }
    //test
    std::cout<<"I met something wrong in loopSend func"<<std::endl;
    return 0;      
}

int shouldNotCache(const Response& resp)
{
    //test
    std::cout<<"I am in shouldNotCache func!"<<std::endl;
    if(resp.getStatus()!=200)
        return 1;
    if((resp.getNoStore() == true) || (resp.getPrivate() == true))
    {
        return 2;
    }
    if(resp.getNoCache() == true)
    {
        if((resp.getETag() == "") && (resp.getLM() == ""))
        {
            return 3;
        }

    }
    if ((resp.getETag() == "") && (resp.getLM() == "") && (resp.getMaxAge() == "") && (resp.getExpires() == ""))
    {
        return 4;
    }
    return 0;

}
//in the case of not chunked
int handleGETresponse(const HttpClient* pHttpClient, LRUCache& lruCache, const std::string& uri, const Response& recvResp, const std::vector<char>& recvMsg, const int recvSize){
    //test
    std::cout<<"I am in handleGETresponse func!"<<std::endl;
    if(!shouldNotCache(recvResp)){
        try{
            lruCache.put(uri, recvResp);
        }
        catch(...){
            return -1;
        }
        if(recvResp.getMaxAge()!=""){
            TimeInfo * timeinfo = new TimeInfo();
            timeinfo->parse(recvResp.getDate());
            struct tm * tm_s = timeinfo->convertGMT();
            int max_age_time = std::stoi(recvResp.getMaxAge() );
            time_t expire_time = mktime(tm_s) + max_age_time - 36000;
            struct tm * tm = gmtime(&expire_time);
            const char * t = asctime(tm);
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": cached, expires at "<<t<<std::endl; //remain to be filled
            pthread_mutex_unlock(&lock);
        }
        else if(recvResp.getExpires()!=""){
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": cached, expires at "<< recvResp.getExpires() <<std::endl; 
            pthread_mutex_unlock(&lock);
        }
        else{
            pthread_mutex_lock(&lock);
            logmsg <<pHttpClient->id<<": cached, but requires re-validation"<<std::endl; 
            pthread_mutex_unlock(&lock);
        }
        //test
        const Response& temResp=lruCache.getResponse(uri);
        const std::string& tem=temResp.getHeader();
        std::cout<<"I saved sth in cache whose headers is "<<tem.size()<<" bytes and is:"<<std::endl;
        for(unsigned i=0;i<tem.size();++i)
            std::cout<<tem[i];
        std::cout<<std::endl;
        std::cout<<"(uri is "<<uri<<std::endl;
    }
    else if(shouldNotCache(recvResp)==1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": not cacheable because response status is not 200 OK"<<std::endl; 
        pthread_mutex_unlock(&lock);
    }
    else if(shouldNotCache(recvResp)==2){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": not cacheable because there's no-store or private tag"<<std::endl; 
        pthread_mutex_unlock(&lock);
    }
    else if(shouldNotCache(recvResp)==3){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": not cacheable because there's no-cache tag but without revalidation header"<<std::endl; 
        pthread_mutex_unlock(&lock);
    }
    else if(shouldNotCache(recvResp)==4){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": not cacheable because there's no expiration and revalidation header"<<std::endl; 
        pthread_mutex_unlock(&lock);
    }
    if(loopSend(pHttpClient->fd, recvMsg, recvSize)==-1){
        return -1;
    } 
    return 0;

}//check if out-dated  in the order of  Max-age(has priority) and Expires
int validateFresh(Response& response, const HttpClient* pHttpClient)
{
    std::time_t curr_time = getCurTimeSecond();

    //test
    std::cout<<"current time is "<<curr_time<<std::endl;
    if(response.getMaxAge() != "")
    {
        time_t response_time = response.getDateSecond();
        int max_age_time = std::stoi(response.getMaxAge() );
        
        time_t expire_time = response_time + max_age_time;
        //test
        std::cout<<"Date is "<<response.getDate()<<std::endl;
        std::cout<<"DateSecond is "<<response.getDateSecond()<<std::endl;
        std::cout<<"ori max_age_time is "<<response.getMaxAge()<<std::endl;
        std::cout<<"int max_age_time is "<<max_age_time<<std::endl;
        std::cout<<"expire_time is "<<expire_time<<std::endl;
        if(expire_time < curr_time)//means it is stale
        {
            TimeInfo * timeinfo = new TimeInfo();
            timeinfo->parse(response.getDate());
            struct tm * tm_s = timeinfo->convertGMT();
            int max_age_time = std::stoi(response.getMaxAge() );
            time_t expire_time = mktime(tm_s) + max_age_time - 36000;
            struct tm * tm = gmtime(&expire_time);
            const char * t = asctime(tm);
            pthread_mutex_lock(&lock);
            logmsg << pHttpClient->id << ": in cache, but expired at " << t<<std::endl;
            pthread_mutex_unlock(&lock);
            //it is stale
            return -1;

        }
        else
        {   
            pthread_mutex_lock(&lock);
            logmsg << pHttpClient->id << ": in cache, valid" <<std::endl;
            pthread_mutex_unlock(&lock);
            //it is fresh
            return 0;
        }
    }
    if(response.getExpires() != "")
    {
        time_t expire_time_ex = response.getExpiresSecond();
        std::string expire = response.getExpires();
        //test
        std::cout<<"expire_time is "<<expire_time_ex<<std::endl;
        if(expire_time_ex < curr_time)
        {
             //it is stale
            pthread_mutex_lock(&lock);
            logmsg << pHttpClient->id << ": in cache, but expired at " << expire <<std::endl;
            pthread_mutex_unlock(&lock);
            return -1;
        }
        else
        {   
            pthread_mutex_lock(&lock);
            logmsg << pHttpClient->id << ": in cache, valid" <<std::endl;
            pthread_mutex_unlock(&lock);
            //it is fresh
            return 0;
        }
    }
    //it means have no two tags,must revalidate
    pthread_mutex_lock(&lock);
    logmsg << pHttpClient->id << ": in cache, requires validation" << std::endl;
    pthread_mutex_unlock(&lock);
    return -2;
}

int sendClientReqHandler(LRUCache& lruCache, const std::string& uri, const Client* pProxyClient, const HttpClient* pHttpClient, const std::vector<char>& clientMsg, const int clientSize, const Request& clientReq){
    //test
    std::cout<<"I am in sendClientReqHandler func!"<<std::endl;
    std::vector<char> recvMsg(MAX_SIZE, 0);
    int recvSize=0;
    //test
    std::cout<<"The size of clientMsg is : "<<clientSize<<std::endl;
    std::cout<<"clientMsg is :"<<std::endl;
    for(int i=0;i<clientSize;++i)
        std::cout<<clientMsg[i];
    std::cout<<std::endl;
    //test
    std::cout<<"I started sending client req to origianl server!"<<std::endl;
    // //test
    // if(send(pProxyClient->socket_fd, &(clientMsg.data()[0]), clientSize, MSG_NOSIGNAL)==-1){
    //     return -1;
    // }
    if(send(pProxyClient->socket_fd, &(clientMsg.data()[0]), clientSize, MSG_NOSIGNAL)==-1){//actually no need to use loopSend 
        return -1;
    }
    //test
    std::cout<<"I finisheded sending client req to origianl server!"<<std::endl;
    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Requesting \""<<clientReq.getReqLine()<<"\" from "<<clientReq.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);
    if((recvSize=recv(pProxyClient->socket_fd, &(recvMsg.data()[0]), MAX_SIZE, 0))==-1){
        return -1;
    }
    Response recvResp;
    //test
    std::cout<<"The recvSize of first msg is "<<recvSize<<std::endl;
    std::cout<<"recvMsg is :"<<std::endl;
    for(int i=0;i<recvSize;++i)
        std::cout<<recvMsg[i];
    std::cout<<std::endl;
    std::cout<<"I started parseResponse!"<<std::endl;
    recvResp.parseResponse(recvMsg, recvSize);
    if(!recvResp.getIsValid()){
        pthread_mutex_lock(&lock);
        logmsg<<pHttpClient->id<<": WARNING original server sent an invalid format response"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    //test
    std::cout<<"I finished parseResponse!"<<std::endl;
    //test
    std::ofstream outputFile2("GETDirectResp.txt", std::ios::app);
    outputFile2 << recvResp.getResponseLine()+"\r\n"+recvResp.getHeader()+"\r\n";
    outputFile2.close();

    pthread_mutex_lock(&lock);
    logmsg<<pHttpClient->id<<": Received \""<<recvResp.getResponseLine()<<"\" from "<<clientReq.getTargetHost()<<std::endl; 
    pthread_mutex_unlock(&lock);
    if(recvResp.getIsChunked()){//directly send to client
        pthread_mutex_lock(&lock);
        logmsg<<pHttpClient->id<<": not cacheable because it's chunked"<<std::endl; 
        pthread_mutex_unlock(&lock);
        //test
        std::cout<<"I started relayChunkedMsgToClient func!"<<std::endl;
        if(relayChunkedMsgToClient(recvMsg, recvSize, pHttpClient, pProxyClient)==-1)
            return -1;
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": Responding "<<recvResp.getResponseLine()<<std::endl; 
        pthread_mutex_unlock(&lock);
        //test
        std::cout<<"I'm done relayChunkedMsgToClient func!"<<std::endl;
        return 0;
    }
    else{ //message from server is not chunked
        if(recvAllResponse(pProxyClient->socket_fd, recvMsg, recvSize, recvResp)==-1)
            return -1;
        if(handleGETresponse(pHttpClient, lruCache, uri, recvResp, recvMsg, recvSize)==-1)//return 0 or -1
            return -1;
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": Responding "<<recvResp.getResponseLine()<<std::endl; 
        pthread_mutex_unlock(&lock);
        return 0;
    }
}
int createClientThenRevalidateAndSendBack(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Request& req, const std::vector<char>& msg, int size){
    Client* pProxyClient;
    try{ // create a client connected to the original server
        pProxyClient = new Client(req.getTargetHost().c_str(), req.getTargetPort().c_str()); // is it safe?
    }
    catch(const int& e){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed to create a client connected to the original server in GET request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if(revalidateAndSendBack(lruCache, uri, pHttpClient, pProxyClient, msg, size, req)==-1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed in the revalidation or following sending back process in GET request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    return 0;
}
int createClientThenSendClientReqHandler(LRUCache& lruCache, const std::string& uri, const HttpClient* pHttpClient, const Request& req, const std::vector<char>& msg, int size){
    //test
    std::cout<<"I am in createClientThenSendClientReqHandler func!"<<std::endl;
    Client* pProxyClient;
    //test
    std::cout<<"pProxyClient's targetHost is "<<req.getTargetHost()<<std::endl;
    std::cout<<"pProxyClient's targetPort is "<<req.getTargetPort()<<std::endl;
    try{ // create a client connected to the original server
        pProxyClient = new Client(req.getTargetHost().c_str(), req.getTargetPort().c_str()); // is it safe?
    }
    catch(const int& e){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed to create a client connected to the original server in GET request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    // //test
    // loopSend(pProxyClient->socket_fd, msg, size);
    // std::vector<char> buf(MAX_SIZE, 0);
    // std::cout<<"Test: The size of first message recived is "<<recv(pProxyClient->socket_fd, &(buf.data()[0]),65536, 0)<<std::endl;
    //test
    std::cout<<"I started sendClientReqHandler func!"<<std::endl;
    if(sendClientReqHandler(lruCache, uri, pProxyClient, pHttpClient, msg, size, req)==-1){
        pthread_mutex_lock(&lock);
        logmsg <<pHttpClient->id<<": ERROR Failed to send or recv info in GET request"<<std::endl; 
        pthread_mutex_unlock(&lock);
        return -1;
    }
    //test
    std::cout<<"I'm done with sendClientReqHandler func and exit createClientThenSendClientReqHandler func!"<<std::endl;
    return 0;
}
bool fitsMaxStale(const std::string& maxStale, const Response& response){
    time_t curr_time = getCurTimeSecond();
    time_t expired_time;
    if(response.getMaxAge()!=""){
        expired_time = response.getDateSecond() + std::stoi(response.getMaxAge());
    }
    else{
        expired_time = response.getExpiresSecond();
    }
    return std::stoi(maxStale)>=(curr_time-expired_time);
}

std::string getCurTime(){
    time_t now = time(0);
    tm * nowTime = gmtime(&now);
    const char * t = asctime(nowTime);
    return std::string(t);
}

// time_t getCurTimeSecond(){
//     // Get current GMT time
//     time_t now = time(0);
//     tm * nowTime = gmtime(&now);

//     // Convert current GMT time to time_t value
//     return std::mktime(nowTime);
// }
time_t getCurTimeSecond(){
    // Get current GMT time
    
    std::time_t current_time = std::time(0);

    struct tm * gmt = gmtime(&current_time);
    

    char timeStr[80];
    std::strftime(timeStr, sizeof(timeStr), "%a, %d %b %Y %H:%M:%S GMT", gmt);
   
    std::string currentDateTime(timeStr);
    std::cout<<"cur time string is "<< currentDateTime << std::endl;


    TimeInfo * timeinfo = new TimeInfo();

    timeinfo->parse(currentDateTime);

    tm * result = timeinfo->convertGMT();
    std::time_t time = std::mktime(result);

    return time;
}