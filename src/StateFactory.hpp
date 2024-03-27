#ifndef StateFactory_H
#define StateFactory_H

#include "Piece.hpp"
#include "Gamestate.hpp"
#include "Board.hpp"

#include <iostream>

using namespace std;

/* 
    Strings for different board states.
    Four parts to a state: 
      1. Turn: Which team has the current turn.
      2. Turn Count: How many turns have been taken in the game total.
      3. Capture Delta: How many turns has it been since a piece was captured.
      4. Board: How does the board look.
    1. Turn: 
      "r" : Red team's turn
      "b" : Black team's turn
      "n" : Null turn. Used for debugging
    2. Turn Count: String is converted to integer.
    3. Capture Delta: When this reaches 50, the game ends in a draw.
    4. Board:
    Each piece is represented by up to 3 chars but a minimum of 2 chars.
    
    [0]: { - r b }            The team the piece is on. r = Red. b = Black, - = NullTeam
    [1]: { - r n b q k p }    The piece type. n = knight. All other are self explanatory
    [2]: { - 1 2 e }          The number of moves a piece has made. Only track up to 3
                                since after 2 moves, no piece's moves are further changed. 
                               In the absence of this character or '-' is used, assume that 
                                 number of moves is 0.
                               e = En Passant available. Only on the turn right after a pawn 
                                 moves 2 squares on the first turn can an opponent can perform 
                                 an En Passant on that pawn.
    
    
    NOTE: The space at the end of each line is crucial to parsing the board state correctly unless
           everything is included within one pair of "".
    A "--" or "---" designates a "null piece" or an empty square.
*/

const int REP_PIECE_MAX_LEN = 3;  // Maximum char length of a string representation for a piece

const regex STATE_REGEX_TURN_REP("^([rbn])$");
const regex STATE_REGEX_COUNT_REP("^([0-9]){0,5}");
const regex STATE_REGEX_DELTA_REP("^([0-9]){0,2}");
const regex STATE_REGEX_BOARD_REP("^([-rb])([-rnbqkp])([-12e])?");

