#include "ChessException.hpp"

using namespace std;


// ChessException superclass ==================================================

ChessException::ChessException(){
    this->msg = "Chess Exception";
}
ChessException::ChessException(const char* msg){
    this->msg = msg;
}
ChessException::ChessException(string msg){
    this->msg = msg.c_str();
}

ChessException::~ChessException() =  default;

const char* ChessException::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {
    return this->msg;
}

void ChessException::setMsg(const char* msg){
    this->msg = msg;
}

// Subclasses  ================================================================

// InvalidTeamException
InvalidTeamException::InvalidTeamException(){
    this->setMsg("Cannot select or move opponent's pieces");
}
InvalidTeamException::InvalidTeamException(const char* msg){
    this->setMsg(msg);
}

// MissingSourceException
MissingSourceException::MissingSourceException(){
    this->setMsg("No piece selected");
}
MissingSourceException::MissingSourceException(const char* msg){
    this->setMsg(msg);
}

// MissingDestinationException
MissingDestinationException::MissingDestinationException(){
    this->setMsg("No destination specified");
}
MissingDestinationException::MissingDestinationException(const char* msg){
    this->setMsg(msg);
}

// InvalidMoveException
InvalidMoveException::InvalidMoveException(){
    this->setMsg("Invalid move");
}
InvalidMoveException::InvalidMoveException(const char* msg){
    this->setMsg(msg);
}

// InvalidCommandException
InvalidCommandException::InvalidCommandException(){
    this->setMsg("Invalid command input");
}
InvalidCommandException::InvalidCommandException(const char* msg){
    this->setMsg(msg);
}

// InvalidPromotionException
InvalidPromotionException::InvalidPromotionException(){
    this->setMsg("Invalid promotion. Pawn cannot be promoted to value");
}
InvalidPromotionException::InvalidPromotionException(const char* msg){
    this->setMsg(msg);
}
