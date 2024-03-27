#include "Move.hpp"
#include "Board.hpp"
#include "Piece.hpp"
#include "ChessException.hpp"
#include "Util.hpp"
#include "Debug.hpp"

#include <regex>
#include <vector>
#include <cmath>
#include <cstring>
#include <bits/stdc++.h>

using namespace std;


Move::Move(TeamColor tc){
    this->reset(tc);
}

Move::Move(TeamColor tc, int src, int dest){
    this->reset(tc);
    this->sourceIndex = src;
    this->destIndex = dest;
}

Move::Move(TeamColor tc, int src, int dest, SpecialMove spec){
    this->reset(tc);
    this->sourceIndex = src;
    this->destIndex = dest;
    this->special = spec;
    this->secondaryMove = NULL;
}

Move::~Move(){
    this->allDestIndices.clear();
    this->obstructedIndices.clear();
    this->allSpecialIndices.clear();
    if(this->secondaryMove != NULL){
        delete this->secondaryMove;
    }
}

// Setters ===================================

void Move::setSourceIndex(int selected){
    this->sourceIndex = selected;
}

void Move::setDestIndex(int dest){
    this->destIndex = dest;
}

void Move::setTeamColor(TeamColor team){
    this->team = team;
}

void Move::setKingChecked(bool b){
    this->kingChecked = b;
}

void Move::setSpecial(SpecialMove spec){
    this->special = spec;
}

void Move::setSecondaryMove(Move* m){
    /** @attention 
     * This may bug out if my understanding of deleting pointers is wrong.
     * 'm' is allocated with 'new' in calcCastling, and calcEnPassant 
    */
    if(this->secondaryMove != NULL){
        delete this->secondaryMove;
    }
    this->secondaryMove = m;
}

// Getters ===================================

SpecialMove Move::getSpecial(){
    return this->special;
}

Move* Move::getSecondaryMove(){
    return this->secondaryMove;
}

int Move::getSourceIndex(){
    return this->sourceIndex;
}

int Move::getDestIndex(){
    return this->destIndex;
}

TeamColor Move::getTeamColor(){
    return this->team;
}

bool Move::getOpponentCapture(){
    return this->opponentCapture;
}

int* Move::getAllDestIndices(){
    int* arr[this->allDestIndices.size()];
    for(int i=0; i < this->allDestIndices.size(); i++){
        arr[i] = &this->allDestIndices.at(i);
    }
    return *arr;
}

int Move::getAllDestIndexAt(int ind){
    return this->allDestIndices.at(ind);
}

int Move::getSizeofAllDestIndices(){
    return this->allDestIndices.size();
}

bool Move::getAreMovesCalculated(){
    return this->areMovesCalculated;
}

bool Move::getKingChecked(){
    return this->kingChecked;
}

map<int, SpecialMove> Move::getAllSpecialIndices(){
    return this->allSpecialIndices;
}

// Calculations ===================================

/*
    Error checks a move then executes the move if it is valid.
    Throws ChessException when moves are invalid.
*/
Board* Move::move(Board* board){
    // Will throw a ChessException if the move is invalid
    // All error checking and exception throwing for the move happens here
    this->isValidMove(board);
    return this->rawMove(board);
}

