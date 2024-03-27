#include "Prompt.hpp"
#include "Util.hpp"
#include "ChessException.hpp"
#include "Debug.hpp"

#include <cstring>
#include <iostream>
#include <regex>
#include <string>

using namespace std;


Prompt::Prompt(){
    this->cmdArgs = new int[MAX_ARGS];
    this->input = new char[MAX_INPUT_SIZE];
    this->resetCmdArgs();
    this->resetInputArray();
    this->resetPromoArg();
}

Prompt::~Prompt(){
    delete[] this->cmdArgs;
    delete[] this->input;
}

/*
    Prompt user with promptStr and return an array of int[] that represent different commands    
*/
void Prompt::promptInput(string promptStr){
    this->resetInputArray();
    cout << promptStr << ": ";

    cin.getline(this->input, MAX_INPUT_SIZE);

    this->cmdArgs = Prompt::parseInput();
}

/*
    Prompt user for input specific to pawn promotion
*/
void Prompt::promotionInput(){
    cout << "You can promote a pawn!\nOptions: Queen (q), Rook (r), Bishop (b), Knight (n)" << endl;
    string promptStr = "Enter a single character to choose: ";
    this->resetInputArray();
    cout << promptStr;
    
    cin.getline(this->input, MAX_INPUT_SIZE);

    try{
        this->promoArg = Prompt::parsePromotionInput();
    }
    catch(const invalid_argument &ex){
        cerr << ex.what() << endl;
    }
    catch(const InvalidPromotionException &cex){
        cerr << cex.what() << endl;
    }
}

/**
    private method
    Parse input for pawn promotion.
    @returns Integer associated with promotion piece type
*/
int Prompt::parsePromotionInput(){
    vector<string> inArgs = {};
    regex re("^([qrbnQRBN])$" );
    char* token = strtok(this->input, " ");

    this->resetPromoArg();

    // put all args in array
    while(token != NULL){
        inArgs.push_back( (string) token);
        token = strtok(NULL, " ");
    }
    if(inArgs.size() != 1){
        if(inArgs.size() == 0){
            throw invalid_argument("No arguments. Expected 1");
        }
        if(inArgs.size() > 1){ // only expecting 1 argument
            throw invalid_argument("Too many arguments");
        }
    }
    if(regex_match(inArgs.at(0), re)){
        int promo = -1; // int that corresponds with piece type
        // regex matches chars only, send to lower case
        char c = (int) std::tolower(inArgs.at(0).at(0));
        switch(c){
            case('q'):
                promo = 2;
                break;
            case('r'):
                promo = 3;
                break;
            case('b'):
                promo = 4;
                break;
            case('n'):
                promo = 5;
                break;
        }
        return promo;
    }
    else{
        throw InvalidPromotionException();
    }    
}

