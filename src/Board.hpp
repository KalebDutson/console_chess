#ifndef Board_H
#define Board_H

#include "Piece.hpp"

#include <vector>
#include <iostream>

using namespace std;

/*
    Special moves have an additional move that they executing since they either effect surrounding pieces or have
     secondary effects
*/
enum SpecialMove {
    NonSpecial,     // standard move
    Prequel,        // The move a special move executes before itself
    Sequel,         // The move a special move executes after itself
    Castling,       // Special move for castling. Has secondary move.
    EnPassant,      // Special move for En Passant. Has secondary move
    PawnStart,      // semi-special move for pawn. Doesn't have secondary move
    PawnPromo       // special state for pawn, pawn can be promoted
};

class Board
{    
    int rowSize;
    int colSize;
    Piece* internalboard;
    char* displayboard;
    int selectedIndex;
    int turnCount;
    bool loaderInit;  // True if the StateFactory class will be loading the initial state for Board
    vector<int> moveHighlightIndices;
    map<int, SpecialMove> specialHighlightIndices;
    vector<int> promotablePawn;
    vector<int> checkingPieceIndices;  // Indices that contain a piece that is putting a king in check
    vector<int> potentialCheckingIndices; // all indices that could potentially threaten the king
    vector<int> threatenedKingIndices;  // Indices of all Kings in check
    vector<int> checkmatingIndices;  // Indices that contain a piece putting a king in checkmate
    
public:
    Board(bool);
    Board(Board*);
    ~Board();
    void clone(Board*);
    void display();
    void clear();
    void reset();
    void printInternal();
    bool isIndexOccupied(int);
    void clearAllHighlightedIndices();
    void clearPromotablePawn();
    void load(Piece*, int);
    bool isCheck(TeamColor);
    bool isCheckmate(TeamColor);  // Determine if specific team is checkmated
    void printCheckingPieces(); // prints all piece locations that are putting the king in check
    void printMoveHighlightIndices();
    void printSpecialIndices();
    void printPotentialCheckingIndices();
    void pruneEnPassantPawns();
    int findPromotablePawn(TeamColor);
    int calcActivePieceCount();
    // getters
    Piece getPiece(int);
    int getTurnCount();
    int getKingIndex(TeamColor);
    vector<int> getTeamPieceIndices(TeamColor);
    vector<int> getPotentialCheckingIndices();
    vector<int> getThreatenedKingIndices();
    // setters
    void setPiece(int, Piece);
    void setPiece(int, TeamColor, PieceType, int);
    void removePiece(int);
    void setMoveHighlightIndices(vector<int>);
    void setTurnCount(int);
    void incrementTurnCount();
    void appendMoveHighlightIndex(int*, int);
    void setCheckingPieceIndices(vector<int>);
    void setCheckedKingIndices(vector<int>);
    void appendCheckedKingIndices(int);
    void appendPotentialCheckingIndices(vector<int>, bool);
    void appendSpecialIndices(map<int, SpecialMove>);
    void setSelectedIndex(int);
    void promotePiece(int, PieceType);    
    
private:
    void fullInitialization();
    void initMembers();
    void initBoards();
    void loadPromotablePawns();
};

#endif