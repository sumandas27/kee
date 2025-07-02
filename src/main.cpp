#include "raylib-cpp.hpp"

int main(int argc, char** argv)
{
    constexpr int window_width = 800;
    constexpr int window_height = 450;
    const raylib::Window window(window_width, window_height);

    SetTargetFPS(60);
    while (!window.ShouldClose())
    {
        BeginDrawing();
        DrawText("Hello World", 160, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    return EXIT_SUCCESS;
}