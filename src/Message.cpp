#include "Message.hpp"

#include <iostream>
#include <string>

using namespace std;

/**
 * Construct Notification object
 * @param m The string message
 * @param nt Determines if the message is displayed once or continuously
 * @param h Determines if notification appears above or below the board display.
*/
Message::Message(string m, MFreq nt, MHeight h){
    this->content = m;
    this->mtype = nt;
    this->height = h;
}

/**
 * Construct Notification object.
 * Notification is displayed once, above the board display.
 * @param m The string message
*/
Message::Message(string m){
    this->content = m;
    this->mtype = ONCE;
    this->height = ABOVE;
}

Message::~Message() = default;


/* ==================== Methods ==================== */

bool Message::operator== (const Message& rhs){
    if(this->content == rhs.content && this->mtype == rhs.mtype && this->height == rhs.height){
        return true;
    }
    return false;
}

// Print the message using cout. New line is added to end of message.
void Message::print(){
    cout << this->content << endl;
}

// Append to message string
void Message::append(string s){
    this->content += s;
}


/* ==================== Setters ==================== */

void Message::setContent(string m){
    this->content = m;
}

void Message::setMFreq(MFreq nt){
    this->mtype = nt;
}

void Message::setMHeight(MHeight h){
    this->height = h;
}


/* ==================== Getters ==================== */

string Message::getContent(){
    return this->content;
}

MFreq Message::getMFreq(){
    return this->mtype;
}

MHeight Message::getMHeight(){
    return this->height;
}