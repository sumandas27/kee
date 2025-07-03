cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
project(kee)

add_subdirectory(externals/raylib)
add_subdirectory(externals/raylib-cpp)

file(GLOB_RECURSE KEE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${KEE_SRC})

# /W4 /permissive           - Enable all compile-time warnings.
# -Wall -Wextra -Werror     - ^^^ (non-msvc)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp)