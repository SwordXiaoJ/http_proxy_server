#include<ctime>
#include<string>
#include<map>
class timemap {
 public:
  std::map<std::string, int> time_map;

 public:
  timemap() {
    time_map.insert(std::pair<std::string, int>("Jan", 1));
    time_map.insert(std::pair<std::string, int>("Feb", 2));
    time_map.insert(std::pair<std::string, int>("Mar", 3));
    time_map.insert(std::pair<std::string, int>("Apr", 4));
    time_map.insert(std::pair<std::string, int>("May", 5));
    time_map.insert(std::pair<std::string, int>("Jun", 6));
    time_map.insert(std::pair<std::string, int>("Jul", 7));
    time_map.insert(std::pair<std::string, int>("Aug", 8));
    time_map.insert(std::pair<std::string, int>("Sep", 9));
    time_map.insert(std::pair<std::string, int>("Oct", 10));
    time_map.insert(std::pair<std::string, int>("Nov", 11));
    time_map.insert(std::pair<std::string, int>("Dec", 12));
    time_map.insert(std::pair<std::string, int>("Sun", 0));
    time_map.insert(std::pair<std::string, int>("Mon", 1));
    time_map.insert(std::pair<std::string, int>("Tue", 2));
    time_map.insert(std::pair<std::string, int>("Wed", 3));
    time_map.insert(std::pair<std::string, int>("Thu", 4));
    time_map.insert(std::pair<std::string, int>("Fri", 5));
    time_map.insert(std::pair<std::string, int>("Sat", 6));
  }
  int getMap(std::string str) { return time_map.find(str)->second; }
};


class TimeInfo{
    struct tm timeinfo;

public:
    TimeInfo() {}
    void parse(std::string time_str)
    {
        timemap dt_map;
        //test
        std::cout<<"time_str size is"<<time_str<<std::endl;
        std::cout<<"time_str is"<<time_str<<std::endl;
        this->timeinfo.tm_sec = std::stoi(time_str.substr(23),nullptr);
        //test
        std::cout<<"after time_str.substr(23) statement in parse"<<std::endl;
        this->timeinfo.tm_min = std::stoi(time_str.substr(20),nullptr);
        this->timeinfo.tm_hour = std::stoi(time_str.substr(17),nullptr);
        this->timeinfo.tm_wday = dt_map.getMap(time_str.substr(0, 3).c_str());
        this->timeinfo.tm_mday =  std::stoi(time_str.substr(5),nullptr);
        this->timeinfo.tm_mon = dt_map.getMap(time_str.substr(8, 3).c_str()) - 1;
        this->timeinfo.tm_year = std::stoi(time_str.substr(12),nullptr) - 1900;
        this->timeinfo.tm_isdst = 0;
    }

    struct tm * convertGMT()
    {
        time_t t = mktime(&this->timeinfo);
        struct tm * gmt = gmtime(&t);
        return gmt;
    }
    

};