#ifndef __CACHE_HPP__
#define __CACHE_HPP__
#include<pthread.h>
#include <unordered_map>
#include<iostream>
#include <string>
#include <vector>
#include "response.hpp"

class LRUCache {
    struct Node{
        std::string URI;//URI
        Response response;// response object
        //size_t respSize; //we can add this to improce efficiency
        Node* next;
        Node* prev;
        //reference as the arguments should be fine?
        Node(const std::string& uri, const Response& resp):URI(uri),response(resp),next(NULL),prev(NULL){}
        Node():next(NULL),prev(NULL){}
    };
    Node* dummyHead;
    Node* dummyTail;
    int cap;
    std::unordered_map<std::string,Node*> cache;
    // pthread_mutex_t cacheLock;
    pthread_rwlock_t cacheLock;

public:
    LRUCache() {
        cap=1000;//dafault value
        dummyHead=new Node();
        try{
            dummyTail=new Node();// can throw
        }
        catch(...){
            delete dummyHead;
            throw;
        }
        dummyHead->next=dummyTail;
        dummyTail->prev=dummyHead;
        // cacheLock=PTHREAD_MUTEX_INITIALIZER;
        pthread_rwlock_init(&cacheLock, NULL);
    }
    LRUCache(int capacity) {
        cap=capacity;
        dummyHead=new Node();
        try{
            dummyTail=new Node();// can throw
        }
        catch(...){
            delete dummyHead;
            throw;
        }
        dummyHead->next=dummyTail;//don't forget
        dummyTail->prev=dummyHead;//
        // cacheLock=PTHREAD_MUTEX_INITIALIZER;
        pthread_rwlock_init(&cacheLock, NULL);
    }
    ~LRUCache(){//
        Node* p=dummyHead->next;
        while(p!=dummyTail){
            delete p->prev;
            p=p->next;
        }
        delete dummyTail;
        pthread_rwlock_destroy(&cacheLock);
    }
    //test
    std::string showCache(){
        //test
        std::cout<<"I'm in cache showCache func"<<std::endl;
        std::string ret;
        pthread_rwlock_rdlock(&cacheLock);
        Node* p=dummyHead->next;
        while(p!=dummyTail){
            ret+=p->URI+"\r\n"+p->response.getResponseLine()+"\r\n"+p->response.getHeader()+"\r\n";
            p=p->next;
        }
        //test
        std::cout<<"I'm just before unlock in cache showCache func"<<std::endl;
        pthread_rwlock_unlock(&cacheLock);
        //test
        std::cout<<"I'm just after unlock in cache showCache func"<<std::endl;
        return ret;
    }
    bool exists(const std::string& URI){//do not update the location
        //test
        std::cout<<"I'm in cache exists func"<<std::endl;
        pthread_rwlock_rdlock(&cacheLock);
        bool ret=cache.find(URI)!=cache.end();
        //test
        std::cout<<"I'm just before unlock in cache exists func"<<std::endl;
        pthread_rwlock_unlock(&cacheLock);
        //test
        std::cout<<"I'm just after unlock in cache exists func"<<std::endl;
        return ret; 
    }
    std::vector<char> getMsg(const std::string& URI) {//is it ok to return reference?
        //test
        std::cout<<"I'm in cache getMsg func"<<std::endl;
        pthread_rwlock_wrlock(&cacheLock);
        if(cache.find(URI)==cache.end()){
            pthread_rwlock_unlock(&cacheLock);
            throw -1;//if there's no info in the cache and we still try to get, throw -1
        }
        else{
           deleteAndAdd(URI);
           std::vector<char> ret=cache[URI]->response.getWholeMsg();
           //test
            std::cout<<"I'm just before unlock in cache getMsg func"<<std::endl;
           pthread_rwlock_unlock(&cacheLock);
           //test
            std::cout<<"I'm just after unlock in cache getMsg func"<<std::endl;
           return ret;
        }
    }
    // can modify the response in the cache through this method, do not update the node
    Response& getResponse(const std::string& URI){
        //test
        std::cout<<"I'm in cache getResponse func"<<std::endl;
        pthread_rwlock_rdlock(&cacheLock);
        //test
        std::cout<<"I'm just after lock in cache getResponse func"<<std::endl;
        if(cache.find(URI)==cache.end()){
            //test
            std::cout<<"I'm just before unlock in cache getResponse func"<<std::endl;
            pthread_rwlock_unlock(&cacheLock);
            throw -1;//if there's no info in the cache and we still try to get, throw -1
       }
       else{
           Response& ret=cache[URI]->response;
           //test
            std::cout<<"I'm just before unlock in cache getResponse func"<<std::endl;
           pthread_rwlock_unlock(&cacheLock);
           //test
           std::cout<<"I'm just after unlock in cache getResponse func"<<std::endl;
           return ret;
       }
    }
    //test
    int getNum(){ //do not update the location
        //test
        std::cout<<"I'm in cache getNum func"<<std::endl;
        pthread_rwlock_rdlock(&cacheLock);
        int ret=cache.size();
        //test
        std::cout<<"I'm just before unlock in cache getNum func"<<std::endl;
        pthread_rwlock_unlock(&cacheLock);
        //test
        std::cout<<"I'm just after unlock in cache getNum func"<<std::endl;
        return ret;
    }

    void put(const std::string& URI, const Response& resp);
    //update the location and move the node with passed-in URI to the front
    void deleteAndAdd(const std::string& URI);
};

#endif