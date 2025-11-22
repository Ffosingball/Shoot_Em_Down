#include <SFML/Graphics.hpp>
#include "ECSGame.h"

int main(int argc, char* argv[])
{
    //Start the game!
    ECSGame::Instance().Run(1920, 1080, "Shoot `Em Down");
    return 0;
}