/*
    Private Method 
    Execute a move without any error checking
*/ 
Board* Move::rawMove(Board* board){
    Piece src;
    Piece dest;

    // Execute secondary Prequel move if present. For special moves
    if(this->special != NonSpecial){
        /** @attention TODO: Pawn promotion may also use prequels */
        if(this->special == SpecialMove::EnPassant && this->secondaryMove == NULL){
            throw ChessException("Move.cpp: Missing Prequel! Special Move has no Prequel action attached to it!");
        }
        if(this->secondaryMove != NULL){
            if(this->secondaryMove->getSpecial() == SpecialMove::Prequel){
                board = this->secondaryMove->rawMove(board);
            }
        }
    }
    else {
        // This will break recursion if any prequel move has its own secondary move.
        if(this->secondaryMove != NULL){
            throw ChessException("Move.cpp: Unexpected Prequel secondary move! Non-Special move has a Prequel action!");
        }
    }

    // Execution of 'this'
    src = board->getPiece(this->sourceIndex);
    dest = board->getPiece(this->destIndex);
    src.incrementNumMoves(1);   // src Piece is going to move. Increment its move count by 1

    /** @attention May need to check destination instead of source on some of these? */
    // Mark pawns as available for En Passant. Unmarking happens in Board::pruneEnPassantPawns.
    if(src.getType() == Pawn && this->special == SpecialMove::NonSpecial){
        if(src.getNumMoves() == 1){
            // pawn took the big move for its first move. Pawn moved up 2 squares, rowsize is 8, (8*2)=16. So check for movement of 16 in the internal board array.
            if(abs(this->sourceIndex - this->destIndex) == 16){
                if(DEBUG_MODE){
                    cout << "Move.cpp: rawMove: Pawn move from " << Util::reverseParseIndex(this->sourceIndex) << "->" << Util::reverseParseIndex(this->destIndex) << endl;
                    cout << "\tPawn at " << Util::reverseParseIndex(this->destIndex) << " now available for En Passant Capture. " << endl;
                }
                src.setEnPassantCapture(true, board->getTurnCount());
            }
        }
    }
    // Nulify destination piece
    dest.setNull();
    // Move selected piece to destination
    board->setPiece(this->destIndex, src);
    // move the now null piece to the starting location
    board->setPiece(this->sourceIndex, dest);

    /** TODO: @attention */
    // Execute secondary special sequel move if present. For special moves
    if(this->special != NonSpecial){
        if(this->special == SpecialMove::Castling && this->secondaryMove == NULL){
            throw ChessException("Move.cpp: Missing Sequel! Special Move has no Sequel action attached to it!");
        }
        if(this->secondaryMove != NULL){
            if(this->secondaryMove->getSpecial() == SpecialMove::Sequel){
                board = this->secondaryMove->rawMove(board);
            }
        }
    }
    else {
        // This will break recursion if any sequel move has its own secondary move.
        if(this->secondaryMove != NULL){
            throw ChessException("Move.cpp: Unexpected Sequel secondary move! Non-Special move has a Sequel action!");
        }
    }

    return board;
}

void Move::isValidSelection(Board* board){
    if(this->getTeamColor() == NoColor){
        throw ChessException("Move.cpp: No team selected for move");
    }
    // Check against max bounds of the board
    if(this->sourceIndex < 0 || this->sourceIndex > 63){
        throw MissingSourceException();
    }
    // selected piece is a null piece
    else if(board->getPiece(this->sourceIndex).getNull()){
        throw MissingSourceException();
    }
    // Selected piece must match the current player's turn
    else if(board->getPiece(this->sourceIndex).getTeam() != this->team){
        throw InvalidTeamException();
    }
}

// throws ChessException if the attempted move is invalid
void Move::isValidMove(Board* board){
    // check if selected piece is valid
    this->isValidSelection(board);
    if(this->destIndex < 0 || this->destIndex > 63){
        throw MissingDestinationException();
    }
    // Destination of selected piece is on the player's team
    else if(board->getPiece(this->destIndex).getTeam() == this->team){
        throw InvalidMoveException("Move.cpp: Friendly fire will not be tolerated");
    }

    if( !this->areMovesCalculated){
        // only need to calculate all valid moves if it hasn't been done yet and determine if king is in check
        this->calcAllMoves(board, false);
        this->areMovesCalculated = true;
    }
}

void Move::validateMoveset(string mvset){
    // Check if mvset matches moveset regex defined in Piece.hpp
    if(!regex_match(mvset, MOVESET_REGEX)){
        if(DEBUG_MODE) cout << "Move.cpp: mvset: " << mvset << endl;
        throw ChessException("Move.cpp: Moveset is NOT VALID");
    }
}

