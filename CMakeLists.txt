cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
project(kee)

add_subdirectory(externals/raylib)
add_subdirectory(externals/raylib-cpp)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp)