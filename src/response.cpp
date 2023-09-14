#include"response.hpp"
#include<iostream>
#include<ctime>
#include <iomanip>
#include <sstream>
#include"timeInfo.hpp"
// if the response is illegal, set isValid as false, don't throw exception or log messages
void Response::parseResponse(const std::vector<char>& msg, const int size){
    //test
    std::cout<<"I started parseResponse"<<std::endl;
    std::string response;
    for(int i=0;i<size;++i)
        response+=msg[i];
    // test
    std::cout<<"The size of msg to parse is "<<size<<std::endl;
    parseHeaderBody(response);
    parseContentLen(response);
    parseStatus(response);
    parseETag(response);
    parseDate(response);
    parseExpires(response);
    parseLM(response);
    parseCache(response);
    parseIsChunked(response);
    //test
    std::cout<<"I finished parseResponse"<<std::endl;
}
int Response::getStatus()const {
    return status;
}
std::string Response::getResponseLine() const{
    return responseLine;
}
std::string Response::getContentLen() const{
    return content_length;
}
bool Response::getIsValid() const{
    return isValid;
}
bool Response::getIsChunked() const{
    return isChunked;
}

std::string Response::getBody() const{
    return body;
}

std::string Response::getHeader() const{
    return header;
}
std::string Response::getDate() const{
    return Date;
}
time_t Response::getDateSecond() const{
    return DateSecond;
}
std::string Response::getExpires() const{
    return Expires;
}
time_t Response::getExpiresSecond() const{
    return ExpiresSecond;
}
std::string Response::getLM() const{
    return LastModified;
}
time_t Response::getLMSecond() const{
    return LMSecond;
}
std::string Response::getETag() const{
    return ETag;
}
std::string Response::getMaxAge() const{
    return cache_max_age;
}
bool Response::getNoStore() const{
    return cache_no_store;
}
bool Response::getNoCache() const{
    return cache_no_cache;
}
bool Response::getMustVal() const{
    return cache_must_revalidate;
}
bool Response::getPrivate() const{
    return cache_resprivate;
}
bool Response::getPublic() const{
    return cache_respublic;
}

void Response::parseHeaderBody(std::string response)
{
    try{
        //test
    std::cout<<"started parseHeaderBody"<<std::endl;
    size_t body_cur = response.find("\r\n\r\n");
    if(body_cur == std::string::npos)
    {   
        //test
        std::cout<<"lack body_cur"<<std::endl;
       isValid = isValid && false;
       return;
    }

    std::string header_line = response.substr(0, body_cur + 2);
    size_t reqline_cur = header_line.find("\r\n");
    if (reqline_cur == std::string::npos)
    {   
        //test
        std::cout<<"lack reqline_cur"<<std::endl;
        isValid = isValid && false;
        return;
    }

    responseLine = response.substr(0, reqline_cur);
    header = header_line.substr(reqline_cur + 2);
    body = response.substr(body_cur + 4);
    }
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"finished parseHeaderBody"<<std::endl;
}

void Response::parseContentLen(std::string response) {
    try{
        //test
    std::cout<<"started parsing ContentLen"<<std::endl;
    content_length = parseFunc(response, "Content-Length: ");
    }
    catch(...){
        isValid=false;
    }
        //test
    std::cout<<"finished parsing ContentLen"<<std::endl;
}

void Response::parseStatus(std::string response)
{
    try{
        //test
    std::cout<<"I started parseStatus!"<<std::endl;
    size_t status_cur = response.find(" ");
    if (status_cur == std::string::npos)
    {
        //test
        std::cout<<"lack status_cur 0"<<std::endl;
        isValid = isValid && false;
        return;
    }
    std::string status_str_temp = response.substr(status_cur);
    size_t end = status_str_temp.find("\r\n");
    std::string status_str = status_str_temp.substr(1, end);

    size_t split_cur = status_str.find(" ");
    if (status_cur == std::string::npos)
    {
        //test
        std::cout<<"lack status_cur 1"<<std::endl;
        isValid = isValid && false;
        return;
    }
    std::string status_code = status_str.substr(0, split_cur);
    std::string status_word = status_str.substr(split_cur + 1);
    status = std::stoi(status_code);
    }
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"I'm done with parseStatus!"<<std::endl;
}


void Response::parseETag(std::string response)
{
    try{
        //test
    std::cout<<"I started parseETag!"<<std::endl;
    std::string ETag_label0 = "ETag: ";
    std::string ETag_label1 = "Etag: ";
    std::string tem = parseFunc(response, ETag_label0);
    ETag = (tem!="") ? tem : parseFunc(response, ETag_label1);
    }
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"I finished parseETag!"<<std::endl;
}