// TODO: Force the user to make a valid move and reject invalid moves that were not caclulated
/**
 * Calculate all possible moves that can be made
 * @param board The Board pointer to calculate the move on
 * @param determineCheck True if you want to eliminate moves that put the king in check from the list of valid moves.
 * @returns Nothing. But Move.allDestIndices member will be updated.
*/
vector<Move*> Move::calcAllMoves(Board* board, bool determineCheck){
    // determine if selection is valid since this method is call when finding potential moves to display to user.
    this->isValidSelection(board);    
    // Determine if the king is already in check
    if(determineCheck){
        if(DEBUG_MODE && CHECK_DEBUG) cout << "Move.cpp: Determining if king is in check" << endl;
        this->kingChecked = board->isCheck(this->team);
    }
    else if(this->kingChecked == true){
        if(DEBUG_MODE && CHECK_DEBUG) cout << "Move.cpp: King was already checked. Not recomputing." << endl;
    }

    /** DEBUG: king is checked */
    if(this->kingChecked && DEBUG_MODE && CHECK_DEBUG){
        cout << "Move.cpp: Team \"" << teamString[this->team] << "\"'s King is Checked!" << endl;
    }
    // get piece type
    PieceType pt = board->getPiece(this->sourceIndex).getType();
    // get moveset for specific piece
    vector<string> moveset = board->getPiece(this->sourceIndex).getStdMoveset();  

    // validate movesets for this piece
    for(int i=0; i < moveset.size(); i++){
        this->validateMoveset( moveset.at(i) );
    }
    /* 
        (P) Pawns can only move forward, all other pieces have no (N) restrictions
        Direction:       moveset[0] { PN }  (
        Movement vector: moveset[1] { +/|*L }
        Distance:        moveset[2] { 127 }
        Can attack:      moveset[3] { AN }
    */

    // calcMovesetMoves() returns a vector, so store all of them, calcCastling and calcEnPassant do too.
    // Calculate all moves that can be made by each moveset this piece has. Doesn't check if moves check the king
    vector<vector<Move*>> vectorMovesetMoves;

    // Check for special move types
    if(pt == Pawn){
        // Check attack sequence
        moveset.push_back(PAWN_ATTK.at(0)); // TESTING

        // Check if pawn has yet to move
        // Pawn hasn't moved yet
        if(board->getPiece(this->sourceIndex).getNumMoves() == 0){
            moveset.push_back(PAWN_START.at(0)); // TESTING
        }
        // en passant
        try{
            vectorMovesetMoves.push_back(this->calcEnPassant(board));
        } 
        catch(const ChessException &cex){
            if(DEBUG_MODE){
                cerr << cex.what() << endl;
                cout << "Move.cpp: Failed to find valid En Passant Move" << endl;
            }
        }

    }
    else if(pt == King){
        // Check castling availability
        try{
            vectorMovesetMoves.push_back(this->calcCastling(board));
        }
        catch(const ChessException &cex){
           if(DEBUG_MODE) cout << "Move.cpp: Failed to find valid Castling move" << endl;
        }
    }

    for(int i=0; i<moveset.size(); i++){
        vectorMovesetMoves.push_back(this->calcMovesetMoves(moveset.at(i), board));
    }

    // Put all the moves into one vector
    vector<Move*> movesetMoves; // will store all moves in 1 vector
    for(int i=0; i<vectorMovesetMoves.size(); i++){
        for(int j=0; j < vectorMovesetMoves.at(i).size(); j++){
            movesetMoves.push_back(vectorMovesetMoves.at(i).at(j));
        }
    }
    // clean up vectorMovesetMoves
    for(int i=0; i<vectorMovesetMoves.size(); i++){
        for(int j=0; j < vectorMovesetMoves.at(i).size(); j++){
            vectorMovesetMoves.at(j).clear();
        }
    }
    vectorMovesetMoves.clear();

    // For every potential move calculated determine if they put or keep the king in check
    vector<Move*> finalValidMoves = {};  // moves that have been fully validated
    for(int i=0; i<movesetMoves.size(); i++){
        Board* potentialBoardState = new Board(false);
        potentialBoardState->clone(board); // clone current board
        potentialBoardState->clearAllHighlightedIndices();
        Move* m = movesetMoves.at(i);
        potentialBoardState = m->rawMove(potentialBoardState); // ignore error checking for move, since it has already been validated
        m->setKingChecked(potentialBoardState->isCheck(m->getTeamColor()));
         
        // add potentialCheckingMoves from potentialBoardState to board! This lets the isCheckmate() method in Board
        //  highlight all pieces that contribute to a checkmate
        board->appendPotentialCheckingIndices(potentialBoardState->getPotentialCheckingIndices(), false);

        if(DEBUG_MODE && POTENTIAL_STATE_DEBUG){
            string threatLevel = m->getKingChecked() ? "CHECK" : "SAFE";
            cout << Util::reverseParseIndex(m->getDestIndex()) << "->" << threatLevel << " to move here" << endl;
            cout << "Move.cpp: POTENTIAL BOARD STATE!" << endl;
            potentialBoardState->display();
            cout << endl;
        }

        delete(potentialBoardState);
        // move doesnt put or keep king in check. Add to object
        if( !m->kingChecked){
            // dont add dupli`es
            int loc = m->getDestIndex();
            // loc = m->getSourceIndex(); // debug
            // add special moves
            if(m->getSpecial() != SpecialMove::NonSpecial && m->getSpecial() != SpecialMove::Sequel){ 
                if(!this->allSpecialIndices.count(loc)){
                    // map for highlighting
                    this->allSpecialIndices.insert( {loc, m->getSpecial()} );
                    // add to returned vector
                    finalValidMoves.push_back(m);
                }
            }
            else if( !count(begin(this->allDestIndices), end(this->allDestIndices), loc)){
                // array for highlighting
                this->allDestIndices.push_back(loc);
                // add to returned vector
                finalValidMoves.push_back(m);
            }
        }
    }

    // TODO: this will need to move. Only use this when king is not checked
    this->areMovesCalculated = true;
    
    // clean up
    movesetMoves.clear();;

    return finalValidMoves;
}

