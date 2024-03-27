#include "Piece.hpp"

#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>

using namespace std;

const string bold("\033[1m");
const string black("\033[30m");
const string red("\033[38;5;88m");
const string boldReset("\033[22m");
const string green("\033[33m");

Piece::Piece(){
    this->setNull();
}

/*!
    @param team What team this Piece is on.
    @param type What type of Piece this is.
    @param numMoves The number of moves this Piece has taken.
*/
Piece::Piece(TeamColor team, PieceType type, int numMoves){
    this->init(team, type, numMoves);
}

Piece::~Piece(){
    this->stdMoveset.clear();
};

/*
    Overload '=' operator for Piece object
    Note: This DOESN'T overload '=' for Piece* objects since overloading operators for pointers is not possible
*/
Piece& Piece::operator=(const Piece& rhs){
    this->team = rhs.team;
    this->type = rhs.type;
    this->isNull = rhs.isNull;
    this->numMoves = rhs.numMoves;
    this->stdMoveset = rhs.stdMoveset;
    this->enPassantCapture = rhs.enPassantCapture;
    this->enPassantTurn = rhs.enPassantTurn;
    return *this;
}

/*
    Clones a Piece*. Sets all values for 'this' to mirror the values of 'example'.
*/
void Piece::clone(Piece example){
    this->init(example.team, example.type, example.numMoves);
    this->enPassantCapture = example.enPassantCapture;
    this->enPassantTurn = example.enPassantTurn;
}  

/**
 * Promote piece to another type. Preserve numMoves value, but any En Passant members will be lost.
 * @param type PieceType to promote to
 * @returns nothing
*/
void Piece::promote(PieceType type){
    this->init(this->team, type, this->numMoves);
}

/**
    @param team What team this Piece is on.
    @param type What type of Piece this is.
    @param numMoves The number of moves this Piece has taken.
*/
void Piece::init(TeamColor team, PieceType type, int numMoves){
    // determine if piece is null
    if(team == NoColor){
        this->setNull();
    }
    else{
        this->stdMoveset;
        this->team = team;
        this->type = type;
        this->enPassantCapture = false;
        this->enPassantTurn = -1;
        this->setMoveset();
        this->numMoves = numMoves;
        this->isNull = false;
    }
}

void Piece::setTeam(TeamColor team){
    this->team = team;
}

void Piece::setType(PieceType type){
    this->type = type;
}

void Piece::setMoveset(){
    switch(this->type){
        case(NoPiece):
            this->stdMoveset = NO_PIECE_STD;
            break;
        case(King):
            this->stdMoveset = KING_STD;
            break;
        case(Queen):
            this->stdMoveset = QUEEN_STD;
            break;
        case(Rook):
            this->stdMoveset = ROOK_STD;
            break;
        case(Bishop):
            this->stdMoveset = BISHOP_STD;
            break;
        case(Knight):
            this->stdMoveset = KNIGHT_STD;
            break;
        case(Pawn):
            this->stdMoveset = PAWN_STD;
            break;
    }
}

void Piece::setEnPassantCapture(bool b){
    this->enPassantCapture = b;
}

// When En Passant becomes available, the turn number needs to be tracked
void Piece::setEnPassantCapture(bool b, int turnNum){
    this->enPassantCapture =  b;
    this->enPassantTurn = turnNum;
}

bool Piece::getEnPassantCapture(){
    return this->enPassantCapture;
}

int Piece::getEnPassantTurn(){
    return this->enPassantTurn;
}

TeamColor Piece::getTeam(){
    return this->team;
}

PieceType Piece::getType(){
    return this->type;
}

vector<string> Piece::getStdMoveset(){
    return this->stdMoveset;
}

void Piece::setNull(){
    this->team = NoColor;
    this->type = NoPiece;
    this->enPassantCapture = false;
    this->enPassantTurn = -1;
    this->isNull = true;
    this->numMoves = 0;
    this->stdMoveset = NO_PIECE_STD;
}

bool Piece::getNull(){
    return this->isNull;
}

int Piece::getNumMoves(){
    return this->numMoves;
}

void Piece::setNumMoves(int num){
    this->numMoves = num;
}

void Piece::incrementNumMoves(int num){
    this->numMoves += num;
}

string Piece::toString(){
    string s = "";
    s += bold;
    switch(team){
        case(0): // this is for the NoColor team (which is the null team)
            s += green;
            break;
        case(1):
            s += red;
            break;
        case(2):
            s += black;
            break;
    }
    s += pieceString[this->getType()];
    s += boldReset;

    return s;
}