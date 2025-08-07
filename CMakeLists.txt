cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 23)
project(kee)

add_subdirectory(externals/boost EXCLUDE_FROM_ALL)
add_subdirectory(externals/raylib)
add_subdirectory(externals/raylib-cpp)

file(GLOB_RECURSE KEE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${KEE_SRC})

# TODO: only keep msvc for now when back from vacay

# /W4 /permissive- -Wall -Wextra -Werror     - Enable all compile-time warnings.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp Boost::optional)