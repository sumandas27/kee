#include "kee/game.hpp"

/* TODO: comment back in `ifdef` once i come back from vacay */

int main()
{
/*#ifdef __APPLE__
    std::println(stderr, "`kee` does not support Apple machines! This may change in the future.");
    std::println();
    std::println(stderr, "`kee` uses the Raylib game engine which is backed by OpenGL, a graphics API that Apple has deprecated.");

    return EXIT_FAILURE;
#elif*/
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
//#endif
}