void Response::parseDate(std::string response)
{
    // //test
    // std::cout<<"I started parseDate!"<<std::endl;
    // std::string Dates_label = "Date: "; 
    // std::string Dates_str = parseFunc(response, Dates_label);
    
    // Date = Dates_str;
    // if(Dates_str == "")
    // {
    //     isValid = isValid && false;
    //     return;
    // }
   
    // DateSecond = convertTime(Dates_str);
    // //test
    // std::cout<<"I finished parseDate!"<<std::endl;
    
    //test
    std::cout<<"I started parseDate!"<<std::endl;
    try{
        std::string Dates_label = "Date: "; 
    std::string Dates_str = parseFunc(response, Dates_label);
    
    Date = Dates_str;
    if(Dates_str == "")
    {
        //test
        std::cout<<"lack Dates_str"<<std::endl;
        isValid = isValid && false;
        return;
    }

    TimeInfo * timeinfo = new TimeInfo();
    timeinfo->parse(Dates_str);
    tm * result = timeinfo->convertGMT();

    DateSecond = std::mktime(result);;
    }
    catch(...){
        isValid=false;
    }
   
    //test
    std::cout<<"Date is : "<<Date<<std::endl;
    std::cout<<"DateSecond is : "<<DateSecond<<std::endl;
    std::cout<<"I finished parseDate!"<<std::endl;
}

void Response::parseExpires(std::string response)
{
    // //test
    // std::cout<<"I started parseExpires!"<<std::endl;
    // std::string Expires_label = "Expires: ";
    // std::string Expires_str = parseFunc(response, Expires_label);
    // Expires = Expires_str;
    // if(Expires_str == "")
    // {
    //     return;
    // }
    // ExpiresSecond = convertTime(Expires_str);
    // //test
    // std::cout<<"I finished parseExpires!"<<std::endl;

    //test
    std::cout<<"I started parseExpires!"<<std::endl;
    try{
        std::string Expires_label = "Expires: "; 
        std::string Expires_str = parseFunc(response, Expires_label);
    
        Expires = Expires_str;
        if(Expires_str == "" || Expires_str.size()!=29)
        {
        return;
        }

        TimeInfo * timeinfo = new TimeInfo();
        timeinfo->parse(Expires_str);
        tm * result = timeinfo->convertGMT();

        ExpiresSecond = std::mktime(result);;
        std::cout<<"DateSecond in parseExpires is : "<<ExpiresSecond<<std::endl;
    }
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"I finished parseExpires!"<<std::endl;
}

void Response::parseLM(std::string response)
{
    // //test
    // std::cout<<"I started parseLM!"<<std::endl;
    // std::string LM_label = "Last-Modified: ";
    // std::string LM_str = parseFunc(response, LM_label);
    // LastModified = LM_str;
    // if(LM_str == "")
    // {
    //     return;
    // }

    // LMSecond = convertTime(LM_str);
    // //test
    // std::cout<<"I finished parseLM!"<<std::endl;

    //test
    std::cout<<"I started parseLM!"<<std::endl;
    try{
        std::string LM_label = "Last-Modified: "; 
    std::string LM_str = parseFunc(response, LM_label);
    
    LastModified = LM_str;
    if(LM_str == "")
    {
        return;
    }

    TimeInfo * timeinfo = new TimeInfo();
    timeinfo->parse(LM_str);
    tm * result = timeinfo->convertGMT();

    LMSecond = std::mktime(result);
    }
    catch(...){
        isValid=false;
    }
    std::cout<<"LMSecond in parseLM is : "<<LMSecond<<std::endl;

    //test
    std::cout<<"I finished parseLM!"<<std::endl;
}


std::string Response::parseFunc(const std::string& response, const std::string& label)
{
    size_t start_cur = response.find(label);
    //test
    std::cout<<"label is : "<<label<<std::endl;
    std::cout<<"start_cur is : "<<start_cur<<std::endl;
    if(start_cur == std::string::npos)
    {
       return "";
    }
    //test
    std::cout<<"parseFunc substr started"<<std::endl;
    std::cout<<"response size is:"<<response.size()<<std::endl;
    std::string label_temp = response.substr(start_cur + label.length());
    //test
    std::cout<<"parseFunc substr finished"<<std::endl;
    size_t end = label_temp.find("\r\n");
    if(end == std::string::npos)
    {
       return "";
    }
    std::string label_str = label_temp.substr(0, end);
    //test
    std::cout<<"parseFunc finished"<<std::endl;
    return label_str;
}

// time_t Response::convertTime(std::string time_str)
// {
//     //test
//     std::cout<<"In convertTime func, date string passed in is: "<<time_str<<std::endl;
//     //std::string datetime_str = "Mon, 27 Feb 2023 00:55:00 GMT";
//     //const char* format_str = "%a, %d %b %Y %H:%M:%S %Z";
//     // struct tm tm;
//     // strptime(datetime_str.c_str(), format_str, &tm);
//     // time_t timestamp = mktime(&tm);
//     // std::cout<<timestamp;

//     struct tm tm_;
//     std::istringstream ss(time_str);
//     ss >> std::get_time(&tm_,"%a, %d %b %Y %H:%M:%S %Z");
//     //const char* format_str = "%a, %d %b %Y %H:%M:%S %Z";
//     //strptime(time_str.c_str(), format_str, &tm_);
    
