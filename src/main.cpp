#include "kee/game.hpp"

/* TODO: add key color decorations (this is big addition) */
/* TODO: start marketing (set up steam page + trailer + maybe devlog) */

int main()
{
    if constexpr (!magic_enum::is_magic_enum_supported)
    {
        std::println(stderr, "[ERROR] `kee` uses `magic_enum` as a dependency, which is not supported on your C++ compiler");
        return EXIT_FAILURE;
    }

#ifdef __APPLE__
    std::println(stderr, "[WARNING] `kee` may not be functional on Apple machines! `kee` uses the Raylib game engine");
    std::println(stderr, "backed by OpenGL, a graphics API which Apple has deprecated.");
#endif

    //try {
        kee::game game;
        game.begin_main_loop();
        return EXIT_SUCCESS;
    /*}
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

    return EXIT_FAILURE;*/
}