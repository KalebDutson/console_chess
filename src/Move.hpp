#ifndef Move_H
#define Move_H

#include <vector>
#include <bits/stdc++.h>

#include "Piece.hpp"
#include "Board.hpp"

const int MAX_1_STEP_MOVES = 9;  // The maximum number of different moves a single Piece can have while only moving 1 step. (1 step is usually movement of 1 square)


class Move {
    int sourceIndex;
    int destIndex;
    TeamColor team;
    SpecialMove special;
    Move* secondaryMove;
    bool opponentCapture; // True if this move will capture an opponents piece
    bool kingChecked;
    bool areMovesCalculated; // True if all valid moves have been calculated already
    map<int, SpecialMove> allSpecialIndices; // <destination, special> All special moves. Used for highlighting in board.
    vector<int> allDestIndices; // all board indexes that the selected piece can move to.
    vector<int> obstructedIndices; // All board values that have a non-null piece occupying that position.

    public:
        Move(TeamColor);
        Move(TeamColor, int, int);
        Move(TeamColor, int, int, SpecialMove);
        ~Move();
        // Setters
        void setSourceIndex(int);
        void setDestIndex(int);
        void setTeamColor(TeamColor);
        void setKingChecked(bool);
        void setSpecial(SpecialMove);
        void setSecondaryMove(Move*);
        // Getters
        SpecialMove getSpecial();
        Move* getSecondaryMove();
        int getSourceIndex();
        int getDestIndex();
        TeamColor getTeamColor();
        bool getOpponentCapture();
        int* getAllDestIndices();
        map<int, SpecialMove> getAllSpecialIndices();
        int getAllDestIndexAt(int);
        int getSizeofAllDestIndices();
        bool getAreMovesCalculated();
        bool getKingChecked();
        // Calculations
        Board* move(Board*);
        void isValidSelection(Board*);
        void isValidMove(Board*);
        vector<Move*> calcAllMoves(Board*, bool);
        void validateMoveset(string);
        void reset(TeamColor);
        bool equals(Move*);
        void printAllDestIndices();
        vector<Move*> calcMovesetMoves(string, Board*);
        vector<Move*> calcCastling(Board*); // Calculate castling moves starting from specificed PieceType
        vector<Move*> calcEnPassant(Board*);
        void enPassantCapture(Board*);
    
    private:
        // Calculations
        void appendAllDestIndices(int*, int);
        static int* generateBounds(int, int*);
        Move* calcNewMove(Board*, int);
        Move* calcNewMove(Board*, int, int);
        Board* rawMove(Board*);
        

};

#endif