//     time_t t_ = mktime(&tm_);
//     //test
//     std::cout<<"In convertTime func, second time is: "<<t_<<std::endl;
//     return t_;
// }

bool Response::findLabel(std::string str,std::string label)
{
    size_t find_result = str.find(label);
    if(find_result  == std::string::npos)
    {
       return false;
    }
    else
    {
        return true;
    }
}

void Response::parseCache(std::string response)
{
    //test
    std::cout<<"I started parseCache!"<<std::endl;
    try{
        std::string cache = parseFunc(response,"Cache-Control: ");
        cache = cache + "\r\n";
        cache_max_age= cacheParse(cache,"max-age=");
    
        cache_no_cache = findLabel(cache,"no-cache");
        cache_no_store = findLabel(cache,"no-store");
        cache_must_revalidate = findLabel(cache,"must-revalidate");
        cache_resprivate = findLabel(cache,"private");
        cache_respublic = findLabel(cache,"public");
    }
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"I finished parseCache!"<<std::endl;
}

//at the end of the cache ,no \r\n
std::string Response::cacheParse(std::string cache,std::string label)
{
    size_t find_result = cache.find(label);
    if(find_result == std::string::npos)
    {
       return "";
    }
    std::string label_temp = cache.substr(find_result + label.length());
    size_t end = label_temp.find(",");
    if(end == std::string::npos)
    {
        end = label_temp.find("\r\n");
        if(end == std::string::npos)
        {
            return "";
        }
       
    }
    std::string label_str = label_temp.substr(0, end);
    return label_str;

}

void Response::parseIsChunked(std::string response){
    //test
    std::cout<<"I started parseIsChunked"<<std::endl;
    try{
        std::string chunked_label = "Transfer-Encoding: ";
    std::string IncludeChunked = parseFunc(response, chunked_label);
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
    catch(...){
        isValid=false;
    }
    //test
    std::cout<<"I finished parseIsChunked"<<std::endl;
}

void Response::setMaxAge(const std::string& maxAge)
{
    std::string cache = parseFunc(header,"Cache-Control: ");
    cache = cache + "\r\n";
   
    std::string to_replace = cacheParse(cache,"max-age=");
    if(to_replace == "")
    {
        return;
    }

    size_t index = header.find(to_replace);
    if (index != std::string::npos) {
        header.replace(index, to_replace.length(), maxAge);
    }
    cache_max_age=maxAge;
    //test
    std::cout<<"I wanted to change maxAge to "<<maxAge<<std::endl;
    std::cout<<"Now actual maxAge in the response is:"<<cache_max_age<<std::endl;
}
void Response::setExpires(const std::string&  expires)
{
    try{
        std::string Dates_label = "Expires: "; 
        std::string to_replace = parseFunc(header, Dates_label);

        if(to_replace == "")
        {
            return;
        }

        size_t index = header.find(to_replace);
        //test
        std::cout<<"Inside setExpires func, index is : "<<index<<std::endl;
        std::cout<<"Inside setExpires func, to_replace is : "<<to_replace<<std::endl;
        if (index != std::string::npos) {
        header.replace(index, to_replace.length(), expires);
        }
        //test
        std::cout<<"Inside setExpires func, after replace func"<<index<<std::endl;
        TimeInfo * timeinfo = new TimeInfo();
        timeinfo->parse(expires);
        tm * result = timeinfo->convertGMT();
        //test
        std::cout<<"Inside setExpires func, after timesecond change func "<<index<<std::endl;
        Expires=expires;
        ExpiresSecond = std::mktime(result);
    }
    catch(...){
        //do nothing
    }
    //test
    std::cout<<"I want to change Expires to "<<expires<<std::endl;
    std::cout<<"Now actual Expires in the response is:"<<Expires<<std::endl;
    std::cout<<"Now actual ExpiresSecond in the response is:"<<ExpiresSecond<<std::endl;

}
void Response::setDate(const std::string& date)
{
    try{
        std::string Dates_label = "Date: "; 
        std::string to_replace = parseFunc(header, Dates_label);

        if(to_replace == "")
        {
        return;
        }

        size_t index = header.find(to_replace);
        if (index != std::string::npos) {
            header.replace(index, to_replace.length(), date);
        }

        TimeInfo * timeinfo = new TimeInfo();
        timeinfo->parse(date);
        tm * result = timeinfo->convertGMT();
        DateSecond = std::mktime(result);
        Date=date;
    }
    catch(...){
        //do nothing
    }
    //test
    std::cout<<"I wanted to change Date to "<<date<<std::endl;
    std::cout<<"Now actual Date in the response is:"<<Date<<std::endl;
    std::cout<<"Now actual DateSecond in the response is:"<<DateSecond<<std::endl;
}


std::vector<char> Response::getWholeMsg()
{
    std::string wholeMsgStr = responseLine + "\r\n" + header + "\r\n" + body + "\r\n\r\n";//added "\r\n"

    std::vector<char> wholeMsgVector(wholeMsgStr.begin(), wholeMsgStr.end());

    return wholeMsgVector;

}
