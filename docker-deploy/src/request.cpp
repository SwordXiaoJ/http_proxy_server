#include"request.hpp"
#include<iostream>

// if the requeset is illegal, set isValid as true, don't throw exception or log messages
void Request::parseRequest(const std::vector<char>& msg, const int size) {
    //test
    std::cout<<"started to parse request"<<std::endl;
    std::string request;
    for(int i=0;i<size;++i)
        request+=msg[i];
    // to be added
    parseHeaderBody(request);
   
    parseMethod(request);
    parseHostAndPort(request);
    parseURI(request);
    parseContentLen(request);
    parseCache(request);
    parseIsChunked(request);
    //test
    std::cout<<"finished parsing request"<<std::endl;
}

std::string Request::getReqLine() const{
    return reqLine;
}
std::string Request::getMethod() const{
    return method;
}
std::string Request::getTargetHost() const{
    return targetHost;
}
std::string Request::getTargetPort() const{
    return targetPort;
}
std::string Request::getContentLen() const{
    return content_length;

}
std::string Request::getHeader() const{
    return header;

}
std::string Request::getBody() const{
    return body;

}
std::string Request::getURI() const{
    return URI;

}
std::string Request::getStale() const{
    return cache_max_stale;
}
bool Request::getIsValid() const{
    return isValid;
}
bool Request::getIsChunked() const{
    return isChunked;
}

void Request::parseMethod(std::string request){
    //test
    std::cout<<"started parsing header and body"<<std::endl;
    size_t method_cur = request.find(" ");
    if (method_cur == std::string::npos)
    {
        isValid = isValid && false;
        return;
    }
   
    method = request.substr(0, method_cur);
    //test
    std::cout<<"finished parsing method"<<std::endl;
}

void Request::parseContentLen(std::string request) {
    //test
    std::cout<<"started parsing ContentLen"<<std::endl;
    content_length = parseFunc(request, "Content-Length: ");
    //test
    std::cout<<"finished parsing ContentLen"<<std::endl;
}

void Request::parseHostAndPort(std::string request)
{   
    //test
    std::cout<<"started parsing HostAndPort"<<std::endl;
   
    size_t Host_cur = request.find("Host: ");
    if (Host_cur == std::string::npos)
    {
        isValid = isValid && false;
        return;
    }
    std::string host_port_temp = request.substr(Host_cur + 6);
    size_t end = host_port_temp.find("\r\n");
    if (end == std::string::npos)
    {
        isValid = isValid && false;
        return;
    }
    std::string host_port = host_port_temp.substr(0, end);
    size_t colon_cur = host_port.find(":");
    if (colon_cur == std::string::npos)
    {
        //not include the port number
        targetHost = host_port;
        targetPort = "80";
    }
    else
    {
        targetHost = host_port.substr(0, colon_cur);
        targetPort = host_port.substr(colon_cur + 1);
    }
    //test
    std::cout<<"finished parsing HostAndPort"<<std::endl;

}


void Request::parseHeaderBody(std::string request)
{
    // //test
    // std::cout<<"started to parse header and body"<<std::endl;
    size_t body_cur = request.find("\r\n\r\n");
    if(body_cur == std::string::npos)
    {
       isValid = isValid && false;
       return;
    }
    //test
    // std::cout<<"In line 121"<<std::endl;
    std::string header_line = request.substr(0, body_cur + 2);
    //test
    // std::cout<<"In line 124"<<std::endl;
    size_t reqline_cur = header_line.find("\r\n");
    if (reqline_cur == std::string::npos)
    {
        isValid = isValid && false;
        return;
    }
    //test
    // std::cout<<"In line 131"<<std::endl;
    //test
    // std::cout<<"In line 133"<<std::endl;
    reqLine = request.substr(0, reqline_cur);
    //test
    // std::cout<<"In line 136"<<std::endl;
    header = header_line.substr(reqline_cur + 2);
    //test
    // std::cout<<"In line 139"<<std::endl;
    body = request.substr(body_cur + 4);
    // //test
    // std::cout<<"finished parsing header and body"<<std::endl;
}

//find label(xx: ) in the response,if find,return the string;if not,return ""
std::string Request::parseFunc(std::string response, std::string label)
{
    // //test
    // std::cout<<"started to execute parseFunc"<<std::endl;
    size_t start_cur = response.find(label);
    if(start_cur == std::string::npos)
    {
       return "";
    }
    std::string label_temp = response.substr(start_cur + label.length());
    size_t end = label_temp.find("\r\n");
    if(end == std::string::npos)
    {
       return "";
    }
    
    std::string label_str = label_temp.substr(0, end);
    // //test
    // std::cout<<"finished executing parseFunc"<<std::endl;
    return label_str;
}

void Request::parseCache(std::string request)
{
    // //test
    // std::cout<<"started to parse cache"<<std::endl;
    std::string cache = parseFunc(request,"Cache-Control: ");
    cache = cache + "\r\n";
  

    cache_max_stale = cacheParse(cache,"max-stale=");
    // //test
    // std::cout<<"finished parsing cache"<<std::endl;
}

//at the end of the cache ,no \r\n
std::string Request::cacheParse(std::string cache,std::string label)
{
    size_t find_result = cache.find(label);
    if(find_result == std::string::npos)
    {
       return "";
    }
    std::string label_temp = cache.substr(find_result + label.length());
    size_t end = label_temp.find(" ");
    if(end == std::string::npos)
    {
       return "";
    }
    std::string label_str = label_temp.substr(0, end);
    return label_str;

}
void Request::parseURI(std::string request){

    size_t URI_cur = reqLine.find(" ");
    if(URI_cur == std::string::npos)
    {
        isValid = isValid && false;
        return;
    }
    std::string after_method = reqLine.substr(URI_cur+1);
    size_t URI_end = after_method.find(" ");

    URI = after_method.substr(0,URI_end);

    //test
    //std::cout<<"URI is " <<URI<<std::endl;
}

void Request::parseIsChunked(std::string request){

    std::string chunked_label = "Transfer-Encoding: ";
    std::string IncludeChunked = parseFunc(request, chunked_label);
    if(IncludeChunked == "")
    {
        isChunked = false;
    }
    else
    {
        if(IncludeChunked.find("chunked") != std::string::npos)
        {
            isChunked = true;
        }
        else
        {
            isChunked = false;
        }
    }
}