/*
    Find all moves that can be made for a moveset, within the bounds of the board.
    Note: Does NOT take into account if a move puts the king in check

    Has the following side effects:
    1) Set values to vector<move*> nextMoves
    2) Set value to int numBranches
    3) Set values for vector<int> allDestIndices
        3a) Pieces that have distance 7 will need to loop through all branch moves and their branch moves to completely compute this value for the root move
    4) Set value for numBranchIndices
*/
vector<Move*> Move::calcMovesetMoves(string moveset, Board* board){
    /*
        Remember that H1 is index 0 in the array.
                      A8 is 63 in the array.

        Because of this, adding and subtracting rowsize for the rows may seem backwards
    */
    char mvector = moveset.at(1);
    int dist = moveset.at(2) - 48; // char string to int. 48 is 0 in ascii. 

    // //DEBUG:
    // cout << "Board.cpp: Searching moves to depth: " << dist << endl;

    vector<Move*> possMoves;

    // Calculate movement vector and distance
    if(mvector == '|' || mvector == '+' || mvector == '*'){
        bool upStop = false;
        bool downStop = false;
        // Vertical movements
        for(int depth = 1; depth < dist + 1; depth++){
            // check if any moves can still be made
            if(upStop && downStop){
                break;
            }
            // recall that 0 index is top left corner of board
            int up = this->getSourceIndex() - 8 * depth;
            int down = this->getSourceIndex() + 8 * depth;

            // Add left and right moves
            if(!upStop){
                possMoves.push_back(this->calcNewMove(board, up));
                upStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), up);
            }
            if(!downStop){
                possMoves.push_back(this->calcNewMove(board, down));
                downStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), down);
            }
        }
    }
    if(mvector == '+' || mvector == '*'){
        // Lateral movements
        bool leftStop = false;
        bool rightStop = false;
        for(int depth = 1; depth < dist + 1; depth++){
            // check if any moves can still be made
            if(leftStop && rightStop){
                break;
            }
            // calcNewMove will calculate the bounds of this row using this index
            int boundi = this->getSourceIndex(); // bound index for calculating bounds of current row
            int left = this->getSourceIndex() - 1 * depth;
            int right = this->getSourceIndex() + 1 * depth;

            // Add left and right moves
            if(!leftStop){
                possMoves.push_back(this->calcNewMove(board, left, boundi));
                leftStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), left);
            }
            if(!rightStop){
                possMoves.push_back(this->calcNewMove(board, right, boundi));
                rightStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), right);
            }
        }
    }
    if(mvector == '/' || mvector == '*'){
        // Diagonal movements
        bool neStop = false;
        bool nwStop = false;
        bool seStop = false;
        bool swStop = false;
        for(int depth = 1; depth < dist + 1; depth++){
            // check if any moves can still be made
            if(nwStop && neStop && swStop && seStop){
                break;
            }
            // Calculate bound indices for row above and below the selected piece's
            int aboveBoundi = this->getSourceIndex() - 8 * depth; // bound index for calculating bounds of row above selected piece
            int belowBoundi = this->getSourceIndex() + 8 * depth; // bound index for calculating bounds of row below selected piece

            int nw = this->getSourceIndex() - (8 - 1) * depth;
            int ne = this->getSourceIndex() - (8 + 1) * depth;
            int sw = this->getSourceIndex() + (8 - 1) * depth;
            int se = this->getSourceIndex() + (8 + 1) * depth;

            // Only add diagonal movements within bounds and not blocked by a piece
            if(!nwStop){ 
                possMoves.push_back(this->calcNewMove(board, nw, aboveBoundi));
                nwStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), nw);
            }
            if(!neStop){ 
                possMoves.push_back(this->calcNewMove(board, ne, aboveBoundi));
                neStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), ne);
            }
            if(!swStop){ 
                possMoves.push_back(this->calcNewMove(board, sw, belowBoundi));
                swStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), sw);
            }
            if(!seStop) { 
                possMoves.push_back(this->calcNewMove(board, se, belowBoundi));
                seStop = std::count(this->obstructedIndices.begin(), this->obstructedIndices.end(), se);
            }
        }
    }
    if(mvector == 'L'){
        // L movements for Knights

        // Calculate bounds for rows +1, +2, and -1, -2 relative to piece's row
        int aboveTwoBoundi = this->getSourceIndex() - 16; // bound index for calculating bounds of 2 rows above selected piece
        int aboveBoundi = this->getSourceIndex() - 8; // bound index for calculating bounds of row above selected piece
        int belowBoundi = this->getSourceIndex() + 8; // bound index for calculating bounds of row below selected piece
        int belowTwoBoundi = this->getSourceIndex() + 16; // boundsindex for calculating bounds of 2 rows below selected piece
        
        // Variables correspond to the hands on a clock, which are the positions a knight can move to on the board
        int one0Clock = this->getSourceIndex() - 16 + 1;
        int two0Clock = this->getSourceIndex() - 8 + 2; 
        int four0Clock = this->getSourceIndex() + 8 + 2;
        int five0Clock = this->getSourceIndex() + 16 + 1;
        int seven0Clock = this->getSourceIndex() + 16 - 1;
        int eight0Clock = this->getSourceIndex() + 8 - 2;
        int ten0Clock = this->getSourceIndex() - 8 - 2;
        int eleven0Clock = this->getSourceIndex() - 16 - 1;

        // calculate moves
        possMoves.push_back(this->calcNewMove(board, eleven0Clock, aboveTwoBoundi));
        possMoves.push_back(this->calcNewMove(board, one0Clock, aboveTwoBoundi));
        possMoves.push_back(this->calcNewMove(board, ten0Clock, aboveBoundi));
        possMoves.push_back(this->calcNewMove(board, two0Clock, aboveBoundi));
        possMoves.push_back(this->calcNewMove(board, eight0Clock, belowBoundi));
        possMoves.push_back(this->calcNewMove(board, four0Clock, belowBoundi));
        possMoves.push_back(this->calcNewMove(board, seven0Clock, belowTwoBoundi));
        possMoves.push_back(this->calcNewMove(board, five0Clock, belowTwoBoundi));
    }

    // Check direction. Remove all non-forward moves for Pawn
    // Black pieces must move to increasing indices, Red pieces must move to decreasing indices.
    if(moveset.at(0) == 'P'){
        for(int i = 0; i < possMoves.size(); i++){
            bool isBehind = true;  // Check pawn direction variables
            if(possMoves.at(i)->getTeamColor() == Black && this->getSourceIndex() < possMoves.at(i)->getDestIndex()){
                // this piece is moving forward relative to Black's start
                isBehind = false;
            }
            else if(possMoves.at(i)->getTeamColor() == Red && this->getSourceIndex() > possMoves.at(i)->getDestIndex()){
                // this piece is moving forward relative to Red's start
                isBehind = false;
            }
            if(isBehind){
                // Reset the move to put its src and dest indices out of the board boundaries. Then the move will never be added when board constraints are checked
                possMoves.at(i)->reset(this->team);
            }
        }
    }

    // Check attack availability. 
    // 'A': Attack only move. Eliminate moves that don't take an opponent piece
    // 'N': Non-attack move. Eliminate moves onto opponent pieces
    // 'U': Unrestricted move. No moves eliminated based on opponent pieces
    if(moveset.at(3) != 'U'){
        for(int i=0; i<possMoves.size(); i++){
            Move* m = possMoves.at(i);
            // check that move is within bounds
            if( !(m->getDestIndex() >= 0 && m->getDestIndex() < 64 && m->getSourceIndex() >= 0 && m->getSourceIndex() < 64)){
                continue;
            }
            // attack availability
            if(moveset.at(3) == 'N'){ 
                // check that destination has no piece
                if(board->getPiece(m->getDestIndex()).getTeam() != NoColor){
                    m->reset(this->team); // reset this move so it never gets added when src and dest indices are checked
                }
            }
            else if(moveset.at(3) == 'A'){ 
                // check if destination has opponent piece or friendly piece (friendly fire eliminated in last loop)
                if(board->getPiece(m->getDestIndex()).getTeam() == NoColor){
                    m->reset(this->team); // reset this move so it never gets added when src and dest indices are checked
                }
            }
        }
    }
 
    // Check bounds on possible moves and prevent friendly fire.
    vector<Move*> movesetMoves;
    for(int i=0; i<possMoves.size(); i++){
        Move* m = possMoves.at(i);
        kingChecked = this->kingChecked;
        if(m->getDestIndex() >= 0 && m->getDestIndex() < 64 && m->getSourceIndex() >= 0 && m->getSourceIndex() < 64){ 
            // friendly fire check
            if(board->getPiece(m->getDestIndex()).getTeam() != this->team){
                // Move is valid to moveset            
                movesetMoves.push_back(m); 
            }
        }
    }
    // clean up possMoves
    possMoves.clear();

    return movesetMoves;
}

