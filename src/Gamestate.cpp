#include "Gamestate.hpp"
#include "Prompt.hpp"
#include "Move.hpp"
#include "Util.hpp"
#include "ChessException.hpp"
#include "StateFactory.hpp"
#include "Message.hpp"
#include "MessageManager.hpp"
#include "Debug.hpp"

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>


Gamestate::Gamestate(){
    // The board state that the game will initialize to
    if(DEBUG_MODE) this->initState = STATE_7; // debug state
    else this->initState = STATE_1; // normal play
    this->board = new Board(true);
    this->prompt = new Prompt();
    this->currentMove = new Move(Red);  // red team always starts
    this->reset();
}

Gamestate::~Gamestate(){
    delete this->board;
    delete this->prompt;
    delete this->currentMove;
    // // TODO: If the player objects are created, uncomment this
    // delete this->checkedPlayer;
    // delete this->checkmatedPlayer;
}

/*
    Non pointer class members need the same starting values regardless of if Gamestate has been 
    initialized before or not
*/
void Gamestate::initNonPointers(){
    // this->turnCount = 0;
    this->gameOver = false;
    this->terminate = false;
    this->winner = NoColor;
    this->checkmated = NoColor;
    this->currentTeamTurn = Red;
    this->captureDelta = 0;
    this->nmanager = MessageManager();
}

/*
    Start the game
*/
void Gamestate::start(){
    while(this->callReset){
        this->callReset = false;
        this->mainGameloop();
    }
}

/*
    Resets the Gamestate. Call reset methods on Board and Move.
*/
void Gamestate::reset(){
    this->reset(this->initState);    
}

/**
    Resets board and initializes a specific state
    @param state State to initialize the board to
    @returns nothing
*/
void Gamestate::reset(vector<string> state){
    this->validSet.clear();
    this->validSet = {};
    this->callReset = true;
    this->board->clear();
    this->initNonPointers();
    StateFactory::loadState(this, state);
    this->currentMove->reset(this->currentTeamTurn);
    this->nmanager.clear();
}

/*
    Display all messages in the NotificationManager and display the Board
*/
void Gamestate::display(){
    this->nmanager.displayTop();
    this->board->display();
    this->nmanager.displayBottom();
}   

void Gamestate::setGameOver(){

}

void Gamestate::initPlayers(){

}

