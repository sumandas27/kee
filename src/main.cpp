#include "kee/game.hpp"

#include <iostream>

int main()
{
    try {
        kee::game game;
        game.main_loop();
        return EXIT_SUCCESS;
    }
    catch (const raylib::RaylibException& e) 
    {
        std::cerr << "Raylib Exception Caught: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) 
    {
        std::cerr << "General Exception Caught." << std::endl;
        return EXIT_FAILURE;
    }
}