/**
    private method
    Parse input into commands and return int array that corresponds to specific command and args
    @returns int list of command arguments
*/
int* Prompt::parseInput(){
    int argsLeft = 0;   // Number of arguments left to parse. Depends on cmd input
    int argCount = 0;   // Total args parsed
    string word;
    string inArgs [MAX_ARGS];
    regex re("(^[a-hA-H])([1-8]$)");
    char* token = strtok(this->input, " ");

    this->resetCmdArgs();

    // put all args in array
    while(token != NULL){
        inArgs[argCount] = (string) token;
        argCount++;
        token = strtok(NULL, " "); 
    }

    for(int i=0; i<MAX_ARGS; i++){
        word = inArgs[i];

        if(argCount > MAX_ARGS){
            this->cmdArgs[0] = InvalidCmd;
            /** DEBUG:*/
            if(DEBUG_MODE) cout << "Too many args" << endl;
            break;
        }
        if(i == 0){
            if(word == "help" || word == "h" || word == "-h" || word == "--h"){
                this->cmdArgs[0] = HelpCmd;
                break;
            }
            else if(word == "sel" && argCount == 2){
                // Usage: sel <src>
                //  ex: sel a4 -> selects piece at a4
                this->cmdArgs[0] = SelectCmd;
                argsLeft = 1;
            }
            else if(word == "mv" && argCount == 2){
                // Usage: mv <dest>
                //  ex: mv c4 -> moves selected piece to c4
                this->cmdArgs[0] = MoveCmd;
                argsLeft = 1;
            }
            else if(word == "mv" && argCount == 3){
                // Usage: mv <src> <dest>
                //  ex: mv a4 c4 -> move piece at a4 to c4
                this->cmdArgs[0] = SelectMoveCmd;
                argsLeft = 2;
            }
            else if(DEBUG_MODE && word == "add" && argCount == 4){
                // Usage: add <dest> <TeamColor> <PieceType>
                //  ex: add a4 1 6 -> adds a red pawn to a4
                this->cmdArgs[0] = AddCmd;
                argsLeft = 3;
            }
            else if(DEBUG_MODE && word == "turn" && argCount == 2){
                // Usage: turn <TeamColor>
                //  ex: turn 1 -> changes current turn to Red team's turn
                this->cmdArgs[0] = TurnCmd;
                argsLeft = 1;
            }
            else if(DEBUG_MODE && word == "rm" && argCount == 2){
                // Usage: rm <dest>
                //  ex: rm a4 -> removes piece at a4
                this->cmdArgs[0] = RemoveCmd;
                argsLeft = 1;
            }
            else if(word == "reset"){
                // Resets the chess game
                this->cmdArgs[0] = ResetCmd;
                break;
            }
            else if(DEBUG_MODE && word == "clear"){
                // Clears the gameboard of all pieces
                this->cmdArgs[0] = ClearCmd;
                break;
            }
            else if(word == "exit" || word == "q" || word == "quit"){
                // Exits the game
                this->cmdArgs[0] = ExitCmd;
                break;
            }
            else if(DEBUG_MODE && word == "load" && argCount == 2){
                // Usage: load <state>
                //  ex: load state_1 -> loads the corresponding state from StateFactory.hpp
                this->cmdArgs[0] = LoadCmd;
                argsLeft = 1;
            }
            else{
                this->cmdArgs[0] = InvalidCmd;
                break;
            }
        }

        else{
            if(argsLeft > 0){
                try{
                    // parse <dest> and <src> commands
                    if(regex_match(word, re) && word.size() == 2){
                        this->cmdArgs[argCount - argsLeft] = Util::parseIndex(word);
                    }
                    // Parse 'add' command args <TeamColor> and <PieceType>
                    else if(this->cmdArgs[0] == AddCmd && argsLeft < 3){
                        /** DEBUG: */
                        if(DEBUG_MODE){
                            cout << "Checking add" << endl;
                            cout << "Word: " << word << " | argsLeft: " << argsLeft << endl;
                        }
                        // set Team for added Piece
                        if(argsLeft == 2){
                            this->cmdArgs[argCount - argsLeft] = this->parseTeamColor(word);
                        }
                        // set PieceType for added Piece
                        else if(argsLeft == 1){
                            this->cmdArgs[argCount - argsLeft] =  this->parsePieceType(word);
                        }
                    }
                    // Parse 'turn' command arg <TeamColor>
                    else if(this->cmdArgs[0] == TurnCmd){
                        this->cmdArgs[argCount - argsLeft] = this->parseTeamColor(word);
                    }
                    /** Parse 'load' command arg <state> 
                     *  TODO: It would be nice to have all the state strings loaded from a file, then
                     *          I could more easily shift through all the names.
                    */
                    else if(this->cmdArgs[0] == LoadCmd){
                        this->cmdArgs[argCount - argsLeft] = stoi(word);
                    }
                    else{
                        throw ChessException();
                    }
                }
                catch(const ChessException &cex){
                    /** DEBUG:*/
                    if(DEBUG_MODE){
                        cout << "Prompt.cpp: Caught ChessException. Invalidating command" << endl;
                        cerr << cex.what() << endl;
                    }
                    this->cmdArgs[0] = InvalidCmd;
                    argsLeft = 0;
                    break;
                }
                catch(const exception &ex){
                    /** DEBUG:*/
                    if(DEBUG_MODE){
                        cout << "Prompt.cpp: Caught unhandled exception when parsing" << endl;
                        cerr << ex.what() << endl;
                    }
                    this->cmdArgs[0] = InvalidCmd;
                    argsLeft = 0;
                    break;
                }
            }
            argsLeft--;
        }

        token = strtok(NULL, " ");        
    }

    return cmdArgs;
}


/**
 * Static method
 * private method
 * Parse int 0-9 from string
*/
int Prompt::parseIntSize1(string word){
    int parsedInt;
    // Expecting value from 0 - 9
    if(word.size() != 1){
        throw ChessException("Failed to parse non-size 1 int");
    }
    parsedInt = (int) word.at(0) - 48; // parse char to int
    return parsedInt;
}

// private method
int Prompt::parseTeamColor(string word){
    int parsedInt = parseIntSize1(word);
    string msg;
    // check bounds of TeamColor enum
    // valid turns are Red (1) and Black (2). The null turn won't be possible to select.
    if(parsedInt >= 1 && parsedInt < 3){
        return parsedInt;
    }
    else{
        throw ChessException("Prompt.cpp: Invalid TeamColor: out of bounds");
    }
    return -1;
}

// private method
int Prompt::parsePieceType(string word){
    int parsedInt = parseIntSize1(word);
    string msg;
    // check bounds of PieceType enum
    if(parsedInt >= 0 && parsedInt < 7){
        return parsedInt;
    }
    else{
        throw ChessException("Prompt.cpp: Invalid PieceType: out of bounds");
    }
    return -1;
}

// private method
void Prompt::resetCmdArgs(){
    for(int i=0; i < MAX_ARGS; i++){
        this->cmdArgs[i] = -1;
    }
}

// private method
void Prompt::resetPromoArg(){
    this->promoArg = -1;
}

// private method
void Prompt::resetInputArray(){
    memset(this->input, 0, MAX_INPUT_SIZE*sizeof(char));
}

bool Prompt::testRegex(string s){    
    char p1;
    char p2;
    string word = "";
    regex re("(^[a-hA-H])([1-8]$)");

    return regex_match(s, re);
}

int* Prompt::getCmdArgs(){
    return this->cmdArgs;
}

int Prompt::getPromoArg(){
    return this->promoArg;
}