/**
 * Core of all game logic
*/
void Gamestate::mainGameloop(){
    int activePieceCount = 0; // The number of pieces from both teams on the board.
    const string HELP_STRING =
        "Commands:\n"
        "   sel <pos1>         : Select piece at <pos1>. Ex: sel a2\n"
        "   mv <pos2>          : Move piece selected with \"sel\" to <pos2>. Ex: mv a4\n"
        "   mv <pos1> <pos2>   : Select piece at <pos1> and move to <pos2>. Ex: mv a2 a4\n"
        "   reset              : Resart the chess game.\n"
        "   quit/exit/q        : Exit the game.\n";
    Message helpMessage = Message(HELP_STRING, ONCE, BELOW);

    while(!this->terminate){
        activePieceCount = this->board->calcActivePieceCount(); // update count
        bool validMove = false;     // checks if the move made is value and ends the current player's turn
        // reset from last turn
        bool turnChecked = false;
        string turnString = teamString[this->currentTeamTurn];
        this->currentMove->setTeamColor(this->currentTeamTurn);
        
        if(!DEBUG_MODE){ system("clear"); }
        else{ cout << "======================New Turn Start ======================" << endl; }

        // Check if the last move made put this team in check
        turnChecked = this->board->isCheck(this->currentTeamTurn);
        
        if(turnChecked){
            // Determine if this turn's player has a king in checkmate
            bool checkmate = this->board->isCheckmate(this->currentTeamTurn);
            if(checkmate){
                this->winner = (this->currentTeamTurn == Red) ? Black : Red;
                this->checkmated = this->currentTeamTurn;
                this->gameOver = true;
                
                this->nmanager.addMessage( Message(string_format("%s's King has been Checkmated.", teamString[this->checkmated].c_str()), ONCE, ABOVE) );
                this->nmanager.addMessage( Message(string_format("%s team has won!\nGame Over!",teamString[this->winner].c_str()), ONCE, ABOVE) );
            }
            else{
                this->nmanager.addMessage( Message(string_format("%s's King is in Check!", teamString[this->currentTeamTurn].c_str()), ONCE, ABOVE) );
            }
        }

        if( !this->gameOver){
            this->nmanager.addMessage( Message(string_format("%s's Turn", turnString.c_str()), ONCE, ABOVE) );
            this->nmanager.addMessage( Message(string_format("Turn Counter: %i", this->board->getTurnCount()), ONCE, ABOVE) );
            this->nmanager.addMessage( Message(string_format("Moves since last capture: %i", this->captureDelta), ONCE, ABOVE) );
        }
        else{
            Message gameoverMessage("Game has ended. Either \"reset\" the game or \"quit\".", ONCE, BELOW);
            this->nmanager.addMessage(gameoverMessage);
        }
        
        this->display();

        this->prompt->promptInput("\nEnter Command");
        int* parsedArgs = this->prompt->getCmdArgs();

        /** DEBUG: */
        if(DEBUG_MODE){
            cout << "parsed commands: ";
            for( int i=0; i<MAX_ARGS; i++){
                cout << parsedArgs[i] << ", ";
            }
            cout << endl;
        }
        
        try{
            switch(parsedArgs[0]){
                case(InvalidCmd):
                    this->nmanager.addMessage( Message("Invalid command", ONCE, BELOW) );
                    break;
                case(HelpCmd):
                    if(DEBUG_MODE) cout << "Help Command" << endl;
                    this->nmanager.addMessage(helpMessage);
                    break;
                case(SelectCmd):
                    // Stop movement commands after game has ended
                    if(this->gameOver){
                        break;
                    }
                    if(DEBUG_MODE) cout << "Select" << endl;

                    // Selecting the same piece twice will deselecte it or selecting a new piece will deselect the old one
                    if(this->currentMove->getSourceIndex() == parsedArgs[1]){
                        this->currentMove->reset(this->currentTeamTurn);
                        // Clear highlighted indices
                        this->board->clearAllHighlightedIndices();
                        this->validSet.clear();
                    }
                    // Selecting new piece
                    else{
                        if(this->currentMove->getSourceIndex() != parsedArgs[1]){
                            // new piece was selected
                            this->currentMove->reset(this->currentTeamTurn);
                            this->board->clearAllHighlightedIndices();
                            this->validSet.clear();
                        }
                        // Select piece
                        this->currentMove->setSourceIndex(parsedArgs[1]);
                        this->board->setSelectedIndex(parsedArgs[1]);

                        /** DEBUG: Switch turn to selected Piece */
                        if(DEBUG_MODE){
                            cout << "Gamestate.cpp: DEBUG: Currently switching turn to selected piece color." << endl;
                            this->currentTeamTurn = this->board->getPiece(parsedArgs[1]).getTeam();
                            this->currentMove->setTeamColor(this->currentTeamTurn);
                        }

                        // Calculate possible moves and highlight them. Determine if king is checked. Also adds the moves this->validSet
                        this->validSet = this->currentMove->calcAllMoves(this->board, true);                        
                        // add special moves to board for highlighting
                        this->board->appendSpecialIndices(currentMove->getAllSpecialIndices());
                        // add normal moves to board for highlighting
                        this->board->appendMoveHighlightIndex(this->currentMove->getAllDestIndices(), this->currentMove->getSizeofAllDestIndices());
                    }
                    break;
                case(MoveCmd):
                    // Stop movement commands after game has ended
                    if(this->gameOver){
                        break;
                    }
                    if(DEBUG_MODE) cout << "Move" << endl;
                    // check Move object to see if a piece has been selected by 'sel' before 'mv' was used
                    //  All unset indices in Move are set to -1 by default
                    if(this->currentMove->getSourceIndex() > -1){
                        this->currentMove->setDestIndex(parsedArgs[1]);
                    }
                    else{
                        this->currentMove->setDestIndex(parsedArgs[2]);
                    }
                    break;
                case(SelectMoveCmd):
                    if(DEBUG_MODE) cout << "SelectMoveCmd" << endl;
                    // Stop movement commands after game has ended
                    if(this->gameOver){
                        break;
                    }
                    this->currentMove->setSourceIndex(parsedArgs[1]);
                    this->currentMove->setDestIndex(parsedArgs[2]);
                    // calculate all valid moves
                    this->validSet = this->currentMove->calcAllMoves(this->board, true);
                    break;
                case(ClearCmd):
                    
                    if(DEBUG_MODE) cout << "ClearCmd" << endl;
                    this->reset(STATE_0);
                    break;
                case(ResetCmd):
                    if(DEBUG_MODE) cout << "ResetCmd" << endl;
                    this->reset();
                    break;
                case(AddCmd):
                    if(DEBUG_MODE) cout << "AddCmd" << endl;
                    this->board->setPiece(parsedArgs[1], (TeamColor) parsedArgs[2], (PieceType) parsedArgs[3], false);
                    break;
                case(RemoveCmd):
                    if(DEBUG_MODE) cout << "RemoveCmd" << endl;
                    this->board->removePiece(parsedArgs[1]);
                    break;
                case(TurnCmd):
                    if(DEBUG_MODE) cout << "TurnCmd" << endl;
                    this->currentTeamTurn = ((TeamColor) parsedArgs[1]);
                    break;
                case(LoadCmd):
                    if(DEBUG_MODE) cout << "LoadCmd" << endl;
                    switch(parsedArgs[1]){
                        case(0): 
                            reset(STATE_0);
                            break;
                        case(1):
                            reset(STATE_1);
                            break;
                        case(2):
                            reset(STATE_2);
                            break;
                        case(20):
                            reset(STATE_2_0);
                            break;
                        case(21):
                            reset(STATE_2_1);
                            break;
                        case(3):
                            reset(STATE_3);
                            break;
                        case(31):
                            reset(STATE_3_1);
                            break;
                        case(4):
                            reset(STATE_4);
                            break;
                        case(41):
                            reset(STATE_4_1);
                            break;
                        case(42):
                            reset(STATE_4_2);
                            break;
                        case(43):
                            reset(STATE_4_3);
                            break;
                        case(5):
                            reset(STATE_5);
                            break;
                        case(51):
                            reset(STATE_5_1);
                            break;
                        case(52):
                            reset(STATE_5_2);
                            break;
                        case(53):
                            reset(STATE_5_3);
                            break;
                        case(54):
                            reset(STATE_5_4);
                            break;
                        case(55):
                            reset(STATE_5_5);
                            break;
                        case(56):
                            reset(STATE_5_6);
                            break;
                        case(57):
                            reset(STATE_5_7);
                            break;
                        case(6):
                            reset(STATE_6);
                            break;
                        case(7):
                            reset(STATE_7);
                            break;
                    }
                    break;
                case(ExitCmd):
                    if(DEBUG_MODE) cout << "Exit" << endl;
                    this->terminate = true;
                    return;
            }

            if(parsedArgs[0] == InvalidCmd){
                continue;
            }

            // Only execute a move if the game is still in play
            if(!this->gameOver){
                /** DEBUG: move info */
                if(DEBUG_MODE){
                    cout << "+++ GameState.cpp Debug +++" << endl;
                    cout << "Number of moves in this->validSet: " << this->validSet.
                    size() << endl;
                    this->printValidSet();
                    if(this->currentMove->getSourceIndex() >= 0){
                        cout << "Gamestate.cpp: Selected Piece has made: " << this->board->getPiece(this->currentMove->getSourceIndex()).getNumMoves() << " moves." << endl;
                    }
                    string b = this->currentMove->getAreMovesCalculated() ? "True" : "False";
                    cout << "Are all moves calculated?: " << b << endl;
                    this->currentMove->printAllDestIndices();

                    /** DEBUG: team info*/
                    cout << "Team Indices: ";
                    vector<int> teamIndices = this->board->getTeamPieceIndices(this->currentTeamTurn);
                    cout << "[ ";
                    for(int i=0; i < teamIndices.size(); i++){
                        cout << Util::reverseParseIndex(teamIndices.at(i));
                        if(i < teamIndices.size() - 1){
                            cout << ", ";
                        }
                    }
                    cout << " ]" << endl;
                    /** DEBUG: piece info */
                    Piece selp = this->board->getPiece(this->currentMove->getSourceIndex());
                    cout << "Selected: " << Util::reverseParseIndex(this->currentMove->getSourceIndex()) << endl;;
                    cout << "Type: " <<  pieceString[selp.getType()] << endl;
                    cout << "Team: " << teamString[selp.getTeam()] << endl;
                    cout << "Moves Made: " << selp.getNumMoves() << endl;
                    cout << "En Passant Available: " << selp.getEnPassantCapture() << endl;
                    cout << "En Passant Turn: " << selp.getEnPassantTurn() << endl;
                }

                // Only attempt a move if a destination index has been set
                if(this->currentMove->getDestIndex() < 0){
                    if(DEBUG_MODE) cout << "No destination set. Not computing move." << endl;
                    continue;
                }

                if(DEBUG_MODE) cout << "Attempting move: " << Util::reverseParseIndex(this->currentMove->getSourceIndex()) << "->" << Util::reverseParseIndex(this->currentMove->getDestIndex()) << endl;

                /*  Find all pieces the selected piece could move to and error check.
                    Check if currentMove is equal to any move in this->validSet. 
                    If true, set currentMove to the Move in valid set. */
                if( !this->moveInSet(this->currentMove)){
                    throw InvalidMoveException("The attempted move is not in the set of valid moves");
                }
                Move* equiv = this->getMoveFromSet(this->currentMove);
                delete this->currentMove; // delete old reference
                this->currentMove = equiv; // moves in this->validSet will have been initialized using new
                
                this->board = this->currentMove->move(this->board);

                /* Pawn promotion logic */
                // Check for pawn promotion on move that was just made
                if(this->currentMove->getDestIndex() == this->board->findPromotablePawn(this->currentTeamTurn)){
                    if(DEBUG_MODE) cout << "Gamestate.cpp: The last move made the pawn at " << Util::reverseParseIndex(this->currentMove->getDestIndex()) << " available for promotion!" << endl;
                    this->board->clearAllHighlightedIndices(); // clear highlights from move
                    this->board->findPromotablePawn(this->currentTeamTurn); // find the pawn again for highlighting                    

                    // Prompt user for what piece to promote the pawn to
                    bool validPromotion = false;
                    while( !validPromotion){
                        this->display();
                        this->prompt->promotionInput();
                        int arg = this->prompt->getPromoArg();

                        // Error checking
                        // -1 is default value for for promoArg in Prompt class
                        if(arg != -1){
                            if(DEBUG_MODE) cout << "Promoting pawn at " << Util::reverseParseIndex(this->currentMove->getDestIndex()) << " to " << verbosePieceString[arg] << endl;

                            // promote pawn
                            this->board->promotePiece(this->currentMove->getDestIndex(), (PieceType) arg );
                            validPromotion = true;
                        }
                    }
                }

                // Switch turn
                this->currentTeamTurn = (this->currentTeamTurn == Red) ? Black : Red;
                // Reset piece data for previous move and set team for next turn
                this->currentMove->reset(this->currentTeamTurn);
                // clear highlighted indices
                this->board->clearAllHighlightedIndices();
                this->validSet.clear();
                this->board->incrementTurnCount(); 
                // remove en passant availability from pieces that were not captured
                this->board->pruneEnPassantPawns();
                // check activePieceCount and update captureDelta
                if(activePieceCount > this->board->calcActivePieceCount()){
                    // piece was captured
                    this->captureDelta = 0;
                }
                else{
                    // no capture
                    this->captureDelta++;
                }  
                // If 50 consecutive moves have been made without a capture, the game ends in a draw
                if(captureDelta == 50){
                    // cout << "50 moves have been made without a piece captured. An automatic draw has been declared." << endl;
                    // cout << "Draw!" << endl << "Game Over!" << endl;

                    this->nmanager.addMessage( Message("An automatic draw has been declared.\n  Reason: 50 moves have been made without a piece captured.\nDraw!\nGame Over!", CONTINUOUS, ABOVE) );
                    this->gameOver = true;
                } 

            }
        }
        catch(const InvalidTeamException &ex){
            if(DEBUG_MODE){
                cerr << ex.what() << endl;
            }
            this->nmanager.addMessage(Message(ex.what(), ONCE, BELOW));
            // Reset move since player could have selected an opponent's piece
            this->currentMove->reset(this->currentTeamTurn); 
            // Clear highlighted indices
            this->board->clearAllHighlightedIndices();
            this->validSet.clear();
        }
        catch(const MissingSourceException &ex){
            if(DEBUG_MODE){
                cerr << "Gamestate.cpp: ";
                cerr << ex.what() << endl;
            }
            this->nmanager.addMessage(Message("No piece selected", ONCE, BELOW));
            this->currentMove->reset(this->currentTeamTurn);
            this->board->clearAllHighlightedIndices();
            this->validSet.clear();
        }
        catch(const MissingDestinationException &ex){
            if(DEBUG_MODE){
                cerr << "Gamestate.cpp: Input destination" << endl;
            }
            this->nmanager.addMessage(Message("Invalid move destination", ONCE, BELOW));
        }
        catch(const InvalidMoveException &ex){
            this->nmanager.addMessage(Message(ex.what(), ONCE, BELOW));
        }
        catch(const ChessException &ex){
            if(DEBUG_MODE){
                cerr << "Gamestate.cpp: Caught chess exception" << endl;
                cerr << ex.what() << endl;
            }
        }
        catch(const exception &ex){
            if(DEBUG_MODE){
                cerr << "Gamestate.cpp: Caught unhandled exception" << endl;
                cerr << ex.what() << endl;
            }
        }
        catch(...){
            if(DEBUG_MODE){
                cerr << "Gamestate.cpp: Unknown failure occured" << endl;
            }
        }

    }
}