Move* Move::calcNewMove(Board* board, int dest){
    return this->calcNewMove(board, dest, -2); // default bound index is -2
}

/* 
    Generates a new move based on board, dest, and bounds.
    Source is pulled from the current move
    Adds values to this->obstructedIndices
*/
Move* Move::calcNewMove(Board* board, int dest, int boundIndex){
    int* bounds = new int[2];
    int boardBounds[2] = {0, 64};
    // default bounds values
    if(boundIndex == -2){
        bounds[0] = boardBounds[0];
        bounds[1] = boardBounds[1];
    }
    else{
        bounds = generateBounds(boundIndex, bounds);
    }
    
    // check generated bounds and board bounds
    Move* newMove;
    if(dest >= bounds[0] && dest < bounds[1] && dest >= boardBounds[0] && dest < boardBounds[1]){
        // Move objects are added to possMoves in calcMovesetMoves() method where they are managed and deleted
        newMove = new Move(this->team, this->sourceIndex, dest);
        // save obstructed index
        if(board->isIndexOccupied(dest)){
            this->obstructedIndices.push_back(dest);
        }
    }
    // if move is invalid. Set move to null move and it will be discarded when possMoves is checked in calcMovesetMoves()
    //   set obstruction to true since the piece cant move outside the board
    else{
        this->obstructedIndices.push_back(dest);
        newMove = new Move(NoColor, -1, -1);
    }
    // clean up
    delete[] bounds;
    return newMove;
}

