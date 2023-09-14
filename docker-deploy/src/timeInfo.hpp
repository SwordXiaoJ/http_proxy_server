#include<ctime>
#include<string>
#include<unordered_map>
class DateMap {
 public:
  std::unordered_map<std::string, int> timeMap;

 public:
  DateMap() {
    timeMap.insert({"Jan", 1});
    timeMap.insert({"Feb", 2});
    timeMap.insert({"Mar", 3});
    timeMap.insert({"Apr", 4});
    timeMap.insert({"May", 5});
    timeMap.insert({"Jun", 6});
    timeMap.insert({"Jul", 7});
    timeMap.insert({"Aug", 8});
    timeMap.insert({"Sep", 9});
    timeMap.insert({"Oct", 10});
    timeMap.insert({"Nov", 11});
    timeMap.insert({"Dec", 12});
    timeMap.insert({"Sun", 0});
    timeMap.insert({"Mon", 1});
    timeMap.insert({"Tue", 2});
    timeMap.insert({"Wed", 3});
    timeMap.insert({"Thu", 4});
    timeMap.insert({"Fri", 5});
    timeMap.insert({"Sat", 6});
  }
  int getMapValue(std::string str) { 
        return timeMap.find(str)->second; 
    }
};


class TimeInfo{
    tm timeinfo;

public:
    TimeInfo() {}
    void parse(std::string time_str)
    {
        DateMap dateMap;
        //test
        std::cout<<"time_str size is"<<time_str<<std::endl;
        std::cout<<"time_str is"<<time_str<<std::endl;
        
        //test
        std::cout<<"after time_str.substr(23) statement in parse"<<std::endl;
        this->timeinfo.tm_isdst = 0;
        this->timeinfo.tm_year = std::stoi(time_str.substr(12),nullptr) - 1900; 
        this->timeinfo.tm_mon = dateMap.getMapValue(time_str.substr(8, 3).c_str()) - 1;
        this->timeinfo.tm_mday =  std::stoi(time_str.substr(5),nullptr);
        this->timeinfo.tm_wday = dateMap.getMapValue(time_str.substr(0, 3).c_str());
        this->timeinfo.tm_hour = std::stoi(time_str.substr(17),nullptr);
        this->timeinfo.tm_min = std::stoi(time_str.substr(20),nullptr);
        this->timeinfo.tm_sec = std::stoi(time_str.substr(23),nullptr);
        
    }

    tm * convertGMT()
    {
        time_t t = mktime(&this->timeinfo);
        return gmtime(&t);
    }
    

};