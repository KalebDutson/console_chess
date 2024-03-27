#ifndef Prompt_H
#define Prompt_H

#include <iostream>

using namespace std;


enum Command {
    InvalidCmd, // Keep as FIRST command in enum
    HelpCmd,
    SelectCmd,
    MoveCmd,
    SelectMoveCmd,
    ResetCmd,
    ClearCmd,
    AddCmd,
    TurnCmd,
    RemoveCmd,
    LoadCmd,
    ExitCmd // Keep as LAST command in enum
};
const int MAX_CMDS = Command::ExitCmd - Command::InvalidCmd + 1;
const int MAX_ARGS = 4; // maximum number of arguments any command can take.
const int MAX_INPUT_SIZE = 32;  // maximum number of input chars the prompt will accept

class Prompt
{   private:
        int* cmdArgs;
        char* input;
        int promoArg;
        // TODO: stack for error messages

    public:
        Prompt();
        ~Prompt();
        void promptInput(string);
        void promotionInput();
        bool testRegex(string);
        int* getCmdArgs();
        int getPromoArg();

    private:
        int* parseInput();
        int parsePromotionInput();
        static int parseIntSize1(string);
        int parseTeamColor(string);
        int parsePieceType(string);
        void resetCmdArgs();
        void resetPromoArg();
        void resetInputArray();
};

#endif