vector<Move*> Move::calcCastling(Board* board){    
    // check number of moves of king and validate king's position. Black = 4, Red = 60
    bool invalid = false;
    switch(this->team){
        case(Red):
            if(board->getKingIndex(this->team) != 60){ invalid = true; }
            break;
        case(Black):
            if(board->getKingIndex(this->team) != 4){ invalid = true; }
            break;
    }
    // King can't castle if in check
    int kingSource = board->getKingIndex(this->team);
    if(board->isCheck(this->team)){
        invalid = true;
    }
    if(invalid){
        if(DEBUG_MODE) cout << "Move.cpp: Invalid King position for Castling." << endl;
        throw ChessException("Move.cpp: Invalid King position for Castling.");
    }
    /* valid black rook indices: 0 (left), 7 (right)
       valid red rook indices: 56 (left), 63 (right) */
    bool leftRookAvailable = true;
    bool rightRookAvailable = true;
    // indices on board for black and red rooks
    int kingDestLeft = kingSource - 2;
    int kingDestRight = kingSource + 2;
    int leftRookSource = this->team == Black ? 0 : 56;
    int leftRookDest = this->team == Black ? 3 : 59; 
    int rightRookSource = this->team == Black ? 7 : 63;
    int rightRookDest = this->team == Black ? 5 : 61;
    // find rooks in valid position
    if( !(board->getPiece(leftRookSource).getType() == Rook && board->getPiece(leftRookSource).getTeam() == this->team)) leftRookAvailable = false;
    if( !(board->getPiece(rightRookSource).getType() == Rook && board->getPiece(rightRookSource).getTeam() == this->team)) rightRookAvailable = false;
    // check number of moves of rooks
    Piece leftRook = board->getPiece(leftRookSource);
    Piece rightRook = board->getPiece(rightRookSource);
    if(leftRookAvailable){
        if(leftRook.getNumMoves() > 0) leftRookAvailable = false;
    }
    if(rightRookAvailable){
        if(rightRook.getNumMoves() > 0) rightRookAvailable = false;
    }
    // Check for gap between left rook and king
    if(leftRookAvailable){
        bool blocked = false;
        for(int i=leftRookSource + 1; i < kingSource; i++){
            if(board->getPiece(i).getType() != NoPiece) blocked = true; break;
        }
        if(blocked) leftRookAvailable = false;
    }
    // Check for gap between right rook and king
    if(rightRookAvailable){
        bool blocked = false;
        for(int i=kingSource + 1; i < rightRookSource; i++){
            if(board->getPiece(i).getType() != NoPiece) blocked = true; break;
        }
        if(blocked) rightRookAvailable = false;
    }
    // Look for check conditions on left rook gap
    if(leftRookAvailable){
        bool passThroughCheck = false;
        for(int i=kingDestLeft + 1; i < kingSource; i++){
            // place a king at each square in the gap to look for check
            Board* potentialBoard = new Board(board);
            Move* theoMove = new Move(this->team, kingSource, i);
            theoMove->rawMove(potentialBoard);
            passThroughCheck = potentialBoard->isCheck(this->team);
            if(passThroughCheck){
                leftRookAvailable = false;
                break;
            }
            // clean up
            delete(theoMove);
            delete(potentialBoard);
        }
    }
    // Look for check conditions on right rook gap
    if(rightRookAvailable){
        bool passThroughCheck = false;
        for(int i=kingSource + 1; i < kingDestRight; i++){
            // place a king at each square in the gap to look for check
            Board* potentialBoard = new Board(board);
            Move* theoMove = new Move(this->team, kingSource, i);
            theoMove->rawMove(potentialBoard);
            passThroughCheck = potentialBoard->isCheck(this->team);
            if(passThroughCheck){
                rightRookAvailable = false;
                break;
            }
            // clean up
            delete(theoMove);
            delete(potentialBoard);
        }
    }
    // Create valid castling move and add secondary move to castling move that repositions the castled rook
    vector<Move*> castlingMoves = {};
    /** @attention 
     * The secondary Move() object should be managed in Move::setSecondaryMove() and the 
     * move destructor.
    */
    if(leftRookAvailable){
        if(DEBUG_MODE) cout << "Move.cpp : Left rook can castle!" << endl;

        // Move for King
        Move* m = new Move(this->team, kingSource, kingDestLeft);
        m->setSpecial(SpecialMove::Castling);
        // secondary move to move left rook
        Move* sec = new Move(this->team, leftRookSource, leftRookDest);
        sec->setSpecial(SpecialMove::Sequel);
        m->setSecondaryMove(sec);
        // add to vector
        castlingMoves.push_back(m);
    }
    if(rightRookAvailable){
        if(DEBUG_MODE) cout << "Move.cpp : Right rook can castle!" << endl;

        // Move for king
        Move* m = new Move(this->team, kingSource, kingDestRight);
        m->setSpecial(SpecialMove::Castling);
        // secondary move to move right rook
        Move* sec = new Move(this->team, rightRookSource, rightRookDest);
        sec->setSpecial(SpecialMove::Sequel);
        m->setSecondaryMove(sec);
        // add to vector
        castlingMoves.push_back(m);
    }
    
    return castlingMoves;
}

