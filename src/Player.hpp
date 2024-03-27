#ifndef Player_H
#define Player_H

#include "Piece.hpp"

class Player{

    TeamColor team;

    public:
        Player();
        ~Player();
        TeamColor getTeam();
};

#endif