// void Gamestate::setTurnCount(int count){
//     this->turnCount = count;
// }

void Gamestate::setCurrentTeamTurn(TeamColor team){
    this->currentTeamTurn = team;
    this->currentMove->setTeamColor(team);
}

void Gamestate::setBoard(Board* b){
    this->board->clear(); // destroy any pre-existing objects in board
    this->board = b;
}

void Gamestate::setCaptureDelta(int cd){
    this->captureDelta = cd;
}

bool Gamestate::moveInSet(Move* potm){
    for(int i=0; i<this->validSet.size(); i++){
        if(this->validSet.at(i)->equals(potm)) return true;
    }
    return false;
}

/**
 * Since find the equivalent move from this->validSet and return it based on the equivalent Move.
 * Moves only need the same team, source, and destination to be equal, but the moves in this->validSet also have correct secondary moves.
*/
Move* Gamestate::getMoveFromSet(Move* equalMove){
    for(int i=0; i<this->validSet.size(); i++){
        if(this->validSet.at(i)->equals(equalMove)){
            return this->validSet.at(i);
        }
    }
    // If that loop doesn't find anything, throw and error
    throw ChessException("Couldn't find move in set");
}

void Gamestate::setCheck(Player p){

}

void Gamestate::setCheckmate(Player p){

}

void Gamestate::setStalemate(){

}

void Gamestate::setTurn(TeamColor playerTeam){

}

TeamColor Gamestate::getTurn(){
    return this->currentTeamTurn;
}

Board* Gamestate::getBoard(){
    return this->board;
}

void Gamestate::printValidSet(){
    cout << "Gamestate.cpp: Moves in Valid Set" << endl;
    for(int i=0; i < this->validSet.size(); i++){
        Move* m = this->validSet.at(i);
        cout << Util::reverseParseIndex(m->getSourceIndex()) << "->" << Util::reverseParseIndex(m->getDestIndex());
        cout << "  (" << m->getSourceIndex() << "->" << m->getDestIndex() << ")" << endl;
    }
    cout << endl;
    
}