vector<Move*> Move::calcEnPassant(Board* board){
    if(this->sourceIndex < 0 || this->sourceIndex > 63){
        throw ChessException("Move.cpp: Selected index is out of bounds.");
    }
    if(board->getPiece(this->sourceIndex).getType() != Pawn){
        throw ChessException("Move.cpp: Cannot calculate En Passant for non-pawn piece!");
    }
    if(DEBUG_MODE) cout << "Move.cpp: Calculating En Passant" << endl;

    // Get row this pawn is on
    int* myBounds = new int[2];
    myBounds = this->generateBounds(this->sourceIndex, myBounds);
    // pawn must be shoulder-shoulder with opponent pawn
    bool leftAvail = true;
    bool rightAvail = true;
    // locations of potential opponent pieces
    int leftOppIndex = this->sourceIndex - 1;
    int rightOppIndex = this->sourceIndex + 1;
    int* leftBounds = new int[2];
    leftBounds = this->generateBounds(leftOppIndex, leftBounds);
    int* rightBounds = new int[2];
    rightBounds = this->generateBounds(rightOppIndex, rightBounds);

    // bounds is array of length 2
    if(myBounds[0] != leftBounds[0] || myBounds[1] != leftBounds[1]){ leftAvail = false; }
    if(myBounds[0] != rightBounds[0] || myBounds[1] != rightBounds[1]){ rightAvail = false; }

    // clean up. Bounds are not needed after sides are verified as pawns on the same row
    delete[] myBounds;
    delete[] leftBounds;
    delete[] rightBounds;
    TeamColor oppTeam = (this->team == Red) ? Black : Red;
    Piece lp = board->getPiece(leftOppIndex);
    Piece rp = board->getPiece(rightOppIndex);
    // verify pieces to left and right are pawns of the opponent team
    if(leftAvail){
        if(lp.getType() != Pawn || lp.getTeam() != oppTeam){
            leftAvail = false;
        }
    }
    if(rightAvail){
        if(rp.getType() != Pawn || lp.getTeam() != oppTeam){
            rightAvail = false;
        }
    }
    // verify pawns to left and right are available for en passant
    if(leftAvail){
        if(!lp.getEnPassantCapture()){
            leftAvail = false;
        }
    }
    if(rightAvail){
        if(!rp.getEnPassantCapture()){
            rightAvail = false;
        }
    }
    /* 
        For each team:
        Black: black pawn is on 4th rank -> can capture red pawns on this rank. [32 - 39] in array
        Red: red pawn is on 5th rank -> can capture black pawns on this rank.   [24 - 31] in array   
    */
    
    // Pawns can be captured with En Passant move
    // Red pawns need to decrease index when moving, Black pawns need to increase index when moving
    // the 8 is for the row size in the board array. Board is 8*8 array
    int direction = this->team == Black ? 1 : -1;
    int leftDest = this->sourceIndex + (direction * 8) - 1;  
    int rightDest = this->sourceIndex + (direction * 8) + 1;
    vector<Move*> mvs = {};
    if(leftAvail){
        if(DEBUG_MODE) cout << "Move.cpp : Left Pawn can be captured via En Passant!" << endl;
        Move* leftEP = new Move(this->team, this->sourceIndex, leftDest, SpecialMove::EnPassant);
        mvs.push_back(leftEP);
        // add prequel action to move opponent pawn to destination so the actual move captures the pawn
        Move* leftPreq = new Move(oppTeam, leftOppIndex, leftDest, SpecialMove::Prequel);
        leftEP->setSecondaryMove(leftPreq);
    } 
    if(rightAvail){
        if(DEBUG_MODE) cout << "Move.cpp : Right Pawn can be captured via En Passant!" << endl;
        Move* rightEP = new Move(this->team , this->sourceIndex, rightDest, SpecialMove::EnPassant);
        mvs.push_back(rightEP);
        // add prequel action to move opponent pawn to destination so the actual move captures the pawn
        Move* rightPreq = new Move(oppTeam, rightOppIndex, rightDest, SpecialMove::Prequel);
        rightEP->setSecondaryMove(rightPreq);

    }

    return mvs;
}

