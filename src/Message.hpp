#ifndef Message_H
#define Message_H

#include <iostream>
#include <string>

using namespace std;

enum MFreq {
    ONCE,
    CONTINUOUS
};

enum MHeight {
    ABOVE,
    BELOW
};

class Message
{
    string content;
    MFreq mtype;
    MHeight height;

    public:
        Message(string, MFreq, MHeight);
        Message(string);
        ~Message();
        // methods
        bool operator==(const Message&);
        void print();
        void append(string);
        // setters
        void setContent(string);
        void setMFreq(MFreq);
        void setMHeight(MHeight);
        // getters
        string getContent();
        MFreq getMFreq();
        MHeight getMHeight();
};

#endif