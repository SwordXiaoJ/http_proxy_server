#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__
#include<string>
#include<vector>
class Request{
    private:
        std::string method;
        std::string reqLine;
        std::string targetHost;
        std::string targetPort;
        std::string content_length;

        //maybe used
        //header include \r\n in the end
        std::string header;
        //body include \r\n in the end
        std::string body;
        std::string URI;//include hostname and port
       
        bool isValid;
        bool isChunked;
        // to be added

        std::string cache_max_stale;

    public:
        Request():isValid(true),isChunked(false){}
        //call mutiple parse functions inside and change the value of isValid
        void parseRequest(const std::vector<char>& msg, const int size);
        std::string getReqLine() const;
        std::string getMethod() const;
        std::string getTargetHost() const;
        std::string getTargetPort() const;
        std::string getContentLen() const;
        std::string getHeader() const;
        std::string getStale() const;
        std::string getBody() const;
        std::string getURI() const; 
        bool getIsValid() const;
        bool getIsChunked() const;

        void parseMethod(std::string request);
        void parseContentLen(std::string request);
        void parseHostAndPort(std::string request);
        void parseHeaderBody(std::string request);
        void parseCache(std::string request);
        void parseURI(std::string request);//currently is the easiest way!!!
        void parseIsChunked(std::string request);
        //auxiliary func
        std::string parseFunc(std::string response, std::string label);
        std::string cacheParse(std::string cache,std::string label);

};

#endif