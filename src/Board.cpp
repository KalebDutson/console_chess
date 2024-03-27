#include "Board.hpp"
#include "Move.hpp"
#include "Util.hpp"
#include "ChessException.hpp"
#include "Debug.hpp"

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>

using namespace std;

const int NUM_COL_ROWS = 8; // number of columns and rows in the chessboard
const int SQUARE_WIDTH = 5; // Width of the board squares in chars
const int SQUARE_HEIGHT = 3; // Number of lines each board square occupies

/*
    Stores the board.
*/
Board::Board(bool usingLoader) {
    this->loaderInit = usingLoader;
    this->fullInitialization();
}

/*
    Build a copy of an existing board as a new object
*/
Board::Board(Board* example){
    this->clone(example);
}

Board::~Board(){
    delete[] this->internalboard;
    delete[] this->displayboard;
    this->moveHighlightIndices.clear();
    this->checkingPieceIndices.clear();
    this->threatenedKingIndices.clear();
    this->checkmatingIndices.clear();
    this->potentialCheckingIndices.clear();
    this->specialHighlightIndices.clear();
    promotablePawn.clear();
}

void Board::clear(){
    // Clear board by setting all pieces to null
    for(int i=0; i<8*8; i++){
        this->internalboard[i].setNull();
    }
    this->clearAllHighlightedIndices();
}

void Board::clearAllHighlightedIndices(){
    // vectors
    this->moveHighlightIndices.clear();
    this->checkingPieceIndices.clear();
    this->threatenedKingIndices.clear();
    this->checkmatingIndices.clear();
    this->potentialCheckingIndices.clear();
    // map
    this->specialHighlightIndices.clear();
    // member
    this->selectedIndex = -1;
}

void Board::clearPromotablePawn(){
    this->promotablePawn.clear();
}

/*
    Clones a Board*. Sets all values for 'this' to mirror the values of 'example'.
*/
void Board::clone(Board* example){
    // Board* cloned = new Board();
    this->initMembers();
    // copy Pieces on board
    for(int i=0; i < 64; i++){ // internal board is 8*8
        this->internalboard[i].clone(example->internalboard[i]);
    }
    // copy highlighted indices
    for(int i=0; i < moveHighlightIndices.size(); i++){
        this->moveHighlightIndices.push_back(example->moveHighlightIndices.at(i));
    }
    // copy display board
    for(int i=0; i < this->rowSize*this->colSize; i++){ // display board size is same as in initMembers() method
        this->displayboard[i] = example->displayboard[i];
    }
    //Copy specialHighlightIndices
    for(auto const& [key, val] : example->specialHighlightIndices){
        this->specialHighlightIndices.insert( {key, val} );
    }
    // copy promotable pawns (there should only be a max of 1)
    this->promotablePawn = example->promotablePawn;
    
    /** NOTE: If any issues arise involving missing cloned values: clone the following 
     * this->checkingPieceIndices
     * this->threatenedKingIndices
     * this->checkmatingIndices
     * this->specialHighlightIndices
     * 
     * This most likely won't be a problem since all of these values are not 
     * neccessary for move computating and only serve to highlight squares on 
     * the board for the user.
    */
}

/*
    Loads a board state
*/
void Board::load(Piece* internal, int count){
    // delete memory for existing pieces in the internal board
    this->clear();
    delete[] this->internalboard;
    // update the internal board to be the new loaded array
    this->internalboard = internal;
    this->turnCount = count;
    this->loadPromotablePawns();
}

