#include "kee/game.hpp"

#include <print>

int main()
{
    try {
        kee::game game;
        game.main_loop();
        return EXIT_SUCCESS;
    }
    catch (const raylib::RaylibException& e) 
    {
        std::println(stderr, "Raylib Exception Caught: {}", e.what());
    }
    catch (const std::exception& e) 
    {
        std::println(stderr, "C++ Exception Caught: {}", e.what());
    }
    catch (...) 
    {
        std::println(stderr, "General Exception Caught");
    }

    return EXIT_FAILURE;
}