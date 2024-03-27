#ifndef Gamestate_H
#define Gamestate_H

#include "Board.hpp"
#include "Player.hpp"
#include "Piece.hpp"
#include "Prompt.hpp"
#include "Move.hpp"
#include "MessageManager.hpp"

#include <vector>
#include <iostream>
#include <string>

class Gamestate
{      
    vector<string> initState;
    Board* board;
    Prompt* prompt;
    vector<Move*> validSet;  // all valid moves that can be made based on the selected piece
    Move* currentMove;
    MessageManager nmanager;
    TeamColor currentTeamTurn;
    
    // int turnCount;
    int captureDelta;
    bool stalemate;
    bool gameOver;
    bool terminate;
    bool callReset;  // member to keep the game running after game over if the user wants to start another game.
    TeamColor winner;
    TeamColor checkmated;  // The losing team

    public:
        Gamestate();
        ~Gamestate();
        void start();
        void setCurrentTeamTurn(TeamColor);
        void setBoard(Board*);
        void setCaptureDelta(int);
        Board* getBoard();
        void display();

    private:
        bool moveInSet(Move*);
        Move* getMoveFromSet(Move*);
        void initNonPointers();
        void reset();
        void reset(vector<string>);
        void setGameOver();
        void initPlayers();
        void mainGameloop();
        void setCheck(Player);
        void setCheckmate(Player);
        void setStalemate();
        void setTurn(TeamColor);
        TeamColor getTurn();
        void printValidSet();
};

#endif