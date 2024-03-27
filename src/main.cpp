#include <iostream>

#include "Warnings.hpp"
#include "Gamestate.hpp"

using namespace std;


int main(){
    Gamestate* g = new Gamestate();
    g->start();

    delete g;

    return 1;
}