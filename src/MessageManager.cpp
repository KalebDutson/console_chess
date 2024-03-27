#include "MessageManager.hpp"
#include "Message.hpp"

#include <stack>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


MessageManager::MessageManager(){
    this->top = {};
    this->bottom = {};
    this->recurringMessages = {};
}

MessageManager::~MessageManager(){
    this->clear();
}

/* ==================== Methods ==================== */

/*
    Destroy all objects stored in both queues
*/
void MessageManager::clear(){
    this->clearTop();
    this->clearBottom();
    this->recurringMessages.clear();
}

void MessageManager::clearTop(){
    this->clearWrapper(this->top, ABOVE);
}

void MessageManager::clearBottom(){
    this->clearWrapper(this->bottom, BELOW);
}

/*
    Pop all notifications off queue and print them.
*/
void MessageManager::displayAll(){
    this->displayTop();
    this->displayBottom();
}

void MessageManager::displayTop(){
    this->displayWrapper(this->top, ABOVE);
}

void MessageManager::displayBottom(){
    this->displayWrapper(this->bottom, BELOW);
}

/* ==================== Setters ==================== */

/**
 * Adds a Message object to the manager
 * @param ms Message object to be added.
*/
void MessageManager::addMessage(Message ms){
    MHeight h = ms.getMHeight();
    MFreq mt = ms.getMFreq();
    if(h != BELOW && h != ABOVE){
        throw invalid_argument("NotificationManager.cpp: Cannot add Message to manager, invalid MHEIGHT!");
    }
    if(mt != ONCE && mt != CONTINUOUS){
        throw invalid_argument("NotificationManager.cpp: Cannot add Message to manager, invalid MTYPE!");
    }

    if(mt == ONCE){
        if(h == ABOVE){
            this->top.push(ms);
        }
        else if(h == BELOW){
            this->bottom.push(ms);
        }
    }
    else{
        this->recurringMessages.push_back(ms);
    }
}


/* ==================== Private Methods ==================== */

/** Wrapper for clearing the top and bottom Message queues */
void MessageManager::clearWrapper(queue<Message> &q, MHeight mh){
    while( !q.empty()){
        q.pop();
    }
    
    // Erase all permanent messages
    vector<Message>::iterator newEnd = this->recurringMessages.end();
    for(int i=0; i<this->recurringMessages.size(); i++){
        if(this->recurringMessages.at(i).getMHeight() == mh){ 
            // 'removed' elements are placed at tail end of vector, remove() returns the new end of the array that excludes the tail elements
            vector<Message>::iterator newEnd = std::remove(this->recurringMessages.begin(), this->recurringMessages.end(), this->recurringMessages.at(i));
        }
    }
    // erase 'removed' elements
    this->recurringMessages.erase(newEnd, this->recurringMessages.end());
}

/** Wrapper for displaying the top and bottom Message queues */
void MessageManager::displayWrapper(queue<Message> &q, MHeight mh){ 
    // Continuous messages are displayed first   
    for(int i=0; i<this->recurringMessages.size(); i++){
        if(this->recurringMessages.at(i).getMHeight() == mh){ 
            this->recurringMessages.at(i).print();
        }
    }
    // Display once off messages
    while( !q.empty()){
        q.front().print();
        q.pop();
    }
}