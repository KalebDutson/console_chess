#include "StateFactory.hpp"
#include "Piece.hpp"
#include "ChessException.hpp"
#include "Gamestate.hpp"
#include "Board.hpp"
#include "Debug.hpp"

#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <regex>

using namespace std;

/**
    Static method
    Loads a pre-constructed game state into a Gamestate object
    @param gs The Gamestate object to load the state into
    @param state The string vector that contains all values of the state to load
    @returns nothing
*/
void StateFactory::loadState(Gamestate* gs, vector<string> state){
    // parse turn, turn count, capture delta
    if(state.size() != 4){
        throw ChessException("StateFactory.cpp: State length is invalid");
    }
    if( !regex_match(state[0], STATE_REGEX_TURN_REP)){
        throw ChessException("StateFactory.cpp: State TurnValue invalid");
    }
    if( !regex_match(state[1], STATE_REGEX_COUNT_REP)){
        throw ChessException("StateFactory.cpp: State TurnCount invalid!");
    }
    if( !regex_match(state[2], STATE_REGEX_DELTA_REP)){
        throw ChessException("StateFactory.cpp: State CaptureDelta invalid!");
    }

    TeamColor turnTeam = NoColor;
    if(state[0] == "r"){
        turnTeam = Red;
    }
    else if(state[0] == "b"){
        turnTeam = Black;
    }
    else if(state[0] == "n"){
        turnTeam = NoColor;
    }

    // pparse this from string to int
    int turnCount = stoi(state[1]);
    int captureDelta = stoi(state[2]);
    // Update Gamestate board object
    loadBoard(gs->getBoard(), state[3], turnCount);
    // Update other members
    gs->setCurrentTeamTurn(turnTeam);
    gs->setCaptureDelta(captureDelta);
}

/**
    Static method
    Loads a pre-constructed board state into a Board object
    @param b The Board object to load the board state into
    @param boardState The string that represents the board
    @returns nothing
*/
void StateFactory::loadBoard(Board* b, string boardState, int turnCount){
    b->load(build(boardState, turnCount), turnCount);
}

/**
    Static method
    Builds an array of length 64 for use as a Board object's 'internalboard' member.
    Use this to build board states without having to create them during play.
    @param str A string representation of a Board object. See header file for documentation on them.
    @returns An array of Piece* objects
*/
Piece* StateFactory::build(string str, int count){
    vector<string> allSquares = {};
    string del = " "; // delimiter for splitting the string representation for the board state.
    int start = 0;
    int end = -1 * del.length();
    // parse the string and extract all the piece information and put it in a vector
    do{
        start = end + del.size();
        end = str.find(del, start);
        allSquares.push_back(str.substr(start, end - start));
    } while(end != -1);

    // Examine each string representing a Piece and create the object then add to array
    Piece* pieces = new Piece[64]; // Internal board for Board object is 64. A chess board is 8*8 squares.
    // Board has 64 squares, the extra +1 is from the trailing ' ' included in the board state string.
    if(allSquares.size() != 65){
        /** DEBUG: */
        if(DEBUG_MODE) cout << "StateFactory.cpp: AllSquares.size: " << allSquares.size() << endl;
        throw ChessException("StateFactory.cpp: Invalid Board state representation. Size is not 65");
    }
    // IMPORTANT NOTE: Expects the last char in the string to be ' ', which is why the loop range is "allSquares.size() - 1"
    string word = "";
    bool match = false;
    for(int i=0; i < allSquares.size() - 1; i++){
        word = allSquares.at(i);
        if(word.length() > REP_PIECE_MAX_LEN){
            throw ChessException("StateFactory.cpp: Invalid string representation of Piece. Length too long.");
        }
        match = regex_match(word, STATE_REGEX_BOARD_REP);
        
        if( !match){
            /** DEBUG: print all values read */
            if(DEBUG_MODE){
                string msg = "P"; // DEBUG variable
                for(int i=0; i < allSquares.size(); i++){
                    cout << allSquares.at(i) << " ";
                    if(i % 8 == 7){
                        cout << endl;
                    }
                }
                cout << endl;
                msg = "F";
                cout << word << ": ";
                cout  << msg << " | ";
                if(i % 8 == 7 && i <= allSquares.size() - 1){
                    cout << endl;
                }
            }
            throw ChessException("StateFactory.cpp: Failed to match string representation to regex. Piece couldn't be constructed");
        }
        bool nullPiece = false;
        TeamColor team = NoColor;
        PieceType type = NoPiece;
        int numMoves = 0;
        bool enPassant =  false;
        // set team
        if(word.at(0) == 'r'){ team = Red; }
        else if(word.at(0) == 'b'){ team = Black; }
        else { nullPiece = true; }
        //  set type
        if(word.at(1) == 'r'){ type = Rook; }
        else if(word.at(1) == 'n'){ type = Knight; }
        else if(word.at(1) == 'b'){ type = Bishop; }
        else if(word.at(1) == 'q'){ type = Queen; }
        else if(word.at(1) == 'k'){ type = King; }
        else if(word.at(1) == 'p'){ type = Pawn; }
        else { nullPiece = true; }
        // set num moves
        if(word.length() == 3){
            if(word.at(2) == '-'){ numMoves = 0; }
            if(word.at(2) == '1'){ numMoves = 1; }
            if(word.at(2) == '2'){ numMoves = 2; }
            if(word.at(2) == 'e'){ numMoves = count; enPassant = true; } // en passant is only available when a piece has made 1 move
        }
        // initialize piece
        if(nullPiece){
            pieces[i].setNull();
        }
        else{
            pieces[i].init(team, type, numMoves);
            if(enPassant){
                pieces[i].setEnPassantCapture(true, numMoves - 1);  // 0 is the turn at which en passant became available. When running the game, the turn counter starts at 1.
            }
        }
    }

    return pieces;
}