void Move::reset(TeamColor tc){
    this->special = NonSpecial;
    this->team = tc;
    this->opponentCapture = false;
    this->sourceIndex = -1;
    this->destIndex = -1;
    this->areMovesCalculated = false;
    this->secondaryMove = NULL;
    // vector clean up
    this->allDestIndices.clear();
    this->obstructedIndices.clear();
    this->allSpecialIndices.clear();

}

/**
 * Static method
 * Generate the bounds for a specific row on the Board
 * @param index The piece to examine and find the row of
 * @param bounds Integer array of length two
 * @returns Bounds for row on the Board
*/
int* Move::generateBounds(int index, int* bounds){
    // Bound variables
    int lowerBound; // inclusive. lowerBound <= x
    int upperBound; // non-inclusive. x < upperBound
    float n;
    float whole, deci;
    float lowerDeci;
    float upperDeci;

    n = (index / 8.0);
    deci = std::modf(n, &whole);
    upperDeci = index <= 8 / 2 ? deci : abs(1 - deci);
    lowerDeci = abs(1 - upperDeci);

    lowerBound = (int) index - (8 * lowerDeci);
    upperBound = (int) index + (8 * upperDeci);
    // Math fails if index is too low
    if(index < 8 / 2 && index >= 0){
        lowerBound = 0;
        upperBound = 8;
    }
    bounds[0] = lowerBound;
    bounds[1] = upperBound;
    return bounds;
}

void Move::appendAllDestIndices(int* newIndices, int size){
    for(int i=0; i < size; i++){
        this->allDestIndices.push_back(newIndices[i]);
    }
}  

void Move::printAllDestIndices(){
    cout << "Move.cpp: Printing Move Indices:" << endl;
    cout << "   Selected piece has: " << this->allDestIndices.size() << " potential moves." << endl;
    cout << "   All possible moves: [ ";
    for(int i=0; i < this->allDestIndices.size(); i++){
        cout << this->allDestIndices.at(i);
        if(i < this->allDestIndices.size() - 1){
            cout << ", ";
        }
    }
    cout << " ]" << endl;
}

/*
    Two moves are equivalent if they have the same team, source index, and destination index
*/
bool Move::equals(Move* other){
    if(other->getSourceIndex() == this->sourceIndex && other->getDestIndex() == this->destIndex && this->team == other->getTeamColor()){
        return true;
    }
    return false;
}