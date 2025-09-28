#include "kee/game.hpp"

int main()
{
#ifdef __APPLE__
    std::println(stderr, "[WARNING] `kee` may not be functional on Apple machines! `kee` uses the Raylib game engine");
    std::println(stderr, "backed by OpenGL, a graphics API which Apple has deprecated.");
#endif

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