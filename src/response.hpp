#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__
#include<string>
#include<vector>

class Response{
    private:
        int status;
        std::string responseLine;//has no "\r\n" at the end
        std::string content_length;
        
        bool isValid;
        bool isChunked;
        //to be added
        
        //maybe used
        std::string header;//has one "\r\n" at the end
        std::string body;//has no "\r\n" at the end

        std::string ETag;
        std::string Expires;
        time_t ExpiresSecond;
        std::string Date;
        time_t DateSecond;
        std::string LastModified;
        time_t LMSecond;
    
        std::string cache_max_age;
        bool cache_no_store = false;
        bool cache_must_revalidate= false;
        bool cache_no_cache= false;
        bool cache_resprivate= false;
        bool cache_respublic= false;

    public:
        Response():isValid(true),isChunked(false){}
        void parseResponse(const std::vector<char>& msg, const int size);
        int getStatus() const;
        std::string getResponseLine() const;
        std::string getContentLen() const;
        bool getIsValid() const;
        bool getIsChunked() const;
        std::string getBody() const;
        std::string getHeader() const;
        std::string getDate() const;
        time_t getDateSecond() const;
        std::string getExpires() const;
        time_t getExpiresSecond() const;
        std::string getLM() const;//LastModified
        time_t getLMSecond() const;
        std::string getETag() const;
        std::string getMaxAge() const;
        bool getNoStore() const;
        bool getNoCache() const;
        bool getMustVal() const;
        bool getPrivate() const;
        bool getPublic() const;

        void parseHeaderBody(std::string response);
        void parseContentLen(std::string response);
        void parseStatus(std::string response);
        void parseETag(std::string response);
        void parseDate(std::string response);
        void parseExpires(std::string response);
        void parseLM(std::string response);

        std::string parseFunc(const std::string& response, const std::string& label);

        time_t convertTime(std::string time_str);
        void parseCache(std::string response);
        bool findLabel(std::string str,std::string label);
        std::string cacheParse(std::string cache,std::string label);
        void parseIsChunked(std::string response);

        //change not only cache_max_age(and Expires, ExpiresSecondand, Date, DateSecond) but also header
        void setMaxAge(const std::string& maxAge);
        void setExpires(const std::string& expires);
        void setDate(const std::string& date);
        //can't modify the response content via getWholeMsg()
        //getWholeMsg function is to put info together into a vector<char> so that we can send it in send or loopSend func
        std::vector<char> getWholeMsg();
};

#endif