// Empty board
const vector<string> STATE_0 = 
    { "r", "0", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "};

// Standard board setup
const vector<string> STATE_1 =
    { "r", "1", "0",
    "br bn bb bq bk bb bn br "
    "bp bp bp bp bp bp bp bp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rp rp rp rp rp rp rp rp "
    "rr rn rb rq rk rb rn rr "};

// Red King in check
const vector<string> STATE_2_0 =
    { "r", "1", "0",
    "br bn bb bq -- bb bn br "
    "bp bp bp bp bp bp bp bp "
    "-- -- -- -- rk -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- rq -- -- -- -- "
    "rp rp rp rp rp rp rp rp "
    "rr rn rb -- -- rb rn rr "};

// Red King and Black King in check
const vector<string> STATE_2 =
    { "r", "1", "0",
    "br bn bb bq -- bb bn br "
    "bp bp bp bp bp bp bp bp "
    "-- -- -- -- rk -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- bk -- rq -- -- -- -- "
    "rp rp rp rp rp rp rp rp "
    "rr rn rb -- -- rb rn rr "};

// Red King and Black King in check, but black has extra bishops, and king has limited movement
const vector<string> STATE_2_1 =
    { "r", "1", "0",
    "-- -- bb bb -- bb bb -- "
    "-- -- -- bb bb bb -- -- "
    "-- -- -- -- rk rp -- -- "
    "-- -- -- -- rp -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- bk -- rq -- -- -- -- "
    "rp rp rp rp rp rp rp rp "
    "rr rn rb -- -- rb rn rr "};

// Red King checkmated by 9 pieces
const vector<string> STATE_3 =
    { "r", "1", "0",
    "br bn bb bq -- bb bn br "
    "bp bn bp rk bp bp bp bp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "};

// Red King checkmated by 4 pieces
const vector<string> STATE_3_1 =
    { "r", "1", "0",
    "-- bn bb -- -- br -- -- "
    "-- bn rp rk rp -- -- -- "
    "-- -- rp -- rp -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "};

// Red King one move from checkmate
const vector<string> STATE_3_2 =
    { "r", "1", "0",
    "-- bn bb -- -- br -- -- "
    "-- bn rp -- rp -- -- -- "
    "-- -- rp rk rp -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "};

// Black King checkmated by 4 pieces
const vector<string> STATE_3_3 =
    { "b", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- bp -- bp -- -- -- "
    "-- rn bp bk bp -- -- -- "
    "-- rn rb -- -- rr -- -- "};

// Red Pawn can En Passant a Black Pawn
const vector<string> STATE_4 =
    { "r", "3", "2",
    "br bn bb bq bk bb bn br "
    "bp bp bp bp -- bp bp bp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- bpe rp2 -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rp rp rp rp rp -- rp rp "
    "rr rn rb rq rk rb rn rr "};

// Red Pawn can NOT En Passant a Black Pawn since the Black Pawn
//  moved twice.
const vector<string> STATE_4_1 =
    { "r" , "5", "4",
    "br bn bb bq bk bb bn br "
    "bp bp bp bp -- bp bp bp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- bp2 rp2 -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rp rp rp rp rp -- rp rp "
    "rr rn rb rq rk rb rn rr "};

// Black pawn can't En Passant red pawn since Black didn't execute the move after the 
//  red pawn moved into En Passant position.
const vector<string> STATE_4_2 = 
    { "b", "7", "6",
    "-- -- -- -- -- -- -- -- "
    "bp bp bp -- bp bp bp -- "
    "-- -- -- -- -- -- -- bp1 "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- bp2 rp1 -- -- -- "
    "rp1 rp1 -- -- -- -- -- -- "
    "-- -- rp rp -- rp rp rp "
    "-- -- -- -- -- -- -- -- "};

// Red pawn at g5 can En Passant both Black Pawns
// Black pawn at c4 can En Passant both red Pawns. 
const vector<string> STATE_4_3 = 
     { "r" , "5", "4",
    "br bn bb bq bk bb bn br "
    "bp bp -- bp -- bp bp bp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- bpe rp2 bpe "
    "-- rpe bp2 rpe -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rp rp rp rp rp rp -- rp "
    "rr rn rb rq rk rb rn rr "};

// Red King can castle with left rook
const vector<string> STATE_5 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- -- "};

// Red King can castle with left and right rooks
const vector<string> STATE_5_1 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- rr "};

// Red King CAN'T castle - King is in check
const vector<string> STATE_5_2 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- br -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- rr "};

// Red King CAN'T castle left rook- can't move over squares that would put him in check
const vector<string> STATE_5_3 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- br -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- rr "};

// Red King can castle left rook- king only moves left 2 squares, and doesn't move over check
const vector<string> STATE_5_4 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- br -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- -- "};

// Red King CAN'T castle right rook- can't move over squares that would put him in check
const vector<string> STATE_5_5 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- br -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- -- rk -- -- rr "};

// Red King CAN'T castle - King has moved at least once
const vector<string> STATE_5_6 = 
    { "r", "3", "2",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr -- -- rk1 -- -- -- rr "};

// Red King CAN'T castle left rook - Rook has moved at least once
const vector<string> STATE_5_7 = 
    { "r", "1", "0",
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "rr1 -- -- -- rk -- -- rr "};

// Red and Black pawns are 1 move from promotion
const vector<string> STATE_6 =
    { "r" , "5", "4",
    "br bn bb bk bq bb bn -- "
    "-- -- bp bp bp bp rp rp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "bp bp rp rp rp rp -- -- "
    "-- rn rb rq rk rb rn rr "};

// Red and Black pawns are 1 move from promotion. Game is 1 move from a draw.
const vector<string> STATE_7 =
    { "r" , "50", "49",
    "br bn bb bk bq bb bn -- "
    "-- -- bp bp bp bp rp rp "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "-- -- -- -- -- -- -- -- "
    "bp bp rp rp rp rp -- -- "
    "-- rn rb rq rk rb rn rr "};

class StateFactory
{
    public:
        static void loadState(Gamestate*, vector<string>);
        static void loadBoard(Board*, string, int);
        static Piece* build(string, int);
};

#endif