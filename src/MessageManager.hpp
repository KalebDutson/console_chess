#ifndef MessageManager_H
#define MessageManager_H

#include "Message.hpp"

#include <queue>


class MessageManager
{   

    queue<Message> top;
    queue<Message> bottom;
    vector<Message> recurringMessages;

    public:
        MessageManager();
        ~MessageManager();
        // methods
        void clear();
        void clearTop();
        void clearBottom();
        void displayAll();
        void displayTop();
        void displayBottom();
        // setters
        void addMessage(Message);

    private:
        void clearWrapper(queue<Message>&, MHeight);
        void displayWrapper(queue<Message>&, MHeight);        
};

#endif
