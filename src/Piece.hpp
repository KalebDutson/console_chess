#ifndef Piece_H
#define Piece_H

#include <iostream>
#include <vector>
#include <regex>

using namespace std;

enum PieceType {
    NoPiece,    // 0
    King,       // 1
    Queen,      // 2
    Rook,       // 3
    Bishop,     // 4
    Knight,     // 5
    Pawn        // 6
};

const string pieceString[7] = {
    " ",
    "K",
    "Q",
    "r",
    "b",
    "n",
    "p"
};

const string verbosePieceString[7] = {
    "NULL_PIECE ",
    "King",
    "Queen",
    "Rook",
    "Bishop",
    "Knight",
    "Pawn"
};

enum TeamColor {
    NoColor,    // 0
    Red,        // 1
    Black       // 2
};

const string teamString[3] = {
    "NoColor - Invalid", // 0
    "Red",               // 1
    "Black"              // 2
};

/*
    Movement types for all pieces. Some pieces such as the King and Pawn 
    have special movesets. Rules like 'Castling', 'En Passant' are more specialized
    and do not easily fit into a moveset.
*/
const vector<string> NO_PIECE_STD = {""};
const vector<string> KING_STD = {"N*1U"};
const vector<string> QUEEN_STD = {"N*7U"};
const vector<string> ROOK_STD = {"N+7U"};
const vector<string> BISHOP_STD = {"N/7U"};
const vector<string> KNIGHT_STD = {"NL1U"};
const vector<string> PAWN_STD = {"P|1N"};
// Special movesets that only activate under specific circumstances
const vector<string> PAWN_ATTK = {"P/1A"};
const vector<string> PAWN_START = {"P|2N"};
// Special movesets that require additional logic than above movesets
const vector<string> KING_THREAT_DETECTION = {
    KING_STD[0], QUEEN_STD[0], ROOK_STD[0], BISHOP_STD[0], KNIGHT_STD[0], PAWN_ATTK[0]
    };  // All movesets that a king can be attacked with. Used to determine check

// All movesets need to match this to be valid
const regex MOVESET_REGEX("([PN])([+/|*L])([127])([ANU])");


class Piece
{
    // ANY change to this members will need to update the '=' overload method
    TeamColor team;
    PieceType type;
    bool isNull;
    int numMoves;
    bool enPassantCapture;  // True if an opponent can perform an En Passant on this Piece. Only valid for Pawns.
    int enPassantTurn;  // Tracks what turn number it was when a Pawn became available to be captured by an En Passant
    vector<string> stdMoveset;

// TODO: Add methods for tracking and determining if a Pawn can have En Passant executed on it.
public:
    Piece();
    Piece(TeamColor, PieceType, int);
    ~Piece();
    void init(TeamColor, PieceType, int);
    Piece& operator=(const Piece&);
    void clone(Piece);
    string toString();
    void promote(PieceType);
    // getters
    bool getNull();
    TeamColor getTeam();
    PieceType getType();
    int getNumMoves();
    vector<string> getStdMoveset();
    bool getEnPassantCapture();
    int getEnPassantTurn();
    // setters
    void setNull();
    void setNumMoves(int);
    void incrementNumMoves(int);
    void setEnPassantCapture(bool);
    void setEnPassantCapture(bool, int);
    
    private:
        void setTeam(TeamColor);
        void setType(PieceType);
        void setMoveset();

};

#endif