/**
    Method for determining if a king is in check. Will check all potential movesets originating from the king. 
    If it finds an enemy piece with a matching moveset, then the king is in check.
    @param team Determine if the king of this team is under threat
    @param printNotif Print notification for debugging
    @returns true if king is checked. Updates this->threatenedKingIndices, this->potentialCheckingIndices, and this->checkingPieceIndices
*/
bool Board::isCheck(TeamColor team){
    bool check = false; // true if at least 1 piece threatens the king
    TeamColor opponentTeam = (team == Red) ? Black : Red;
    // get this king's index and create a move where the king is selected
    int kingIndex = this->getKingIndex(team);
    // find all locations where an opponent piece is
    vector<int> opponentIndices = this->getTeamPieceIndices(opponentTeam);

    /** DEBUG: Show all the move the moves the king is examining */
    if(DEBUG_MODE && CHECK_DEBUG) cout << "Board.cpp: CODE ORANGE: Orange highlights courtesy of Board::isKingInCheck()" << endl;

    // Find all attack paths to king
    // create new move for each moveset checked to accurately track which moveset corresponds to which enemy piece could attack
    for(int i=0; i<KING_THREAT_DETECTION.size(); i++){
        Move* kingSelection = new Move(team, kingIndex, -1);  // destination set to -1 since king is not being moved
        vector<Move*> threatMoves = kingSelection->calcMovesetMoves(KING_THREAT_DETECTION.at(i), this);

        // check all destination indices for moveset and try to find an opponent with that same moveset
        for(int j=0; j < threatMoves.size(); j++){

            int threatIndex = threatMoves.at(j)->getDestIndex();
            
            /** DEBUG: Show all the move the moves the king is examining */
            if(DEBUG_MODE && CHECK_DEBUG){
                if( !count(begin(this->threatenedKingIndices), end(this->threatenedKingIndices), threatIndex)){
                    this->threatenedKingIndices.push_back(threatIndex);
                }
            }
            // Opponent piece exists at j index
            if(count(opponentIndices.begin(), opponentIndices.end(), threatIndex)){
                bool checkFound = false; // True if this specific move causes a check.
                // determine the moveset checked and what opponent piece was found
                if(KING_THREAT_DETECTION.at(i) == KING_STD[0] && this->getPiece(threatIndex).getType() == King){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" King has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                else if(KING_THREAT_DETECTION.at(i) == QUEEN_STD[0] && this->getPiece(threatIndex).getType() == Queen){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" Queen has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                else if(KING_THREAT_DETECTION.at(i) == ROOK_STD[0] && this->getPiece(threatIndex).getType() == Rook){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" Rook has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                else if(KING_THREAT_DETECTION.at(i) == BISHOP_STD[0] && this->getPiece(threatIndex).getType() == Bishop){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" Bishop has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                else if(KING_THREAT_DETECTION.at(i) == KNIGHT_STD[0] && this->getPiece(threatIndex).getType() == Knight){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" Knight has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                else if(KING_THREAT_DETECTION.at(i) == PAWN_ATTK[0] && this->getPiece(threatIndex).getType() == Pawn){
                    checkFound = true;
                    if(DEBUG_MODE && CHECK_DEBUG){
                        cout << "A \"" << teamString[opponentTeam] << "\" Pawn has put the \"" << teamString[team] << "\" King in Check!" << endl;
                    }
                }
                // save checking piece
                if(checkFound){
                    check = true;
                    // don't add dupes
                    if( !count(begin(this->checkingPieceIndices), end(this->checkingPieceIndices), threatIndex)){
                        this->checkingPieceIndices.push_back(threatIndex);
                    }
                    if( !count(begin(this->potentialCheckingIndices), end(this->potentialCheckingIndices), threatIndex)){
                        this->potentialCheckingIndices.push_back(threatIndex);
                    } 
                    if( !count(begin(this->threatenedKingIndices), end(this->threatenedKingIndices), kingIndex)){
                        this->threatenedKingIndices.push_back(kingIndex);
                    }                      
                }
            }
        }
        // clean up
        delete(kingSelection); 
        threatMoves.clear();
    }

    /** DEBUG: */
    if(DEBUG_MODE && CHECK_DEBUG){
        cout << "Board.cpp: Check Logic DEBUG:" << endl;
        cout << "   Size of potentialCheckingIndices: " << this->potentialCheckingIndices.size();
        cout << " | values: [ ";
        for(int i=0; i< this->potentialCheckingIndices.size(); i++){
            cout << this->potentialCheckingIndices.at(i);
            if(i < this->potentialCheckingIndices.size() - 1){
                cout << ", ";
            }
        }
        cout << " ]" << endl;
        cout << "   Size of threatenedKingIndices: " << this->threatenedKingIndices.size();
        cout << " | values: [ ";
        for(int i=0; i< this->threatenedKingIndices.size(); i++){
            cout << this->threatenedKingIndices.at(i);
            if(i < this->threatenedKingIndices.size() - 1){
                cout << ", ";
            }
        }
        cout << " ]" << endl;
        cout << "   The " << teamString[team] << "'s King index: " << kingIndex << endl;
        cout << "   Opponent's team: " << teamString[opponentTeam] << endl;
        cout << "   Opponent Indices: [ ";
        for(int i=0; i< opponentIndices.size(); i++){
            cout << opponentIndices.at(i);
            if(i < opponentIndices.size() - 1){
                cout << ", ";
            }
        }
        cout << " ]" << endl;
        this->printCheckingPieces();
    }

    // clean up
    opponentIndices.clear();

    return check;
}

/**
 * TODO: Determine if a specific team is Checkmated. Count total of available moves.
 * @param team Check all moves for this team to see if it has been Checkmated.
 * @return True if no valid moves can be made.
*/
bool Board::isCheckmate(TeamColor team){
    vector<int> pieceIndices = this->getTeamPieceIndices(team);
    vector<int> allCheckingIndices = {};
    int totalMoves = 0;
    for(int i=0; i<pieceIndices.size(); i++){
        Board* board = new Board(this); // clone this board for calculations so this board isn't altered
        Piece p = board->getPiece(pieceIndices.at(i));
        Move* m = new Move(team);
        m->setSourceIndex(pieceIndices.at(i));
        m->calcAllMoves(board, true);
        
        /** DEBUG: */
        if(DEBUG_MODE){
            cout << "isCheckmate: potential checks: ";
            board->printCheckingPieces();
        }

        // track checking indices for all potential moves in board. These are checkmating pieces if 
        //  checkmate is found. Only tally this up when checking the king
        if(p.getType() == King){
            for(int x=0; x < board->getPotentialCheckingIndices().size(); x++){
                int checkingIndex = board->getPotentialCheckingIndices().at(x);
                if( !count( begin(allCheckingIndices), end(allCheckingIndices), checkingIndex )){
                    allCheckingIndices.push_back(checkingIndex);
                }
            }
        }

        totalMoves += m->getSizeofAllDestIndices();
        delete(m);
        delete(board);
    }
    /** DEBUG: */
    if(DEBUG_MODE) cout << "Board.cpp: Found " << totalMoves << " potential moves for " << teamString[team] << " team." << endl;
    // clean up
    pieceIndices.clear();
    
    if(totalMoves != 0){
        allCheckingIndices.clear();
        return false;
    }
    else{
        for(int i=0; i<allCheckingIndices.size(); i++){
            this->checkmatingIndices.push_back(allCheckingIndices.at(i));
        }
        allCheckingIndices.clear();
        return true;
    }
}

void Board::printCheckingPieces(){
    string location;  // location of piece in terms of the board. Ex: A8 instead of 0.
    int index;
    if(this->threatenedKingIndices.size() > 0){
        cout << "Board.cpp: PrintCheckingPieces():";
        cout << "  Locations: [ ";
        for(int i=0; i < this->checkingPieceIndices.size(); i++){
            index = this->checkingPieceIndices.at(i);
            location = Util::reverseParseIndex(index);   // board value for index. ex A2 or H8.
            cout << "(" << location << ", " << index << ")";
            if(i < this->checkingPieceIndices.size() - 1){
                cout << ", ";
            }
        }
        cout << " ]" << endl;
    }
    else{
        cout << "Board.cpp: printCheckingPieces(): King is NOT checked" << endl;
    }
}

void Board::printPotentialCheckingIndices(){
    cout << "Board.cpp: printPotentialCheckingPieces():";
    cout << "  Locations: [ ";
    if(this->potentialCheckingIndices.size() > 0){
        for(int i=0; i < this->potentialCheckingIndices.size(); i++){
            int index = this->potentialCheckingIndices.at(i);
            string location = Util::reverseParseIndex(index);   // board value for index. ex A2 or H8.
            cout << "(" << location << ", " << index << ")";
            if(i < this->potentialCheckingIndices.size() - 1){
                cout << ", ";
            }
        }
        cout << " ]" << endl;
    }
}

void Board::printSpecialIndices(){
    string specialStrs[7] = {
        "NonSpecial",
        "Prequel",
        "Sequel",
        "Castling", 
        "EnPassant",
        "PawnStart",
        "PawnPromo"
    };
    int count = 0;

    cout << "Board.cpp: printSpecialIndices(): [ ";

    if(this->specialHighlightIndices.size() > 0){
        
        for(auto const& [key, val] : this->specialHighlightIndices){
            cout << "{ ";
            cout << key << ":" << specialStrs[val];
            cout << " }";
            if(count < this->specialHighlightIndices.size() - 1){
                cout << ", ";
            }
            count++;
        }
    }
    cout << " ]" << endl;
}

void Board::printMoveHighlightIndices(){
string location;  // location of piece in terms of the board. Ex: A8 instead of 0.
    int index;
    if(this->moveHighlightIndices.size() > 0){
        if(DEBUG_MODE){
            cout << "Board.cpp: printMoveHighlightIndices():";
            cout << "  Locations: [ ";
            for(int i=0; i < this->moveHighlightIndices.size(); i++){
                index = this->moveHighlightIndices.at(i);
                location = Util::reverseParseIndex(index);   // board value for index. ex A2 or H8.
                cout << "(" << location << ", " << index << ")";
                if(i < this->moveHighlightIndices.size() - 1){
                    cout << ", ";
                }
            }
            cout << " ]" << endl;
        }
    }
    else{
        if(DEBUG_MODE) cout << "Board.cpp: printCheckingPieces(): King is NOT checked" << endl;
    }
}

void Board::display(){
    const string reset("\033[0m");
    const string tanBackground("\033[48;2;210;164;121m");
    const string yellowForeground("\033[38;2;255;235;153m");
    const string yellowBackground("\033[48;2;255;235;153m");
    const string selectedBackground("\033[48;2;128;223;255m"); // 
    const string availMoveBackground("\033[48;2;255;255;255m"); // Tsquare is and available move
    const string checkingBackground("\033[48;2;255;77;77m");     // piece is putting a king in check
    const string checkedKingBackground("\033[48;2;255;204;0m");// this king is in check
    const string checkingTargetBackground("\033[48;2;255;204;204m"); // this piece can be captured and it checking the king
    const string checkmatingBackground = checkingBackground; // this piece is putting a king in checkmate 
    const string specialBackground("\033[48;2;238;204;255m");   
    const string blink("\033[5m");
    const string blinkReset("\033[25m");
    const string inverse("\033[7m");
    const string inverseReset("\033[27m");

    // build each line of the board to reduce I/O overhead
    string line;
    string displayOut = "";
    int color;
    int squareIndex = 0;
    Piece piece;
    char rowNumber;
    string outerPadding = "       ";
    string letterPadding = " ";

    // Local function to print out the letters that corespond with columns on the board
    auto uiLetters = [](int rowSize, string padding) -> void {
        string line = "";
        // Add some extra padding since there is 2 chars length from left corner of board to first square center.
        cout << " " << padding;
        for(int i=0; i<rowSize; i++){
            // determine which values of i line up with the center of a square and print out letter's char. 32 is ascii for ' ' 
            char letter = (i % 2 == 0 && (i / 2) % 2 != 0) ? ((i + 2) / 4 ) + 64 : 32;
            cout << letter;
        }
        cout << endl;
    };

    // print letters at top
    uiLetters(this->rowSize, outerPadding + letterPadding);

    // string bg;  // background color for each square
    bool checkingbg = false;
    bool availbg = false;
    for (int j=0; j<this->colSize; j++){
        line = "";
        for (int i=0; i<this->rowSize; i++){
            string bg = "";
            checkingbg = false;
            availbg = false;
            // Color board border area
            if(j % 2 == 0 || i % 4 == 0){
                line += yellowForeground;
                line += tanBackground;
                line += Board::displayboard[j*this->rowSize + i];
                line += reset;
            }
            // Draw entire square, which is 3 chars wide.
            else{
                // create checkered pattern of board
                if(j % 4 == 1){  // account for the edges of rows when coloring squares
                    bg = (squareIndex % 2 != 0) ? tanBackground : yellowBackground;
                }
                else{
                    bg = (squareIndex % 2 == 0) ? tanBackground : yellowBackground;
                }
                // These 3 are all if's to override the basic selection background color
                // check if this square has a checked king
                if(find( begin(this->threatenedKingIndices), end(this->threatenedKingIndices), squareIndex) != end(this->threatenedKingIndices)){
                    bg = checkedKingBackground;
                }
                // check if this square is an available move that needs to be highlighted
                if(find( begin(this->moveHighlightIndices), end(this->moveHighlightIndices), squareIndex) != end(this->moveHighlightIndices)){
                    bg = availMoveBackground;
                    availbg = true;
                }
                // This move is a special move
                if(this->specialHighlightIndices.count(squareIndex)){
                    SpecialMove stype = this->specialHighlightIndices.at(squareIndex);
                    // This move it a potential Castling move
                    if(stype == Castling){
                        bg = specialBackground;
                    }
                    // This move it a potential Castling move
                    else if(stype == EnPassant){
                        bg = specialBackground;
                    }
                    // This pawn can be promoted
                    else if(stype == PawnPromo){
                        bg = specialBackground;
                    }
                }
                // check if this square has a piece putting a king in check
                if(find( begin(this->checkingPieceIndices), end(this->checkingPieceIndices), squareIndex) != end(this->checkingPieceIndices)){
                    bg = checkingBackground;
                    checkingbg = true;
                }
                // this piece is creating a checkmate
                if(find( begin(this->checkmatingIndices), end(this->checkmatingIndices), squareIndex) != end(this->checkmatingIndices)){
                    bg = checkmatingBackground;
                }
                // piece is checking the king, but also can be targeted by a piece
                if(checkingbg && availbg){
                    bg = checkingTargetBackground;
                }
                // specific piece was selected
                if(squareIndex == this->selectedIndex){
                    bg = selectedBackground;
                }
                
                line += bg;
                line += " ";
                line += this->internalboard[squareIndex].toString();
                line += " ";
                i += 2; 
                squareIndex++;
            }
            line += reset;
        }
        // Actually had the numbers and letters flipped, so this is actually a number.
        //   48 is 0 in ascii
        rowNumber = (j % 2 != 0) ? ((j - 209) / -2) - 48 : 32;

        cout << outerPadding;
        printf("%c", rowNumber);
        cout << letterPadding;
        cout << line;
        cout << letterPadding;
        printf("%c\n", rowNumber);
    }
    // print letters at bottom
    uiLetters(this->rowSize, outerPadding + letterPadding);
    // Extra padding at bottom to separate board from input prompt
    cout << endl;
}

// Fully initialize the board
void Board::fullInitialization(){
    this->initMembers();
    this->initBoards();
}

void Board::initMembers(){
    // The board is an 8x8 grid ofsquares, with each square being a 3x5 grid of chars
    this->moveHighlightIndices = {};
    this->checkingPieceIndices = {};
    this->checkmatingIndices = {};
    this->threatenedKingIndices = {};
    this->specialHighlightIndices = {};
    this->promotablePawn = {};
    this->internalboard = new Piece[8*8];
    this->rowSize = 5*8-7;  // 43 -> Each square is 5 chars wide
    this->colSize = 3*8-7;  // 17 -> Each square is 3 chars in height
    this->displayboard = new char[this->rowSize*this->colSize];
    this->selectedIndex = -1;
    this->turnCount = 0;
}

// Initialize the internal gameboard and the board used to display to the user
void Board::initBoards(){
    // initialize internal board if loader will not be used
    if( !this->loaderInit){
        for(int j=0; j<8; j++){
            for(int i=0; i<8; i++){
                switch(j){
                    case(0):    // top row black
                        if(i == 0 || i == 7){ this->internalboard[j*8 + i].init(Black, Rook, 0); }
                        if(i == 1 || i == 6){ this->internalboard[j*8 + i].init(Black, Knight, 0); }
                        if(i == 2 || i == 5){ this->internalboard[j*8 + i].init(Black, Bishop, 0); }
                        if(i == 3) { this->internalboard[j*8 + i].init(Black, Queen, 0); }
                        if(i == 4) { this->internalboard[j*8 + i].init(Black, King, 0); }   
                        break;
                    case(1):    // lower row black
                        this->internalboard[j*8 + i].init(Black, Pawn, 0);
                        break;
                    case(6):    // upper row white
                        this->internalboard[j*8 + i].init(Red, Pawn, 0);
                        break;
                    case(7):    // lower row white
                        if(i == 0 || i == 7){ this->internalboard[j*8 + i].init(Red, Rook, 0); }
                        if(i == 1 || i == 6){ this->internalboard[j*8 + i].init(Red, Knight, 0); }
                        if(i == 2 || i == 5){ this->internalboard[j*8 + i].init(Red, Bishop, 0); }
                        if(i == 3) { this->internalboard[j*8 + i].init(Red, Queen, 0); }
                        if(i == 4){ this->internalboard[j*8 + i].init(Red, King, 0); }
                        break;
                    default:    // all empty squares
                        this->internalboard[j*8 + i].setNull();
                }
            }
        }
    }

    // initialize displayboard values
    for(int j=0; j<colSize; j++){
        // Generate seam between squares on the board
        if(j % 2 == 0){
            for (int i=0; i< this->rowSize; i++){
                if(i % 4 == 0){
                    this->displayboard[j*this->rowSize + i] = '+';
                }
                else{
                    this->displayboard[j*this->rowSize + i] = '=';
                }
            }
        }
        // Generate empty square areas of the board
        else{
            for (int i=0; i< this->rowSize; i++){
                if(i % 4 == 0){
                    this->displayboard[j*this->rowSize + i] = '|';
                }
                else if(i % 4 == 2){
                    // Where individual pieces will be displayed on the board
                    this->displayboard[j*this->rowSize + i] = '0';
                }
                else{
                    this->displayboard[j*this->rowSize + i] = ' ';
                }
            }
        }
    }
}

void Board::reset(){
    this->clear();
    this->fullInitialization();
}

// get all indices that have pieces of TeamColor
vector<int> Board::getTeamPieceIndices(TeamColor tc){
    vector<int> vectorIndices;
    for(int i=0; i < 64; i++){  // there are 64 squares on the internal board
        if(this->getPiece(i).getTeam() == tc){
            vectorIndices.push_back(i);
        }
    }
    return vectorIndices;
}

vector<int> Board::getPotentialCheckingIndices(){
    return this->potentialCheckingIndices;
}

vector<int> Board::getThreatenedKingIndices(){
    return this->threatenedKingIndices;
}

void Board::setMoveHighlightIndices(vector<int> newIndices){
    this->moveHighlightIndices = newIndices;
}

void Board::appendMoveHighlightIndex(int* newIndices, int size){
    for(int i=0; i < size; i++){
        this->moveHighlightIndices.push_back(newIndices[i]);
    }
}

/**
 * Append all values in vector to potentialCheckingIndices
 * @param newIndices Vector to add values from
 * @param duplicates If duplicate values should be added to the array
*/
void Board::appendPotentialCheckingIndices(vector<int> newIndices, bool duplicates){
    for(int i=0; i < newIndices.size(); i++){
        int val = newIndices.at(i);
        if( !duplicates){
            if( !count(begin(this->potentialCheckingIndices), end(this->potentialCheckingIndices), val)){
                this->potentialCheckingIndices.push_back(val);
            }
        }
        else{
            this->potentialCheckingIndices.push_back(val);
        }
    }
}

void Board::setCheckingPieceIndices(vector<int> indices){
    this->checkingPieceIndices = indices;
}

void Board::setCheckedKingIndices(vector<int> indices){
    this->checkingPieceIndices = indices;
}

void Board::appendCheckedKingIndices(int index){
    this->threatenedKingIndices.push_back(index);
}

void Board::appendSpecialIndices(map<int, SpecialMove> table){
    for(auto it = table.begin(); it != table.end(); it++){
        int key = it->first;
        SpecialMove val = it->second;
        if( !this->specialHighlightIndices.count(key)){
            this->specialHighlightIndices.insert( {key, val} );
        }
    }
}

// return index of the king
int Board::getKingIndex(TeamColor tc){
    for(int i=0; i < 64; i++){
        if(this->internalboard[i].getTeam() == tc && this->internalboard[i].getType() == King){
            return i;
        }
    }
    return -1;
}

void Board::printInternal(){
    string line;
    Piece p;
    const string reset("\033[0m");

    for(int j=0; j<8; j++){
        line = "";
        for(int i=0; i<8; i++){
            p = this->internalboard[j*8 + i];
            line += p.toString();
            line += " ";
        }
        line += reset;
        std::cout << "           ";
        std::cout << line << endl;
    }
}

Piece Board::getPiece(int index){
    // DEBUG:
    // cout << "Board.cpp: Index: " << index << endl;
    if(index < 0 || index > 63){
        throw ChessException("Board.cpp: getPiece(): Index is out of bounds");
    }
    return this->internalboard[index];
}

void Board::setPiece(int index, Piece piece){
    this->internalboard[index] = piece;
}

void Board::setPiece(int index, TeamColor tc, PieceType pt, int numMoves){
    this->internalboard[index].init(tc, pt, numMoves);
}

void Board::removePiece(int index){
    this->internalboard[index].setNull();
}

void Board::setSelectedIndex(int index){
    this->selectedIndex = index;
}

// returns true if the index has a non-null piece at the position
bool Board::isIndexOccupied(int index){
    if( !this->getPiece(index).getNull()){
        return true;
    }
    return false;
}

int Board::getTurnCount(){
    return this->turnCount;
}

void Board::setTurnCount(int count){
    this->turnCount = count;
}

void Board::incrementTurnCount(){
    this->turnCount += 1;
}

/**
 * Find a pawn on the board that can be promoted for a team
 * @param tc What team's pawns to examine
 * @returns The index of a promotable pawn. Returns -1 otherwise. Updates specialHighlightIndices for Board.
*/
int Board::findPromotablePawn(TeamColor tc){
    if(tc == NoColor){
        throw InvalidTeamException("Board.cpp: Cannot find promotable pawns for the Null Team");
    }
    int index = -1;
    // index values for top and bottom rows of the board
    // Red team checks index values 0 - 7. Black team checks 56 - 63.
    int start = (tc == Red) ? 0 : 56;
    int end = (tc == Red) ? 8 : 64;
    // Examine back row relative to pawn team
    for(int i = start; i < end; i++){
        if(this->internalboard[i].getTeam() == tc && this->internalboard[i].getType() == Pawn){
            index = i;
            break;
        }
    }
    if(index != -1){
        // this->promotablePawn.push_back(index);
        this->specialHighlightIndices.insert( {index, SpecialMove::PawnPromo} );
    }
    return index;    
}

/**
 * Private method
 * Check the board for promotable pawns on both teams.
 * Used when loading a board state
*/
void Board::loadPromotablePawns(){
    this->findPromotablePawn(Red);
    this->findPromotablePawn(Black);
}

/**
 * Go through all Pawns on the board and remove their En Passant 
 * availability if the window has expired
 * The availability window is the turn right after a pawn becomes
 * available to be captured by En Passant
*/
void Board::pruneEnPassantPawns(){
    if(DEBUG_MODE) cout << "Board.cpp: Pruning pawn En Passant availability" << endl;
    
    for(int i=0; i < 63; i++){ // check internal board, that has 63 elements
        Piece p = this->internalboard[i];
        if(p.getType() == Pawn && p.getTeam() != NoColor){
            if(p.getEnPassantCapture()){
                if(this->turnCount > p.getEnPassantTurn() + 1){
                    this->internalboard[i].setEnPassantCapture(false);
                    if(DEBUG_MODE) cout << "Board.cpp: Prune: Pawn at position: " << Util::reverseParseIndex(i) << " was pruned." << endl;
                }
            }
        }
    }
}

/**
 * Update the PieceType of a specific Piece on the board
 * @param index Location of piece on internalboard array
 * @param type Type of piece
 * @returns nothing
*/
void Board::promotePiece(int index, PieceType type){
    this->internalboard[index].promote(type);
}

/**
    Count all valid pieces on the board
    @returns number of valid pieces on the board
*/
int Board::calcActivePieceCount(){
    int count = 0;
    for(int i=0; i < 64; i++){ //internal board is length 64
        if(this->internalboard[i].getTeam() != NoColor){
            count++;
        }
    }
    return count;
}