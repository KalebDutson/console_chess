#ifndef ChessException_H
#define ChessException_H

#include <iostream>

using namespace std;


class ChessException : public std::exception
{   
    const char* msg;

    public:
        ChessException(const char*);
        ChessException(string);
        ChessException();
        ~ChessException();
        const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW;
        void setMsg(const char*);
};

// Subclasses  ================================================================

class InvalidTeamException : public ChessException
{
    public:
        InvalidTeamException();
        InvalidTeamException(const char*);
};

class MissingSourceException : public ChessException
{
    public:
        MissingSourceException();
        MissingSourceException(const char*);
};

class MissingDestinationException : public ChessException
{
    public:
        MissingDestinationException();
        MissingDestinationException(const char*);
};

class InvalidMoveException : public ChessException
{
    public:
        InvalidMoveException();
        InvalidMoveException(const char*);
};

class InvalidCommandException : public ChessException
{
    public:
        InvalidCommandException();
        InvalidCommandException(const char*);
};

class InvalidPromotionException : public ChessException
{
    public:
        InvalidPromotionException();
        InvalidPromotionException(const char*);
};

#endif