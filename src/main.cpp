#include "kee/game.hpp"

int main()
{
#ifdef __APPLE__
    std::println(stderr, "`kee` does not support Apple machines! This may change in the future.");
    std::println();
    std::println(stderr, "`kee` uses the Raylib game engine backed by OpenGL, a graphics API which Apple has deprecated.");

    return EXIT_FAILURE;
#else
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
#endif
}