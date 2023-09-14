#include"cache.hpp"

void LRUCache::put(const std::string& URI, const Response& resp){
    //test
    std::cout<<"I'm in cache put func"<<std::endl;
    pthread_rwlock_wrlock(&cacheLock);
    if(cache.find(URI)!=cache.end()){//
        cache[URI]->response=resp;
        deleteAndAdd(URI);
    }
    else{
        Node* newNode;
        try{
            newNode=new Node(URI, resp);//can throw
        }
        catch(...){
            pthread_rwlock_unlock(&cacheLock);//avoid deadlock
            throw;
        }
        newNode->prev=dummyHead;
        newNode->next=dummyHead->next;
        dummyHead->next=newNode;//
        newNode->next->prev=newNode;//
        cache[URI]=newNode;
        if(cache.size()>(unsigned)cap){//lruCache is full
            cache.erase(dummyTail->prev->URI);
            Node*temp=dummyTail->prev;
            temp->prev->next=dummyTail;
            dummyTail->prev=temp->prev;
            delete temp;
        }
    }
    //test
    std::cout<<"I'm just before unlock in cache put func"<<std::endl;
    pthread_rwlock_unlock(&cacheLock);
    //test
    std::cout<<"I'm just after unlock in cache put func"<<std::endl;
}

//move the node to the head of doubly linked list
//can't add lock because it's a helper function, adding lock will lead to deadlock
void LRUCache::deleteAndAdd(const std::string& URI){
    //test
    std::cout<<"I'm in cache deleteAndAdd func"<<std::endl;
    //delete the node in the doubly linkedlist
    Node* temp=cache[URI];
    temp->prev->next=temp->next;
    temp->next->prev=temp->prev;
    //add to the head 
    temp->next=dummyHead->next;
    temp->prev=dummyHead;
    dummyHead->next->prev=temp;//!!
    dummyHead->next=temp;//don't forget!
    //test
    std::cout<<"I'm just before unlock in cache deleteAndAdd func"<<std::endl;
    //test
    std::cout<<"I'm just after unlock in cache deleteAndAdd func"<<std::endl;
}