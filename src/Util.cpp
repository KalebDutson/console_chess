#include "Util.hpp"

#include <iostream>
#include <cstring>

using namespace std;

/*
    All methods are static in Util
*/


/*
    Convert board coordinates to an integer index. Board is 8*8 grid.
    Position comes in as string. ex: H1 or h1. Ranges from a-h, and 1-8.
    Board array pos (0,0) corresponds to H1. Pos (7,7) corresponds to A8
*/
int Util::parseIndex(string pos){
    int letterChr;
    int numChr;
    int rowInt;
    int colInt;
    int index;

    if(pos.length() != 2){
        throw std::invalid_argument("Util.cpp: Cannot parse non-length 2 strings!");
    }

    // Row size on the board is 8. Board is array from 0-63.
    // convert pos[0] from char to value from 0 - 7
    letterChr = (int) std::tolower(pos.at(0));
    // 7 is 8-1, where 8 is rowsize
    colInt = 7 - (104 - letterChr); // 104 is char for 'h'
    // convert pos[1] from char to value from 0 - 7
    numChr = pos.at(1);
    rowInt = 7 - (numChr - 49); // 49 is char for '1'
    index = rowInt * 8 + colInt;
    
    return index;
}

/*
    Convert integer index value into board coordinates. Board is 8*8 grid.
    Position comes in as an integer within range of 0-63.
    Returns length 2 string. Ex: H1 or h1. Ranges from a-h, and 1-8.
*/
string Util::reverseParseIndex(int index){
    string s = "";
    char letter;
    char number;

    if(index < 0 || index > 63){
        throw std::invalid_argument("Util.cpp: Cannot only calculate values between 0-63");
    }
    letter = (index % 8) + 65;  // 8 for row size on board. 65 is 'A'. 72 is 'H'
    number = abs((index / 8) - 56);  // 8 for row size on board. 49 is '1', 56 is '8'. Abs() is due to 0 index being A8 position.

    s += letter;
    s += number;

    return s;
}